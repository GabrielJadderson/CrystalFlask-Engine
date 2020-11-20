#pragma once


struct CFBin
{
    enum CFBinOptions
    {
        CFBinOptions_None = 0,
        CFBinOptions_Streaming = 1 << 0,
        //cf_binary_options_something = 1 << 1,
        //cf_binary_options_None = 1 << 2,
        //cf_binary_options_None = 1 << 3,
        //cf_binary_options_None = 1 << 4,
        //cf_binary_options_None = 1 << 5,
        //cf_binary_options_None = 1 << 6,
        //cf_binary_options_None = 1 << 7,
    };
    
    
    enum CFBinType
    {
        CFBinType_NONE = 0,
        CFBinType_TXT = 1,
        //textures
        CFBinType_JPG = 2,
        CFBinType_PNG = 3,
        
        CFBinType_OBJ = 4,
        CFBinType_FBX = 5,
        
        CFBinType_CFSH = 6,
        
        //cf_binary_options_something = 1 << 1,
        //cf_binary_options_None = 1 << 2,
        //cf_binary_options_None = 1 << 3,
        //cf_binary_options_None = 1 << 4,
        //cf_binary_options_None = 1 << 5,
        //cf_binary_options_None = 1 << 6,
        //cf_binary_options_None = 1 << 7,
    };
    
    struct CFBinHeaderNode //represents an item
    {
        bool IsDirty = false; //when marked dirty, we need to reload it.
        unsigned long long timestamp;
        unsigned long long index; //index in the CFBinContent arrays.
    };
    
    struct CFBinResource
    {
        unsigned long long position;
        char* name;
    };
    
    struct CFBinHeader //4 bytes + 8 + 8 + 8 = 28 bytes;
    {
        unsigned int MAGIC_SIGNATURE_CFBIN = 0x31F06C3E; //magic
        unsigned long long options = 0;
        unsigned long long number_of_items = 0;
        unsigned long long header_size = 0;		//size of entire header section
        unsigned long long content_size = 0;	//size of entire content section
        unsigned long long data_size = 0;
        //
    };
    
    //this structure is contained within data along with the filepath and the actual data.
    struct CFBinContentDescriptor //sizeof(CFBinContentDescriptor) = 24
    {
        unsigned char UNUSUED_PADDING1 = 0; //unused/abailable
        unsigned char UNUSUED_PADDING2 = 0; //unused/available
        unsigned char Type = 0;
        unsigned char FilepathLength = 0;
        unsigned int Hash = 0; //using int hash
        unsigned long long NextPosition = 0; //This is the position of the next resource. At the begining this should be the size of the header
        unsigned long long DataLength = 0;
    };
    
    struct CFBinContent
    {
        unsigned char* data;
    };
    
    CFBin( char* filepath, memory_arena* ArenaPtr); //unpack
    CFBin();
    ~CFBin();
    
    unsigned long long AddResource( char* filepath, unsigned char type, unsigned char* data, unsigned long long data_length);
    void RemoveResource(char* fileNamePath);
    void RemoveResource(unsigned long long hash);
    void Pack(char* filepath, unsigned int options = 0);
    void Load(char* filepath);
    void Init();
    
    
    struct DataBuffer
    {
        u64 size;
        unsigned char* ptr;
    };
    DataBuffer get_data_for( char* filepath);
    
    std::vector<std::string> ResourceNameList();
    
    //static void DisplayResources(CFBin& binarypack, Editor& editor);
    CFBinHeader Header;
    CFBinContent Content;
    bool Writeable = false;
    memory_arena *Allocator = NULL;
    
    array<CFBinResource> Resources;
};
