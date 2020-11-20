

//feed in a buffer of 256!!
char* Win32GetLastErrorMessage(char(*buffer)[256])
{
    FormatMessageA(
                   FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   GetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (char*)*buffer,
                   255,
                   NULL);
    return *buffer;
}

//feed in a buffer of 256
wchar_t* Win32GetLastErrorMessageW(wchar_t(*buffer)[256])
{
    FormatMessageW(
                   FORMAT_MESSAGE_FROM_SYSTEM,
                   NULL,
                   GetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (wchar_t*)*buffer,
                   255,
                   NULL);
    return *buffer;
}

internal s32
Win32MessageBoxError(char* Message)
{
    s32 Result = MessageBoxA(NULL, Message, NULL, MB_OK | MB_ICONEXCLAMATION);
    return Result;
}


internal s32
Win32MessageBoxError(const char* Message, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, Message);
    vsnprintf(buf, ArrayCount(buf), Message, args);
    buf[ArrayCount(buf)-1] = 0;
    va_end(args);
    
    s32 Result = MessageBoxA(NULL, buf, NULL, MB_OK | MB_ICONEXCLAMATION);
    return Result;
}


enum file_type
{
    file_type_ALL = 0,         //*.*
    file_type_Scene = 1,       //.cfs
    file_type_Material = 2,    //.cfm
    file_type_Texture = 3,     //.cft
    file_type_Mesh = 4,        //.cfmesh
    
    file_type_Entity = 7,      //.cfen
    file_type_Project = 8,     //.cfp
    file_type_CFBIN = 9,       //.cfbin
    file_type_ShaderNode = 6,   //.cfnode
    
    file_type_xWB = 10,        //.xwb
    file_type_wav = 11,        //.wav
    file_type_txt = 12,        //.txt
    file_type_png = 13,        //.png
    file_type_tga = 14,        //.tga
};


char*
GetIOFilter(file_type Type)
{
    switch (Type)
    {
        case file_type_ALL: { return "All Files (*.*)\0*.*\0\0"; } break;
        case file_type_Texture: { return "All Files (*.*)\0*.*\0\0"; } break;
        case file_type_Entity: { return "CrystalFlask Entity (*.cfen)\0*.cfen\0\0";  } break;
        case file_type_Project: { return "CrystalFlask Project (*.cfp)\0*.cfp\0\0"; } break;
        case file_type_Mesh:
        {
            return "All Files (*.*)\0*.*\0\0";
            //TODO(Gabriel): Constrain the file extensions to only the supported ones by assimp.
            //return "\0OBJ (*.obj)\0*.obj\0STL (*.stl)\0*.stl\0Collada (*.dae)\0*.dae\0CrystalFlask Mesh (*.cfo)\0*.cfo\0\0";
        } break;
        case file_type_Material:
        {
            return "All Files (*.*)\0*.*\0\0";
            //TODO(Gabriel): Constrain the file extensions to only the supported ones by assimp.
            //return "CrystalFlask Material (*.cfm)\0*.cfm\0\0";
        } break;
        case file_type_Scene: { return "CrystalFlask Scene (*.cfs)\0*.cfs\0\0"; } break;
        case file_type_CFBIN: { return "CrystalFlask Binary (*.cfbin)\0*.cfbin\0\0"; } break;
        case file_type_xWB: { return "Wave Bank (*.xwb)\0*.xwb\0\0"; } break;
        case file_type_wav: { return "wave File (*.wav)\0*.wav\0\0"; } break;
        
        default: { return "All Files (*.*)\0*.*\0\0"; }  break;
    }
}


char*
GetIOFilterExtension(file_type Type)
{
    switch (Type)
    {
        case file_type_ALL: { return NULL; } break;
        case file_type_Texture: { return "cftex"; } break;
        case file_type_Entity: { return "cfen";  } break;
        case file_type_Project: { return "cfp"; } break;
        case file_type_Mesh: { return "cfmesh"; } break;
        case file_type_Material: { return "cfmat"; } break;
        case file_type_Scene: { return "cfs"; } break;
        case file_type_CFBIN: { return "cfbin"; } break;
        case file_type_xWB: { return "xwb"; } break;
        case file_type_wav: { return "wav"; } break;
        default: { return NULL; }  break;
    }
}


internal char*
Win32OpenFileDialog(file_type Type, HWND Window)
{
    //https://docs.microsoft.com/en-us/windows/win32/api/commdlg/ns-commdlg-openfilenamea?redirectedfrom=MSDN
    OPENFILENAMEA ofn = {};       // common dialog box structure
    ofn.lStructSize = sizeof(OPENFILENAME);
    CHAR szFile[260] = { 0 };       // if using TCHAR macros
    
    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = Window;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = GetIOFilter(Type);
    //ofn.lpstrFilter = "All Files (*.*)\0*.*\0\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrDefExt = GetIOFilterExtension(Type);
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    
    if (GetOpenFileNameA(&ofn) == TRUE)
    {
        char* Result = (char*)malloc(ofn.nMaxFile);
        memcpy(Result, ofn.lpstrFile, ofn.nMaxFile);
        return Result;
    }
    return NULL;
}

