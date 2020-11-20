
struct opengl_instance
{
    b32 SupportsSRGBFramebuffer;
    u32 DefaultInternalTextureFormat;
};

global_variable opengl_instance OpenGL;

struct opengl_info
{
    b32 ModernContext;
    
    char *OpenGLVersion;
    char *Vendor;
    char *Renderer;
    char *Version;
    char *ShadingLanguageVersion;
    char *Extensions;
    
    b32 GL_EXT_texture_sRGB;
    b32 GL_EXT_framebuffer_sRGB;
    //b32 GL_ARB_framebuffer_object;
};
global_variable opengl_info OpenGLInfo;

global_variable b32 IsOpenGLInitialized = false;
