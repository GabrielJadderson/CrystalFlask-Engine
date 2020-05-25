
#include "..\..\util\pool.h"
#include "..\..\util\array.cpp"
#include "..\..\IO\crystalflask_datapacker.cpp"

#include "constants.cpp"

#include "crystalflask_math.h"
#include "crystalflask_imgui.cpp"
#include "shader.cpp"
#include "material.cpp"
#include "texturer.cpp"

#include "model.h"
#include "mesh.cpp"
#include "entities.cpp"
#include "scene.cpp"

#include "crystalflask_imgui_console.cpp"

#include "camera.cpp"
#include "editor.cpp"



global_variable glm::mat4 ProjectionMatrix, ViewMatrix, MVP;
global_variable camera Camera = {};

//skybox
global_variable mesh_data *SkyboxMesh = NULL;
global_variable texture_descriptor SkyboxCubemap;

u32 texture3 = 0;
u32 texture4 = 0;


#include "renderer.cpp"


global_variable glm::vec2 MousePos = glm::vec2();

global_variable u32 LightID;
global_variable u32 MatrixID2;
global_variable u32 ViewMatrixID;
global_variable u32 ModelMatrixID;
global_variable u32 TextureID;

global_variable b32 IsOpenGLInitialized = false;

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum camera_movement {
    CameraMovment_FORWARD,
    CameraMovment_BACKWARD,
    
    CameraMovment_LEFT,
    CameraMovment_RIGHT
};

global_variable glm::vec2 Resolution;


global_variable glm::vec2 InitialMousePosition;
global_variable glm::vec3 InitialFocalPoint, InitialRotation;

global_variable u32 VertexArrayID;
global_variable u32 MatrixID;



// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;
const float ZOOM        =  45.0f;


internal opengl_info
OpenGLGetInfo(b32 ModernContext)
{
    opengl_info Result = {};
    
    Result.ModernContext = ModernContext;
    Result.Vendor = (char *)glGetString(GL_VENDOR);
    Result.Renderer = (char *)glGetString(GL_RENDERER);
    Result.Version = (char *)glGetString(GL_VERSION);
    
    if(Result.ModernContext)
    {
        Result.ShadingLanguageVersion = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    }
    else
    {
        Result.ShadingLanguageVersion = "(none)";
    }
    
    Result.Extensions = (char *)glGetString(GL_EXTENSIONS);
    
    if (Result.Extensions)
    {
        
    }
    
    return(Result);
}

internal void
OpenGLUpdateViewport(s32 X, s32 Y, s32 ViewportWidth, s32 ViewportHeight)
{
    if (IsOpenGLInitialized)
    {
        glViewport(X, Y, ViewportWidth, ViewportHeight);
    }
}


internal opengl_info
OpenGLInit(HWND Window, b32 FramebufferSupportsSRGB)
{
    ImGuiInit(Window);
    opengl_info Result = OpenGLGetInfo(true);
    
    Result.OpenGLVersion = (char*)glGetString(GL_VERSION);
    
    OutputDebugStringA(Result.OpenGLVersion);
    
    OpenGL.DefaultInternalTextureFormat = GL_RGBA8;
    if(FramebufferSupportsSRGB && Result.GL_EXT_texture_sRGB && Result.GL_EXT_framebuffer_sRGB)
    {
        OpenGL.DefaultInternalTextureFormat = GL_SRGB8_ALPHA8;
        glEnable(GL_FRAMEBUFFER_SRGB);
    }
    
    IsOpenGLInitialized = true;
    return(Result);
}



global_variable u32 DDS;

global_variable mesh_data *MeshDat = NULL;
global_variable mesh_data *MeshQuad = NULL;

global_variable u32 DebugCubeMapTexture = 0;


