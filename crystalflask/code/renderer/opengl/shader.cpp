

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

/*
0 Position
1 Normal
2 Tangent
3 Binormal
4 TexCoord
*/

internal void
LoadShaders() {
    
    char *HeaderCode =
        R"FOO(
          #version 430 core
          // Header code
          )FOO";
    
    
    char *VertexCode =
        R"FOO(
        
        layout (location = 0) in vec3 Position;
        layout (location = 1) in vec3 Normal;
        layout (location = 2) in vec3 Tangent;
        layout (location = 3) in vec3 Binormal;
        layout (location = 4) in vec2 TexCoord;
        
        out vec2 UV;
        
        uniform mat4 MVP;
        
        void main()
        {
        gl_Position = MVP * vec4(Position, 1.0f);
        UV = TexCoord;
        }
        
          )FOO";
    
    char *FragmentCode =
        R"FOO(
        out vec4 FragColor;
        
        in vec2 UV;
        
        uniform sampler2D Texture;
        
        void main()
        {
        // linearly interpolate between both textures (80% container, 20% awesomeface)
        FragColor = texture(Texture, UV);
        }
          )FOO";
    
    //compile
    GlobalShaderCache.BasicProgram = OpenGLCreateProgram(HeaderCode, VertexCode, FragmentCode, "BasicShader");
    
    //create definitions
    shader* BasicProgramShader = PushStruct(&GlobalResourceArena, shader);
    ZeroMemory(BasicProgramShader, sizeof(shader));
    
    BasicProgramShader->ProgramID = GlobalShaderCache.BasicProgram;
    BasicProgramShader->TextureLocations[0] = glGetUniformLocation(GlobalShaderCache.BasicProgram, "Texture");
    BasicProgramShader->TextureCount = 1;
    
    glUniform1i(BasicProgramShader->TextureLocations[0], 0);
    
    //Store the shader in the cache.
    GlobalShaderDataCache.BasicProgram = BasicProgramShader;
    
    
    
    
    //~
    
    ////https://www.shadertoy.com/view/lt2GDd
    char *SkyboxVertexCode =
        R"FOO(
        layout (location = 0) in vec3 Position;
        layout (location = 4) in vec2 TexCoord;

uniform mat4 u_MVP;

out vec2 v_TexCoord;

void main()
{
	vec4 pos = u_MVP * vec4(Position, 1.0);
	gl_Position = pos;

	v_TexCoord = TexCoord;
}


        
          )FOO";
    
    char *SkyboxFragmentCode =
        R"FOO(

const float pi = 3.1415926536;

layout(location = 0) out vec4 fragColor;

in vec2 v_TexCoord;

//uniform sampler2D u_Texture;
uniform samplerCube u_Texture;
uniform sampler2D u_Texture2;
uniform vec2 u_resolution;
uniform float u_time;

