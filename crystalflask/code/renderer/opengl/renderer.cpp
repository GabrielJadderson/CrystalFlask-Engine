
struct renderer
{
    HWND Window;
    
    internal renderer* Start();
    
    internal void BeginFrame();
    internal void Render(r32 DeltaTime);
    internal void EndFrame();
    
};


global_variable renderer *Renderer = NULL;

//called after the shaders have been loaded in OpenGLStart()
renderer*
renderer::Start()
{
    if (Renderer == NULL)
    {
        Renderer = (renderer*)PushStruct(&GlobalOpenGLArena, renderer);
    }
    
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
    
    // Cull triangles which normal is not towards the camera
	//glEnable(GL_CULL_FACE);
    //glEnable(GL_FRONT_AND_BACK);
    //glFrontFace(GL_CCW);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    
    return Renderer;
}

internal void
Clear()
{
    glClearColor(0.0f, 0.05f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


unsigned int texture1, texture2;


void
renderer::Render(r32 DeltaTime)
{
    
    glUseProgram(GlobalShaderCache.BasicProgram);
    
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);
    
    
    
    //glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
    //glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
    
    
    
    if (GlobalScene)
    {
        
        
        // TODO(Gabriel): remove this!
        static bool check = false;
        if (!check)
        {
            char Buffer[30];
            sprintf_s(Buffer, 30, "Entities Loaded: %llu", GlobalScene->EntityCacheCount);
            CrystalFlaskConsole.AddLog(Buffer);
            check = true;
        }
        
        //render entities
        for (u32 EntityIndex = 0; EntityIndex <  GlobalScene->EntityCacheCount; EntityIndex++)
        {
            UpdateAndRenderEntity(&GlobalScene->EntityCache[EntityIndex], ViewMatrix, ProjectionMatrix);
        }
        
        
        {
            glUseProgram(GlobalShaderCache.PBRProgram);
            
            glm::vec3 SkyboxPosition2 = {0, 3 ,0};
            glm::vec3 SkyboxScale2 = {10, 10, 10};
            glm::mat4 SkyboxModelMatrix2 = glm::mat4(1.0);
            
            SkyboxModelMatrix2 = glm::translate(SkyboxModelMatrix2, SkyboxPosition2);
            SkyboxModelMatrix2 = glm::scale(SkyboxModelMatrix2, SkyboxScale2);
            glm::mat4 ViewProjection = ProjectionMatrix * ViewMatrix;
            //glm::mat4 ViewProjection = ProjectionMatrix * ViewMatrix * SkyboxModelMatrix2;
            
            ShaderPBRSubmit(ViewProjection, SkyboxModelMatrix2);
            //ShaderBasicSubmit(ViewProjection);
            RenderMesh(SkyboxMesh);
        }
        
        
        
        
        //Skybox
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, SkyboxCubemap.TextureID);
        //glBindTexture(GL_TEXTURE_2D, texture4);
        
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, texture3);
        //glBindTexture(GL_TEXTURE_CUBE_MAP, SkyboxCubemap.TextureID);
        //glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        
        glUseProgram(GlobalShaderCache.SkyboxProgram);
        
        glm::vec3 SkyboxPosition = {0,3,0};
        glm::vec3 SkyboxScale = {100, 100, 100};
        glm::mat4 SkyboxModelMatrix = glm::mat4(1.0);
        //Translate * Rotate * Scale
        SkyboxModelMatrix = glm::translate(SkyboxModelMatrix, SkyboxPosition);
        SkyboxModelMatrix = glm::scale(SkyboxModelMatrix, SkyboxScale);
        glm::mat4 SkyboxMVP = ProjectionMatrix * ViewMatrix * SkyboxModelMatrix;
        
        // TODO(Gabriel): REMOVE THIS to somewhere dedicated.
        // TODO(Gabriel): Add imgui panel for skybox, so that we can control the skybox
        static r32 DeltaTimeIncrementer = 0.0f;
        DeltaTimeIncrementer += 0.35f;
        
        ShaderSkyboxSubmit(SkyboxMVP, DeltaTime * DeltaTimeIncrementer);
        //ShaderBasicSubmit(SkyboxMVP);
        RenderMesh(SkyboxMesh);
        
        glDepthFunc(GL_LESS); // set depth function back to default
        
    }
    
}


void
renderer::BeginFrame()
{
    Clear();
}

void
renderer::EndFrame()
{
    
}