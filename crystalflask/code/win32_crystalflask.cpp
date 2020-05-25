#include "crystalflask_platform.h"

#define _WINSOCKAPI_
#include <windows.h>
#include <ShlObj.h>
#include <atlbase.h>  // for CComPtr, CComHeapPtr

#include <stdio.h>
#include <malloc.h>
#include <xinput.h>
#include <dsound.h>
#include <tchar.h>


#include <gl3w.h>
#include "win32_crystalflask.h"
#include "win32_util.cpp"

#include "crystalflask_network.cpp"

//~ imgui
#include <imgui.h>
#include "renderer/opengl/imgui_impl_opengl3.cpp"
#include "imgui_impl_win32.cpp"
#include <imgui_internal.h>

//~ BEGIN stb
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "stretchy_buffer.h"
//~ END stb

//meow hash
#pragma warning( push )
#pragma warning( disable : 4324 )
#include "ThirdParty/meow_hash/meow_hash_x64_aesni.h"
#pragma warning( pop )
//meow hash


#include "renderer/opengl/opengl.h"
global_variable opengl_instance OpenGL;
global_variable opengl_info OpenGLInfo;
global_variable HGLRC GlobalGLRC;
global_variable HWND GlobalWindow;

global_variable memory_arena GlobalOpenGLArena;
global_variable memory_arena GlobalResourceArena;

global_variable s64 GlobalPerfCountFrequency;
global_variable b32 GlobalRunning;
global_variable b32 GlobalPause;
global_variable WINDOWPLACEMENT GlobalWindowPosition = {sizeof(GlobalWindowPosition)};

global_variable s32 GlobalWidth = 1280;
global_variable s32 GlobalHeight = 720;

#include "renderer/opengl/opengl.cpp"


//game code
win32_game_code GlobalGameCode = {};


DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
    debug_read_file_result Result = {};
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            u32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            if(Result.Contents)
            {
                DWORD BytesRead;
                if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) &&
                   (FileSize32 == BytesRead))
                {
                    Result.ContentsSize = FileSize32;
                }
                else
                {
                    char ErrorBuffer[256];
                    Win32MessageBoxError("Failed to read file %s: %s", Filename, Win32GetLastErrorMessage(&ErrorBuffer));
                    DEBUGPlatformFreeFileMemory(Thread, Result.Contents);
                    Result.Contents = 0;
                }
            }
            else
            {
                Win32MessageBoxError("Failed to read file %s: Could not allocate memory for the file", Filename);
            }
        }
        else
        {
            Win32MessageBoxError("Failed to read file %s: Failed to get the size of the file.", Filename);
        }
        
        CloseHandle(FileHandle);
    }
    else
    {
        Win32MessageBoxError("Failed to read file %s: Could not get a handle for the file!", Filename);
    }
    
    return(Result);
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
    b32 Result = false;
    
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
        {
            Result = (BytesWritten == MemorySize);
        }
        else
        {
            char ErrorBuffer[256];
            Win32MessageBoxError("Failed in writing to file \"%s\": %s", Filename, Win32GetLastErrorMessage(&ErrorBuffer));
        }
        
        CloseHandle(FileHandle);
    }
    else
    {
        Win32MessageBoxError("Failed in writing to file \"%s\": Could not get a handle for the file.", Filename);
    }
    
    return(Result);
}



internal win32_game_code
Win32LoadGameCode(char *SourceDLLName, char *TempDLLName, char *LockFileName)
{
    win32_game_code Result = {};
    
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(!GetFileAttributesEx(LockFileName, GetFileExInfoStandard, &Ignored))
    {
        Result.DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);
        
        CopyFile(SourceDLLName, TempDLLName, FALSE);
        
        Result.GameCodeDLL = LoadLibraryA(TempDLLName);
        if(Result.GameCodeDLL)
        {
            Result.UpdateAndRender = (game_update_and_render *)
                GetProcAddress(Result.GameCodeDLL, "GameUpdateAndRender");
            
            Result.GetSoundSamples = (game_get_sound_samples *)
                GetProcAddress(Result.GameCodeDLL, "GameGetSoundSamples");
            
            Result.IsValid = (Result.UpdateAndRender &&
                              Result.GetSoundSamples);
        }
    }
    
    if(!Result.IsValid)
    {
        Result.UpdateAndRender = 0;
        Result.GetSoundSamples = 0;
    }
    
    return(Result);
}

