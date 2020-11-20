global_variable u32 *GlobalUploadedTextures = NULL;
global_variable u64 GlobalUploadedTexturesCount = 0;


u32
TextureCalculateMipMapCount(u32 Width, u32 Height)
{
    u32 Levels = 1;
    while ((Width | Height) >> Levels)
        Levels++;
    return Levels;
}


enum texture_type
{
    TextureType_PNG,
    TextureType_JPG,
    TextureType_TGA,
    TextureType_BMP,
    TextureType_DDS,
};

struct texture_descriptor
{
    texture_type Type;
    s32 Width, Height, NumberOfChannels;
    u32 TextureID;
    b32 Loaded;
    b32 Uploaded;
    u64 UploadedTextureIndex;
};


//~ BEGIN DDS
//Derived from https://github.com/opengl-tutorials/ogl/blob/master/common/texture.cpp
struct dds_pixelformat {
    DWORD Size;
    DWORD Flags;
    DWORD FourCC;
    DWORD RGBBitCount;
    DWORD RBitMask;
    DWORD GBitMask;
    DWORD BBitMask;
    DWORD ABitMask;
};

struct dds_header {
    DWORD           Size;
    DWORD           Flags;
    DWORD           Height;
    DWORD           Width;
    DWORD           PitchOrLinearSize;
    DWORD           Depth;
    DWORD           MipMapCount;
    DWORD           Reserved1[11];
    dds_pixelformat ddspf;
    DWORD           Caps;
    DWORD           Caps2;
    DWORD           Caps3;
    DWORD           Caps4;
    DWORD           Reserved2;
};

#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
internal u32
RegisterDDS(char *TexturePath){
    
    win32_file File = Win32ReadDataFromFile(TexturePath);
    if (File.Size == 0) return 0;
    
    char* Data = (char*)File.Data;
    
    char Magic[] = {"DDS "};
    
    if (StringCMP(4, &Data[0], &Magic[0]))
        return 0;
    
    u64 DataIndex = 4;
    dds_header* DDSHeader = (dds_header*)&Data[DataIndex];
    
    u32 Height      = DDSHeader->Height;
    u32 Width       = DDSHeader->Width;
    u32 LinearSize  = DDSHeader->PitchOrLinearSize;
    u32 MipMapCount = DDSHeader->MipMapCount;
    u32 FourCC      = DDSHeader->ddspf.FourCC;
    
    char *Buffer;
    u32 BufferSize;
    /* how big is it going to be including all mipmaps? */
    BufferSize = MipMapCount > 1 ? LinearSize * 2 : LinearSize;
    Buffer = &Data[DataIndex];
    
    u32 Components  = (FourCC == FOURCC_DXT1) ? 3 : 4;
    u32 Format;
    switch(FourCC)
    {
        case FOURCC_DXT1:
        Format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
        case FOURCC_DXT3:
        Format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
        case FOURCC_DXT5:
        Format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
        default:
        Buffer = 0;
        return 0;
    }
    
    // Create one OpenGL texture
    u32 TextureID;
    glGenTextures(1, &TextureID);
    
    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, TextureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    
    u32 BlockSize = (Format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
    u32 Offset = 0;
    
    /* load the mipmaps */
    for (u32 Level = 0; Level < MipMapCount && (Width || Height); ++Level)
    {
        u32 Size = ((Width+3)/4)*((Height+3)/4)*BlockSize;
        glCompressedTexImage2D(GL_TEXTURE_2D, Level, Format, Width, Height,
                               0, Size, Buffer + Offset);
        
        Offset += Size;
        Width  /= 2;
        Height /= 2;
        
        // Deal with Non-Power-Of-Two textures. This code is not included in the webpage to reduce clutter.
        if(Width < 1) Width = 1;
        if(Height < 1) Height = 1;
        
    }
    
    free(File.Data);
    
    return TextureID;
}

//~ END DDS

internal texture_descriptor
RegisterTexture(char* TexturePath, texture_type Type, b32 FlipTheTexture)
{
    texture_descriptor Texture = {};
    Texture.Type = Type;
    
    if (Type == TextureType_DDS)
    {
        Texture.TextureID = RegisterDDS(TexturePath);
        Texture.Loaded = true;
        Texture.Uploaded = true;
        return Texture;
    }
    
    
    u32 TextureID = 0;
    
    
    glGenTextures(1, &TextureID);
    glBindTexture(GL_TEXTURE_2D, TextureID);
    
    // set the texture wrapping parameters
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    
    
    // set texture filtering parameters
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, -1000);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 1000);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1000);
    
    // load image, create texture and generate mipmaps
    stbi_set_flip_vertically_on_load(FlipTheTexture); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *data = stbi_load(TexturePath, &Texture.Width, &Texture.Height, &Texture.NumberOfChannels, 4);
    if (data)
    {
        if (Type == TextureType_PNG)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Texture.Width, Texture.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        } else if (Type == TextureType_JPG)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Texture.Width, Texture.Height, 0,
                         GL_RGB, GL_UNSIGNED_BYTE, data);
        } else if (Type == TextureType_TGA)
        {
            // TODO(Gabriel): LOOK INTO TGA Loading, it works sometimes... doesn't support all formats and all comps.
            int comp = 0;
            int x= 0;
            int y = 0;
            stbi_info(TexturePath, &x, &y, &comp);
            
            if (comp == 3)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Texture.Width, Texture.Height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            }
            else if (comp == 4)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Texture.Width, Texture.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            }
            else
            {
                Win32MessageBoxError("Error unsupported TGA Comp: %d", comp);
            }
        }
        else if (Type == TextureType_BMP)
        {
            glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, Texture.Width, Texture.Height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
        }
        
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        char PathBuffer[256] = {};
        sprintf_s(PathBuffer, 256, "Failed to Register Texture: %s", TexturePath);
        int Result = MessageBoxA(NULL, PathBuffer,
                                 NULL, MB_OK | MB_ICONEXCLAMATION);
        if (Result == 1)
        {
            exit(1);
        }
    }
    stbi_image_free(data);
    stbi_set_flip_vertically_on_load(!FlipTheTexture); //reset stb flip
    
    Texture.TextureID = TextureID;
    Texture.Loaded = true;
    Texture.Uploaded = true;
    
    // TODO(Gabriel): Fix this and Fix Arrays -> make the memory arena work.
    /*
    if (GlobalUploadedTextures == NULL)
    {
        GlobalUploadedTextures = PushArray(&GlobalOpenGLArena, 64, u32);
        GlobalUploadedTexturesCount = 0;
    } else
    {
        Texture.UploadedTextureIndex = GlobalUploadedTexturesCount;
        GlobalUploadedTextures[GlobalUploadedTexturesCount++] = TextureID;
    }
    */
    
    return Texture;
}

