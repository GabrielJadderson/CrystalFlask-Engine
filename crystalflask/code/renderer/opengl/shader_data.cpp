/*
0 Position
1 Normal
2 Tangent
3 Binormal
4 TexCoord
*/

char *ShaderHeaderCode =
R"FOO(
  #version 430 core
  // Header code
  )FOO";

char *ShaderColorVertexCode =
R"FOO(
      
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Tangent;
layout (location = 3) in vec3 a_Binormal;
layout (location = 4) in vec2 a_TexCoord;

void main(){
 gl_position = a_Position;
 }

  )FOO";


char *ShaderColorFragmentCode =
R"FOO(
      
out vec3 color;
void main(){
  color = vec3(1,0,0);
}

  )FOO";

internal void
ShaderArrangeColorProgram(shader* Shader, u32 ShaderId)
{
}

//~


char *BasicShaderVertexCode =
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

char *BasicShaderFragmentCode =
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


internal void
ShaderArrangeBasicProgram(shader* Shader, u32 ShaderId)
{
    Shader->TextureLocations[0] = glGetUniformLocation(ShaderId, "Texture");
    Shader->TextureCount = 1;
    
    glUniform1i(Shader->TextureLocations[0], 0);
}

internal void
ShaderBasicUpload(glm::mat4 &MVP)
{
    // TODO(Gabriel): store MVP uniform location in the shader instead of querying opengl.
    
    //NOTE(Gabriel): Submit just the MVP instead of model,view and projection that way we get double precision from calculating the MVP on the CPU and more efficiency when we have lots of vertices in a model.
    glUniformMatrix4fv(glGetUniformLocation(GlobalShaderCache.BasicProgram, "MVP"), 1, GL_FALSE, &MVP[0][0]);
}


//~ basic shader
char *PBRVertexCode =
R"FOO(

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec3 a_Binormal;
layout(location = 4) in vec2 a_TexCoord;

uniform mat4 u_ViewProjectionMatrix;
uniform mat4 u_Transform;

out VertexOutput
{
vec3 WorldPosition;
vec3 Normal;
vec2 TexCoord;
mat3 WorldNormals;
mat3 WorldTransform;
vec3 Binormal;
} vs_Output;

void main()
{
vs_Output.WorldPosition = vec3(u_Transform * vec4(a_Position, 1.0));
vs_Output.Normal = mat3(u_Transform) * a_Normal;
vs_Output.TexCoord = vec2(a_TexCoord.x, 1.0 - a_TexCoord.y);
vs_Output.WorldNormals = mat3(u_Transform) * mat3(a_Tangent, a_Binormal, a_Normal);
vs_Output.WorldTransform = mat3(u_Transform);
vs_Output.Binormal = a_Binormal;

gl_Position = u_ViewProjectionMatrix * u_Transform * vec4(a_Position, 1.0);
}


  )FOO";

char *PBRFragmentCode2 =
R"FOO(

in VertexOutput
{
vec3 WorldPosition;
vec3 Normal;
vec2 TexCoord;
mat3 WorldNormals;
mat3 WorldTransform;
vec3 Binormal;
} vs_Output;

layout(location = 0) out vec4 color;

void main()
{
color = vec4(1.0, 0.0, 1.0, 1.0);
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
	float Multiplier;
};

in VertexOutput
{
	vec3 WorldPosition;
    vec3 Normal;
	vec2 TexCoord;
	mat3 WorldNormals;
	mat3 WorldTransform;
	vec3 Binormal;
} vs_Input;

layout(location = 0) out vec4 color;

uniform Light lights;
uniform vec3 u_CameraPosition;

// PBR texture inputs
uniform sampler2D u_AlbedoTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_MetalnessTexture;
uniform sampler2D u_RoughnessTexture;

// Environment maps
uniform samplerCube u_EnvRadianceTex;
uniform samplerCube u_EnvIrradianceTex;

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
		vec3 Lradiance = lights.Radiance * lights.Multiplier;
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
	vec3 specularIrradiance = textureLod(u_EnvRadianceTex, RotateVectorAboutY(u_EnvMapRotation, Lr), (m_Params.Roughness) * u_EnvRadianceTexLevels).rgb;

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