internal void
Win32UnloadGameCode(win32_game_code *GameCode)
{
    if(GameCode->GameCodeDLL)
    {
        FreeLibrary(GameCode->GameCodeDLL);
        GameCode->GameCodeDLL = 0;
    }
    
    GameCode->IsValid = false;
    GameCode->UpdateAndRender = 0;
    GameCode->GetSoundSamples = 0;
}

//~ game



internal void
Win32RecompileSelf()
{
    win32_state Win32State = {};
    Win32GetEXEFileName(&Win32State);
    
    char BuildFileFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "..\\crystalflask\\code\\build.bat", sizeof(BuildFileFullPath), BuildFileFullPath);
    
	char SourceGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "crystalflask.dll", sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);
    
    char TempGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "crystalflask_temp.dll", sizeof(TempGameCodeDLLFullPath), TempGameCodeDLLFullPath);
    
    char GameCodeLockFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "lock.temp",
                              sizeof(GameCodeLockFullPath), GameCodeLockFullPath);
    
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;
    
    if (!CreateProcessA(BuildFileFullPath, "",
                        NULL, NULL, 0, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
    {
        //TODO(Gabriel): Add Debugging colors to the output error.
        char ErrorBuffer[256];
        wsprintfA(ErrorBuffer, "Failed to recompile self. Error code (0x0069)\n%d\n", GetLastError());
        
        MessageBoxA(NULL, ErrorBuffer, "CrystalFlask", MB_OK | MB_ICONERROR);
        
    } else
    {
        //TODO(Gabriel): play a sound?
        Win32UnloadGameCode(&GlobalGameCode);
        GlobalGameCode = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath, GameCodeLockFullPath);
    }
}



internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;
    
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    
    return(Result);
}


