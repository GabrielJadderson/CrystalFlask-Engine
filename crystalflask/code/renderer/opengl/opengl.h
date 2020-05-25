#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/ext/matrix_transform.hpp>

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


struct opengl_instance
{
    b32 SupportsSRGBFramebuffer;
    u32 DefaultInternalTextureFormat;
};


struct Light
{
    glm::vec3 Direction;
    glm::vec3 Radiance;
};

Light Light;
float LightMultiplier = 0.3f;