// NOTE(Gabriel): Only supports 6 faced cubemaps.
internal texture_descriptor
RegisterCubeMap(char* CubeMapFaceRightPath,
                char* CubeMapFaceLeftPath,
                char* CubeMapFaceTopPath,
                char* CubeMapFaceBottomPath,
                char* CubeMapFaceBackPath,
                char* CubeMapFaceFrontPath)
{
    texture_descriptor Texture = {};
    
    glGenTextures(1, &Texture.TextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, Texture.TextureID);
    
    s32 width = 0;
    s32 height = 0;
    s32 nrChannels = 0;
    
    for (u8 i = 0; i < 6; i++)
    {
        
        char* Path = 0;
        switch (i)
        {
            case 0: Path = CubeMapFaceRightPath; break;
            case 1: Path = CubeMapFaceLeftPath;  break;
            case 2: Path = CubeMapFaceTopPath;    break;
            case 3: Path = CubeMapFaceBottomPath;  break;
            case 4: Path = CubeMapFaceBackPath;  break;
            case 5: Path = CubeMapFaceFrontPath; break;
        }
        
        stbi_set_flip_vertically_on_load(false);
        u8 *data = stbi_load(Path, &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            char PathBuffer[256] = {};
            sprintf_s(PathBuffer, 256, "Failed to Register Texture: %s", Path);
            int Result = MessageBoxA(NULL,
                                     PathBuffer,
                                     NULL,
                                     MB_OK | MB_ICONEXCLAMATION);
            if (Result == 1)
            {
                exit(1);
            }
        }
        
    }
    
    u32 MipMapLevels = TextureCalculateMipMapCount(width, height);
    
    glTextureStorage2D(Texture.TextureID, MipMapLevels, GL_RGB, width, height);
    glTextureParameteri(Texture.TextureID, GL_TEXTURE_MIN_FILTER, MipMapLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(Texture.TextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTextureParameteri(Texture.TextureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(Texture.TextureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(Texture.TextureID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    Texture.Loaded = true;
    Texture.Uploaded = true;
    
    return Texture;
}

//~

//~ BEGIN Text

global_variable u32 Text2DTextureID = 0;
global_variable u32 Text2DVertexBufferID = 0;
global_variable u32 Text2DUVBufferID = 0;
global_variable u32 Text2DShaderID = 0;
global_variable u32 Text2DUniformID = 0;

internal void
RegisterText2D(char* TexturePath)
{
    texture_descriptor Text2DTex = RegisterTexture(TexturePath, TextureType_DDS, true);
    Text2DTextureID = Text2DTex.TextureID;
    
    // Initialize VBO
	glGenBuffers(1, &Text2DVertexBufferID);
	glGenBuffers(1, &Text2DUVBufferID);
    
    
    // Initialize Shader
	Text2DShaderID = GlobalShaderCache.TextProgram;
    
	//Initialize uniforms' IDs
	Text2DUniformID = glGetUniformLocation(Text2DShaderID, "myTextureSampler");
}

internal void
Text2D(char * Text, s32 X, s32 Y, s32 Size){
    
    u32 TextLength = (u32)StringLength(Text);
    
    v2 *Vertices = PushArray(&GlobalOpenGLArena, 1024, v2);
    u64 VerticesCount = 0;
    
    v2 *UVs = PushArray(&GlobalOpenGLArena, 1024, v2);
    u64 UVsCount = 0;
    
	// Fill buffers
	//std::vector<glm::vec2> vertices;
	//std::vector<glm::vec2> UVs;
	for ( u32 Index = 0 ; Index < TextLength ; Index++ ){
		
		v2 VertexUpLeft    = {(float)(X + Index * Size)       , (float)(Y + Size) };
		v2 VertexUpRight   = {(float)(X + Index * Size + Size), (float)(Y + Size) };
        v2 VertexDownRight = {(float)(X + Index * Size + Size), (float)(Y)      };
        v2 VertexDownLeft  = {(float)(X + Index * Size)       , (float)(Y)      };
        
        arrput(Vertices, VertexUpLeft);
        arrput(Vertices, VertexDownLeft);
        arrput(Vertices, VertexUpRight);
        
        arrput(Vertices, VertexDownRight);
        arrput(Vertices, VertexUpRight);
        arrput(Vertices, VertexDownLeft);
        VerticesCount += 6;
        
        
		char Character = Text[Index];
		float UVX = (Character%16)/16.0f;
		float UVY = (Character/16)/16.0f;
        
        
        v2 UVUpLeft    = {UVX           , UVY};
        v2 UVUpRight   = {UVX+1.0f/16.0f, UVY};
        v2 UVDownRight = {UVX+1.0f/16.0f, UVY+ 1.0f/16.0f};
        v2 UVDownLeft  = {UVX           , UVY+ 1.0f/16.0f};
        
        
        arrput(UVs, UVUpLeft);
        arrput(UVs, UVDownLeft);
        arrput(UVs, UVUpRight);
        
        arrput(UVs, UVDownRight);
        arrput(UVs, UVUpRight);
        arrput(UVs, UVDownLeft);
        UVsCount += 6;
	}
    
	glBindBuffer(GL_ARRAY_BUFFER, Text2DVertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, VerticesCount * sizeof(glm::vec2), &Vertices[0], GL_STATIC_DRAW);
    
	glBindBuffer(GL_ARRAY_BUFFER, Text2DUVBufferID);
	glBufferData(GL_ARRAY_BUFFER, UVsCount * sizeof(glm::vec2), &UVs[0], GL_STATIC_DRAW);
    
	// Bind shader
	glUseProgram(Text2DShaderID);
    
	// Bind texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, Text2DTextureID);
	// Set our "myTextureSampler" sampler to use Texture Unit 0
	glUniform1i(Text2DUniformID, 0);
    
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, Text2DVertexBufferID);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    
	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, Text2DUVBufferID);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
	// Draw call
	glDrawArrays(GL_TRIANGLES, 0, (u32)VerticesCount);
    
	glDisable(GL_BLEND);
    
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
    
    
    char text[256];
    sprintf(text,"Hello World");
    Text2D(text, 10, 500, 60);
    
}

internal void
DeleteText2D(){
    
	// Delete buffers
	glDeleteBuffers(1, &Text2DVertexBufferID);
	glDeleteBuffers(1, &Text2DUVBufferID);
    
	// Delete texture
	glDeleteTextures(1, &Text2DTextureID);
    
	// Delete shader
	glDeleteProgram(Text2DShaderID);
}
//~ END text

#include <array>

internal u32
RegisterOpenGLTextureCube(char *Path)
{
    s32 width = 0;
    s32 height = 0;
    s32 channels = 0;
    stbi_set_flip_vertically_on_load(false);
    u8* data = stbi_load(Path, &width, &height, &channels, STBI_rgb);
    if (data == NULL) return 0;
    
    u32 faceWidth = width / 4;
    u32 faceHeight = height / 3;
    
    //u8* faces = PushArray(&GlobalTempArena, 6, u8);
    
    std::array<u8*,6> faces;
    
    for (size_t i = 0; i < 6; i++)
        faces[i] = new u8[faceWidth * faceHeight * 3]; // 3 BPP
    
    //(row * numColumns) + column
    
    u64 faceIndex = 0;
    
    for (size_t i = 0; i < 4; i++)
    {
        for (size_t y = 0; y < faceHeight; y++)
        {
            size_t yOffset = y + faceHeight;
            for (size_t x = 0; x < faceWidth; x++)
            {
                size_t xOffset = x + i * faceWidth;
                faces[faceIndex][(x + y * faceWidth) * 3 + 0] = data[(xOffset + yOffset * width) * 3 + 0];
                faces[faceIndex][(x + y * faceWidth) * 3 + 1] = data[(xOffset + yOffset * width) * 3 + 1];
                faces[faceIndex][(x + y * faceWidth) * 3 + 2] = data[(xOffset + yOffset * width) * 3 + 2];
            }
        }
        faceIndex++;
    }
    
    for (size_t i = 0; i < 3; i++)
    {
        // Skip the middle one
        if (i == 1)
            continue;
        
        for (size_t y = 0; y < faceHeight; y++)
        {
            size_t yOffset = y + i * faceHeight;
            for (size_t x = 0; x < faceWidth; x++)
            {
                size_t xOffset = x + faceWidth;
                faces[faceIndex][(x + y * faceWidth) * 3 + 0] = data[(xOffset + yOffset * width) * 3 + 0];
                faces[faceIndex][(x + y * faceWidth) * 3 + 1] = data[(xOffset + yOffset * width) * 3 + 1];
                faces[faceIndex][(x + y * faceWidth) * 3 + 2] = data[(xOffset + yOffset * width) * 3 + 2];
            }
        }
        faceIndex++;
    }
    
    u32 TextureID = 0;
    glGenTextures(1, &TextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, TextureID);
    
    glTextureParameteri(TextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(TextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(TextureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(TextureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(TextureID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTextureParameterf(TextureID, GL_TEXTURE_MAX_ANISOTROPY, 1.0f);
    
    u32 format = GL_RGB;
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[2]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[0]);
    
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[4]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[5]);
    
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[1]);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format, faceWidth, faceHeight, 0, format, GL_UNSIGNED_BYTE, faces[3]);
    
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    for (size_t i = 0; i < faces.size(); i++)
        delete[] faces[i];
    
    stbi_image_free(data);
    return TextureID;
}


enum texture_format
{
    TextureFormat_RGB,
    TextureFormat_RGBA,
    TextureFormat_Float16,
};

static GLenum
TextureFormatToOpenGLFormat(texture_format Format)
{
    switch (Format)
    {
        case TextureFormat_RGB:     return GL_RGB;
        case TextureFormat_RGBA:    return GL_RGBA;
        case TextureFormat_Float16: return GL_RGBA16F;
    }
    return 0;
}


internal u32
CreateEmptyTexture(texture_format TextureFormat, s32 width, s32 height)
{
    u32 MipMapLevels = TextureCalculateMipMapCount(width, height);
    
    u32 TextureID = 0;
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &TextureID);
    glTextureStorage2D(TextureID, MipMapLevels, TextureFormatToOpenGLFormat(TextureFormat), width, height);
    glTextureParameteri(TextureID, GL_TEXTURE_MIN_FILTER, MipMapLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTextureParameteri(TextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}