void main()
{
vec2 P = (v_TexCoord.xy) / (u_resolution.xy/720);
	float theta = (1.0 - P.y) * pi;
	float phi   = P.x * pi * 2.0;
	
	// Animate to make this less boring
	phi += u_time;
	
	// Equation from http://graphicscodex.com  [sphry]
	vec3 dir = vec3(sin(theta) * sin(phi), cos(theta), sin(theta) * cos(phi));
	
	fragColor = sqrt(texture(u_Texture, dir));
}
          )FOO";
    
    
    GlobalShaderCache.SkyboxProgram = OpenGLCreateProgram(HeaderCode, SkyboxVertexCode, SkyboxFragmentCode, "SkyboxShader");
    glUseProgram(GlobalShaderCache.SkyboxProgram);
    
    
    //create definitions
    shader* SkyboxProgramShader = PushStruct(&GlobalResourceArena, shader);
    ZeroMemory(SkyboxProgramShader, sizeof(shader));
    
    SkyboxProgramShader->ProgramID = GlobalShaderCache.SkyboxProgram;
    
    SkyboxProgramShader->VertexUniformLocations[0] = glGetUniformLocation(GlobalShaderCache.SkyboxProgram, "u_MVP");
    
    
    
    SkyboxProgramShader->FragmentUniformLocations[0] = glGetUniformLocation(GlobalShaderCache.SkyboxProgram, "u_resolution");
    SkyboxProgramShader->FragmentUniformLocations[1] = glGetUniformLocation(GlobalShaderCache.SkyboxProgram, "u_time");
    
    
    
    SkyboxProgramShader->TextureLocations[0] = glGetUniformLocation(GlobalShaderCache.SkyboxProgram, "u_Texture");
    SkyboxProgramShader->TextureLocations[1] = glGetUniformLocation(GlobalShaderCache.SkyboxProgram, "u_Texture2");
    SkyboxProgramShader->TextureCount = 2;
    
    
    // set texture location to 0
    glUniform1i(SkyboxProgramShader->TextureLocations[0], 0);
    glUniform1i(SkyboxProgramShader->TextureLocations[1], 1);
    
    //Store the shader in the cache.
    GlobalShaderDataCache.SkyboxProgram = SkyboxProgramShader;
    
    
    //~ skybox
    
    char *TextVertexCode =
        R"FOO(
        
        // Input vertex data, different for all executions of this shader.
        layout(location = 0) in vec2 vertexPosition_screenspace;
        layout(location = 1) in vec2 vertexUV;
        
        // Output data ; will be interpolated for each fragment.
        out vec2 UV;
        
        void main(){
        
        // Output position of the vertex, in clip space
        // map [0..800][0..600] to [-1..1][-1..1]
        
        vec2 vertexPosition_homoneneousspace = vertexPosition_screenspace - vec2(400,300); // [0..800][0..600] -> [-400..400][-300..300]
        vertexPosition_homoneneousspace /= vec2(400,300);
        gl_Position =  vec4(vertexPosition_homoneneousspace,0,1);
        
        // UV of the vertex. No special space for this one.
        UV = vertexUV;
        }
        
          )FOO";
    
    char *TextFragmentCode =
        R"FOO(
        out vec4 FragColor;
        
        in vec3 TexCoords;
        
        uniform samplerCube skybox;
        
        void main()
        {
        FragColor = texture(skybox, TexCoords);
        }
        
          )FOO";
    
    
    GlobalShaderCache.TextProgram = OpenGLCreateProgram(HeaderCode, TextVertexCode, TextFragmentCode, "TextProgramShader");
    glUseProgram(GlobalShaderCache.TextProgram);
    
    char *MeshVertexCode =
        R"FOO(
        
        // Input vertex data, different for all executions of this shader.
        layout(location = 0) in vec3 Position;
        layout(location = 1) in vec3 Normal;
        layout(location = 2) in vec3 Tangent;
        layout(location = 3) in vec3 Binormal;
        layout(location = 4) in vec2 TexCoord;
        
        // Output data ; will be interpolated for each fragment.
        out vec2 UV;
        out vec3 Position_worldspace;
        out vec3 EyeDirection_cameraspace;
        out vec3 LightDirection_cameraspace;
        
        out vec3 LightDirection_tangentspace;
        out vec3 EyeDirection_tangentspace;
        
        // Values that stay constant for the whole mesh.
        uniform mat4 MVP;
        uniform mat4 V;
        uniform mat4 M;
        uniform mat3 MV3x3;
        uniform vec3 LightPosition_worldspace;
        
        void main(){
        
        // Output position of the vertex, in clip space : MVP * position
        gl_Position =  MVP * vec4(Position, 1.0f);
        
        // Position of the vertex, in worldspace : M * position
        Position_worldspace = (M * vec4(Position, 1.0)).xyz;
        
        // Vector that goes from the vertex to the camera, in camera space.
        // In camera space, the camera is at the origin (0,0,0).
        vec3 vertexPosition_cameraspace = ( V * M * vec4(Position, 1.0f)).xyz;
        EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;
        
        // Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
        vec3 LightPosition_cameraspace = ( V * vec4(LightPosition_worldspace,1)).xyz;
        LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;
        
        // UV of the vertex. No special space for this one.
        UV = TexCoord;
        
        // model to camera = ModelView
        vec3 vertexTangent_cameraspace = MV3x3 * Tangent;
        vec3 vertexBitangent_cameraspace = MV3x3 * Binormal;
        vec3 vertexNormal_cameraspace = MV3x3 * Normal;
        
        mat3 TBN = transpose(mat3(
        vertexTangent_cameraspace,
        vertexBitangent_cameraspace,
        vertexNormal_cameraspace
        )); // You can use dot products instead of building this matrix and transposing it. See References for details.
        
        LightDirection_tangentspace = TBN * LightDirection_cameraspace;
        EyeDirection_tangentspace =  TBN * EyeDirection_cameraspace;
        
        }
        
        
          )FOO";
    
    char *MeshFragmentCode =
        R"FOO(
        
        // Interpolated values from the vertex shaders
        in vec2 UV;
        in vec3 Position_worldspace;
        in vec3 EyeDirection_cameraspace;
        in vec3 LightDirection_cameraspace;
        
        in vec3 LightDirection_tangentspace;
        in vec3 EyeDirection_tangentspace;
        
        // Ouput data
        out vec3 color;
        
        // Values that stay constant for the whole mesh.
        uniform sampler2D DiffuseTextureSampler;
        uniform sampler2D NormalTextureSampler;
        uniform sampler2D SpecularTextureSampler;
        uniform mat4 V;
        uniform mat4 M;
        uniform mat3 MV3x3;
        uniform vec3 LightPosition_worldspace;
        
        void main(){
        
        // Light emission properties
        // You probably want to put them as uniforms
        vec3 LightColor = vec3(1,1,1);
        float LightPower = 40.0;
        
        // Material properties
        vec3 MaterialDiffuseColor = texture( DiffuseTextureSampler, UV ).rgb;
        vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
        vec3 MaterialSpecularColor = texture( SpecularTextureSampler, UV ).rgb * 0.3;
        
        // Local normal, in tangent space. V tex coordinate is inverted because normal map is in TGA (not in DDS) for better quality
        vec3 TextureNormal_tangentspace = normalize(texture( NormalTextureSampler, vec2(UV.x,-UV.y) ).rgb*2.0 - 1.0);
        
        // Distance to the light
        float distance = length( LightPosition_worldspace - Position_worldspace );
        
        // Normal of the computed fragment, in camera space
        vec3 n = TextureNormal_tangentspace;
        // Direction of the light (from the fragment to the light)
        vec3 l = normalize(LightDirection_tangentspace);
        // Cosine of the angle between the normal and the light direction,
        // clamped above 0
        //  - light is at the vertical of the triangle -> 1
        //  - light is perpendicular to the triangle -> 0
        //  - light is behind the triangle -> 0
        float cosTheta = clamp( dot( n,l ), 0,1 );
        
        // Eye vector (towards the camera)
        vec3 E = normalize(EyeDirection_tangentspace);
        // Direction in which the triangle reflects the light
        vec3 R = reflect(-l,n);
        // Cosine of the angle between the Eye vector and the Reflect vector,
        // clamped to 0
        //  - Looking into the reflection -> 1
        //  - Looking elsewhere -> < 1
        float cosAlpha = clamp( dot( E,R ), 0,1 );
        
        color =
        // Ambient : simulates indirect lighting
        MaterialAmbientColor +
        // Diffuse : "color" of the object
        MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
        // Specular : reflective highlight, like a mirror
        MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);
        
        }
        
        )FOO";
    
    
    //GlobalShaderCache.MeshProgram = OpenGLCreateProgram(HeaderCode, MeshVertexCode, MeshFragmentCode);
    //glUseProgram(GlobalShaderCache.MeshProgram);
    
    
    
    char *PBRVertexCode =
        R"FOO(

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

