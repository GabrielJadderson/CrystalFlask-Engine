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


global_variable glm::vec3 LightPosition   = glm::vec3(0, 0, 0);
global_variable glm::vec3 LightDirection   = glm::vec3(0, 0, 0);
global_variable glm::vec3 LightRadiance  = glm::vec3(0, 0, 0);


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

global_variable glm::vec3 AlbedoColor = { 0.972f, 0.96f, 0.915f }; // Silver, from https://docs.unrealengine.com/en-us/Engine/Rendering/Materials/PhysicallyBased

global_variable u32 AlbedoTexture = 0;
global_variable u32 NormalTexture = 0;
global_variable u32 MetalnessTexture = 0;
global_variable u32 RoughnessTexture = 0;
global_variable u32 EnvIrradianceTexture = 0;
global_variable u32 EnvRadianceTexture = 0;
global_variable u32 BRDFLUTTexture = 0;

global_variable r32 Metalness = 1.0f;
global_variable r32 Roughness = 0.2f;
global_variable r32 EnvMapRotation = 0;

global_variable b32 RadiancePrefilterToggle = 0;
global_variable b32 AlbedoTexToggle = 1;
global_variable b32 NormalTexToggle = 1;
global_variable b32 MetalnessTexToggle = 1;
global_variable b32 RoughnessTexToggle = 1;






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

struct color
{
    r32 r;
    r32 g;
    r32 b;
    r32 a;
};
//~ color

struct light
{
    glm::vec3 Direction = { 0.0f, 0.0f, 0.0f };
    glm::vec3 Radiance = { 0.0f, 0.0f, 0.0f };
    
    float Multiplier = 1.0f;
    color LightColor;
    float Luminosity;
};

//~ Light

struct shader_opengl_struct_uniform //represents a struct uniform in opengl
{
    u32 StructLocations[20];
    u32 StructLocationsCount;
};


enum shader_type
{
    //special shaders
    ShaderType_NONE = 0,
    ShaderType_ColorProgram,
    ShaderType_SkyboxProgram,
    ShaderType_EditorFloorGridProgram,
    ShaderType_TextProgram,
    
    //mesh
    ShaderType_ColorShadingProgram,
    ShaderType_BasicProgram,
    ShaderType_PBRProgram,
    ShaderType_SimpleShadingProgram,
};

enum shader_uniform_type
{
    //primitives
    ShaderUniformType_Bool,
    ShaderUniformType_Int,
    ShaderUniformType_UInt,
    ShaderUniformType_Float,
    ShaderUniformType_Double,
    
    //vec
    ShaderUniformType_Vec2,
    ShaderUniformType_Vec3,
    ShaderUniformType_Vec4,
    
    //mat
    ShaderUniformType_Mat2,
    ShaderUniformType_Mat3,
    ShaderUniformType_Mat4,
    
    //samplers
    ShaderUniformType_Sampler,
    ShaderUniformType_Sampler2D,
    ShaderUniformType_SamplerCube,
    
    
};
struct shader_uniform_value
{
    shader_uniform_type ShaderUniformType;
    
    union
    {
        //primitives
        bool Bool;
        s32 Int;
        u32 UInt;
        r32 Float;
        r64 Double;
        
        //vectors
        glm::vec2 Vec2;
        glm::vec3 Vec3;
        glm::vec4 Vec4;
        
        //mat
        glm::mat2 Mat2;
        glm::mat3 Mat3;
        glm::mat4 Mat4;
        
        //samplers
        //TODO(Gabriel): add samplers
        struct
        {
            u32 *SamplerArray;
            u64 SamplerArrayCount;
        } Samplers;
        
        u32 Sampler2D;
        u32 SamplerCube;
        
    } Value;
};

struct shader_uniform
{
    char* UniformName;
    u64 UniformNameLength;
    
    shader_uniform_value ShaderValue;
};


struct shader
{
    //not opengl specific
    shader_type ShaderType;
    
    struct shader_genus
    {
        char* VertexCode = NULL;
        char* FragmentCode = NULL;
        
        char* ProgramName = NULL;
        u64 ProgramNameCount = 0;
        void (*ShaderArrangeFunction)(shader*, u32) = NULL;
    } ShaderGenus;
    
    
    //opengl specific
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
    u32 ColorProgram;
    u32 SkyboxProgram;
    u32 TextProgram; //in-game Text rendering
    u32 EditorFloorGridProgram;
    
    //mesh
    u32 ColorShadingProgram;
    u32 BasicProgram;
    u32 PBRProgram;
    u32 SimpleShadingProgram;
};

struct shader_data_list
{
    //Special Programs
    shader* ColorProgram = NULL;
    shader* SkyboxProgram = NULL;
    shader* TextProgram = NULL;
    shader* EditorFloorGridProgram = NULL;
    
    //Mesh
    shader* ColorShadingProgram = NULL;
    shader* BasicProgram = NULL;
    shader* PBRProgram = NULL;
    shader* SimpleShadingProgram = NULL;
};

global_variable shader_list GlobalShaderCache = {};
global_variable shader_data_list GlobalShaderDataCache = {};


//~ shaders


//NOTE(Gabriel): materials are only for meshes.
struct material
{
    color ShaderColor; //used for color shading
    
    //Shader
    shader *Shader;
    
    //2D Textures
    u32 Textures2D[10];
    u32 TexturesCount;
    
    //
};

/*
struct material_instance
{
    material *Material;
    string Name;
    
    Buffer VSUniformStorageBuffer;
    Buffer PSUniformStorageBuffer;
};
*/

//~ materials

struct stack_string_64
{
    u64 Length;
    char String[64];
};

enum entity_type
{
    EntityType_GameObject,
    EntityType_Light,
};

struct entity
{
    union
    {
        //GameObject
        light* Light;
    };
    
    stack_string_64 *Name;
    glm::vec3 Position;
    glm::vec3 Rotation;
    glm::vec3 Scale;
    
    entity_type EntityType;
    
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

struct camera
{
    glm::vec3 Position;
    glm::vec3 Direction;
    glm::vec3 Rotation;
    glm::vec3 Forward;
    
    glm::mat4 ProjectionMatrix;
    glm::mat4 ViewMatrix;
    r32 Exposure;
    
    r32 FOV;
    r32 ZNear;
    r32 ZFar;
};

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


global_variable u32 EditorTexture_Cube = 0;


//~ END Editor

//~ Forward declarations

entity* CreateEntity(scene *Scene, char *Name);
entity* CreateEntityPrimitive(scene *Scene, char *Name, primitives Primitive);
entity* CreateEntityFromFile(scene *Scene, char* FilePath);
void SerializeEntityToFile(entity *Entity, char* FilePath);
void DeleteEntity(scene *Scene, entity* Entity);
//scene* CreateScene(const char *SceneName);
