

//not used yet.
internal void
RenderMaterial(material Material)
{
    
    //1. Figure out which shader we have bound and it's properties.
    glUseProgram(Material.Shader->ProgramID);
    
    //2. bind texture units
    for (u32 TextureIndex = 0; TextureIndex < Material.TexturesCount; TextureIndex++)
    {
        //Activate Texture
        glActiveTexture(GL_TEXTURE0 + TextureIndex);
        
        //Bind it
        glBindTexture(GL_TEXTURE_2D, Material.Textures2D[TextureIndex]);
        
        //Upload to the shader which texture unit corresponds to which texture uniform.
        glUniform1i(Material.Shader->TextureLocations[TextureIndex], TextureIndex);
    }
    
    
}