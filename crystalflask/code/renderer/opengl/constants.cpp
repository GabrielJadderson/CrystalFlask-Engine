#pragma once

global_variable Executor GlobalExecutor{};

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/ext/matrix_transform.hpp>

//camera

global_variable glm::vec3 CameraPosition = glm::vec3(12, 23, 12);
global_variable glm::vec3 CameraRotation = glm::vec3(0, 0, 0);
global_variable glm::vec3 FocalPoint     = glm::vec3(13, 14, 21);

global_variable r32 HorizontalAngle = 3.14f;
global_variable r32 VerticalAngle = 0.0f;
global_variable r32 Fov = 45.0f;
global_variable r32 ZNear = 0.1f;
global_variable r32 ZFar = 10000.0f;
global_variable r32 Speed = 3.0f; // 3 units / second
global_variable r32 MouseSpeed = 0.005f;
global_variable r32 Distance;
global_variable r32 PanSpeed = 0.15f;
global_variable r32 RotationSpeed = 0.3f;
global_variable r32 ZoomSpeed = 5.0f;
global_variable r32 Pitch = 31.960;
global_variable r32 Yaw = 31.227f;

//camera


global_variable glm::vec3 LightDirection, LightRadiance, AlbedoColor;

global_variable u32 AlbedoTexture, NormalTexture, MetalnessTexture, RoughnessTexture, EnvIrradianceTexture, EnvRadianceTexture, BRDFLUTTexture;

global_variable r32 Metalness, Roughness, EnvMapRotation;

global_variable b32 RadiancePrefilterToggle, AlbedoTexToggle, NormalTexToggle, MetalnessTexToggle, RoughnessTexToggle;






//~ Misc

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec3 Tangent;
    glm::vec3 Binormal;
    glm::vec2 Texcoord;
};
static const int NumAttributes = 5;

struct Index
{
    uint32_t V1, V2, V3;
};

struct Submesh
{
    u32 BaseVertex;
    u32 BaseIndex;
    u32 MaterialIndex;
    u32 IndexCount;
};


struct mesh_data
{
    glm::mat4 InverseTransform;
    
    Submesh *Submesh;
    u64 SubmeshCount;
    
    Vertex *Vertices;
    u64 VertexCount;
    
    Index *Indices;
    u64 IndexCount;
    
    u32 VAO;
    u32 VertexBuffer;
    u32 IndexBuffer;
    b32 IsInitialized = false;
    b32 IsUploaded = false;
};

enum primitives
{
    PRIMITIVE_SPHERE,
    PRIMITIVE_PLANE,
    PRIMITIVE_CUBE,
    PRIMITIVE_CUBE_BLENDER,
    PRIMITIVE_CYLINDER,
    PRIMITIVE_CONE,
    PRIMITIVE_ICOSPHERE,
    PRIMITIVE_MONKEY,
    PRIMITIVE_WATER,
    
    PRIMITIVE_COUNT = 10,  //COUNT
};

global_variable mesh_data *GlobalPrimitiveMeshDataCache[(unsigned char)PRIMITIVE_COUNT];

global_variable mesh_data *GlobalUnUploadedMeshDataCache[128];
global_variable u64 GlobalUnUploadedMeshDataCacheCounter = 0;
global_variable HANDLE GlobalUnUploadedMeshDataCacheMUTEX;


//~ mesh


struct texture
{
    u32 TextureID;
    b32 Enabled;
};

//~ textures

struct shader_opengl_struct_uniform //represents a struct uniform in opengl
{
    u32 StructLocations[20];
    u32 StructLocationsCount;
};

struct shader
{
    u32 ProgramID;
    
    u32 VertexUniformLocations[20];
    u32 VertexUniformCount;
    
    u32 FragmentUniformLocations[20];
    u32 FragmentUniformCount;
    
    shader_opengl_struct_uniform StructUniform[5]; //represents a struct uniform in opengl
    u32 StructUniformCount;
    
    u32 TextureLocations[10]; //location ID in the shader. NOTE(Gabriel) 10 should be enough for what we're doing?
    u32 TextureCount; //not used
};

struct shader_list
{
    //special programs
    u32 SkyboxProgram;
    u32 TextProgram; //in-game Text rendering
    u32 EditorFloorGridProgram;
    
    //mesh
    u32 BasicProgram;
    u32 PBRProgram;
};

struct shader_data_list
{
    //Special Programs
    shader* SkyboxProgram;
    shader* TextProgram;
    shader* EditorFloorGridProgram;
    
    //Mesh
    shader* BasicProgram;
    shader* PBRProgram;
};

global_variable shader_list GlobalShaderCache = {};
global_variable shader_data_list GlobalShaderDataCache = {};

//~ shaders


//NOTE(Gabriel): materials are only for meshes.
struct material
{
    //Shader
    shader *Shader;
    
    //2D Textures
    u32 Textures2D[10];
    u32 TexturesCount;
    
    //
};

//~ shaders

struct stack_string_64
{
    u64 Length;
    char String[64];
};

struct entity
{
    stack_string_64 *Name;
    glm::vec3 Position;
    glm::vec3 Rotation;
    glm::vec3 Scale;
    
    material *Material;
    mesh_data *MeshData;
    
    b32 Enabled;
    b32 IsDeleted;
    
    s32 SortIndex; //used by the renderer for sorting
    u32 EntityIndex; //The entity ID
    
    b32 HasChild;
    entity *Child; //entity children???
};


global_variable entity *GlobalPointerEntityCache = NULL;
global_variable u64 GlobalPointerEntityCacheCount = 0;

global_variable array<entity*> *GlobalPointerDeletedEntityCache = NULL;

global_variable u32 GlobalPointerEntityCount = 0; //Count of all entity currently stored, 1 indexed

//~ entities

struct scene
{
    char SceneName[256];
    
    entity* EntityCache;
    u64 EntityCacheCount;
    u64 EntityCacheMaxCount;
    
    array<entity*> DeletedEntityCache;
    
    u64 EntityCount;
};

global_variable scene *GlobalScene = NULL;

//~ scene


//~ BEGIN Editor
global_variable b32 GlobalIsEditorEnabled = false; //always disable it by default, it helps to profile performance.
global_variable bool GlobalIsConsoleVisible = false;
global_variable bool GlobalPopupSettings = false;
global_variable bool GlobalPropertiesEnabled = true;
global_variable bool GlobalIsInfoOverlayVisible = true; // TODO(Gabriel): Add a toggle switch in settings for this there is no way to enable it when disabled ATM.


global_variable mesh_data* EditorGridPlane = NULL;
global_variable r32 EditorGridPlaneScale = 20;
global_variable glm::vec3 EditorGridPosition = {0, -0.01, 0};
global_variable glm::vec3 EditorGridScale = {10, 1, 10};

global_variable entity* GlobalSelectedEntity = NULL;
global_variable entity *GlobalEditorSelectedEntity = NULL;
global_variable b32 GlobalEditorSelectedEntityWasSelected = false;
global_variable b32 GlobalEditorSelectedEntityWasSelectedTriggerOnce = false;

//~ END Editor

//~ Forward declarations

entity* CreateEntity(scene *Scene, char *Name);
entity* CreateEntityPrimitive(scene *Scene, char *Name, primitives Primitive);
entity* CreateEntityFromFile(scene *Scene, char* FilePath);
void SerializeEntityToFile(entity *Entity, char* FilePath);
void DeleteEntity(scene *Scene, entity* Entity);
//scene* CreateScene(const char *SceneName);
