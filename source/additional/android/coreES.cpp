//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
/* only required for OpenGL ES 2.0 applications,
 * implements unavailable definitions and dummy-functions */

#include "Core.h"
#include <EGL/egl.h>


std::string g_sExtensions = "";
bool m_bGL_EXT_discard_framebuffer = false;

PFNGLDISCARDFRAMEBUFFEREXTPROC glDiscardFramebufferEXT = NULL;

GLenum glewInit()
{
    g_sExtensions = r_cast<const char*>(glGetString(GL_EXTENSIONS));
    m_bGL_EXT_discard_framebuffer = (g_sExtensions.find("GL_EXT_discard_framebuffer") != std::string::npos);
    if(m_bGL_EXT_discard_framebuffer) glDiscardFramebufferEXT = (PFNGLDISCARDFRAMEBUFFEREXTPROC)eglGetProcAddress("glDiscardFramebufferEXT");
    
    return GLEW_OK;
}