internal LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    if (ImGui::GetCurrentContext() == NULL)
    {
        return DefWindowProcA(Window, Message, WParam, LParam);
    }
    
    ImGuiIO& io = ImGui::GetIO();
    
    
    LRESULT Result = 0;
    
    UINT msg = Message;
    
    switch (msg)
    {
        
        case WM_SHOWWINDOW:
        {
            if (WParam == TRUE)
            {
                GlobalPause = false;
            } else
            {
                GlobalPause = true;
            }
            return 0;
        }
        
        
        case WM_SIZING:
        {
            RECT* Rect = (RECT*)LParam;
            GlobalWidth = Rect->right - Rect->left;
            GlobalHeight = Rect->bottom - Rect->top;
            OpenGLUpdateViewport(0, 0, GlobalWidth, GlobalHeight);
            return 1;
        }
        break;
        
        
        
        case WM_SIZE:
        {
            if (WParam == SIZE_MAXIMIZED && SIZE_MINIMIZED)
            {
                DWORD Width = LOWORD(LParam);
                DWORD Height = HIWORD(LParam);
                GlobalWidth = Width;
                GlobalHeight = Height;
                
                RECT WindowRect;
                if (!GetWindowRect(Window, &WindowRect))
                {
                    return 1;
                }
                GlobalWidth = WindowRect.right - WindowRect.left;
                GlobalHeight = WindowRect.bottom - WindowRect.top;
                OpenGLUpdateViewport(0, 0, GlobalWidth, GlobalHeight);
            }
            
            
            
            if (WParam == SIZE_MINIMIZED)
            {
                GlobalPause = true;
            }
            else if (WParam == SIZE_RESTORED)
            {
                if (GlobalPause)
                {
                    GlobalPause = false;
                    return 0;
                }
                
                DWORD Width = LOWORD(LParam);
                DWORD Height = HIWORD(LParam);
                GlobalWidth = Width;
                GlobalHeight = Height;
                OpenGLUpdateViewport(0, 0, GlobalWidth, GlobalHeight);
            }
            
            
            return 0;
        }
        break;
        
        case WM_ACTIVATE:
        {
            if (WParam == WA_ACTIVE || WParam == WA_CLICKACTIVE)
            {
                GlobalPause = false;
            }
            else if (WParam == WA_INACTIVE)
            {
                GlobalPause = true;
            }
            return 0;
        }
        break;
        
        
        case WM_ACTIVATEAPP:
        {
            if(WParam == TRUE)
            {
                GlobalPause = false;
            }
            else
            {
                GlobalPause = true;
            }
            return 0;
        }
        
        
        case WM_CLOSE:
        {
            GlobalRunning = false;
            return 0;
        }
        break;
        
        
        case WM_SETCURSOR:
        {
            //if(DEBUGGlobalShowCursor)
            //{
            //Result = DefWindowProcA(Window, Message, WParam, LParam);
            //}
            //else
            
            
            //SetCursor(LoadCursorW(NULL, IDC_ARROW));
            
            
            if (LOWORD(LParam) == HTCLIENT && ImGui_ImplWin32_UpdateMouseCursor())
                return 1;
            else
                return DefWindowProcA(Window, Message, WParam, LParam);
            
            //SetCursor(0);
        }
        break;
        
        
        case WM_QUIT:
        {
            GlobalRunning = false;
            return 0;
        }
        break;
        
        case WM_DESTROY:
        {
            GlobalRunning = false;
            return 0;
        }
        break;
        
        case WM_DEVICECHANGE:
        {
            if ((UINT)WParam == DBT_DEVNODES_CHANGED)
                g_WantUpdateHasGamepad = true;
            return 0;
        }
        break;
        
        case WM_DISPLAYCHANGE:
        {
            g_WantUpdateMonitors = true;
            return 0;
        }
        break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT paint;
            HDC deviceContext = BeginPaint(Window, &paint);
            EndPaint(Window, &paint);
            return 0;
        }
        break;
        
        
        
        case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
        {
            
            int button = 0;
            if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK) { button = 0; }
            if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK) { button = 1; }
            if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK) { button = 2; }
            if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK) { button = (GET_XBUTTON_WPARAM(WParam) == XBUTTON1) ? 3 : 4; }
            if (!ImGui::IsAnyMouseDown() && ::GetCapture() == NULL)
                ::SetCapture(Window);
            io.MouseDown[button] = true;
            
            return 0;
        }
        break;
        
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
        {
            
            int button = 0;
            if (msg == WM_LBUTTONUP) { button = 0; }
            if (msg == WM_RBUTTONUP) { button = 1; }
            if (msg == WM_MBUTTONUP) { button = 2; }
            if (msg == WM_XBUTTONUP) { button = (GET_XBUTTON_WPARAM(WParam) == XBUTTON1) ? 3 : 4; }
            io.MouseDown[button] = false;
            if (!ImGui::IsAnyMouseDown() && ::GetCapture() == Window)
                ::ReleaseCapture();
            
            return 0;
        }
        break;
        
        
        case WM_MOUSEWHEEL:
        {
            
            io.MouseWheel += (float)GET_WHEEL_DELTA_WPARAM(WParam) / (float)WHEEL_DELTA;
            
            return 0;
        }
        break;
        
        case WM_MOUSEHWHEEL:
        {
            
            io.MouseWheelH += (float)GET_WHEEL_DELTA_WPARAM(WParam) / (float)WHEEL_DELTA;
            return 0;
        }
        break;
        
        case WM_CHAR:
        {
            // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
            if (WParam > 0 && WParam < 0x10000)
                io.AddInputCharacterUTF16((unsigned short)WParam);
            return 0;
        }
        break;
        
        //SEE https://www.khronos.org/opengl/wiki/Platform_specifics:_Windows
        case WM_ERASEBKGND:
        {
            return TRUE;
        }
        break;
        
        default:
        {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
            
            // TODO(Gabriel): add unicode support
            //Result = DefWindowProcW(Window, Message, WParam, LParam);
        } break;
    }
    
    
    return(Result);
}