internal char*
Win32SaveFileDialog(file_type Type, HWND Window)
{
    OPENFILENAMEA ofn = {};       // common dialog box structure
    ofn.lStructSize = sizeof(OPENFILENAME);
    CHAR szFile[260] = { 0 };       // if using TCHAR macros
    
    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = Window;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = GetIOFilter(Type);
    //ofn.lpstrFilter = "All Files (*.*)\0*.*\0\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrDefExt = GetIOFilterExtension(Type);
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    
    if (GetSaveFileNameA(&ofn) == TRUE)
    {
        char* Result = (char*)malloc(ofn.nMaxFile);
        memcpy(Result, ofn.lpstrFile, ofn.nMaxFile);
        return Result;
    }
    
    return NULL;
}

inline FILETIME
Win32GetLastWriteTime(char *Filename)
{
    FILETIME LastWriteTime = {};
    
    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesEx(Filename, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }
    
    return(LastWriteTime);
}



internal void
Win32GetEXEFileName(win32_state *State)
{
    // NOTE(casey): Never use MAX_PATH in code that is user-facing, because it
    // can be dangerous and lead to bad results.
    DWORD SizeOfFilename = GetModuleFileNameA(0, State->EXEFileName, sizeof(State->EXEFileName));
    State->OnePastLastEXEFileNameSlash = State->EXEFileName;
    for(char *Scan = State->EXEFileName;
        *Scan;
        ++Scan)
    {
        if(*Scan == '\\')
        {
            State->OnePastLastEXEFileNameSlash = Scan + 1;
        }
    }
}

internal void
Win32BuildEXEPathFileName(win32_state *State, char *FileName,
                          int DestCount, char *Dest)
{
    CatStrings(State->OnePastLastEXEFileNameSlash - State->EXEFileName, State->EXEFileName,
               StringLength(FileName), FileName,
               DestCount, Dest);
}


internal void
Win32SaveDataToFile(char* FilePath, unsigned char* DataToWrite, DWORD DataSizeToWrite)
{
    HANDLE FH = CreateFileA(FilePath, FILE_GENERIC_WRITE,
                            0, 0, CREATE_ALWAYS, 0, 0);
    
    if (FH == INVALID_HANDLE_VALUE)
    {
        char ErrorBuffer[256];
        Win32MessageBoxError("Failed to create file: %s", Win32GetLastErrorMessage(&ErrorBuffer));
    }
    
    DWORD BytesWritten = 0;
	WriteFile(FH, DataToWrite, DataSizeToWrite, &BytesWritten, 0);
    CloseHandle(FH);
}


