///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"
#include <EGL/egl.h>

coreString  g_sExtensions = "";
coreContext g_CoreContext = {};

coreBool CORE_GL_ES2_restriction = false;


// ****************************************************************
/* init OpenGL ES */
void __coreInitOpenGLES()
{
    // reset context structure
    std::memset(&g_CoreContext, 0, sizeof(g_CoreContext));

    // get full extension string
    if(g_sExtensions.empty()) g_sExtensions.assign(r_cast<const coreChar*>(glGetString(GL_EXTENSIONS))).append(1, ' ');

    // get OpenGL ES version
    g_CoreContext.__fVersion = coreData::StrVersion(r_cast<const coreChar*>(glGetString(GL_VERSION)));
    g_CoreContext.__bES30    = (g_CoreContext.__fVersion >= 3.0f);
    g_CoreContext.__bES31    = (g_CoreContext.__fVersion >= 3.1f);
    g_CoreContext.__bES32    = (g_CoreContext.__fVersion >= 3.2f);
    const coreBool  bES30    =  g_CoreContext.__bES30;

    // handle support for deprecated features
    CORE_GL_ES2_restriction = !bES30;

    // implement GL_EXT_discard_framebuffer
    if(__CORE_GLES_CHECK(GL_EXT_discard_framebuffer, false))
    {
        __CORE_GLES_FUNC_FETCH(glDiscardFramebuffer, EXT, false)
    }
    else if(__CORE_GLES_CHECK(GL_EXT_discard_framebuffer, bES30))
    {
        // override function
        g_CoreContext.__glDiscardFramebufferEXT = r_cast<decltype(g_CoreContext.__glDiscardFramebufferEXT)>(eglGetProcAddress("glInvalidateFramebuffer"));
    }

    // implement GL_EXT_texture_storage
    if(__CORE_GLES_CHECK(GL_EXT_texture_storage, bES30))
    {
        __CORE_GLES_FUNC_FETCH(glTexStorage2D, EXT, bES30)
    }

    // implement GL_EXT_texture_filter_anisotropic
    __CORE_GLES_CHECK(GL_EXT_texture_filter_anisotropic, false);

    // implement GL_EXT_texture_compression_rgtc
    __CORE_GLES_CHECK(GL_EXT_texture_compression_rgtc, false);

    // implement GL_EXT_texture_compression_s3tc
    __CORE_GLES_CHECK(GL_EXT_texture_compression_s3tc, false);
    if(g_sExtensions.find("GL_NV_texture_compression_s3tc ")   != coreString::npos) g_CoreContext.__GL_EXT_texture_compression_s3tc = true;
    if(g_sExtensions.find("GL_WEBGL_compressed_texture_s3tc ") != coreString::npos) g_CoreContext.__GL_EXT_texture_compression_s3tc = true;

    // implement GL_EXT_texture_norm16
    __CORE_GLES_CHECK(GL_EXT_texture_norm16, false);

    // implement GL_EXT_texture_type_2_10_10_10_rev
    __CORE_GLES_CHECK(GL_EXT_texture_type_2_10_10_10_rev, bES30);

    // implement GL_KHR_parallel_shader_compile
    if(__CORE_GLES_CHECK(GL_KHR_parallel_shader_compile, false))
    {
        __CORE_GLES_FUNC_FETCH(glMaxShaderCompilerThreads, KHR, false)
        if(!g_CoreContext.__glMaxShaderCompilerThreadsKHR) g_CoreContext.__glMaxShaderCompilerThreadsKHR = [](GLuint){};   // not available in WebGL
    }

    // implement GL_NV_pixel_buffer_object
    __CORE_GLES_CHECK(GL_NV_pixel_buffer_object, bES30);

    // implement GL_NV_framebuffer_blit
    if(__CORE_GLES_CHECK(GL_NV_framebuffer_blit, bES30))
    {
        __CORE_GLES_FUNC_FETCH(glBlitFramebuffer, NV, bES30)
    }
    else if(g_sExtensions.find("GL_ANGLE_framebuffer_blit "))
    {
        // override function
        g_CoreContext.__GL_NV_framebuffer_blit = true;
        g_CoreContext.__glBlitFramebufferNV = r_cast<decltype(g_CoreContext.__glBlitFramebufferNV)>(eglGetProcAddress("glBlitFramebufferANGLE"));
    }

    // implement GL_NV_framebuffer_multisample
    if(__CORE_GLES_CHECK(GL_NV_framebuffer_multisample, bES30))
    {
        __CORE_GLES_FUNC_FETCH(glRenderbufferStorageMultisample, NV, bES30)
    }
    else if(g_sExtensions.find("GL_ANGLE_framebuffer_multisample "))
    {
        // override function
        g_CoreContext.__GL_NV_framebuffer_multisample = true;
        g_CoreContext.__glRenderbufferStorageMultisampleNV = r_cast<decltype(g_CoreContext.__glRenderbufferStorageMultisampleNV)>(eglGetProcAddress("glRenderbufferStorageMultisampleANGLE"));
    }

    // implement GL_OES_vertex_array_object
    if(__CORE_GLES_CHECK(GL_OES_vertex_array_object, bES30))
    {
        __CORE_GLES_FUNC_FETCH(glBindVertexArray,    OES, bES30)
        __CORE_GLES_FUNC_FETCH(glDeleteVertexArrays, OES, bES30)
        __CORE_GLES_FUNC_FETCH(glGenVertexArrays,    OES, bES30)
    }

    // implement GL_OES_vertex_type_2_10_10_10_rev
    __CORE_GLES_CHECK(GL_OES_vertex_type_2_10_10_10_rev, bES30);

    // implement GL_OES_depth_texture
    __CORE_GLES_CHECK(GL_OES_depth_texture, bES30);
    if(g_sExtensions.find("GL_ANGLE_depth_texture ") != coreString::npos) g_CoreContext.__GL_OES_depth_texture = true;
    if(g_sExtensions.find("GL_WEBGL_depth_texture ") != coreString::npos) g_CoreContext.__GL_OES_depth_texture = true;

    // implement GL_OES_texture_stencil8
    __CORE_GLES_CHECK(GL_OES_texture_stencil8, false);

    // implement GL_OES_packed_depth_stencil
    __CORE_GLES_CHECK(GL_OES_packed_depth_stencil, bES30);
    if(g_sExtensions.find("GL_ANGLE_depth_texture ") != coreString::npos) g_CoreContext.__GL_OES_packed_depth_stencil = true;
}


// ****************************************************************
/* get extension string */
void coreExtensions(coreString* OUTPUT psOutput)
{
    // copy saved extension string
    (*psOutput) = g_sExtensions;
}


// ****************************************************************
/* get platform-specific extension string */
void corePlatformExtensions(coreString* OUTPUT psOutput)
{
    // return nothing
    (*psOutput) = "Unknown Platform Extensions";
}