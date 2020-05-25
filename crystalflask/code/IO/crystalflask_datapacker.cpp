#include "crystalflask_datapacker.h"

#pragma warning( push )
#pragma warning( disable : 4172 )
char *
OpenFolderDialog()
{
    // Initialize COM to be able to use classes like IFileOpenDialog.
    CoInitialize(nullptr);
    
    // Create an instance of IFileOpenDialog.
    CComPtr<IFileOpenDialog> pFolderDlg;
    pFolderDlg.CoCreateInstance(CLSID_FileOpenDialog);
    
    // Set options for a filesystem folder picker dialog.
    FILEOPENDIALOGOPTIONS opt{};
    pFolderDlg->GetOptions(&opt);
    pFolderDlg->SetOptions(opt | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM);
    
    char* ResultBuffer = (char*)malloc(MAX_PATH * sizeof(char));
    // Show the dialog modally.
    if (SUCCEEDED(pFolderDlg->Show(nullptr)))
    {
        CComPtr<IShellItem> pSelectedItem;
        pFolderDlg->GetResult(&pSelectedItem);
        
        CComHeapPtr<wchar_t> pPath;
        pSelectedItem->GetDisplayName(SIGDN_FILESYSPATH, &pPath);
        
        wcstombs_s(NULL, ResultBuffer, MAX_PATH, pPath, MAX_PATH);
    }
    CoUninitialize();
    return ResultBuffer;
}
#pragma warning( pop )


//free the memory when you're done!
#pragma warning( push )
#pragma warning( disable : 4172 )
char*
SaveFolderDialog()
{
    // Initialize COM to be able to use classes like IFileSaveDialog.
    CoInitialize(nullptr);
    // Create an instance of IFileSaveDialog.
    CComPtr<IFileSaveDialog> pFolderDlg;
    pFolderDlg.CoCreateInstance(CLSID_FileSaveDialog);
    
    // Set options for a filesystem folder picker dialog.
    FILEOPENDIALOGOPTIONS opt{};
    pFolderDlg->GetOptions(&opt);
    pFolderDlg->SetOptions(opt | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM);
    
    char* ResultBuffer = (char*)malloc(MAX_PATH * sizeof(char));
    // Show the dialog modally.
    if (SUCCEEDED(pFolderDlg->Show(nullptr)))
    {
        CComPtr<IShellItem> pSelectedItem;
        pFolderDlg->GetResult(&pSelectedItem);
        
        CComHeapPtr<wchar_t> pPath;
        pSelectedItem->GetDisplayName(SIGDN_FILESYSPATH, &pPath);
        
        wcstombs_s(NULL, ResultBuffer, MAX_PATH, pPath, MAX_PATH);
    }
    CoUninitialize();
    return ResultBuffer;
}
#pragma warning( pop )

char*
file_ext_lookup_table[]
{
    "",
    "cfs",
    "cfm",
    "cft",
    "cfmes",
    "cfsh",
    "cfnod",
    "cfprj",
    "cfbin",
    "xwb",
    "wav",
    "txt",
    "png",
};

char
TryInferType( char* ext)
{
    for (unsigned char i = 0; i < sizeof(file_ext_lookup_table); ++i)
    {
        if (strcmp(ext, file_ext_lookup_table[i]) == 0)
        {
            //return (enum SaveFileType) i;
            return 0;
        } else continue;
    }
    //return SaveFileType::file_type_ALL;
    return 0;
}

char SIZEOF_CFBinContentDescriptor = sizeof(CFBin::CFBinContentDescriptor); //24

CFBin::CFBin(char* Filepath, memory_arena *ArenaPtr)
{
    Allocator = ArenaPtr;
    Load(Filepath);
}

void CFBin::Init()
{
    Content.data = Allocator->Base;
    Writeable = true;
}