internal void
ShaderArrangePBR(shader* Shader, u32 ShaderId)
{
    Shader->VertexUniformLocations[0] = glGetUniformLocation(ShaderId,
                                                             "u_ViewProjectionMatrix");
    Shader->VertexUniformLocations[1] = glGetUniformLocation(ShaderId,
                                                             "u_Transform");
    Shader->VertexUniformCount = 2;
    
    //structs
    Shader->StructUniform[0].StructLocations[0] = glGetUniformLocation(ShaderId, "lights.Direction");
    Shader->StructUniform[0].StructLocations[1] = glGetUniformLocation(ShaderId, "lights.Radiance");
    Shader->StructUniform[0].StructLocations[2] = glGetUniformLocation(ShaderId, "lights.Multiplier");
    Shader->StructUniform[0].StructLocationsCount = 3;
    Shader->StructUniformCount = 1;
    
    //fragment uniforms
    Shader->FragmentUniformLocations[0] = glGetUniformLocation(ShaderId, "lights");
    Shader->FragmentUniformLocations[1] = glGetUniformLocation(ShaderId, "u_CameraPosition");
    
    Shader->FragmentUniformLocations[2] = glGetUniformLocation(ShaderId, "u_AlbedoColor");
    Shader->FragmentUniformLocations[3] = glGetUniformLocation(ShaderId, "u_Metalness");
    Shader->FragmentUniformLocations[4] = glGetUniformLocation(ShaderId, "u_Roughness");
    Shader->FragmentUniformLocations[5] = glGetUniformLocation(ShaderId, "u_EnvMapRotation");
    
    Shader->FragmentUniformLocations[6] = glGetUniformLocation(ShaderId, "u_RadiancePrefilter");
    Shader->FragmentUniformLocations[7] = glGetUniformLocation(ShaderId, "u_AlbedoTexToggle");
    Shader->FragmentUniformLocations[8] = glGetUniformLocation(ShaderId, "u_NormalTexToggle");
    Shader->FragmentUniformLocations[9] = glGetUniformLocation(ShaderId, "u_MetalnessTexToggle");
    Shader->FragmentUniformLocations[10] = glGetUniformLocation(ShaderId, "u_RoughnessTexToggle");
    Shader->FragmentUniformCount = 11;
    
    //textures
    Shader->TextureLocations[0] = glGetUniformLocation(ShaderId, "u_AlbedoTexture");
    Shader->TextureLocations[1] = glGetUniformLocation(ShaderId, "u_NormalTexture");
    Shader->TextureLocations[2] = glGetUniformLocation(ShaderId, "u_MetalnessTexture");
    Shader->TextureLocations[3] = glGetUniformLocation(ShaderId, "u_RoughnessTexture");
    Shader->TextureLocations[4] = glGetUniformLocation(ShaderId, "u_EnvRadianceTex");
    Shader->TextureLocations[5] = glGetUniformLocation(ShaderId, "u_EnvIrradianceTex");
    Shader->TextureLocations[6] = glGetUniformLocation(ShaderId, "u_BRDFLUTTexture");
    Shader->TextureCount = 7;
}

internal void
ShaderPBRUpload(glm::mat4 &MVP, glm::mat4 &ViewProjectionMatrix, glm::mat4 &ModelMatrix)
{
    shader* Shader = GlobalShaderDataCache.PBRProgram;
    
    //vetex uniforms
    glUniformMatrix4fv(Shader->VertexUniformLocations[0], 1, GL_FALSE, &ViewProjectionMatrix[0][0]);
    glUniformMatrix4fv(Shader->VertexUniformLocations[1], 1, GL_FALSE, &ModelMatrix[0][0]);
    
    //structs
    glUniform3fv(Shader->StructUniform[0].StructLocations[0], 1, &LightDirection[0]);
    glUniform3fv(Shader->StructUniform[0].StructLocations[1], 1, &LightRadiance[0]);
    glUniform1f(Shader->StructUniform[0].StructLocations[2], LightMultiplier);
    
    //fragments
    glUniform3fv(Shader->FragmentUniformLocations[1], 1, &CameraPosition[0]);
    glUniform3fv(Shader->FragmentUniformLocations[2], 1, &AlbedoColor[0]);
    
    
    
    glUniform1f(Shader->FragmentUniformLocations[3], Metalness);
    glUniform1f(Shader->FragmentUniformLocations[4], Roughness);
    glUniform1f(Shader->FragmentUniformLocations[5], EnvMapRotation);
    
    glUniform1f(Shader->FragmentUniformLocations[6], (float)RadiancePrefilterToggle);
    glUniform1f(Shader->FragmentUniformLocations[7], (float)AlbedoTexToggle);
    glUniform1f(Shader->FragmentUniformLocations[8], (float)NormalTexToggle);
    glUniform1f(Shader->FragmentUniformLocations[9], (float)MetalnessTexToggle);
    glUniform1f(Shader->FragmentUniformLocations[10], (float)RoughnessTexToggle);
    
    
}



//~ PBR shader


char *SkyboxVertexCode =
R"FOO(
layout (location = 0) in vec3 Position;
layout (location = 4) in vec2 TexCoord;

uniform mat4 u_MVP;

out vec3 v_TexCoord;

void main()
{
gl_Position =  u_MVP * vec4(Position, 1.0);
v_TexCoord = Position;
}



  )FOO";

