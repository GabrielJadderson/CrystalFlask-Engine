#include "shader_data.cpp"

internal u32
OpenGLCreateProgram(char *HeaderCode, char *VertexCode, char *FragmentCode, const char* ShaderName)
{
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLchar *VertexShaderCode[] =
    {
        HeaderCode,
        VertexCode,
    };
    glShaderSource(VertexShaderID, ArrayCount(VertexShaderCode), VertexShaderCode, 0);
    glCompileShader(VertexShaderID);
    
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    GLchar *FragmentShaderCode[] =
    {
        HeaderCode,
        FragmentCode,
    };
    
    glShaderSource(FragmentShaderID, ArrayCount(FragmentShaderCode), FragmentShaderCode, 0);
    glCompileShader(FragmentShaderID);
    
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
    
    glValidateProgram(ProgramID);
    GLint Linked = false;
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Linked);
    if(!Linked)
    {
        GLsizei Ignored;
        char VertexErrors[4096];
        char FragmentErrors[4096];
        char ProgramErrors[4096];
        glGetShaderInfoLog(VertexShaderID, sizeof(VertexErrors), &Ignored, VertexErrors);
        glGetShaderInfoLog(FragmentShaderID, sizeof(FragmentErrors), &Ignored, FragmentErrors);
        glGetProgramInfoLog(ProgramID, sizeof(ProgramErrors), &Ignored, ProgramErrors);
        
        if (StringLength(VertexErrors) > 0)
        {
            char ShaderNameBuffer[128];
            sprintf_s(ShaderNameBuffer,128, "Vertex Shader Error in: %s", ShaderName);
            int Result = MessageBoxA(NULL, VertexErrors, ShaderNameBuffer,
                                     MB_OK | MB_ICONEXCLAMATION);
            CrystalFlaskConsole.AddLog("[ERROR]: %s\n%s_______________________________________________", ShaderNameBuffer, VertexErrors);
        }
        if (StringLength(FragmentErrors) > 0)
        {
            char ShaderNameBuffer[128];
            sprintf_s(ShaderNameBuffer, 128, "Fragment Shader Error in: %s", ShaderName);
            int Result = MessageBoxA(NULL, FragmentErrors, ShaderNameBuffer,
                                     MB_OK | MB_ICONEXCLAMATION);
            CrystalFlaskConsole.AddLog("[ERROR]: %s\n%s_______________________________________________", ShaderNameBuffer, VertexErrors);
        }
        if (StringLength(ProgramErrors) > 0)
        {
            char ShaderNameBuffer[128];
            sprintf_s(ShaderNameBuffer, 128, "Shader Program Error in: %s", ShaderName);
            int Result = MessageBoxA(NULL, ProgramErrors, ShaderNameBuffer,
                                     MB_OK | MB_ICONEXCLAMATION);
            CrystalFlaskConsole.AddLog("[ERROR]: %s\n%s_______________________________________________", ShaderNameBuffer, VertexErrors);
        }
    }
    return(ProgramID);
}

//~


internal void
Shader1i(u32 ProgramID, char* Name, s32 Value)
{
    glUniform1i(glGetUniformLocation(ProgramID, Name), Value);
}

internal void
Shader1f(u32 ProgramID, char* Name, float Value)
{
    glUniform1f(glGetUniformLocation(ProgramID, Name), Value);
}

internal void
Shader2fv(u32 ProgramID, char* Name, glm::vec2 &Value)
{
    glUniform2fv(glGetUniformLocation(ProgramID, Name), 1, &Value[0]);
}


internal void
Shader2fv(u32 ProgramID, char* Name, v2 *Value)
{
    glUniform2fv(glGetUniformLocation(ProgramID, Name), 1, &Value->E[0]);
}

internal void ShaderSet2f(u32 ProgramID, char* Name, float X, float Y)
{
    glUniform2f(glGetUniformLocation(ProgramID, Name), X, Y);
}

internal void
Shader3fv(u32 ProgramID, char* Name, glm::vec3 &Value)
{
    glUniform3fv(glGetUniformLocation(ProgramID, Name), 1, &Value[0]);
}

