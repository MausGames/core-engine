///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"
#include <EGL/egl.h>

coreString  g_sExtensions = "";
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
    g_CoreContext.__bES30    = (g_CoreContext.__fVersion >= 3.0f);
    g_CoreContext.__bES31    = (g_CoreContext.__fVersion >= 3.1f);
    g_CoreContext.__bES32    = (g_CoreContext.__fVersion >= 3.2f);
    const coreBool  bES30    =  g_CoreContext.__bES30;

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
    if(g_sExtensions.find("GL_WEBGL_compressed_texture_s3tc") != coreString::npos) g_CoreContext.__GL_EXT_texture_compression_s3tc = true;

    // implement GL_NV_pixel_buffer_object
    __CORE_GLES_CHECK(GL_NV_pixel_buffer_object, bES30);

    // implement GL_NV_framebuffer_blit
    if(__CORE_GLES_CHECK(GL_NV_framebuffer_blit, bES30))
    {
        __CORE_GLES_FUNC_FETCH(glBlitFramebuffer, NV, bES30)
    }

    // implement GL_NV_framebuffer_multisample
    if(__CORE_GLES_CHECK(GL_NV_framebuffer_multisample, bES30))
    {
        __CORE_GLES_FUNC_FETCH(glRenderbufferStorageMultisample, NV, bES30)
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

    // implement GL_OES_texture_stencil8
    __CORE_GLES_CHECK(GL_OES_texture_stencil8, false);

    // implement GL_OES_packed_depth_stencil
    __CORE_GLES_CHECK(GL_OES_packed_depth_stencil, bES30);
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
    (*psOutput) = "";
}