char *SkyboxFragmentCode =
R"FOO(

layout(location = 0) out vec4 fragColor;

in vec3 v_TexCoord;

uniform samplerCube u_Texture;
uniform vec2 u_resolution;

void main()
{

fragColor = texture(u_Texture, v_TexCoord);
}
  )FOO";

internal void
ShaderArrangeSkybox(shader* Shader, u32 ShaderId)
{
    Shader->VertexUniformLocations[0] = glGetUniformLocation(ShaderId, "u_MVP");
    Shader->VertexUniformCount = 1;
    
    Shader->FragmentUniformLocations[0] = glGetUniformLocation(ShaderId, "u_resolution");
    Shader->FragmentUniformLocations[1] = glGetUniformLocation(ShaderId, "u_time");
    Shader->FragmentUniformCount = 2;
    
    Shader->TextureLocations[0] = glGetUniformLocation(ShaderId, "u_Texture");
    Shader->TextureLocations[1] = glGetUniformLocation(ShaderId, "u_Texture2");
    Shader->TextureCount = 2;
    
    // set texture location to 0
    glUniform1i(Shader->TextureLocations[0], 0);
    glUniform1i(Shader->TextureLocations[1], 1);
}

internal void
ShaderSkyboxUpload(glm::mat4 &MVP)
{
    glUniformMatrix4fv(GlobalShaderDataCache.SkyboxProgram->VertexUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
    
    glm::vec2 Resolution = {GlobalWidth, GlobalHeight};
    glUniform2fv(GlobalShaderDataCache.SkyboxProgram->FragmentUniformLocations[0], 1, &Resolution[0]);
    
}


//~ Skybox Program


char *ColorShaderVertexCode =
R"FOO(
layout (location = 0) in vec3 Position;

uniform mat4 u_MVP;

void main()
{
gl_Position = u_MVP * vec4(Position, 1.0f);
}

  )FOO";

char *ColorShaderFragmentCode =
R"FOO(

uniform vec4 u_color;

out vec4 FragColor;

void main()
{
FragColor = u_color;
}
  )FOO";

internal void
ShaderArrangeColorShadingProgram(shader* Shader, u32 ShaderId)
{
    Shader->VertexUniformLocations[0] = glGetUniformLocation(ShaderId, "u_MVP");
    Shader->FragmentUniformLocations[0] = glGetUniformLocation(ShaderId, "u_color");
}


internal void
ShaderColorShadingUpload(glm::mat4 &MVP, color &Color)
{
    glUniformMatrix4fv(GlobalShaderDataCache.SkyboxProgram->VertexUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
    glUniform4f(GlobalShaderDataCache.EditorFloorGridProgram->FragmentUniformLocations[0],
                Color.r, Color.g, Color.b,Color.a);
}


//~ color Shading Program


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
//UV = vertexUV;
UV = vec2(vertexUV.x, 1.0 - vertexUV.y);

}

  )FOO";

char *TextFragmentCode =
R"FOO(
out vec4 FragColor;

in vec2 UV;

    uniform sampler2D u_Texture;
  
void main()
{
     FragColor= texture(u_Texture, UV);
}

  )FOO";

internal void
ShaderArrangeTextProgram(shader* Shader, u32 ShaderId)
{
    Shader->TextureLocations[0] = glGetUniformLocation(ShaderId, "u_Texture");
    Shader->TextureCount = 1;
    glUniform1i(Shader->TextureLocations[0], 0);
}


internal void
ShaderTextUpload()
{
    
    
}

//~ text shader


char *SimpleShadingVertexCode =
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
out vec3 Normal_cameraspace;
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

void main()
{

// Output position of the vertex, in clip space : MVP * Position
gl_Position =  MVP * vec4(Position, 1);

// Position of the vertex, in worldspace : M * Position
Position_worldspace = (M * vec4(Position,1)).xyz;

// Vector that goes from the vertex to the camera, in camera space.
// In camera space, the camera is at the origin (0,0,0).
vec3 vertexPosition_cameraspace = ( V * M * vec4(Position,1)).xyz;
EyeDirection_cameraspace = vec3(0,0,0) - vertexPosition_cameraspace;

// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
vec3 LightPosition_cameraspace = ( V * vec4(LightPosition_worldspace,1)).xyz;
LightDirection_cameraspace = LightPosition_cameraspace + EyeDirection_cameraspace;

// UV of the vertex. No special space for this one.
UV = vec2(TexCoord.x, 1.0 - TexCoord.y);

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

char *SimpleShadingFragmentCode =
R"FOO(

in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;

in vec3 LightDirection_tangentspace;
in vec3 EyeDirection_tangentspace;

// Ouput data
out vec4 FragColor;

// Values that stay constant for the whole mesh.

uniform sampler2D DiffuseTextureSampler;
uniform sampler2D NormalTextureSampler;
uniform sampler2D SpecularTextureSampler;
uniform mat4 V;
uniform mat4 M;
uniform mat3 MV3x3;
uniform vec3 LightPosition_worldspace;

void main()
{

// Light emission properties
// You probably want to put them as uniforms
vec3 LightColor = vec3(1,1,1);
float LightPower = 50.0f;


// Material properties
vec3 MaterialDiffuseColor = texture(DiffuseTextureSampler, UV).rgb;
vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
vec3 MaterialSpecularColor = texture(SpecularTextureSampler, UV).rgb * 0.3;

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

vec3 color =
// Ambient : simulates indirect lighting
MaterialAmbientColor +
// Diffuse : "color" of the object
MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance) +
// Specular : reflective highlight, like a mirror
MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance);