internal void
Shader3fv(u32 ProgramID, char* Name, v3 *Value)
{
    glUniform3fv(glGetUniformLocation(ProgramID, Name), 1, &Value->E[0]);
}

internal void
Shader3f(u32 ProgramID, char* Name, float X, float Y, float Z)
{
    glUniform3f(glGetUniformLocation(ProgramID, Name), X, Y, Z);
}


internal void
Shader4fv(u32 ProgramID, char* Name, glm::vec4 &Value)
{
    glUniform4fv(glGetUniformLocation(ProgramID, Name), 1, &Value[0]);
}


internal void
Shader4fv(u32 ProgramID, char* Name, v4 *Value)
{
    glUniform4fv(glGetUniformLocation(ProgramID, Name), 1, &Value->E[0]);
}

internal void
Shader4f(u32 ProgramID, char* Name, float X, float Y, float Z, float W)
{
    glUniform4f(glGetUniformLocation(ProgramID, Name), X, Y, Z, W);
}

internal void
ShaderMatrix2fv(u32 ProgramID, char* Name, glm::mat2 &Mat)
{
    glUniformMatrix2fv(glGetUniformLocation(ProgramID, Name), 1, GL_FALSE, &Mat[0][0]);
}

internal void
ShaderMatrix3fv(u32 ProgramID, char* Name, glm::mat3 &Mat)
{
    glUniformMatrix3fv(glGetUniformLocation(ProgramID, Name), 1, GL_FALSE, &Mat[0][0]);
}

internal void
ShaderMatrix4fv(u32 ProgramID, char* Name, glm::mat4 &Mat)
{
    glUniformMatrix4fv(glGetUniformLocation(ProgramID, Name), 1, GL_FALSE, &Mat[0][0]);
}