layout(location = 5) in ivec4 a_BoneIndices;
layout(location = 6) in vec4 a_BoneWeights;

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_ModelMatrix;

const int MAX_BONES = 100;
uniform mat4 u_BoneTransforms[100];

out VertexOutput
{
	vec3 WorldPosition;
    vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	vec3 Binormal;
} vs_Output;

void main()
{

	mat4 boneTransform = u_BoneTransforms[a_BoneIndices[0]] * a_BoneWeights[0];
	boneTransform += u_BoneTransforms[a_BoneIndices[1]] * a_BoneWeights[1];
	boneTransform += u_BoneTransforms[a_BoneIndices[2]] * a_BoneWeights[2];
	boneTransform += u_BoneTransforms[a_BoneIndices[3]] * a_BoneWeights[3];

	vec4 localPosition = boneTransform * vec4(a_Position, 1.0);
	
	vs_Output.WorldPosition = vec3(u_ModelMatrix * boneTransform * vec4(a_Position, 1.0));
	vs_Output.Normal = mat3(boneTransform) * a_Normal;
	vs_Output.TexCoord = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);
	vs_Output.WorldNormals = mat3(u_ModelMatrix) * mat3(a_Tangent, a_Binormal, a_Normal);
	vs_Output.Binormal = mat3(boneTransform) * a_Binormal;

	//gl_Position = u_ViewProjectionMatrix * u_ModelMatrix * vec4(a_Position, 1.0);