//TODO(Gabriel): Doesn't work as expected, need to close the handle??
win32_file
Win32LoadDataFromFileByMapView(char* FilePath)
{
    win32_file file;
    file.Data = NULL;
    file.Size = 0;
    
    HANDLE FH = CreateFileA(
                            FilePath,
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
    
    if (FH == INVALID_HANDLE_VALUE)
    {
        char print_buffer[256];
        Win32GetLastErrorMessage(&print_buffer);
    	char TextBuffer[512];
        _snprintf_s(TextBuffer, sizeof(TextBuffer), "Failed to CreateFileA \"%s\" : %s\n", FilePath, print_buffer);
        
        MessageBoxA(NULL, TextBuffer, "CrystalFlask Error code (0x0055)", MB_OK | MB_ICONERROR);
        
        return file;
    }
    
    HANDLE FMH = CreateFileMappingA(
                                    FH,
                                    NULL,
                                    PAGE_READONLY, //| SEC_NOCACHE,
                                    NULL,
                                    NULL,
                                    NULL);
    
    if (FMH == NULL)
    {
        char print_buffer[256];
        Win32GetLastErrorMessage(&print_buffer);
        char TextBuffer[512];
        _snprintf_s(TextBuffer, sizeof(TextBuffer), "Failed to CreateFileMappingA \"%s\" : %s\n", FilePath, print_buffer);
        
        MessageBoxA(NULL, TextBuffer, "Crystalflask Error code (0x0054)", MB_OK | MB_ICONERROR);
        
        return file;
    }
    
    LARGE_INTEGER file_size;
    
    
    if (GetFileSizeEx(FH, &file_size) == 0)
    {
        char print_buffer[256];
        Win32GetLastErrorMessage(&print_buffer);
        char TextBuffer[512];
        _snprintf_s(TextBuffer, sizeof(TextBuffer), "Failed to GetFileSizeEx  \"%s\" : %s\n", FilePath, print_buffer);
        
        MessageBoxA(NULL, TextBuffer, "Crystalflask Error code (0x0053)", MB_OK | MB_ICONERROR);
        
        return file;
    }
    
    DWORD sizeof_file = (DWORD)(file_size.u.LowPart | ((unsigned long long)file_size.u.HighPart << 32));
    
    void* map = MapViewOfFile(FMH, FILE_MAP_READ, 0, 0, sizeof_file);
    if (map == NULL)
    {
        char print_buffer[256];
        Win32GetLastErrorMessage(&print_buffer);
        char TextBuffer[512];
        _snprintf_s(TextBuffer, sizeof(TextBuffer), "Failed to MapViewOfFile \"%s\" : %s\n", FilePath, print_buffer);
        
        MessageBoxA(NULL, TextBuffer, "Crystalflask Error code (0x0052)", MB_OK | MB_ICONERROR);
        
        return file;
    }
    
    file.Data = (unsigned char*)map;
    file.Size = sizeof_file;
    
    CloseHandle(FMH);
    
    return file;
}



internal win32_file
Win32ReadDataFromFile(char* FilePath)
{
    win32_file File = {};
    
    HANDLE FH = CreateFileA(FilePath, GENERIC_READ, 0, 0,
                            OPEN_EXISTING, 0, 0);
    
    if (FH == INVALID_HANDLE_VALUE)
    {
        char ErrorBuffer[256];
        Win32MessageBoxError("Failed to read file: %s", Win32GetLastErrorMessage(&ErrorBuffer));
    }
    
    
    LARGE_INTEGER FileSize;
    
    if (GetFileSizeEx(FH, &FileSize) == 0)
    {
        char print_buffer[256];
        Win32GetLastErrorMessage(&print_buffer);
        char TextBuffer[512];
        _snprintf_s(TextBuffer, sizeof(TextBuffer), "Failed to GetFileSizeEx  \"%s\" : %s\n", FilePath, print_buffer);
        
        MessageBoxA(NULL, TextBuffer, "Crystalflask Error code (0x0050)", MB_OK | MB_ICONERROR);
        
        return File;
    }
    
    File.Size = (DWORD)FileSize.QuadPart;
    File.Data = (unsigned char*)malloc(sizeof(char)*File.Size);
    
    DWORD BytesRead = 0;
    if (!ReadFile(FH, File.Data, File.Size, &BytesRead, 0))
    {
        char print_buffer[256];
        Win32GetLastErrorMessage(&print_buffer);
        char TextBuffer[512];
        _snprintf_s(TextBuffer, sizeof(TextBuffer), "Failed to ReadFile  \"%s\" : %s\n", FilePath, print_buffer);
        
        MessageBoxA(NULL, TextBuffer, "Crystalflask Error code (0x0051)", MB_OK | MB_ICONERROR);
    }
    
    CloseHandle(FH);
    
    return File;
}







internal win32_startup_config
Win32LoadWindowConfig()
{
    win32_startup_config Config = {};
    Config.WindowPositionX = CW_USEDEFAULT;
    Config.WindowPositionY = CW_USEDEFAULT;
    Config.WindowWidth = CW_USEDEFAULT;
    Config.WindowHeight = CW_USEDEFAULT;
    size_t win32_startup_config_struct_size = sizeof(win32_startup_config);
    
    
    win32_state Win32State = {};
    Win32GetEXEFileName(&Win32State);
    
	char ConfigFileFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "..\\crystalflask\\data\\crystalflask.config", sizeof(ConfigFileFullPath), ConfigFileFullPath);
    
    
    win32_file File = Win32ReadDataFromFile(ConfigFileFullPath);
    if (File.Size == 0) return Config;
    
    if (File.Size == win32_startup_config_struct_size)
        Config = *(win32_startup_config*)File.Data;
    
    free(File.Data);
    
    return Config;
}


internal void
Win32SaveWindowConfig(HWND* Window)
{
    RECT WindowRect;
    if (!GetWindowRect(*Window, &WindowRect))
    {
        return;
    }
    
    win32_startup_config config;
    config.WindowPositionX = WindowRect.left;
    config.WindowPositionY = WindowRect.top;
    config.WindowWidth = WindowRect.right - WindowRect.left;
    config.WindowHeight = WindowRect.bottom - WindowRect.top;
    
    DWORD win32_startup_config_struct_size = sizeof(win32_startup_config);
    
    //cast the entire header struct to an unsigned char*
    unsigned char* ConfigStructData = (unsigned char*)&config;
    
    win32_state Win32State = {};
    Win32GetEXEFileName(&Win32State);
    
    char ConfigFileFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "..\\crystalflask\\data\\crystalflask.config", sizeof(ConfigFileFullPath), ConfigFileFullPath);
    
    Win32SaveDataToFile(ConfigFileFullPath, ConfigStructData, win32_startup_config_struct_size);
}

internal s32
Win32ReadBuildCounter()
{
    win32_state Win32State = {};
    Win32GetEXEFileName(&Win32State);
    
	char ConfigFileFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(&Win32State, "..\\crystalflask\\data\\build.counter", sizeof(ConfigFileFullPath), ConfigFileFullPath);
    
    win32_file File = Win32ReadDataFromFile(ConfigFileFullPath);
    if (File.Size == 0) return 0;
    
    s32 Result = *(s32*)File.Data;
    
    free(File.Data);
    
    return Result;
}


