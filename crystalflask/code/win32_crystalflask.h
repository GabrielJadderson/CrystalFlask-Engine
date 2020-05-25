#pragma once

struct win32_startup_config
{
    s32 WindowPositionX;
    s32 WindowPositionY;
    s32 WindowWidth;
    s32 WindowHeight;
};


typedef struct win32_file
{
    unsigned char* Data;
    DWORD Size;
} win32_file;




struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    s32 Width;
    s32 Height;
    s32 Pitch;
    s32 BytesPerPixel;
};

struct win32_window_dimension
{
    s32 Width;
    s32 Height;
};

struct win32_sound_output
{
    s32 SamplesPerSecond;
    u32 RunningSampleIndex;
    s32 BytesPerSample;
    DWORD SecondaryBufferSize;
    DWORD SafetyBytes;
    r32 tSine;
};

struct win32_debug_time_marker
{
    DWORD OutputPlayCursor;
    DWORD OutputWriteCursor;
    DWORD OutputLocation;
    DWORD OutputByteCount;
    DWORD ExpectedFlipPlayCursor;
    
    DWORD FlipPlayCursor;
    DWORD FlipWriteCursor;
};

struct win32_game_code
{
    HMODULE GameCodeDLL;
    FILETIME DLLLastWriteTime;
    
    game_update_and_render *UpdateAndRender;
    game_get_sound_samples *GetSoundSamples;
    
    b32 IsValid;
};

#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH
struct win32_replay_buffer
{
    HANDLE FileHandle;
    HANDLE MemoryMap;
    char FileName[WIN32_STATE_FILE_NAME_COUNT];
    void *MemoryBlock;
};

struct win32_state
{
    u64 TotalSize;
    void *GameMemoryBlock;
    win32_replay_buffer ReplayBuffers[4];
    
    HANDLE RecordingHandle;
    s32 InputRecordingIndex;
    
    HANDLE PlaybackHandle;
    s32 InputPlayingIndex;
    
    char EXEFileName[WIN32_STATE_FILE_NAME_COUNT];
    char *OnePastLastEXEFileNameSlash;
    
    HWND Window;
    
};

struct win32_internal_library
{
    char GameDLLPath[WIN32_STATE_FILE_NAME_COUNT];
    char GameDLLTempPath[WIN32_STATE_FILE_NAME_COUNT];
    char GameDLLLockPath[WIN32_STATE_FILE_NAME_COUNT];
    //Game
};

global_variable win32_internal_library GlobalWin32InternalLibrary = {};