unsigned long long
CFBin::AddResource( char* Filepath, unsigned char Type, unsigned char* Data, unsigned long long DataLength)
{
    if (Writeable == false) Init();
    
    size_t fplen = strlen(Filepath);
    Header.number_of_items = Header.number_of_items + 1;
    Header.data_size += (SIZEOF_CFBinContentDescriptor)+(fplen + 1) + DataLength;
    
    meow_u128 Hash = MeowHash(MeowDefaultSeed, fplen, (char*)&Filepath);
    unsigned  int Hash32 = MeowU32From(Hash, 0);
    
    CFBinContentDescriptor* base_address = (CFBinContentDescriptor*)PushStruct(Allocator, CFBinContentDescriptor);
    base_address->Type = Type;
    base_address->Hash = Hash32;
    base_address->FilepathLength = (unsigned char)fplen + 1;
    base_address->DataLength = DataLength;
    base_address->NextPosition = Header.data_size; //just to make sure that initially the position doesn't get set to 0 because it is set to the size of the header initially.
    
    unsigned char* base_address_data = (unsigned char*)PushSize_(Allocator, fplen + 1 + DataLength);
    
    memcpy(&base_address_data[0], &Filepath[0], fplen);
    base_address_data[fplen] = '\0';
    memcpy(&base_address_data[fplen + 1], &Data[0], DataLength);
    
    return Hash32;
}

void CFBin::RemoveResource( char* fileNamePath)
{
    
}

void CFBin::RemoveResource(unsigned long long hash)
{
    
}

void CFBin::Pack( char* Filepath, u32 options)
{
    //LARGE_INTEGER start = Win32GetWallClock();
    if (options == 0)
    {
        //generate default options.
        options = CFBin::CFBinOptions::CFBinOptions_None;
    }
    
    FILE* filehandle = NULL;
    filehandle = fopen(Filepath, "w");
    
    if (filehandle == NULL)
    {
        char ErrorBuffer[256];
        Win32MessageBoxError("CFBin: Failed to write header bytes, %s, %s", Filepath, Win32GetLastErrorMessage(&ErrorBuffer));
        return;
    }
    
    size_t header_size = sizeof(CFBinHeader);
    Header.header_size = header_size;
    
    //cast the entire header struct to an unsigned char*
    unsigned char* headerBuffer = (unsigned char*)&Header;
    
    size_t write = fwrite(&headerBuffer[0], 1, header_size, filehandle);
    if (write != header_size)
    {
        char ErrorBuffer[256];
        Win32MessageBoxError("CFBin: Failed to write header bytes, %s, %s", Filepath, Win32GetLastErrorMessage(&ErrorBuffer));
        return;
    }
    
    Content.data = Allocator->Base;
    write = fwrite(&Content.data[0], sizeof(unsigned char), Header.data_size, filehandle);
    if (write != Header.data_size)
    {
        char ErrorBuffer[256];
        Win32MessageBoxError("CFBin: Failed to write data in %s, %s", Filepath, Win32GetLastErrorMessage(&ErrorBuffer));
        return;
    }
    
    
    fflush(filehandle);
    fclose(filehandle);
    
    //LARGE_INTEGER end = Win32GetWallClock();
    
    //Win32MessageBoxError("Failed to CreateFileA, %s, %s", Filepath, Win32GetLastErrorMessage(&ErrorBuffer));
    //CF_CORE_TRACE("CFPacker::CFBin: Write \"{0}\" - took {1} ms.", filepath, Win32CalcSecondsElapsedInMs(start, end));
}



CFBin::~CFBin()
{
    //if (Allocator) delete Allocator;
}