internal void
OpenGLStart()
{
    //load textures and stuff
    LoadPrimitives();
    
    LoadShaders();
    
    //Renderer = (renderer*)PushStruct(&GlobalOpenGLArena, renderer);
    Renderer->Start();
    
    
    
    // TODO(Gabriel): Fix this -> see if we move these into material or shader or a different textureallocator??? otherwise keep them here.
    texture_descriptor Texture1 = RegisterTexture("resources/textures/UV1024.png", TextureType_PNG);
    
    texture_descriptor Texture2 = RegisterTexture("resources/textures/StoneFloorTexture.png", TextureType_PNG);
    
    texture_descriptor Texture3 = RegisterTexture("resources/textures/blue_noise.png", TextureType_PNG);
    
    texture_descriptor Texture4 = RegisterTexture("resources/textures/noise.png", TextureType_PNG);
    
    
    texture_descriptor UvmapTex = RegisterTexture("resources/textures/uvmap.DDS", TextureType_DDS);
    
    DDS = UvmapTex.TextureID;
    
    texture3 = Texture3.TextureID;
    texture4 = Texture4.TextureID;
    
    texture1 = Texture1.TextureID;
    texture2 = Texture2.TextureID;
    
    
    scene *Scene = CreateScene("TestScene");
    GlobalScene = Scene;
    
    entity* Cube = CreateEntityPrimitive(Scene, "Cube", PRIMITIVE_CUBE);
    Cube->Position = glm::vec3{0, 0.5, 0};
    
    entity* Plane = CreateEntityPrimitive(Scene, "Plane", PRIMITIVE_PLANE);
    Plane->Position = glm::vec3{4, 0, 0};
    
    entity* Cube1 = CreateEntityPrimitive(Scene, "Cube Blender", PRIMITIVE_CUBE_BLENDER);
    Cube1->Position = glm::vec3{8, 1, 0};
    
    entity* Sphere = CreateEntityPrimitive(Scene, "Sphere", PRIMITIVE_SPHERE);
    Sphere->Position = glm::vec3{8, 1, 4};
    
    entity* Cylinder = CreateEntityPrimitive(Scene, "Cylinder", PRIMITIVE_CYLINDER);
    Cylinder->Position = glm::vec3{4, 1, 4};
    
    entity* Cone = CreateEntityPrimitive(Scene, "Cone", PRIMITIVE_CONE);
    Cone->Position = glm::vec3{0, 1, 4};
    
    entity* Icosphere = CreateEntityPrimitive(Scene, "Icosphere", PRIMITIVE_ICOSPHERE);
    Icosphere->Position = glm::vec3{-4, 1, 4};
    
    entity* Monkey = CreateEntityPrimitive(Scene, "Monkey", PRIMITIVE_MONKEY);
    Monkey->Position = glm::vec3{-8, 1, 4};
    
    
#if 0
    for (u32 Z = 10;  Z < 15; Z+=2)
    {
        for (u32 Y = 10;  Y < 100; Y+=2)
        {
            for (u32 X = 10;  X < 100; X+=2)
            {
                char NameBuffer[60];
                sprintf_s(NameBuffer, 60, "Monkey [X:%d, Y:%d, Z%d]", X, Y, Z);
                entity* en = CreateEntityPrimitive(GlobalScene, NameBuffer, PRIMITIVE_MONKEY);
                en->Position = glm::vec3{X, Z, Y};
            }
        }
    }
#endif
    
    
    EditorGridPlane = GetPrimitiveMeshData(PRIMITIVE_PLANE);
    
    /*
    SkyboxCubemap = RegisterCubeMap("resources/Skybox/2/right.jpg",
                                    "resources/Skybox/2/left.jpg",
                                    "resources/Skybox/2/up.jpg",
                                    "resources/Skybox/2/down.jpg",
                                    "resources/Skybox/2/front.jpg",
                                    "resources/Skybox/2/back.jpg");
    */
    
    SkyboxCubemap = RegisterCubeMap("resources/Skybox/1/right.tga",
                                    "resources/Skybox/1/left.tga",
                                    "resources/Skybox/1/up.tga",
                                    "resources/Skybox/1/down.tga",
                                    "resources/Skybox/1/front.tga",
                                    "resources/Skybox/1/back.tga");
    
    //texture_descriptor AlbedoTex = RegisterTexture("resources/textures/BRDF_LUT.tga", TextureType_TGA);
    
    texture_descriptor AlbedoTex = RegisterTexture("resources/textures/Checkerboard.tga", TextureType_TGA);
    
    texture_descriptor MetalnessTex = RegisterTexture("resources/textures/SphereAlbedo.tga", TextureType_TGA);
    texture_descriptor NormalTex = RegisterTexture("resources/textures/BRDF_LUT.tga", TextureType_TGA);
    
    
    AlbedoTexture = AlbedoTex.TextureID;
    NormalTexture = NormalTex.TextureID;
    MetalnessTexture = MetalnessTex.TextureID;
    RoughnessTexture = NormalTex.TextureID;
    EnvIrradianceTexture = SkyboxCubemap.TextureID;
    EnvRadianceTexture = SkyboxCubemap.TextureID;
    
    BRDFLUTTexture = AlbedoTex.TextureID;
    
    
    SkyboxMesh = GetPrimitiveMeshData(PRIMITIVE_CUBE);
    
    Camera = CameraInit();
    
}



internal glm::quat
GetOrientation()
{
    return glm::quat(glm::vec3(-Pitch, -Yaw, 0.0f));
}

internal glm::vec3
GetUpDirection()
{
    glm::quat Orientation = GetOrientation();
    glm::vec3 Result = glm::rotate(Orientation, glm::vec3(0.0f, 1.0f, 0.0f));
    return Result;
}

internal glm::vec3
GetRightDirection()
{
    return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
}

internal glm::vec3
GetForwardDirection()
{
    return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
}


internal glm::vec3
CalculatePosition()
{
    return FocalPoint - GetForwardDirection() * Distance;
}


internal glm::mat3
RotateXY(glm::vec2 angle)
{
    glm::vec2 c = cos(angle);
    glm::vec2 s = sin(angle);
    
    return glm::mat3(c.y, 0.0, -s.y,
                     s.y * s.x, c.x, c.y * s.x,
                     s.y * c.x, -s.x, c.y * c.x);
}

