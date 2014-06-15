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
bool m_bGL_EXT_texture_filter_anisotropic = false;

PFNGLDISCARDFRAMEBUFFEREXTPROC glDiscardFramebufferEXT = NULL;
PFNGLTEXSTORAGE2DEXT           glTexStorage2DEXT       = NULL;
GLenum                         GL_RGBA8                = GL_RGBA;
GLenum                         GL_RGB8                 = GL_RGB;


// ****************************************************************  
/* check for implemented extensions */
int coreInitES()
{
    // get and save full extension string
    g_sExtensions = r_cast<const char*>(glGetString(GL_EXTENSIONS));

    // check for GL_EXT_discard_framebuffer
    if((m_bGL_EXT_discard_framebuffer = __CORE_ES_CHECK("GL_EXT_discard_framebuffer")))
    {
        // fetch required function
        __CORE_ES_FETCH(PFNGLDISCARDFRAMEBUFFEREXTPROC, glDiscardFramebufferEXT)
    }

    // check for GL_EXT_texture_storage
    if((m_bGL_EXT_texture_storage = __CORE_ES_CHECK("GL_EXT_texture_storage") && __CORE_ES_CHECK("GL_OES_rgb8_rgba8")))
    {
        // fetch required function
        __CORE_ES_FETCH(PFNGLTEXSTORAGE2DEXT, glTexStorage2DEXT)

        // adjust texture format parameters
        GL_RGBA8 = GL_RGBA8_OES;
        GL_RGB8  = GL_RGB8_OES;
    }

    // check for GL_EXT_texture_filter_anisotropic
    m_bGL_EXT_texture_filter_anisotropic = __CORE_ES_CHECK("GL_EXT_texture_filter_anisotropic");

    // write extension status to log
    Core::Log->Info("GL_EXT_discard_framebuffer:        %d (%d)", m_bGL_EXT_discard_framebuffer        ? 1 : 0, glDiscardFramebufferEXT);
    Core::Log->Info("GL_EXT_texture_storage:            %d (%d)", m_bGL_EXT_texture_storage            ? 1 : 0, glTexStorage2DEXT);
    Core::Log->Info("GL_EXT_texture_filter_anisotropic: %d",      m_bGL_EXT_texture_filter_anisotropic ? 1 : 0);

    return 0;
}