//~
internal u32
ShaderBuild(shader_type ShaderType)
{
    shader** ShaderData = NULL;
    void (*ShaderArrangeFunctionPointer)(shader*, u32) = NULL;
    char* VertexCode = NULL;
    char* FragmentCode = NULL;
    char* ProgramName = NULL;
    u32* ShaderIdPointer = NULL;
    
    switch (ShaderType)
    {
        case ShaderType_ColorProgram:
        {
            ShaderData = &GlobalShaderDataCache.ColorProgram;
            ShaderIdPointer = &GlobalShaderCache.ColorProgram;
            VertexCode = ShaderColorVertexCode;
            FragmentCode = ShaderColorFragmentCode;
            ProgramName = "ColorProgram";
            ShaderArrangeFunctionPointer = ShaderArrangeColorProgram;
        }
        case ShaderType_BasicProgram:
        {
            ShaderData = &GlobalShaderDataCache.BasicProgram;
            ShaderIdPointer = &GlobalShaderCache.BasicProgram;
            VertexCode = BasicShaderVertexCode;
            FragmentCode = BasicShaderFragmentCode;
            ProgramName = "BasicShader";
            ShaderArrangeFunctionPointer = ShaderArrangeBasicProgram;
        } break;
        
        case ShaderType_PBRProgram:
        {
            ShaderData = &GlobalShaderDataCache.PBRProgram;
            ShaderIdPointer = &GlobalShaderCache.PBRProgram;
            VertexCode = PBRVertexCode;
            FragmentCode = PBRFragmentCode;
            ProgramName = "PBRProgram";
            ShaderArrangeFunctionPointer = ShaderArrangePBR;
        } break;
        
        case ShaderType_SkyboxProgram:
        {
            ShaderData = &GlobalShaderDataCache.SkyboxProgram;
            ShaderIdPointer = &GlobalShaderCache.SkyboxProgram;
            VertexCode = SkyboxVertexCode;
            FragmentCode = SkyboxFragmentCode;
            ProgramName = "SkyboxProgram";
            ShaderArrangeFunctionPointer = ShaderArrangeSkybox;
        } break;
        
        case ShaderType_ColorShadingProgram:
        {
            ShaderData = &GlobalShaderDataCache.ColorShadingProgram;
            ShaderIdPointer = &GlobalShaderCache.ColorShadingProgram;
            VertexCode = ColorShaderVertexCode;
            FragmentCode = ColorShaderFragmentCode;
            ProgramName = "ColorShadingProgram";
            ShaderArrangeFunctionPointer = ShaderArrangeColorShadingProgram;
        } break;
        
        
        case ShaderType_TextProgram:
        {
            ShaderData = &GlobalShaderDataCache.TextProgram;
            ShaderIdPointer = &GlobalShaderCache.TextProgram;
            VertexCode = TextVertexCode;
            FragmentCode = TextFragmentCode;
            ProgramName = "TextProgram";
            ShaderArrangeFunctionPointer = ShaderArrangeTextProgram;
        } break;
        
        case ShaderType_SimpleShadingProgram:
        {
            ShaderData = &GlobalShaderDataCache.SimpleShadingProgram;
            ShaderIdPointer = &GlobalShaderCache.SimpleShadingProgram;
            VertexCode = SimpleShadingVertexCode;
            FragmentCode = SimpleShadingFragmentCode;
            ProgramName = "SimpleShadingProgram";
            ShaderArrangeFunctionPointer = ShaderArrangeSimpleShadingProgram;
        } break;
        
        case ShaderType_EditorFloorGridProgram:
        {
            ShaderData = &GlobalShaderDataCache.EditorFloorGridProgram;
            ShaderIdPointer = &GlobalShaderCache.EditorFloorGridProgram;
            VertexCode = EditorFloorGridFragmentCode;
            FragmentCode = EditorFloorGridFragmentCode;
            ProgramName = "EditorFloorGridProgram";
            ShaderArrangeFunctionPointer = ShaderArrangeEditorFloorGridProgram;
        } break;
        
        
        default: return 0; break;
    }
    
    if (*ShaderData == NULL)
    {
        *ShaderData = PushStruct(&GlobalResourceArena, shader);
        ZeroMemory(*ShaderData, sizeof(shader));
    }
    shader* Shader = *ShaderData;
    
    Shader->ShaderGenus.VertexCode = VertexCode;
    Shader->ShaderGenus.FragmentCode = VertexCode;
    
    u64 ProgramNameLength = StringLength(ProgramName);
    Shader->ShaderGenus.ProgramName = PushArray(&GlobalResourceArena, ProgramNameLength, char);
    CopyMemory(Shader->ShaderGenus.ProgramName, ProgramName, ProgramNameLength);
    Shader->ShaderGenus.ProgramNameCount = ProgramNameLength;
    Shader->ShaderGenus.ShaderArrangeFunction = ShaderArrangeFunctionPointer;
    
    *ShaderIdPointer= OpenGLCreateProgram(ShaderHeaderCode,
                                          VertexCode, FragmentCode,ProgramName);
    glUseProgram(*ShaderIdPointer);
    
    Shader->ProgramID = *ShaderIdPointer;
    Shader->ShaderType = ShaderType;
    
    if (ShaderArrangeFunctionPointer != NULL)
    {
        ShaderArrangeFunctionPointer(*ShaderData, *ShaderIdPointer);
    }
    
    return *ShaderIdPointer;
}

internal void
ShaderRebuild(shader** ShaderData)
{
    if (ShaderData == NULL || *ShaderData == NULL) return;
    
    shader* Shader = *ShaderData;
    
    Shader->ProgramID = OpenGLCreateProgram(ShaderHeaderCode,
                                            Shader->ShaderGenus.VertexCode, Shader->ShaderGenus.FragmentCode,
                                            Shader->ShaderGenus.ProgramName);
    glUseProgram(Shader->ProgramID);
    
    if (Shader->ShaderGenus.ShaderArrangeFunction != NULL)
    {
        Shader->ShaderGenus.ShaderArrangeFunction(Shader, Shader->ProgramID);
    }
    
}


internal void
LoadShaders()
{
    ShaderBuild(ShaderType_BasicProgram);
    ShaderBuild(ShaderType_ColorShadingProgram);
    ShaderBuild(ShaderType_SkyboxProgram);
    ShaderBuild(ShaderType_TextProgram);
    ShaderBuild(ShaderType_SimpleShadingProgram);
    ShaderBuild(ShaderType_PBRProgram);
    ShaderBuild(ShaderType_EditorFloorGridProgram);
}