internal glm::mat4
LookAt(glm::vec3 point)
{
    return glm::lookAt(CameraPosition, point, GetUpDirection());
}


internal void
MousePan(const glm::vec2& delta)
{
    FocalPoint += -GetRightDirection() * delta.x * PanSpeed * Distance;
    FocalPoint += GetUpDirection() * delta.y * PanSpeed * Distance;
}

internal void
MouseRotate(const glm::vec2& delta)
{
    float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
    Yaw += yawSign * delta.x * RotationSpeed;
    Pitch += delta.y * RotationSpeed;
}

internal void
MouseZoom(float delta)
{
    Distance -= delta * ZoomSpeed;
    if (Distance < 1.0f)
    {
        FocalPoint += GetForwardDirection();
        Distance = 1.0f;
    }
}

internal void
ComputeMatricesFromInputs(game_input* Input)
{
    
    float DeltaTime = Input->dtForFrame;
    
    MousePos.x = (float)Input->MouseX;
    MousePos.y = (float)Input->MouseY;
    
    // Compute new orientation
    HorizontalAngle += MouseSpeed * float(MousePos.x - Input->MouseX );
    VerticalAngle   += MouseSpeed * float(MousePos.y - Input->MouseY );
    
    // Direction : Spherical coordinates to Cartesian coordinates conversion
    glm::vec3 direction(cos(VerticalAngle) * sin(HorizontalAngle),
                        sin(VerticalAngle),
                        cos(VerticalAngle) * cos(HorizontalAngle));
    
    // Right vector
    glm::vec3 right = glm::vec3(sin(HorizontalAngle - M_PI/2.0f),
                                0,
                                cos(HorizontalAngle - M_PI/2.0f));
    
    // Up vector
    glm::vec3 up = glm::cross( right, direction );
    
    
    ProjectionMatrix = glm::perspectiveFov(glm::radians(Fov), (r32)GlobalWidth, (r32)GlobalHeight, ZNear, ZFar);
    ViewMatrix       = glm::lookAt(CameraPosition,  // Camera is here
                                   CameraPosition + direction, // and looks here at the same position, plus "direction"
                                   up); // Head is up (set to 0,-1,0 to look upside-down)
    
}

internal void
OpenGLRender(game_input* Input)
{
    
    ImGuiIO& io = ImGui::GetIO();
    Input->MouseButtons[0].EndedDown = io.MouseDown[0];
    Input->MouseButtons[1].EndedDown = io.MouseDown[1];
    Input->MouseButtons[2].EndedDown = io.MouseDown[2];
    
    
    
    for(int ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        if(Controller->IsAnalog)
        {
        }
        else
        {
            if(Controller->MoveUp.EndedDown)
            {
            }
            if(Controller->MoveDown.EndedDown)
            {
            }
            if(Controller->MoveLeft.EndedDown)
            {
            }
            if(Controller->MoveRight.EndedDown)
            {
            }
            
            
            if (io.KeysDown[VK_MENU])
            {
                MousePos.x = (float)Input->MouseX;
                MousePos.y = (float)Input->MouseY;
                glm::vec2 delta = MousePos - InitialMousePosition;
                InitialMousePosition = MousePos;
                
                delta *= Input->dtForFrame * 0.5f;
                
                if (Input->MouseButtons[2].EndedDown)
                    MousePan(delta);
                
                if (Input->MouseButtons[0].EndedDown)
                    MouseRotate(delta);
                
                if (Input->MouseButtons[1].EndedDown)
                    MouseZoom(delta.y);
            }
            
            
            
        }
    }
    ComputeMatricesFromInputs(Input);
    
    
    
    CameraPosition = glm::lerp(CameraPosition, CalculatePosition(), Input->dtForFrame * 4.0f);
    
    glm::quat orientation = GetOrientation();
    CameraRotation = glm::eulerAngles(orientation) * (180.0f / (float)M_PI);
    ViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 1)) * glm::toMat4(glm::conjugate(orientation)) * glm::translate(glm::mat4(1.0f), -CameraPosition);
    
    ViewMatrix = glm::translate(glm::mat4(1.0f), CameraPosition) * glm::toMat4(orientation);
    ViewMatrix = glm::inverse(ViewMatrix);
    
    
    glm::mat4 ViewProjection = ProjectionMatrix * ViewMatrix;
    
    
    /*
    CameraProject(&Camera);
    CameraUpdateViewMatrix(&Camera);
    
    ViewProjection = Camera.ProjectionMatrix * Camera.ViewMatrix;
    ProjectionMatrix = Camera.ProjectionMatrix;
    ViewMatrix = Camera.ViewMatrix;
    */
    
    
    
    //request new frame
    Renderer->BeginFrame();
    
    
    //render the editor
    RenderEditor(ViewProjection, &Camera);
    
    
    //render the game.
    Renderer->Render(Input->dtForFrame);
    
    
    //finalize imgui
    ImGuiFinish(io);
    
    //end frame
    Renderer->EndFrame();
    //
}