gl_Position = u_ViewProjectionMatrix * u_ModelMatrix * localPosition;
}
        
          )FOO";
    
    char *PBRFragmentCode =
        R"FOO(

const float PI = 3.141592;
const float Epsilon = 0.00001;

const int LightCount = 1;

// Constant normal incidence Fresnel factor for all dielectrics.
const vec3 Fdielectric = vec3(0.04);

struct Light {
	vec3 Direction;
	vec3 Radiance;
};

in VertexOutput
{
	vec3 WorldPosition;
    vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	vec3 Binormal;
} vs_Input;

layout(location=0) out vec4 color;

uniform Light lights;
uniform vec3 u_CameraPosition;

// PBR texture inputs
uniform sampler2D u_AlbedoTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_MetalnessTexture;
uniform sampler2D u_RoughnessTexture;

// Environment maps
uniform samplerCube u_EnvIrradianceTex;
uniform samplerCube u_EnvRadianceTex;
//uniform samplerCube u_EnvSpecularTex;

// BRDF LUT
uniform sampler2D u_BRDFLUTTexture;

uniform vec3 u_AlbedoColor;
uniform float u_Metalness;
uniform float u_Roughness;

uniform float u_EnvMapRotation;

// Toggles
uniform float u_RadiancePrefilter;
uniform float u_AlbedoTexToggle;
uniform float u_NormalTexToggle;
uniform float u_MetalnessTexToggle;
uniform float u_RoughnessTexToggle;

struct PBRParameters
{
	vec3 Albedo;
	float Roughness;
	float Metalness;

	vec3 Normal;
	vec3 View;
	float NdotV;
};

PBRParameters m_Params;

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2
float ndfGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float NdotV, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(cosLi, k) * gaSchlickG1(NdotV, k);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Shlick's approximation of the Fresnel factor.
vec3 fresnelSchlick(vec3 F0, float cosTheta)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(vec3 F0, float cosTheta, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// ---------------------------------------------------------------------------------------------------
// The following code (from Unreal Engine 4's paper) shows how to filter the environment map
// for different roughnesses. This is mean to be computed offline and stored in cube map mips,
// so turning this on online will cause poor performance
float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

vec3 ImportanceSampleGGX(vec2 Xi, float Roughness, vec3 N)
{
	float a = Roughness * Roughness;
	float Phi = 2 * PI * Xi.x;
	float CosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ) );
	float SinTheta = sqrt( 1 - CosTheta * CosTheta );
	vec3 H;
	H.x = SinTheta * cos( Phi );
	H.y = SinTheta * sin( Phi );
	H.z = CosTheta;
	vec3 UpVector = abs(N.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);
	vec3 TangentX = normalize( cross( UpVector, N ) );
	vec3 TangentY = cross( N, TangentX );
	// Tangent to world space
	return TangentX * H.x + TangentY * H.y + N * H.z;
}

