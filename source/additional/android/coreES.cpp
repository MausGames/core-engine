//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"
#include <EGL/egl.h>

std::string g_sExtensions                 = "";
bool m_bGL_EXT_discard_framebuffer        = false;
bool m_bGL_EXT_texture_storage            = false;
bool m_bGL_EXT_map_buffer_range           = false;
bool m_bGL_OES_vertex_array_object        = false;
bool m_bGL_EXT_texture_filter_anisotropic = false;

GLenum                         GL_RGBA8                = GL_RGBA;
GLenum                         GL_RGB8                 = GL_RGB;
PFNGLDISCARDFRAMEBUFFEREXTPROC glDiscardFramebufferEXT = NULL;
PFNGLTEXSTORAGE2DEXTPROC       glTexStorage2DEXT       = NULL;
PFNGLUNMAPBUFFEROESPROC        glUnmapBufferOES        = NULL;
PFNGLMAPBUFFERRANGEEXTPROC     glMapBufferRangeEXT     = NULL;
PFNGLBINDVERTEXARRAYOESPROC    glBindVertexArrayOES    = NULL;
PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES = NULL;
PFNGLGENVERTEXARRAYSOESPROC    glGenVertexArraysOES    = NULL;


// ****************************************************************  
/* check for implemented extensions */
int coreInitES()
{
    // get and save full extension string
    g_sExtensions = r_cast<const char*>(glGetString(GL_EXTENSIONS));

    // check for GL_EXT_discard_framebuffer
    if((m_bGL_EXT_discard_framebuffer = __CORE_ES_CHECK("GL_EXT_discard_framebuffer")))
    {
        // fetch required functions
        __CORE_ES_FETCH(PFNGLDISCARDFRAMEBUFFEREXTPROC, glDiscardFramebufferEXT)
    }

    // check for GL_EXT_texture_storage
    if((m_bGL_EXT_texture_storage = __CORE_ES_CHECK("GL_EXT_texture_storage") && __CORE_ES_CHECK("GL_OES_rgb8_rgba8")))
    {
        // adjust texture format parameters
        GL_RGBA8 = GL_RGBA8_OES;
        GL_RGB8  = GL_RGB8_OES;

        // fetch required functions
        __CORE_ES_FETCH(PFNGLTEXSTORAGE2DEXTPROC, glTexStorage2DEXT)
    }

    // check for GL_EXT_map_buffer_range
    if((m_bGL_EXT_map_buffer_range = __CORE_ES_CHECK("GL_EXT_map_buffer_range") && __CORE_ES_CHECK("GL_OES_mapbuffer")))
    {
        // fetch required functions
        __CORE_ES_FETCH(PFNGLUNMAPBUFFEROESPROC,    glUnmapBufferOES)
        __CORE_ES_FETCH(PFNGLMAPBUFFERRANGEEXTPROC, glMapBufferRangeEXT)
    }

    // check for GL_OES_vertex_array_object
    if((m_bGL_OES_vertex_array_object = __CORE_ES_CHECK("GL_OES_vertex_array_object")))
    {
        // fetch required functions
        __CORE_ES_FETCH(PFNGLBINDVERTEXARRAYOESPROC,    glBindVertexArrayOES)
        __CORE_ES_FETCH(PFNGLDELETEVERTEXARRAYSOESPROC, glDeleteVertexArraysOES)
        __CORE_ES_FETCH(PFNGLGENVERTEXARRAYSOESPROC,    glGenVertexArraysOES)
    }

    // check for GL_EXT_texture_filter_anisotropic
    m_bGL_EXT_texture_filter_anisotropic = __CORE_ES_CHECK("GL_EXT_texture_filter_anisotropic");

    // write extension status to log
    Core::Log->Info("GL_EXT_discard_framebuffer:        %d (%d)",           m_bGL_EXT_discard_framebuffer        ? 1 : 0, glDiscardFramebufferEXT);
    Core::Log->Info("GL_EXT_texture_storage:            %d (%d)",           m_bGL_EXT_texture_storage            ? 1 : 0, glTexStorage2DEXT);
    Core::Log->Info("GL_EXT_map_buffer_range:           %d (%d) (%d)",      m_bGL_EXT_map_buffer_range           ? 1 : 0, glUnmapBufferOES, glMapBufferRangeEXT);
    Core::Log->Info("GL_OES_vertex_array_object:        %d (%d) (%d) (%d)", m_bGL_OES_vertex_array_object        ? 1 : 0, glBindVertexArrayOES, glDeleteVertexArraysOES, glGenVertexArraysOES);
    Core::Log->Info("GL_EXT_texture_filter_anisotropic: %d",                m_bGL_EXT_texture_filter_anisotropic ? 1 : 0);

    return 0;
}