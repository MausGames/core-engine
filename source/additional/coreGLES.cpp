//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"
#include <egl/egl.h>

std::string g_sExtensions = "";
coreContext g_CoreContext = {};


// ****************************************************************
/* init OpenGL ES */
void __coreInitOpenGLES()
{
    // reset context structure
    std::memset(&g_CoreContext, 0, sizeof(g_CoreContext));

    // get full extension string
    if(g_sExtensions.empty()) g_sExtensions = r_cast<const coreChar*>(glGetString(GL_EXTENSIONS));

    // get OpenGL ES version
    g_CoreContext.__fVersion = coreData::StrVersion(r_cast<const coreChar*>(glGetString(GL_VERSION)));
    g_CoreContext.__bES3     = (g_CoreContext.__fVersion >= 3.0f);
    const coreBool& bES3     =  g_CoreContext.__bES3;

    // implement GL_EXT_discard_framebuffer
    if(__CORE_GLES_CHECK(GL_EXT_discard_framebuffer, false))
    {
        __CORE_GLES_FUNC_FETCH(glDiscardFramebuffer, EXT, false)
    }
    else if(__CORE_GLES_CHECK(GL_EXT_discard_framebuffer, bES3))
    {
        // override function
        g_CoreContext.__glDiscardFramebufferEXT = r_cast<decltype(g_CoreContext.__glDiscardFramebufferEXT)>(eglGetProcAddress("glInvalidateFramebuffer"));
    }

    // implement GL_EXT_texture_storage
    if(__CORE_GLES_CHECK(GL_EXT_texture_storage, bES3))
    {
        __CORE_GLES_FUNC_FETCH(glTexStorage2D, EXT, bES3)
    }

    // implement GL_OES_mapbuffer and GL_EXT_map_buffer_range
    if(__CORE_GLES_CHECK(GL_OES_mapbuffer, bES3))
    {
        if(__CORE_GLES_CHECK(GL_EXT_map_buffer_range, bES3))
        {
            __CORE_GLES_FUNC_FETCH(glMapBufferRange, EXT, bES3)
            __CORE_GLES_FUNC_FETCH(glUnmapBuffer,    OES, bES3)
        }
    }

    // implement GL_EXT_instanced_arrays
    if(__CORE_GLES_CHECK(GL_EXT_instanced_arrays, bES3))
    {
        __CORE_GLES_FUNC_FETCH(glDrawArraysInstanced,   EXT, bES3)
        __CORE_GLES_FUNC_FETCH(glDrawElementsInstanced, EXT, bES3)
        __CORE_GLES_FUNC_FETCH(glVertexAttribDivisor,   EXT, bES3)
    }

    // implement GL_EXT_multisample_compatibility
    __CORE_GLES_CHECK(GL_EXT_multisample_compatibility, false);

    // implement GL_EXT_sRGB_write_control
    __CORE_GLES_CHECK(GL_EXT_sRGB_write_control, false);

    // implement GL_EXT_texture_filter_anisotropic
    __CORE_GLES_CHECK(GL_EXT_texture_filter_anisotropic, false);

    // implement GL_EXT_texture_compression_s3tc
    __CORE_GLES_CHECK(GL_EXT_texture_compression_s3tc, false);

    // implement GL_NV_pixel_buffer_object
    __CORE_GLES_CHECK(GL_NV_pixel_buffer_object, bES3);

    // implement GL_NV_framebuffer_blit
    if(__CORE_GLES_CHECK(GL_NV_framebuffer_blit, bES3))
    {
        __CORE_GLES_VAR_SET(GL_READ_FRAMEBUFFER, 0x8CA8)
        __CORE_GLES_VAR_SET(GL_DRAW_FRAMEBUFFER, 0x8CA9)

        __CORE_GLES_FUNC_FETCH(glBlitFramebuffer, NV, bES3)
    }
    else
    {
        __CORE_GLES_VAR_SET(GL_READ_FRAMEBUFFER, GL_FRAMEBUFFER)
        __CORE_GLES_VAR_SET(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER)
    }

    // implement GL_NV_framebuffer_multisample
    if(__CORE_GLES_CHECK(GL_NV_framebuffer_multisample, bES3))
    {
        __CORE_GLES_FUNC_FETCH(glRenderbufferStorageMultisample, NV, bES3)
    }

    // implement GL_OES_vertex_array_object
    if(__CORE_GLES_CHECK(GL_OES_vertex_array_object, bES3))
    {
        __CORE_GLES_FUNC_FETCH(glBindVertexArray,    OES, bES3)
        __CORE_GLES_FUNC_FETCH(glDeleteVertexArrays, OES, bES3)
        __CORE_GLES_FUNC_FETCH(glGenVertexArrays,    OES, bES3)
    }

    // implement GL_OES_vertex_type_2_10_10_10_rev
    __CORE_GLES_CHECK(GL_OES_vertex_type_2_10_10_10_rev, bES3);

    // implement GL_OES_depth_texture
    __CORE_GLES_CHECK(GL_OES_depth_texture, false);

    // implement GL_OES_texture_stencil8
    __CORE_GLES_CHECK(GL_OES_texture_stencil8, false);
}


// ****************************************************************
/* get extension string */
void coreExtensions(std::string* OUTPUT psOutput)
{
    // copy saved extension string
    (*psOutput) = g_sExtensions;
}


// ****************************************************************
/* get platform-specific extension string */
void corePlatformExtensions(std::string* OUTPUT psOutput)
{
    // return nothing
    (*psOutput) = "";
}