internal void
Win32ProcessKeyboardMessage(game_button_state *NewState, b32 IsDown)
{
    if(NewState->EndedDown != IsDown)
    {
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
}

internal void
Win32GetInputFileLocation(win32_state *State, b32 InputStream,
                          int SlotIndex, int DestCount, char *Dest)
{
    char Temp[64];
    wsprintf(Temp, "loop_edit_%d_%s.hmi", SlotIndex, InputStream ? "input" : "state");
    Win32BuildEXEPathFileName(State, Temp, DestCount, Dest);
}

internal win32_replay_buffer *
Win32GetReplayBuffer(win32_state *State, int unsigned Index)
{
    Assert(Index > 0);
    Assert(Index < ArrayCount(State->ReplayBuffers));
    win32_replay_buffer *Result = &State->ReplayBuffers[Index];
    return Result;
}

internal void
Win32BeginRecordingInput(win32_state *State, int InputRecordingIndex)
{
    win32_replay_buffer *ReplayBuffer = Win32GetReplayBuffer(State, InputRecordingIndex);
    if(ReplayBuffer->MemoryBlock)
    {
        State->InputRecordingIndex = InputRecordingIndex;
        
        char FileName[WIN32_STATE_FILE_NAME_COUNT];
        Win32GetInputFileLocation(State, true, InputRecordingIndex, sizeof(FileName), FileName);
        State->RecordingHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
        
#if 0
        LARGE_INTEGER FilePosition;
        FilePosition.QuadPart = State->TotalSize;
        SetFilePointerEx(State->RecordingHandle, FilePosition, 0, FILE_BEGIN);
#endif
        
        CopyMemory(ReplayBuffer->MemoryBlock, State->GameMemoryBlock, State->TotalSize);
    }
}

internal void
Win32EndRecordingInput(win32_state *State)
{
    CloseHandle(State->RecordingHandle);
    State->InputRecordingIndex = 0;
}

internal void
Win32BeginInputPlayBack(win32_state *State, int InputPlayingIndex)
{
    win32_replay_buffer *ReplayBuffer = Win32GetReplayBuffer(State, InputPlayingIndex);
    if(ReplayBuffer->MemoryBlock)
    {
        State->InputPlayingIndex = InputPlayingIndex;
        
        char FileName[WIN32_STATE_FILE_NAME_COUNT];
        Win32GetInputFileLocation(State, true, InputPlayingIndex, sizeof(FileName), FileName);
        State->PlaybackHandle = CreateFileA(FileName, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
        
#if 0
        LARGE_INTEGER FilePosition;
        FilePosition.QuadPart = State->TotalSize;
        SetFilePointerEx(State->PlaybackHandle, FilePosition, 0, FILE_BEGIN);
#endif
        
        CopyMemory(State->GameMemoryBlock, ReplayBuffer->MemoryBlock, State->TotalSize);
    }
}

internal void
Win32EndInputPlayBack(win32_state *State)
{
    CloseHandle(State->PlaybackHandle);
    State->InputPlayingIndex = 0;
}

internal void
Win32RecordInput(win32_state *State, game_input *NewInput)
{
    DWORD BytesWritten;
    WriteFile(State->RecordingHandle, NewInput, sizeof(*NewInput), &BytesWritten, 0);
}

internal void
Win32PlayBackInput(win32_state *State, game_input *NewInput)
{
    DWORD BytesRead = 0;
    if(ReadFile(State->PlaybackHandle, NewInput, sizeof(*NewInput), &BytesRead, 0))
    {
        if(BytesRead == 0)
        {
            int PlayingIndex = State->InputPlayingIndex;
            Win32EndInputPlayBack(State);
            Win32BeginInputPlayBack(State, PlayingIndex);
            ReadFile(State->PlaybackHandle, NewInput, sizeof(*NewInput), &BytesRead, 0);
        }
    }
}

internal void
ToggleFullscreen(HWND Window)
{
    // NOTE(casey): This follows Raymond Chen's prescription
    // for fullscreen toggling, see:
    // http://blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx
    
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if(Style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if(GetWindowPlacement(Window, &GlobalWindowPosition) &&
           GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            
            OpenGLUpdateViewport(MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                                 MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                                 MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top);
            GlobalWidth = MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left;
            GlobalHeight = MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top;
        }
    }
    else
    {
        SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &GlobalWindowPosition);
        SetWindowPos(Window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        
    }
}


internal void
Win32ProcessPendingMessages(win32_state *State, game_controller_input *KeyboardController, game_input* Input)
{
    
    
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        
        if (ImGui::GetCurrentContext() == NULL)
        {
            //TranslateMessage(&Message);
            DispatchMessageA(&Message);
            return;
        }
        
        ImGuiIO& io = ImGui::GetIO();
        
        
        switch(Message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                
                u32 VKCode = (u32)Message.wParam;
                
                // NOTE(casey): Since we are comparing WasDown to IsDown,
                // we MUST use == and != to convert these bit tests to actual
                // 0 or 1 values.
                b32 WasDown = ((Message.lParam & (1 << 30)) != 0);
                b32 IsDown = ((Message.lParam & (1 << 31)) == 0);
                
                if (VKCode < 256)
                    io.KeysDown[VKCode] = IsDown;
                
                if(WasDown != IsDown)
                {
                    if(VKCode == 'W')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown);
                    }
                    else if(VKCode == 'A')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown);
                    }
                    else if(VKCode == 'S')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown);
                    }
                    else if(VKCode == 'D')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown);
                    }
                    else if(VKCode == 'Q')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown);
                    }
                    else if(VKCode == 'E')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown);
                    }
                    else if(VKCode == VK_UP)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionUp, IsDown);
                    }
                    else if(VKCode == VK_LEFT)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionLeft, IsDown);
                    }
                    else if(VKCode == VK_DOWN)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionDown, IsDown);
                    }
                    else if(VKCode == VK_RIGHT)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionRight, IsDown);
                    }
                    else if(VKCode == VK_ESCAPE)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Start, IsDown);
                    }
                    else if(VKCode == VK_SPACE)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Back, IsDown);
                    }
                    
                    
                    if(IsDown)
                    {
                        b32 AltKeyWasDown = (Message.lParam & (1 << 29));
                        
                        if (AltKeyWasDown)
                        {
                            KeyboardController->Key.VK = VK_MENU;
                            KeyboardController->Key.EndedDown = AltKeyWasDown;
                            ++KeyboardController->Key.HalfTransitionCount;
                        }
                        
                        if((VKCode == 'P') && AltKeyWasDown)
                        {
                            if(IsDown)
                            {
                                GlobalPause = !GlobalPause;
                            }
                        }
                        else if((VKCode == 'M') && AltKeyWasDown)
                        {
                            if(IsDown)
                            {
                                GlobalIsConsoleVisible = !GlobalIsConsoleVisible;
                            }
                        }
                        else if((VKCode == 'N') && AltKeyWasDown)
                        {
                            if(IsDown)
                            {
                                GlobalIsEditorEnabled = !GlobalIsEditorEnabled;
                            }
                        }
                        else if((VKCode == 'X') && AltKeyWasDown)
                        {
                            if(IsDown)
                            {
                                Win32RecompileSelf();
                            }
                        }
                        else if((VKCode == 'L') && AltKeyWasDown)
                        {
                            if(IsDown)
                            {
                                if(State->InputPlayingIndex == 0)
                                {
                                    if(State->InputRecordingIndex == 0)
                                    {
                                        Win32BeginRecordingInput(State, 1);
                                    }
                                    else
                                    {
                                        Win32EndRecordingInput(State);
                                        Win32BeginInputPlayBack(State, 1);
                                    }
                                }
                                else
                                {
                                    Win32EndInputPlayBack(State);
                                }
                            }
                        }
                        
                        if((VKCode == VK_F4) && AltKeyWasDown)
                        {
                            GlobalRunning = false;
                        }
                        if((VKCode == VK_ESCAPE) && AltKeyWasDown)
                        {
                            GlobalRunning = false;
                        }
                        if((VKCode == 'Q') && AltKeyWasDown)
                        {
                            GlobalRunning = false;
                        }
                        if((VKCode == VK_RETURN) && AltKeyWasDown)
                        {
                            if(Message.hwnd)
                            {
                                ToggleFullscreen(Message.hwnd);
                            }
                        }
                    }
                    
                }
                TranslateMessage(&Message);
                //DispatchMessageA(&Message);
            } break;
            
            
            
            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        }
    }
    
    
}

