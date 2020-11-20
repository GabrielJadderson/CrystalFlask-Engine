


//not used yet.
internal void
RenderMaterial(material *Material,
               glm::mat4 &MVP,
               glm::mat4 &ViewProjectionMatrix,
               glm::mat4 &ViewMatrix,
               glm::mat4 &ModelMatrix)
{
    
    if (!Material->Shader)
    {
        ShaderBasicUpload(MVP);
        return;
    }
    
    //1. Figure out which shader we have bound and it's properties.
    glUseProgram(Material->Shader->ProgramID);
    
    
    switch (Material->Shader->ShaderType)
    {
        case ShaderType_ColorShadingProgram:
        {
            ShaderColorShadingUpload(MVP, Material->ShaderColor);
        } break;
        
        case ShaderType_BasicProgram:
        {
            for (u32 TextureIndex = 0; TextureIndex < Material->TexturesCount; TextureIndex++)
            {
                //Activate Texture
                glActiveTexture(GL_TEXTURE0 + TextureIndex);
                
                //Bind it
                glBindTexture(GL_TEXTURE_2D, Material->Textures2D[TextureIndex]);
                
                //Upload to the shader which texture unit corresponds to which texture uniform.
                glUniform1i(Material->Shader->TextureLocations[TextureIndex], TextureIndex);
                
            }
            ShaderBasicUpload(MVP);
        } break;
        
        case ShaderType_PBRProgram:
        {
            for (u32 TextureIndex = 0; TextureIndex < Material->TexturesCount; TextureIndex++)
            {
                glActiveTexture(GL_TEXTURE0 + TextureIndex);
                if (TextureIndex > 3)
                {
                    glBindTexture(GL_TEXTURE_CUBE_MAP, Material->Textures2D[TextureIndex]);
                }
                else
                {
                    glBindTexture(GL_TEXTURE_2D, Material->Textures2D[TextureIndex]);
                }
                glUniform1i(Material->Shader->TextureLocations[TextureIndex], TextureIndex);
            }
            ShaderPBRUpload(MVP, ViewProjectionMatrix, ModelMatrix);
        } break;
        
        case ShaderType_SimpleShadingProgram:
        {
            ShaderSimpleShadingUpload(MVP,
                                      ViewMatrix,
                                      ModelMatrix,
                                      LightPosition,
                                      Material->Textures2D[0],
                                      Material->Textures2D[1],
                                      Material->Textures2D[2]);
        } break;
        
        case ShaderType_TextProgram:
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, Material->Textures2D[0]);
            // Set our texture sampler to use Texture Unit 0
            glUniform1i(GlobalShaderDataCache.TextProgram->TextureLocations[0], 0);
        } break;
        
        default: {} break;
    }
    
}