void CFBin::Load( char* Filepath)
{
    //LARGE_INTEGER start = Win32GetWallClock();
    
    
    HANDLE FH = CreateFileA(Filepath,
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
    
    if (FH == INVALID_HANDLE_VALUE)
    {
        char ErrorBuffer[256];
        Win32MessageBoxError("Failed to CreateFileA, %s, %s", Filepath, Win32GetLastErrorMessage(&ErrorBuffer));
        return;
    }
    
    HANDLE FMH = CreateFileMappingA(FH,
                                    NULL,
                                    PAGE_READONLY, //| SEC_NOCACHE,
                                    NULL,
                                    NULL,
                                    NULL);
    
    if (FMH == NULL)
    {
        char ErrorBuffer[256];
        Win32MessageBoxError("Failed to CreateFileMappingA, %s, %s", Filepath, Win32GetLastErrorMessage(&ErrorBuffer));
        return;
    }
    
    LARGE_INTEGER file_size;
    
    
    if (GetFileSizeEx(FH, &file_size) == 0)
    {
        char ErrorBuffer[256];
        Win32MessageBoxError("Failed to GetFileSizeEx, %s, %s", Filepath, Win32GetLastErrorMessage(&ErrorBuffer));
        return;
    }
    
    size_t sizeof_file = file_size.u.LowPart | ((unsigned long long)file_size.u.HighPart << 32);
    
    void* map = MapViewOfFile(FMH, FILE_MAP_READ, 0, 0, sizeof_file);
    if (map == NULL)
    {
        char ErrorBuffer[256];
        Win32MessageBoxError("Failed to MapViewOfFile, %s, %s", Filepath, Win32GetLastErrorMessage(&ErrorBuffer));
        return;
    }
    
    Content.data = (unsigned char*)map;
    
    CFBinHeader* hd = (CFBinHeader*)&Content.data[0];
    
    CFBinContentDescriptor* first_resource = (CFBinContentDescriptor*)&Content.data[hd->header_size];
    size_t last_resource_position = first_resource->NextPosition;
    
    char* fp = (char*)&Content.data[last_resource_position + SIZEOF_CFBinContentDescriptor];
    
    CFBinResource Resource = {};
    Resource.position = last_resource_position;
    Resource.name = fp;
    Resources.Push(Resource);
    
    for (u32 i = 1; i < Header.number_of_items; ++i)
    {
        CFBinContentDescriptor* resource = (CFBinContentDescriptor*)&Content.data[last_resource_position];
        last_resource_position = resource->NextPosition;
        
        fp = (char*)&Content.data[last_resource_position + SIZEOF_CFBinContentDescriptor];
        //std::string ret2(fp, resource->FilepathLength - 1);
        //filepath_indexmap.insert(ret2, last_resource_position);
        
        CFBinResource Res = {};
        Res.position = last_resource_position;
        Res.name = fp;
        Resources.Push(Res);
        
    }
    
    
    //dwFlagsAndAttributes = FILE_FLAG_RANDOM_ACCESS
    //The file or device is being opened or created for asynchronous I/O.
    //When subsequent I/O operations are completed on this handle,
    //the event specified in the OVERLAPPED structure will be set to the signaled state.
    //If this flag is specified, the file can be used for simultaneous read and write operations.
    //
    //If this flag is not specified, then I/O operations are serialized,
    //even if the calls to the read and write functions specify an OVERLAPPED structure.
    //
    //
    //dwFlagsAndAttributes = FILE_FLAG_OVERLAPPED
    
    //HANDLE ha = CreateFileMappingA(h1, NULL, NULL, PAGE_EXECUTE_READ NULL, );
    
    
    
    //fclose(filehandle);
    //allocator = new CrystalFlask::ArenaAllocator(header.total_datasize + 16);
    
    
    //LARGE_INTEGER end = Win32GetWallClock();
    //CF_CORE_TRACE("CFPacker::CFBin: read \"{0}\" - took {1} ms.", filepath, Win32CalcSecondsElapsedInMs(start, end));
}




CFBin::DataBuffer CFBin::get_data_for( char* filepath)
{
    u32 ResourceCount = (u32)Resources.Count;
    for (size_t i = 0; i < ResourceCount; ++i)
    {
        CFBinResource res = Resources[i];
        if (strcmp(res.name, filepath) == 0)
        {
            CFBinContentDescriptor* desc = (CFBinContentDescriptor*)&Content.data[res.position];
            return DataBuffer{ desc->DataLength, &Content.data[res.position + SIZEOF_CFBinContentDescriptor + desc->FilepathLength] };
        }
    }
    return DataBuffer{};
    //unsigned long long position = filepath_indexmap.get(filepath);
    //		CFBinContentDescriptor* desc = (CFBinContentDescriptor*)&content.data[position];
    //		return DataBuffer{ desc->DataLength, &content.data[position + SIZEOF_CFBinContentDescriptor + desc->FilepathLength] };
}

