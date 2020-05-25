//
// TODO(Gabriel): Integrate editor from crystal flask.
// TODO(Gabriel): make the editor control all editor functionality.



internal void
RenderEditor(glm::mat4 &ViewProjection, camera *Camera)
{
    ImGuiNewFrame();
    
    if (GlobalIsEditorEnabled)
    {
        
        SubmitImGuiComponents();
        //CameraImGuiRender2(Camera);
        
        
        
        //render editor grid plane
        glUseProgram(GlobalShaderCache.EditorFloorGridProgram);
        {
            glm::mat4 ModelMatrix = glm::mat4(1.0);
            
            ModelMatrix = glm::translate(ModelMatrix, EditorGridPosition);
            ModelMatrix = glm::scale(ModelMatrix, EditorGridScale);
            
            glm::mat4 MVP = ViewProjection * ModelMatrix;
            
            ShaderEditorFloorGridSubmit(MVP, EditorGridPlaneScale);
            RenderMesh(EditorGridPlane);
        }
        
        
        
    }
    
    
    ImGui::Render();
}