inline LARGE_INTEGER
Win32GetWallClock(void)
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

inline r32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    r32 Result = ((r32)(End.QuadPart - Start.QuadPart) /
                  (r32)GlobalPerfCountFrequency);
    return Result;
}

internal void
Win32InitOpenGL(HWND Window, HDC WindowDC)
{
    PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
    DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
    DesiredPixelFormat.nVersion = 1;
    DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
    DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
    DesiredPixelFormat.cColorBits = 32;
    DesiredPixelFormat.cAlphaBits = 8;
    DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;
    
    int SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &DesiredPixelFormat);
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
    DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex,
                        sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
    SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);
    
    GlobalGLRC = wglCreateContext(WindowDC);
    if(wglMakeCurrent(WindowDC, GlobalGLRC))
    {
        b32 ModernContext = true;
        
        if (!gl3wInit()) {
            OpenGLInfo = OpenGLInit(Window, false);
        }
    }
    else
    {
        MessageBoxA(NULL,
                    "Failed to Create OpenGL Context. Error code (0x0005)",
                    "CrystalFlask", MB_OK | MB_ICONERROR);
    }
}


internal void
Win32ResolveInternalSubstructure(win32_state *Win32State)
{
    
    Win32GetEXEFileName(Win32State);
    
    //game
    Win32BuildEXEPathFileName(Win32State, "crystalflask.dll",
                              sizeof(GlobalWin32InternalLibrary.GameDLLPath), GlobalWin32InternalLibrary.GameDLLPath);
    
    Win32BuildEXEPathFileName(Win32State, "crystalflask_temp.dll",
                              sizeof(GlobalWin32InternalLibrary.GameDLLTempPath), GlobalWin32InternalLibrary.GameDLLTempPath);
    
    Win32BuildEXEPathFileName(Win32State, "lock.tmp",
                              sizeof(GlobalWin32InternalLibrary.GameDLLLockPath), GlobalWin32InternalLibrary.GameDLLLockPath);
    
    
}