float TotalWeight = 0.0;

vec3 PrefilterEnvMap(float Roughness, vec3 R)
{
	vec3 N = R;
	vec3 V = R;
	vec3 PrefilteredColor = vec3(0.0);
	int NumSamples = 1024;
	for(int i = 0; i < NumSamples; i++)
	{
		vec2 Xi = Hammersley(i, NumSamples);
		vec3 H = ImportanceSampleGGX(Xi, Roughness, N);
		vec3 L = 2 * dot(V, H) * H - V;
		float NoL = clamp(dot(N, L), 0.0, 1.0);
		if (NoL > 0)
		{
			PrefilteredColor += texture(u_EnvRadianceTex, L).rgb * NoL;
			TotalWeight += NoL;
		}
	}
	return PrefilteredColor / TotalWeight;
}

// ---------------------------------------------------------------------------------------------------

vec3 RotateVectorAboutY(float angle, vec3 vec)
{
    angle = radians(angle);
    mat3x3 rotationMatrix ={vec3(cos(angle),0.0,sin(angle)),
                            vec3(0.0,1.0,0.0),
                            vec3(-sin(angle),0.0,cos(angle))};
    return rotationMatrix * vec;
}

vec3 Lighting(vec3 F0)
{
	vec3 result = vec3(0.0);
	for(int i = 0; i < LightCount; i++)
	{
		vec3 Li = -lights.Direction;
		vec3 Lradiance = lights.Radiance;
		vec3 Lh = normalize(Li + m_Params.View);

		// Calculate angles between surface normal and various light vectors.
		float cosLi = max(0.0, dot(m_Params.Normal, Li));
		float cosLh = max(0.0, dot(m_Params.Normal, Lh));

		vec3 F = fresnelSchlick(F0, max(0.0, dot(Lh, m_Params.View)));
		float D = ndfGGX(cosLh, m_Params.Roughness);
		float G = gaSchlickGGX(cosLi, m_Params.NdotV, m_Params.Roughness);

		vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
		vec3 diffuseBRDF = kd * m_Params.Albedo;

		// Cook-Torrance
		vec3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * cosLi * m_Params.NdotV);

		result += (diffuseBRDF + specularBRDF) * Lradiance * cosLi;
	}
	return result;
}

vec3 IBL(vec3 F0, vec3 Lr)
{
	vec3 irradiance = texture(u_EnvIrradianceTex, m_Params.Normal).rgb;
	vec3 F = fresnelSchlickRoughness(F0, m_Params.NdotV, m_Params.Roughness);
	vec3 kd = (1.0 - F) * (1.0 - m_Params.Metalness);
	vec3 diffuseIBL = m_Params.Albedo * irradiance;

	int u_EnvRadianceTexLevels = textureQueryLevels(u_EnvRadianceTex);
	float NoV = clamp(m_Params.NdotV, 0.0, 1.0);
	vec3 R = 2.0 * dot(m_Params.View, m_Params.Normal) * m_Params.Normal - m_Params.View;
	vec3 specularIrradiance = vec3(0.0);

	if (u_RadiancePrefilter > 0.5)
		specularIrradiance = PrefilterEnvMap(m_Params.Roughness * m_Params.Roughness, R) * u_RadiancePrefilter;
	else
		specularIrradiance = textureLod(u_EnvRadianceTex, RotateVectorAboutY(u_EnvMapRotation, Lr), sqrt(m_Params.Roughness) * u_EnvRadianceTexLevels).rgb * (1.0 - u_RadiancePrefilter);

	// Sample BRDF Lut, 1.0 - roughness for y-coord because texture was generated (in Sparky) for gloss model
	vec2 specularBRDF = texture(u_BRDFLUTTexture, vec2(m_Params.NdotV, 1.0 - m_Params.Roughness)).rg;
	vec3 specularIBL = specularIrradiance * (F * specularBRDF.x + specularBRDF.y);

	return kd * diffuseIBL + specularIBL;
}