FragColor = vec4(color.xyz, 1.0f);

}

)FOO";

internal void
ShaderArrangeSimpleShadingProgram(shader* Shader, u32 ShaderId)
{
    //vertex uniforms
    Shader->VertexUniformLocations[0] = glGetUniformLocation(ShaderId, "MVP");
    Shader->VertexUniformLocations[1] = glGetUniformLocation(ShaderId, "V");
    Shader->VertexUniformLocations[2] = glGetUniformLocation(ShaderId, "M");
    Shader->VertexUniformLocations[3] = glGetUniformLocation(ShaderId, "MV3x3");
    Shader->VertexUniformLocations[4] = glGetUniformLocation(ShaderId, "LightPosition_worldspace");
    
    //fragment textures
    Shader->TextureLocations[0] = glGetUniformLocation(ShaderId, "DiffuseTextureSampler");
    Shader->TextureLocations[1] = glGetUniformLocation(ShaderId, "NormalTextureSampler");
    Shader->TextureLocations[2] = glGetUniformLocation(ShaderId, "SpecularTextureSampler");
    Shader->TextureCount = 3;
    
}


internal void
ShaderSimpleShadingUpload(glm::mat4 &MVP, glm::mat4 &ViewMatrix, glm::mat4 &ModelMatrix, glm::vec3 &LightPos,
                          u32 DiffuseTextureSampler,
                          u32 NormalTextureSampler,
                          u32 SpecularTextureSampler)
{
    glUniformMatrix4fv(GlobalShaderDataCache.SimpleShadingProgram->VertexUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
    glUniformMatrix4fv(GlobalShaderDataCache.SimpleShadingProgram->VertexUniformLocations[1], 1, GL_FALSE, &ViewMatrix[0][0]);
    glUniformMatrix4fv(GlobalShaderDataCache.SimpleShadingProgram->VertexUniformLocations[2], 1, GL_FALSE, &ModelMatrix[0][0]);
    
    glm::mat3 ModelView3x3Matrix = glm::mat3(ModelMatrix);
    glUniformMatrix4fv(GlobalShaderDataCache.SimpleShadingProgram->VertexUniformLocations[3], 1, GL_FALSE, &ModelView3x3Matrix[0][0]);
    
    glUniform3f(GlobalShaderDataCache.SimpleShadingProgram->VertexUniformLocations[4], LightPos.x, LightPos.y, LightPos.z);
    
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, DiffuseTextureSampler);
    glUniform1i(GlobalShaderDataCache.SimpleShadingProgram->TextureLocations[0], 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, NormalTextureSampler);
    glUniform1i(GlobalShaderDataCache.SimpleShadingProgram->TextureLocations[1], 1);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, SpecularTextureSampler);
    glUniform1i(GlobalShaderDataCache.SimpleShadingProgram->TextureLocations[2], 2);
}


//~ simple shading program


char *EditorFloorGridVertexCode =
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

char *EditorFloorGridFragmentCode =
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

internal void
ShaderArrangeEditorFloorGridProgram(shader *Shader, u32 ShaderId)
{
    Shader->VertexUniformLocations[0] = glGetUniformLocation(ShaderId, "u_MVP");
    Shader->VertexUniformCount = 1;
    
    Shader->FragmentUniformLocations[0] = glGetUniformLocation(ShaderId, "u_Scale");
    Shader->FragmentUniformCount = 1;
}

internal void
ShaderEditorFloorGridUpload(glm::mat4 &MVP, float Scale)
{
    glUniform1f(GlobalShaderDataCache.EditorFloorGridProgram->FragmentUniformLocations[0], Scale);
    
    glUniformMatrix4fv(GlobalShaderDataCache.EditorFloorGridProgram->VertexUniformLocations[0], 1, GL_FALSE, &MVP[0][0]);
    
}


//~ editor floor grid shader