internal b32
Win32CheckAndSetCurrentWorkingDirectory()
{
    DWORD BufferLength = GetCurrentDirectory(NULL, NULL);
    BufferLength += 1;
    
    char* CurrentDirectoryBuffer = (char*)malloc(BufferLength*sizeof(char));
    if (!GetCurrentDirectory(BufferLength, CurrentDirectoryBuffer))
    {
        char ErrorBuffer[256];
        Win32MessageBoxError("Failed to get current directory. %s", Win32GetLastErrorMessage(&ErrorBuffer));
        
        if (CurrentDirectoryBuffer)
        {
            free(CurrentDirectoryBuffer);
            CurrentDirectoryBuffer = NULL;
        }
    }
    
    if (!CurrentDirectoryBuffer)
    {
        return 0;
    }
    
    u32 BufferIndex = BufferLength - 6;
    if (CurrentDirectoryBuffer[BufferIndex    ] == 'd' &&
        CurrentDirectoryBuffer[BufferIndex + 1] == 'a' &&
        CurrentDirectoryBuffer[BufferIndex + 2] == 't' &&
        CurrentDirectoryBuffer[BufferIndex + 3] == 'a')
    {
        return 1;
    } else
    {
        char NewPath[256];
        sprintf_s(NewPath, 256, "%s\\..\\crystalflask\\data", CurrentDirectoryBuffer);
        if (SetCurrentDirectory(NewPath))
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
}


int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCode)
{
    if (!Win32CheckAndSetCurrentWorkingDirectory())
    {
        Win32MessageBoxError("Failed to set correct working directory.");
        exit(1);
    }
    
    //set priority
    //SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
    //SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
    
    
    win32_state Win32State = {};
    Win32ResolveInternalSubstructure(&Win32State);
    
    
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;
    
    
    // NOTE(casey): Set the Windows scheduler granularity to 1ms
    // so that our Sleep() can be more granular.
    u32 DesiredSchedulerMS = 1;
    b32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);
    
    
    WNDCLASSA WindowClass = {};
    
    WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    WindowClass.lpszClassName = "CrystalFlaskWindowClass";
    
    s8 VersionMajor = 0;
    s8 VersionMinor = 1;
    s32 BuildCount =  Win32ReadBuildCounter();
    char TitleBuffer[70] = {};
    sprintf_s(TitleBuffer, 70, "CrystalFlask | Rumjagten 2 | Build: %d.%d.%d", VersionMajor, VersionMinor, BuildCount);
    
    u32 WindowWidth = GlobalWidth;
    u32 WindowHeight = GlobalHeight;
    
    if(RegisterClassA(&WindowClass))
    {
        
        HWND Window =
            CreateWindowExA(
                            0, // WS_EX_TOPMOST|WS_EX_LAYERED,
                            WindowClass.lpszClassName,
                            TitleBuffer,
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            WindowWidth, // Width
                            WindowHeight, // Height,
                            0,
                            0,
                            Instance,
                            0);
        
        if(Window)
        {
            GlobalWindow = Window;
            //ToggleFullscreen(Window);
            
            s32 MonitorRefreshHz = 144;
            //
            HDC RefreshDC = GetDC(Window);
            s32 Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
            
            if(Win32RefreshRate > 1)
            {
                MonitorRefreshHz = Win32RefreshRate;
            }
            
            r32 GameUpdateHz = (MonitorRefreshHz / 2.0f);
            r32 TargetSecondsPerFrame = 1.0f / (r32) GameUpdateHz;
            LARGE_INTEGER LastCounter = Win32GetWallClock();
            
            Win32InitOpenGL(Window, RefreshDC);
            //Win32State->GLRC = GlobalGLRC;
            Win32State.Window = Window;
            ReleaseDC(Window, RefreshDC);
            
            //
            
            GlobalRunning = true;
            
            
            //LPVOID BaseAddress = (LPVOID)Terabytes(1);
            LPVOID BaseAddress = 0;
            
            
            game_memory GameMemory = {};
            GameMemory.PermanentStorageSize = Megabytes(128);
            GameMemory.TransientStorageSize = Megabytes(256);
            GameMemory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
            GameMemory.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
            GameMemory.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;
            
            
            
            Win32State.TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
            
            Win32State.GameMemoryBlock = VirtualAlloc(BaseAddress, (size_t)Win32State.TotalSize,
                                                      MEM_RESERVE|MEM_COMMIT,
                                                      PAGE_READWRITE);
            GameMemory.PermanentStorage = Win32State.GameMemoryBlock;
            GameMemory.TransientStorage = ((u8 *)GameMemory.PermanentStorage +
                                           GameMemory.PermanentStorageSize);
            
            
            if(!GameMemory.IsInitialized)
            {
                
                InitializeArena(&GlobalOpenGLArena,
                                GameMemory.PermanentStorageSize, (u8 *)GameMemory.PermanentStorage);
                
                InitializeArena(&GlobalResourceArena,
                                GameMemory.TransientStorageSize, (u8 *)GameMemory.TransientStorage);
                
                GameMemory.IsInitialized = true;
            }
            
            Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
            
            OpenGLStart();
            
#if 0
            for(int ReplayIndex = 1;
                ReplayIndex < ArrayCount(Win32State.ReplayBuffers);
                ++ReplayIndex)
            {
                win32_replay_buffer *ReplayBuffer = &Win32State.ReplayBuffers[ReplayIndex];
                
                
                Win32GetInputFileLocation(&Win32State, false, ReplayIndex,
                                          sizeof(ReplayBuffer->FileName), ReplayBuffer->FileName);
                
                ReplayBuffer->FileHandle =
                    CreateFileA(ReplayBuffer->FileName,
                                GENERIC_WRITE|GENERIC_READ, 0, 0, CREATE_ALWAYS, 0, 0);
                
                LARGE_INTEGER MaxSize;
                MaxSize.QuadPart = Win32State.TotalSize;
                ReplayBuffer->MemoryMap = CreateFileMapping(
                                                            ReplayBuffer->FileHandle, 0, PAGE_READWRITE,
                                                            MaxSize.HighPart, MaxSize.LowPart, 0);
                
                ReplayBuffer->MemoryBlock = MapViewOfFile(
                                                          ReplayBuffer->MemoryMap, FILE_MAP_ALL_ACCESS, 0, 0, Win32State.TotalSize);
                if(ReplayBuffer->MemoryBlock)
                {
                }
                else
                {
                    
                }
            }
#endif
            
            if(GameMemory.PermanentStorage && GameMemory.TransientStorage)
            {
                
                
                game_input Input[2] = {};
                game_input *NewInput = &Input[0];
                game_input *OldInput = &Input[1];
                
                LARGE_INTEGER LastCounter = Win32GetWallClock();
                
                GlobalGameCode = Win32LoadGameCode(GlobalWin32InternalLibrary.GameDLLPath,
                                                   GlobalWin32InternalLibrary.GameDLLTempPath,
                                                   GlobalWin32InternalLibrary.GameDLLLockPath);
                
                
                
                NetworkStart();
                
                
                
                u32 LoadCounter = 0;
                
                u64 LastCycleCount = __rdtsc();
                
                while(GlobalRunning)
                {
                    
                    
                    NewInput->dtForFrame = TargetSecondsPerFrame;
                    
                    FILETIME NewDLLWriteTime = Win32GetLastWriteTime(GlobalWin32InternalLibrary.GameDLLPath);
                    if(CompareFileTime(&NewDLLWriteTime, &GlobalGameCode.DLLLastWriteTime) != 0)
                    {
                        Win32UnloadGameCode(&GlobalGameCode);
                        GlobalGameCode = Win32LoadGameCode(GlobalWin32InternalLibrary.GameDLLPath,
                                                           GlobalWin32InternalLibrary.GameDLLTempPath,
                                                           GlobalWin32InternalLibrary.GameDLLLockPath);
                        LoadCounter = 0;
                    }
                    
                    
                    
                    game_controller_input *OldKeyboardController = GetController(OldInput, 0);
                    game_controller_input *NewKeyboardController = GetController(NewInput, 0);
                    *NewKeyboardController = {};
                    NewKeyboardController->IsConnected = true;
                    for(int ButtonIndex = 0;
                        ButtonIndex < ArrayCount(NewKeyboardController->Buttons);
                        ++ButtonIndex)
                    {
                        NewKeyboardController->Buttons[ButtonIndex].EndedDown =
                            OldKeyboardController->Buttons[ButtonIndex].EndedDown;
                    }
                    
                    
                    Win32ProcessPendingMessages(&Win32State, NewKeyboardController, NewInput);
                    
                    
                    if(!GlobalPause)
                    {
                        
                        POINT MouseP;
                        GetCursorPos(&MouseP);
                        ScreenToClient(Window, &MouseP);
                        NewInput->MouseX = MouseP.x;
                        NewInput->MouseY = MouseP.y;
                        
                        
                        
                        
                        
                        
                        if(Win32State.InputRecordingIndex)
                        {
                            Win32RecordInput(&Win32State, NewInput);
                        }
                        
                        if(Win32State.InputPlayingIndex)
                        {
                            Win32PlayBackInput(&Win32State, NewInput);
                        }
                        
                        
                        
                        
                        //Render
                        HDC DeviceContext = GetDC(Window);
                        
                        // TODO(Gabriel): FIX the render time, it hugs too much cpu time. Is it ImGui or our own render calls? It's quite possible that we've misinitialized imgui and we're not rendering properly.
                        OpenGLRender(NewInput);
                        
                        game_offscreen_buffer Buffer = {};
                        
                        thread_context Thread = {};
                        if(GlobalGameCode.UpdateAndRender)
                        {
                            GlobalGameCode.UpdateAndRender(&Thread, &GameMemory, NewInput, &Buffer);
                        }
                        
                        SwapBuffers(DeviceContext);
                        ReleaseDC(Window, DeviceContext); //RELEASE THE FUCKING DC!!!!!!!
                        
                        
                        
                        
                        
                        
                        
                        //sleep fixed frame rate
                        LARGE_INTEGER WorkCounter = Win32GetWallClock();
                        r32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);
                        
                        r32 SecondsElapsedForFrame = WorkSecondsElapsed;
                        if (SecondsElapsedForFrame < TargetSecondsPerFrame) {
                            if (SleepIsGranular) {
                                DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame - SecondsElapsedForFrame));
                                if (SleepMS > 0)
                                {
                                    Sleep(SleepMS);
                                }
                            }
                            
                            r32 TestSecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
                            
                            while (SecondsElapsedForFrame < TargetSecondsPerFrame)
                            {
                                SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
                            }
                        }
                        else
                        {
                            // MISSED FRAME!
                        }
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        
                        game_input *Temp = NewInput;
                        NewInput = OldInput;
                        OldInput = Temp;
                        
                    }
                    else
                    {
                        Sleep(33);
                    }
                    
                }
            }
            else
            {
                
                MessageBoxA(NULL,
                            "Failed to allocate enough memory. Error code (0x0003)",
                            "CrystalFlask", MB_OK | MB_ICONERROR);
            }
        }
        else
        {
            // TODO(casey): Logging
            
            
            MessageBoxA(NULL,
                        "Failed to create Window. Windows did not allow us to create a window. Error code (0x0001)",
                        "CrystalFlask", MB_OK | MB_ICONERROR);
        }
    }
    else
    {
        // TODO(casey): Logging
        
        
        MessageBoxA(NULL,
                    "Failed to register window class. Error code (0x0002)",
                    "CrystalFlask", MB_OK | MB_ICONERROR);
        
    }
    
    
    
    //cleanup
    wglMakeCurrent(NULL, NULL); //remove context
    wglDeleteContext(GlobalGLRC);
    //ReleaseDC(Window, DC);
    
    
    return(0);
}