void main()
{
	// Standard PBR inputs
	m_Params.Albedo = u_AlbedoTexToggle > 0.5 ? texture(u_AlbedoTexture, vs_Input.TexCoord).rgb : u_AlbedoColor;
	m_Params.Metalness = u_MetalnessTexToggle > 0.5 ? texture(u_MetalnessTexture, vs_Input.TexCoord).r : u_Metalness;
	m_Params.Roughness = u_RoughnessTexToggle > 0.5 ?  texture(u_RoughnessTexture, vs_Input.TexCoord).r : u_Roughness;
    m_Params.Roughness = max(m_Params.Roughness, 0.05); // Minimum roughness of 0.05 to keep specular highlight

	// Normals (either from vertex or map)
	m_Params.Normal = normalize(vs_Input.Normal);
	if (u_NormalTexToggle > 0.5)
	{
		m_Params.Normal = normalize(2.0 * texture(u_NormalTexture, vs_Input.TexCoord).rgb - 1.0);
		m_Params.Normal = normalize(vs_Input.WorldNormals * m_Params.Normal);
	}

	m_Params.View = normalize(u_CameraPosition - vs_Input.WorldPosition);
	m_Params.NdotV = max(dot(m_Params.Normal, m_Params.View), 0.0);

	// Specular reflection vector
	vec3 Lr = 2.0 * m_Params.NdotV * m_Params.Normal - m_Params.View;

	// Fresnel reflectance, metals use albedo
	vec3 F0 = mix(Fdielectric, m_Params.Albedo, m_Params.Metalness);

	vec3 lightContribution = Lighting(F0);
	vec3 iblContribution = IBL(F0, Lr);

	color = vec4(lightContribution + iblContribution, 1.0);
}


        )FOO";
    
    
    GlobalShaderCache.PBRProgram = OpenGLCreateProgram(HeaderCode, PBRVertexCode, PBRFragmentCode, "PBRShader");
    glUseProgram(GlobalShaderCache.PBRProgram);
    
    
    //create definitions
    shader* PBRProgramShader = PushStruct(&GlobalResourceArena, shader);
    ZeroMemory(PBRProgramShader, sizeof(shader));
    PBRProgramShader->ProgramID = GlobalShaderCache.PBRProgram;
    
    
    
    //vertex uniforms
    //u_ViewProjectionMatrix;
    //u_ModelMatrix;
    PBRProgramShader->VertexUniformLocations[0] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_ViewProjectionMatrix");
    PBRProgramShader->VertexUniformLocations[1] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_ModelMatrix");
    
    
    //struct uniforms
    PBRProgramShader->StructUniform[0].StructLocations[0] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "lights.Direction");
    PBRProgramShader->StructUniform[0].StructLocations[1] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "lights.Radiance");
    PBRProgramShader->StructUniform[0].StructLocationsCount = 2;
    PBRProgramShader->StructUniformCount = 1;
    
    
    //fragment uniforms
    PBRProgramShader->FragmentUniformLocations[0] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_CameraPosition");
    PBRProgramShader->FragmentUniformLocations[1] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_AlbedoColor");
    PBRProgramShader->FragmentUniformLocations[2] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_Metalness");
    PBRProgramShader->FragmentUniformLocations[3] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_Roughness");
    PBRProgramShader->FragmentUniformLocations[4] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_EnvMapRotation");
    PBRProgramShader->FragmentUniformLocations[5] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_RadiancePrefilter");
    PBRProgramShader->FragmentUniformLocations[6] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_AlbedoTexToggle");
    PBRProgramShader->FragmentUniformLocations[7] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_NormalTexToggle");
    PBRProgramShader->FragmentUniformLocations[8] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_MetalnessTexToggle");
    PBRProgramShader->FragmentUniformLocations[9] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_RoughnessTexToggle");
    
    
    //textures
    PBRProgramShader->TextureLocations[0] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_AlbedoTexture");
    PBRProgramShader->TextureLocations[1] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_NormalTexture");
    PBRProgramShader->TextureLocations[2] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_MetalnessTexture");
    PBRProgramShader->TextureLocations[3] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_RoughnessTexture");
    PBRProgramShader->TextureLocations[4] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_EnvIrradianceTex");
    PBRProgramShader->TextureLocations[5] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_EnvRadianceTex");
    PBRProgramShader->TextureLocations[6] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_BRDFLUTTexture");
    PBRProgramShader->TextureCount = 7;
    
    
    // set texture location to 0
    glUniform1i(SkyboxProgramShader->TextureLocations[0], 0);
    glUniform1i(SkyboxProgramShader->TextureLocations[1], 1);
    glUniform1i(SkyboxProgramShader->TextureLocations[2], 2);
    glUniform1i(SkyboxProgramShader->TextureLocations[3], 3);
    glUniform1i(SkyboxProgramShader->TextureLocations[4], 4);
    glUniform1i(SkyboxProgramShader->TextureLocations[5], 5);
    glUniform1i(SkyboxProgramShader->TextureLocations[6], 6);
    
    //Store the shader in the cache.
    GlobalShaderDataCache.PBRProgram = PBRProgramShader;
    
    //~PBR shader
    
    
    
    char *EditorGridFloorVertexCode =
        R"FOO(
        
