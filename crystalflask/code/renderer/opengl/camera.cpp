

internal camera
CameraInit()
{
    camera Result = {};
    Result.FOV = 45.0;
    Result.ZNear = 0.0001;
    Result.ZFar = 10000.0;
    return Result;
}

internal void
CameraProject(camera *Camera)
{
    Camera->ProjectionMatrix = glm::mat4(0.0f);
	Camera->ProjectionMatrix[0].x = 1.0f / tan(Camera->FOV * (float)M_PI / 180.0f);
	Camera->ProjectionMatrix[1].y = 1.0f / tan(Camera->FOV * (float)M_PI / 180.0f);
	Camera->ProjectionMatrix[2].z = -(Camera->ZFar + Camera->ZNear) / (Camera->ZFar - Camera->ZNear);
	Camera->ProjectionMatrix[3].z = -2 * Camera->ZNear*Camera->ZFar / (Camera->ZFar - Camera->ZNear);
	Camera->ProjectionMatrix[2].w = -1.0;
}

internal void
CameraUpdateViewMatrix(camera *Camera)
{
	glm::mat4 identity(1.0f);
    
	glm::quat yawQ = glm::quat(glm::vec3(0.0f, Camera->Rotation.y, 0.0f));
	yawQ = glm::normalize(yawQ);
	glm::mat4 yawMat = glm::mat4_cast(yawQ);
    
	glm::quat pitch = glm::quat(glm::vec3(Camera->Rotation.x, 0.0f, 0.0f));
	pitch = glm::normalize(pitch);
	glm::mat4 pitchMat = glm::mat4_cast(pitch);
    
	Camera->ViewMatrix = pitchMat * yawMat * glm::translate(identity, Camera->Position);
    
	//invViewMatrix = glm::inverse(viewMatrix);
	//transposeInvViewMatrix = glm::transpose(invViewMatrix);
    
	Camera->Forward = glm::normalize(glm::vec3(Camera->ViewMatrix[0][2], Camera->ViewMatrix[1][2], Camera->ViewMatrix[2][2]));
}


internal void
CameraLookAt(camera *Camera, glm::vec3 &Eye, glm::vec3 &Target)
{
	Camera->ViewMatrix = glm::lookAt(Eye, Target, glm::vec3(0, 1, 0));
	Camera->Position = -Eye;
	Camera->Forward = glm::normalize(glm::vec3(Camera->ViewMatrix[0][2], Camera->ViewMatrix[1][2], Camera->ViewMatrix[2][2]));
}

internal void
CameraTranslate(camera *Camera, glm::vec3 &NewPos)
{
    float ForwardDelta = NewPos.z;
	float StrafeDelta = NewPos.x;
	float UpDelta = NewPos.y;
    
	glm::vec3 Forward(Camera->ViewMatrix[0][2], Camera->ViewMatrix[1][2], Camera->ViewMatrix[2][2]);
	glm::vec3 Up(Camera->ViewMatrix[0][1], Camera->ViewMatrix[1][1], Camera->ViewMatrix[2][1]);
	glm::vec3 Strafe(Camera->ViewMatrix[0][0],Camera->ViewMatrix[1][0], Camera->ViewMatrix[2][0]);
    
	glm::vec3 TargetPosition = Forward * ForwardDelta + Strafe * StrafeDelta + Up * UpDelta;
    
    Camera->Position += TargetPosition;
	CameraUpdateViewMatrix(Camera);
}

internal void
CameraRotate(camera *Camera, glm::vec3 &Rotation)
{
    Camera->Rotation += Rotation;
    CameraUpdateViewMatrix(Camera);
}


internal void
CameraImGuiRender2(camera *Camera)
{
    ImGui::Begin("asd Settings");
    
    {
        r32 Width = (float)GlobalWidth;
        r32 Height = (float)GlobalHeight;
        ImGui::SliderFloat("Width", &Width, 1.0f, 1920.0f);
        ImGui::SliderFloat("Height", &Height, 1.0f, 1080.0f);
    }
    
    
    ImGui::SliderFloat("Fov", &Camera->FOV, 0.001f, 179.9f);
    ImGui::SliderFloat("ZNear", &Camera->ZNear, 0.001f, 10000.0f);
    ImGui::SliderFloat("ZFar", &Camera->ZFar, 0.001f, 10000.0f);
    
    r32 imguiPosition3[3] = {Camera->Position[0], Camera->Position[1], Camera->Position[2]};
    ImGui::SliderFloat3("Position", imguiPosition3, -20.001f, 10.0f);
    Camera->Position[0] = imguiPosition3[0];
    Camera->Position[1] = imguiPosition3[1];
    Camera->Position[2] = imguiPosition3[2];
    
    r32 imgui_camera_rotation[3] = {Camera->Rotation[0], Camera->Rotation[1], Camera->Rotation[2]};
    ImGui::SliderFloat3("Rotation", imgui_camera_rotation, -20.001f, 10.0f);
    Camera->Rotation[0] = imgui_camera_rotation[0];
    Camera->Rotation[1] = imgui_camera_rotation[1];
    Camera->Rotation[2] = imgui_camera_rotation[2];
    
    ImGui::End();
}
