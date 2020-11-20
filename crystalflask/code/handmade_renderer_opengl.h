/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#define OPENGL_LIGHT_INDEX_VOXEL_DIM 128
#define OPENGL_MAX_LIGHT_PROBE_COUNT 65536
#define OPENGL_MAX_LIGHT_OCCLUDER_COUNT 65536

struct opengl_info
{
    b32 ModernContext;
    
    char *Vendor;
    char *Renderer;
    char *Version;
    char *ShadingLanguageVersion;
    //    char *Extensions;
    
    b32 OpenGL_EXT_texture_sRGB;
    b32 OpenGL_EXT_framebuffer_sRGB;
    b32 OpenGL_ARB_framebuffer_object;
};

struct opengl_program_common
{
    GLuint ProgHandle;
    
    GLuint VertPID;
    GLuint VertNID;
    GLuint VertUVID;
    GLuint VertColorID;
    GLuint VertTextureIndex;
    
    u32 SamplerCount;
    GLuint Samplers[16];
};

struct zbias_program
{
    opengl_program_common Common;
    
    GLuint TransformID;
    GLuint CameraP;
    GLuint FogDirection;
    GLuint FogColor;
    GLuint FogStartDistance;
    GLuint FogEndDistance;
    GLuint ClipAlphaStartDistance;
    GLuint ClipAlphaEndDistance;
    GLuint AlphaThreshold;
};

struct resolve_multisample_program
{
    opengl_program_common Common;
};

struct fake_seed_lighting_program
{
    opengl_program_common Common;
    
    GLuint DebugLightP;
};

struct multigrid_light_down_program
{
    opengl_program_common Common;
    GLuint SourceUVStep;
};

enum opengl_color_handle_type
{
    // TODO(casey): It's worth thinking about making it so there's only
    // one RGB stored here, and then store an emission power value instead
    // of storing the emission separately?
    OpenGLColor_SurfaceReflect, // NOTE(casey): Reflect RGB, coverage A
    //    OpenGLColor_Emit, // NOTE(casey): Emit RGB, spread A
    //   OpenGLColor_NPL, // NOTE(casey): Nx, Ny, TODO(casey): Lp0, Lp1
    
    OpenGLColor_Count,
};
struct opengl_framebuffer
{
    GLuint FramebufferHandle;
    GLuint ColorHandle[OpenGLColor_Count];
    GLuint DepthHandle;
    
    umm GPUMemoryUsed;
};
enum opengl_framebuffer_flags
{
    OpenGLFramebuffer_Multisampled = 0x1,
    OpenGLFramebuffer_Filtered = 0x2,
    OpenGLFramebuffer_Depth = 0x4,
    OpenGLFramebuffer_Float = 0x8,
};

struct light_buffer
{
    u32 Width;
    u32 Height;
    
    GLuint WriteAllFramebuffer;
    GLuint WriteEmitFramebuffer;
    
    // NOTE(casey): These are all 3-element textures
    GLuint FrontEmitTex;
    GLuint BackEmitTex;
    GLuint SurfaceColorTex;
    GLuint NPTex; // NOTE(casey): This is Nx, Nz, Depth
};

struct open_gl
{
    platform_renderer Header;
    
    game_render_settings CurrentSettings;
    
    GLint MaxColorAttachments;
    GLint MaxSamplersPerShader;
    
    b32x ShaderSimTexReadSRGB;
    b32x ShaderSimTexWriteSRGB;
    
    GLint MaxMultiSampleCount; // TODO(casey): This should probably be renamed to MultiSampleCount
    //b32 sRGBSupport;
    b32 SupportsSRGBFramebuffer;
    
    GLuint DefaultSpriteTextureFormat;
    GLuint DefaultFramebufferTextureFormat;
    
    GLuint VertexBuffer;
    GLuint IndexBuffer;
    GLuint ScreenFillVertexBuffer;
    
    GLuint ReservedBlitTexture;
    
    GLuint TextureArray;
    
    u32 White[4][4];
    GLuint SinglePixelAllZeroesTexture;
    
    b32x Multisampling;
    u32 DepthPeelCount;
    
    u8 PushBufferMemory[65536];
    textured_vertex *VertexArray;
    u16 *IndexArray;
    renderer_texture *BitmapArray;
    
    u32 MaxQuadTextureCount;
    u32 MaxTextureCount;
    u32 MaxVertexCount;
    u32 MaxIndexCount;
    
    u32 MaxSpecialTextureCount;
    GLuint *SpecialTextureHandles;
    
    //
    // NOTE(casey): Dynamic resources that get rereated when settings change:
    //
    opengl_framebuffer ResolveFramebuffer;
    opengl_framebuffer DepthPeelBuffer; //[16];
    opengl_framebuffer DepthPeelResolveBuffer[16];
    zbias_program ZBiasNoDepthPeel; // NOTE(casey): Pass 0
    zbias_program ZBiasDepthPeel; // NOTE(casey): Passes 1 through n
    opengl_program_common PeelComposite; // NOTE(casey): Composite all passes
    opengl_program_common FinalStretch;
    resolve_multisample_program ResolveMultisample;
    
    light_probe_irradiance LightProbeIrradiance[OPENGL_MAX_LIGHT_PROBE_COUNT];
    v3 LightProbePosition[OPENGL_MAX_LIGHT_PROBE_COUNT];
    u16 LightIndexVoxel[OPENGL_LIGHT_INDEX_VOXEL_DIM*OPENGL_LIGHT_INDEX_VOXEL_DIM*OPENGL_LIGHT_INDEX_VOXEL_DIM];
    lighting_box LightOccluders[OPENGL_MAX_LIGHT_OCCLUDER_COUNT];
    
    GLuint LightData0;
    GLuint LightData1;
    
#if 0
    u32x LightBufferCount;
    light_buffer LightBuffers[12];
#endif
    
    game_render_commands RenderCommands;
};

internal void OpenGLInit(open_gl *OpenGL, opengl_info Info, b32 FramebufferSupportsSRGB);
internal opengl_info OpenGLGetInfo(b32 ModernContext);

internal void OpenGLManageTextures(open_gl *OpenGL, renderer_texture_queue *Queue);
internal game_render_commands *OpenGLBeginFrame(open_gl *OpenGL, v2u OSWindowDim, v2u RenderDim, rectangle2i DrawRegion);
internal void OpenGLEndFrame(open_gl *OpenGL, game_render_commands *Commands);

internal void PlatformOpenGLSetVSync(open_gl *Renderer, b32x VSyncEnabled);