layout(location = 0) in vec3 a_Position;
layout(location = 4) in vec2 a_TexCoord;

uniform mat4 u_MVP;

out vec2 v_TexCoord;

void main()
{
	vec4 position = u_MVP * vec4(a_Position, 1.0);
	gl_Position = position;

	v_TexCoord = a_TexCoord;
}

        
          )FOO";
    
    char *EditorGridFloorFragmentCode =
        R"FOO(
        
layout(location = 0) out vec4 color;

uniform float u_Scale;

in vec2 v_TexCoord;


void main()
{
	vec2 uv = (v_TexCoord.xy * 2.0) / vec2(2);
    uv *= u_Scale;	// Scale
    
    // Generate checker pattern
    float c = mod(floor(uv.x) + floor(uv.y), 2.);

    color.rgb = vec3(c*0.15+0.6);
	color.a = 1.0;
}

        )FOO";
    
    
    GlobalShaderCache.EditorFloorGridProgram = OpenGLCreateProgram(HeaderCode, EditorGridFloorVertexCode, EditorGridFloorFragmentCode, "EditorGridFloorShader");
    glUseProgram(GlobalShaderCache.EditorFloorGridProgram);
    
    //create definitions
    shader* EditorFloorGridShader = PushStruct(&GlobalResourceArena, shader);
    ZeroMemory(EditorFloorGridShader, sizeof(shader));
    
    EditorFloorGridShader->ProgramID = GlobalShaderCache.EditorFloorGridProgram;
    
    EditorFloorGridShader->VertexUniformLocations[0] = glGetUniformLocation(GlobalShaderCache.EditorFloorGridProgram, "u_MVP");
    EditorFloorGridShader->VertexUniformCount = 1;
    
    EditorFloorGridShader->FragmentUniformLocations[0] = glGetUniformLocation(GlobalShaderCache.EditorFloorGridProgram, "u_Scale");
    EditorFloorGridShader->FragmentUniformCount = 1;
    
    //Store the shader in the cache.
    GlobalShaderDataCache.EditorFloorGridProgram = EditorFloorGridShader;
    
    
    
}

//BasicProgram
internal void
ShaderBasicSubmit(glm::mat4 &MVP)
{
    // TODO(Gabriel): store MVP uniform location in the shader instead of querying opengl.
    
    //NOTE(Gabriel): Submit just the MVP instead of model,view and projection that way we get double precision from calculating the MVP on the CPU and more efficiency when we have lots of vertices in a model.
    glUniformMatrix4fv(glGetUniformLocation(GlobalShaderCache.BasicProgram, "MVP"), 1, GL_FALSE, &MVP[0][0]);
}


