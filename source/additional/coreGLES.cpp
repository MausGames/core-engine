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

    // change extension status through configuration file (e.g. GL_EXT_framebuffer_object)
    coreData::StrForEachToken(Core::Config->GetString(CORE_CONFIG_GRAPHICS_DISABLEEXTENSIONS), " ,;", [](const coreChar* pcToken) {g_sExtensions.replace(PRINT("%s ", pcToken), "");});

    // get OpenGL ES version
    g_CoreContext.__fVersion = coreData::StrVersion(r_cast<const coreChar*>(glGetString(GL_VERSION)));
    g_CoreContext.__bES30    = (g_CoreContext.__fVersion >= 3.0f);
    g_CoreContext.__bES31    = (g_CoreContext.__fVersion >= 3.1f);
    g_CoreContext.__bES32    = (g_CoreContext.__fVersion >= 3.2f);
    const coreBool  bES30    =  g_CoreContext.__bES30;
    const coreBool  bES32    =  g_CoreContext.__bES32;

    // handle support for deprecated features
    CORE_GL_ES2_restriction = !bES30;

    // implement GL_ANDROID_extension_pack_es31a
    __CORE_GLES_CHECK(GL_ANDROID_extension_pack_es31a, false);
    const coreBool bAndroidPack = g_CoreContext.__GL_ANDROID_extension_pack_es31a;

    // implement GL_ANGLE_texture_usage
    __CORE_GLES_CHECK(GL_ANGLE_texture_usage, false);

    // implement GL_CORE_vertex_type_2_10_10_10_rev
    __CORE_GLES_CHECK(GL_CORE_vertex_type_2_10_10_10_rev, bES30);

    // implement GL_EXT_buffer_storage
    if(__CORE_GLES_CHECK(GL_EXT_buffer_storage, false))
    {
        __CORE_GLES_FUNC_FETCH(glBufferStorage, EXT, false)
    }

    // implement GL_EXT_color_buffer_float
    __CORE_GLES_CHECK(GL_EXT_color_buffer_float, bES32);

    // implement GL_EXT_color_buffer_half_float
    __CORE_GLES_CHECK(GL_EXT_color_buffer_half_float, false);

    // implement GL_EXT_depth_clamp
    __CORE_GLES_CHECK(GL_EXT_depth_clamp, false);

    // implement GL_EXT_discard_framebuffer
    if(__CORE_GLES_CHECK(GL_EXT_discard_framebuffer, false))
    {
        __CORE_GLES_FUNC_FETCH(glDiscardFramebuffer, EXT, false)
    }
    else if(__CORE_GLES_CHECK(GL_EXT_discard_framebuffer, bES30))
    {
        // override function
        g_CoreContext.__glDiscardFramebuffer = r_cast<decltype(g_CoreContext.__glDiscardFramebuffer)>(eglGetProcAddress("glInvalidateFramebuffer"));
    }

    // implement GL_EXT_disjoint_timer_query
    if(__CORE_GLES_CHECK(GL_EXT_disjoint_timer_query, false))
    {
        __CORE_GLES_FUNC_FETCH(glDeleteQueries,       EXT, false)
        __CORE_GLES_FUNC_FETCH(glGenQueries,          EXT, false)
        __CORE_GLES_FUNC_FETCH(glGetQueryObjectui64v, EXT, false)
        __CORE_GLES_FUNC_FETCH(glQueryCounter,        EXT, false)
    }
    else if(g_sExtensions.contains("GL_ANGLE_timer_query "))
    {
        g_CoreContext.__GL_EXT_disjoint_timer_query = true;
        __CORE_GLES_FUNC_FETCH(glDeleteQueries,       ANGLE, false)
        __CORE_GLES_FUNC_FETCH(glGenQueries,          ANGLE, false)
        __CORE_GLES_FUNC_FETCH(glGetQueryObjectui64v, ANGLE, false)
        __CORE_GLES_FUNC_FETCH(glQueryCounter,        ANGLE, false)
    }

    // implement GL_EXT_draw_buffers
    if(__CORE_GLES_CHECK(GL_EXT_draw_buffers, bES30))
    {
        __CORE_GLES_FUNC_FETCH(glDrawBuffers, EXT, bES30)
    }
    else if(g_sExtensions.contains("GL_NV_draw_buffers ") && g_sExtensions.contains("GL_NV_fbo_color_attachments "))
    {
        g_CoreContext.__GL_EXT_draw_buffers = true;
        __CORE_GLES_FUNC_FETCH(glDrawBuffers, NV, false)
    }
    else if(g_sExtensions.contains("GL_WEBGL_draw_buffers "))
    {
        g_CoreContext.__GL_EXT_draw_buffers = true;
        __CORE_GLES_FUNC_FETCH(glDrawBuffers, WEBGL, false)
    }

    // implement GL_EXT_instanced_arrays
    if(__CORE_GLES_CHECK(GL_EXT_instanced_arrays, bES30))
    {
        __CORE_GLES_FUNC_FETCH(glDrawArraysInstanced,   EXT, bES30)
        __CORE_GLES_FUNC_FETCH(glDrawElementsInstanced, EXT, bES30)
        __CORE_GLES_FUNC_FETCH(glVertexAttribDivisor,   EXT, bES30)
    }
    else if(g_sExtensions.contains("GL_ANGLE_instanced_arrays "))
    {
        g_CoreContext.__GL_EXT_instanced_arrays = true;
        __CORE_GLES_FUNC_FETCH(glDrawArraysInstanced,   ANGLE, false)
        __CORE_GLES_FUNC_FETCH(glDrawElementsInstanced, ANGLE, false)
        __CORE_GLES_FUNC_FETCH(glVertexAttribDivisor,   ANGLE, false)
    }
    else if(g_sExtensions.contains("GL_NV_draw_instanced ") && g_sExtensions.contains("GL_NV_instanced_arrays "))
    {
        g_CoreContext.__GL_EXT_instanced_arrays = true;
        __CORE_GLES_FUNC_FETCH(glDrawArraysInstanced,   NV, false)
        __CORE_GLES_FUNC_FETCH(glDrawElementsInstanced, NV, false)
        __CORE_GLES_FUNC_FETCH(glVertexAttribDivisor,   NV, false)
    }

    // implement GL_EXT_sRGB_write_control
    __CORE_GLES_CHECK(GL_EXT_sRGB_write_control, false);

    // implement GL_EXT_texture_compression_rgtc
    __CORE_GLES_CHECK(GL_EXT_texture_compression_rgtc, false);

    // implement GL_EXT_texture_compression_s3tc
    __CORE_GLES_CHECK(GL_EXT_texture_compression_s3tc, false);
    if(g_sExtensions.contains("GL_ANGLE_texture_compression_dxt5 ") &&
       g_sExtensions.contains("GL_EXT_texture_compression_dxt1 "))  g_CoreContext.__GL_EXT_texture_compression_s3tc = true;
    if(g_sExtensions.contains("GL_NV_texture_compression_s3tc "))   g_CoreContext.__GL_EXT_texture_compression_s3tc = true;
    if(g_sExtensions.contains("GL_WEBGL_compressed_texture_s3tc ")) g_CoreContext.__GL_EXT_texture_compression_s3tc = true;

    // implement GL_EXT_texture_filter_anisotropic
    __CORE_GLES_CHECK(GL_EXT_texture_filter_anisotropic, false);

    // implement GL_EXT_texture_norm16
    __CORE_GLES_CHECK(GL_EXT_texture_norm16, false);

    // implement GL_EXT_texture_rg
    __CORE_GLES_CHECK(GL_EXT_texture_rg, bES30);

    // implement GL_EXT_texture_storage
    if(__CORE_GLES_CHECK(GL_EXT_texture_storage, bES30))
    {
        __CORE_GLES_FUNC_FETCH(glTexStorage2D, EXT, bES30)
    }

    // implement GL_EXT_texture_type_2_10_10_10_REV
    __CORE_GLES_CHECK(GL_EXT_texture_type_2_10_10_10_REV, bES30);

    // implement GL_INTEL_conservative_rasterization
    __CORE_GLES_CHECK(GL_INTEL_conservative_rasterization, false);

    // implement GL_INTEL_framebuffer_CMAA
    if(__CORE_GLES_CHECK(GL_INTEL_framebuffer_CMAA, false))
    {
        __CORE_GLES_FUNC_FETCH(glApplyFramebufferAttachmentCMAA, INTEL, false)
    }

    // implement GL_KHR_debug
    if(__CORE_GLES_CHECK(GL_KHR_debug, bES32 || bAndroidPack))
    {
        __CORE_GLES_FUNC_FETCH(glDebugMessageCallback, KHR, bES32)
        __CORE_GLES_FUNC_FETCH(glDebugMessageControl,  KHR, bES32)
        __CORE_GLES_FUNC_FETCH(glObjectLabel,          KHR, bES32)
    }

    // implement GL_KHR_parallel_shader_compile
    if(__CORE_GLES_CHECK(GL_KHR_parallel_shader_compile, false))
    {
        __CORE_GLES_FUNC_FETCH(glMaxShaderCompilerThreads, KHR, false)
        if(!g_CoreContext.__glMaxShaderCompilerThreads) g_CoreContext.__glMaxShaderCompilerThreads = [](GLuint){};   // not available in WebGL
    }

    // implement GL_NV_conservative_raster
    __CORE_GLES_CHECK(GL_NV_conservative_raster, false);

    // implement GL_NV_framebuffer_blit
    if(__CORE_GLES_CHECK(GL_NV_framebuffer_blit, bES30))
    {
        __CORE_GLES_FUNC_FETCH(glBlitFramebuffer, NV, bES30)
    }
    else if(g_sExtensions.contains("GL_ANGLE_framebuffer_blit "))
    {
        g_CoreContext.__GL_NV_framebuffer_blit = true;
        __CORE_GLES_FUNC_FETCH(glBlitFramebuffer, ANGLE, false)
    }

    // implement GL_NV_framebuffer_multisample
    if(__CORE_GLES_CHECK(GL_NV_framebuffer_multisample, bES30))
    {
        __CORE_GLES_FUNC_FETCH(glRenderbufferStorageMultisample, NV, bES30)
    }
    else if(g_sExtensions.contains("GL_ANGLE_framebuffer_multisample "))
    {
        g_CoreContext.__GL_NV_framebuffer_multisample = true;
        __CORE_GLES_FUNC_FETCH(glRenderbufferStorageMultisample, ANGLE, false)
    }

    // implement GL_NV_pixel_buffer_object
    __CORE_GLES_CHECK(GL_NV_pixel_buffer_object, bES30);

    // implement GL_OES_depth_texture
    __CORE_GLES_CHECK(GL_OES_depth_texture, bES30);
    if(g_sExtensions.contains("GL_ANGLE_depth_texture ")) g_CoreContext.__GL_OES_depth_texture = true;
    if(g_sExtensions.contains("GL_WEBGL_depth_texture ")) g_CoreContext.__GL_OES_depth_texture = true;

    // implement GL_OES_geometry_shader
    __CORE_GLES_CHECK(GL_OES_geometry_shader, bES32 || bAndroidPack);
    if(g_sExtensions.contains("GL_EXT_geometry_shader ")) g_CoreContext.__GL_OES_geometry_shader = true;

    // implement GL_OES_get_program_binary
    if(__CORE_GLES_CHECK(GL_OES_get_program_binary, false))
    {
        __CORE_GLES_FUNC_FETCH(glGetProgramBinary, OES, false)
        __CORE_GLES_FUNC_FETCH(glProgramBinary,    OES, false)
    }

    // implement GL_OES_packed_depth_stencil
    __CORE_GLES_CHECK(GL_OES_packed_depth_stencil, bES30);
    if(g_sExtensions.contains("GL_ANGLE_depth_texture ")) g_CoreContext.__GL_OES_packed_depth_stencil = true;

    // implement GL_OES_sample_shading
    if(__CORE_GLES_CHECK(GL_OES_sample_shading, bES32 || bAndroidPack))
    {
        __CORE_GLES_FUNC_FETCH(glMinSampleShading, OES, bES32)
    }

    // implement GL_OES_tessellation_shader
    __CORE_GLES_CHECK(GL_OES_tessellation_shader, bES32 || bAndroidPack);
    if(g_sExtensions.contains("GL_EXT_tessellation_shader ")) g_CoreContext.__GL_OES_tessellation_shader = true;

    // implement GL_OES_texture_float
    __CORE_GLES_CHECK(GL_OES_texture_float, bES30);   // not used

    // implement GL_OES_texture_float_linear
    __CORE_GLES_CHECK(GL_OES_texture_float_linear, false);   // not used

    // implement GL_OES_texture_half_float
    __CORE_GLES_CHECK(GL_OES_texture_half_float, bES30);

    // implement GL_OES_texture_half_float_linear
    __CORE_GLES_CHECK(GL_OES_texture_half_float_linear, bES30);

    // implement GL_OES_texture_stencil8
    __CORE_GLES_CHECK(GL_OES_texture_stencil8, bES32 || bAndroidPack);

    // implement GL_OES_vertex_array_object
    if(__CORE_GLES_CHECK(GL_OES_vertex_array_object, bES30))
    {
        __CORE_GLES_FUNC_FETCH(glBindVertexArray,    OES, bES30)
        __CORE_GLES_FUNC_FETCH(glDeleteVertexArrays, OES, bES30)
        __CORE_GLES_FUNC_FETCH(glGenVertexArrays,    OES, bES30)
    }

    // implement GL_OES_vertex_half_float
    __CORE_GLES_CHECK(GL_OES_vertex_half_float, bES30);

    // implement GL_WEBGL_color_buffer_float
    __CORE_GLES_CHECK(GL_WEBGL_color_buffer_float, false);   // not used

    // map various extensions to GL_CORE_texture_float (# only for half-float)
    if(g_CoreContext.__GL_OES_texture_half_float && g_CoreContext.__GL_OES_texture_half_float_linear && (g_CoreContext.__GL_EXT_color_buffer_float || g_CoreContext.__GL_EXT_color_buffer_half_float))
        g_CoreContext.__GL_CORE_texture_float = true;
}


// ****************************************************************
/* get extension string */
void coreExtensions(coreString* OUTPUT psOutput)
{
    // copy saved extension string
    (*psOutput) = g_sExtensions;
    if(!psOutput->empty()) psOutput->pop_back();
}


// ****************************************************************
/* get platform-specific extension string */
void corePlatformExtensions(coreString* OUTPUT psOutput)
{
    // clear memory
    psOutput->clear();

#if defined(_CORE_ANGLE_)

    // get connection to default display
    EGLDisplay pDisplay = eglGetCurrentDisplay();
    if(pDisplay)
    {
        // get full extension string (EGL)
        (*psOutput) += eglQueryString(pDisplay,       EGL_EXTENSIONS);   // display extensions
        (*psOutput) += ' ';
        (*psOutput) += eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);   // client extensions
    }

#else

    // return nothing
    (*psOutput) = "Unknown Platform Extensions";

#endif
}