//BasicProgram
internal void
ShaderSkyboxSubmit(glm::mat4 &MVP, r32 DeltaTime)
{
    glUniformMatrix4fv(GlobalShaderDataCache.SkyboxProgram->VertexUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
    
    glm::vec2 Resolution = {GlobalWidth, GlobalHeight};
    glUniform2fv(GlobalShaderDataCache.SkyboxProgram->FragmentUniformLocations[0], 1, &Resolution[0]);
    
    
    glUniform1f(GlobalShaderDataCache.SkyboxProgram->FragmentUniformLocations[1], DeltaTime);
}


//PBRProgram
internal void
ShaderPBRSubmit(glm::mat4 &ViewProjectionMatrix, glm::mat4 &ModelMatrix)
{
    
    //vetex uniforms
    glUniformMatrix4fv(GlobalShaderDataCache.SkyboxProgram->VertexUniformLocations[0], 1, GL_FALSE, &ViewProjectionMatrix[0][0]);
    glUniformMatrix4fv(GlobalShaderDataCache.SkyboxProgram->VertexUniformLocations[0], 1, GL_FALSE, &ModelMatrix[0][0]);
    
    
    //fragment struct uniforms
    glUniform3fv(GlobalShaderDataCache.PBRProgram->StructUniform[0].StructLocations[0], 1, &LightDirection[0]);
    glUniform3fv(GlobalShaderDataCache.PBRProgram->StructUniform[0].StructLocations[1], 1, &LightRadiance[0]);
    
    //fragment uniforms
    
    glUniform3fv(GlobalShaderDataCache.PBRProgram->FragmentUniformLocations[0], 1, &CameraPosition[0]);
    
    glUniform3fv(GlobalShaderDataCache.PBRProgram->FragmentUniformLocations[1], 1, &AlbedoColor[0]);
    
    glUniform1f(GlobalShaderDataCache.PBRProgram->FragmentUniformLocations[2], Metalness);
    glUniform1f(GlobalShaderDataCache.PBRProgram->FragmentUniformLocations[3], Roughness);
    glUniform1f(GlobalShaderDataCache.PBRProgram->FragmentUniformLocations[4], EnvMapRotation);
    
    glUniform1f(GlobalShaderDataCache.PBRProgram->FragmentUniformLocations[5], (float)RadiancePrefilterToggle);
    glUniform1f(GlobalShaderDataCache.PBRProgram->FragmentUniformLocations[6], (float)AlbedoTexToggle);
    glUniform1f(GlobalShaderDataCache.PBRProgram->FragmentUniformLocations[7], (float)NormalTexToggle);
    glUniform1f(GlobalShaderDataCache.PBRProgram->FragmentUniformLocations[8], (float)MetalnessTexToggle);
    glUniform1f(GlobalShaderDataCache.PBRProgram->FragmentUniformLocations[9], (float)RoughnessTexToggle);
    
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, AlbedoTexture);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, NormalTexture);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, MetalnessTexture);
    
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, RoughnessTexture);
    
    
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_CUBE_MAP, EnvIrradianceTexture);
    
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP, EnvRadianceTexture);
    
    //textures
    /*
    GlobalShaderDataCache.PBRProgram->TextureLocations[0]
        
        = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_AlbedoTexture");
    PBRProgramShader->TextureLocations[1] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_NormalTexture");
    PBRProgramShader->TextureLocations[2] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_MetalnessTexture");
    PBRProgramShader->TextureLocations[3] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_RoughnessTexture");
    PBRProgramShader->TextureLocations[4] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_EnvIrradianceTex");
    PBRProgramShader->TextureLocations[5] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_EnvRadianceTex");
    PBRProgramShader->TextureLocations[6] = glGetUniformLocation(GlobalShaderCache.PBRProgram, "u_BRDFLUTTexture");
    PBRProgramShader->TextureCount = 7;
    */
    
}


//EditorFloorGridProgram
internal void
ShaderEditorFloorGridSubmit(glm::mat4 &MVP, float Scale)
{
    glUniform1f(GlobalShaderDataCache.EditorFloorGridProgram->FragmentUniformLocations[0], Scale);
    
    glUniformMatrix4fv(GlobalShaderDataCache.EditorFloorGridProgram->VertexUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
    
}

