// Modified version for Core Engine
// Please use the original library from https://github.com/nigels-com/glew

/*
 * The OpenGL Extension Wrangler Library
 * Copyright (C) 2008-2025, Nigel Stewart <nigels[]nigels com>
 * Copyright (C) 2002-2008, Milan Ikits <milan ikits[]ieee org>
 * Copyright (C) 2002-2008, Marcelo E. Magallon <mmagallo[]debian org>
 * Copyright (C) 2002, Lev Povalahev
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * The name of the author may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Mesa 3-D graphics library
 * Version:  7.0
 *
 * Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Copyright (c) 2007 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

#ifndef GLEW_INCLUDE
#  include <GL/glew.h>
#else
#  include GLEW_INCLUDE
#endif

#include <stddef.h>  /* For size_t */

/*
 * Define glewGetProcAddress.
 */

#if defined(__APPLE__)

#include <dlfcn.h>

static void* NSGLGetProcAddress (const char* name)
{
  static void* image = NULL;
  if (!image) image = dlopen("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", RTLD_LAZY);
  return image ? dlsym(image, name) : NULL;
}

#endif

#if defined(_WIN32)

/*
 * If NOGDI is defined, wingdi.h won't be included by windows.h, and thus
 * wglGetProcAddress won't be declared. It will instead be implicitly declared,
 * potentially incorrectly, which we don't want.
 */

#  if defined(NOGDI)
#    undef NOGDI
#  endif
#  include <GL/wglew.h>
#  define glewGetProcAddress(name) wglGetProcAddress((LPCSTR)name)
#elif defined(__linux__)
#  include <GL/glx.h>
#  define glewGetProcAddress(name) glXGetProcAddressARB(name)
#elif defined(__APPLE__)
#  define glewGetProcAddress(name) NSGLGetProcAddress((const char*)name)
#else
#  include <EGL/egl.h>
#  define glewGetProcAddress(name) eglGetProcAddress((const char*)name)
#endif

/*
 * Redefine GLEW_GET_VAR etc without const cast
 */

#undef GLEW_GET_VAR
#define GLEW_GET_VAR(x) (x)

#ifdef WGLEW_GET_VAR
#  undef WGLEW_GET_VAR
#  define WGLEW_GET_VAR(x) (x)
#endif /* WGLEW_GET_VAR */

/*
 * GLEW, just like OpenGL or GLU, does not rely on the standard C library.
 * These functions implement the functionality required in this file.
 */

static GLuint _glewStrLen (const GLubyte* s)
{
  GLuint i=0;
  if (s == NULL) return 0;
  while (s[i] != '\0') i++;
  return i;
}

static GLuint _glewStrCLen (const GLubyte* s, GLubyte c)
{
  GLuint i=0;
  if (s == NULL) return 0;
  while (s[i] != '\0' && s[i] != c) i++;
  return i;
}

static GLuint _glewStrCopy (char *d, const char *s, char c)
{
  GLuint i=0;
  if (s == NULL) return 0;
  while (s[i] != '\0' && s[i] != c) { d[i] = s[i]; i++; }
  d[i] = '\0';
  return i;
}

#if defined(_WIN32)

static GLboolean _glewStrSame (const GLubyte* a, const GLubyte* b, GLuint n)
{
  GLuint i=0;
  if (a == NULL || b == NULL)
    return (a == NULL && b == NULL && n == 0) ? GL_TRUE : GL_FALSE;
  while (i < n && a[i] != '\0' && b[i] != '\0' && a[i] == b[i]) i++;
  return i == n ? GL_TRUE : GL_FALSE;
}

#endif

static GLboolean _glewStrSame1 (const GLubyte** a, GLuint* na, const GLubyte* b, GLuint nb)
{
  while (*na > 0 && (**a == ' ' || **a == '\n' || **a == '\r' || **a == '\t'))
  {
    (*a)++;
    (*na)--;
  }
  if (*na >= nb)
  {
    GLuint i=0;
    while (i < nb && (*a)[i] == b[i]) i++;
    if (i == nb)
    {
      *a = *a + nb;
      *na = *na - nb;
      return GL_TRUE;
    }
  }
  return GL_FALSE;
}

static GLboolean _glewStrSame2 (const GLubyte** a, GLuint* na, const GLubyte* b, GLuint nb)
{
  if (*na >= nb)
  {
    GLuint i=0;
    while (i < nb && (*a)[i] == b[i]) i++;
    if (i == nb)
    {
      *a = *a + nb;
      *na = *na - nb;
      return GL_TRUE;
    }
  }
  return GL_FALSE;
}

static GLboolean _glewStrSame3 (const GLubyte** a, GLuint* na, const GLubyte* b, GLuint nb)
{
  if (*na >= nb)
  {
    GLuint i=0;
    while (i < nb && (*a)[i] == b[i]) i++;
    if (i == nb && (*na == nb || (*a)[i] == ' ' || (*a)[i] == '\n' || (*a)[i] == '\r' || (*a)[i] == '\t'))
    {
      *a = *a + nb;
      *na = *na - nb;
      return GL_TRUE;
    }
  }
  return GL_FALSE;
}

#if defined(_WIN32)

/*
 * Search for name in the extensions string. Use of strstr()
 * is not sufficient because extension names can be prefixes of
 * other extension names. Could use strtok() but the constant
 * string returned by glGetString might be in read-only memory.
 */

static GLboolean _glewSearchExtension (const char* name, const GLubyte *start, const GLubyte *end)
{
  const GLubyte* p;
  GLuint len = _glewStrLen((const GLubyte*)name);
  p = start;
  while (p < end)
  {
    GLuint n = _glewStrCLen(p, ' ');
    if (len == n && _glewStrSame((const GLubyte*)name, p, n)) return GL_TRUE;
    p += n+1;
  }
  return GL_FALSE;
}

#endif

PFNGLCOPYTEXSUBIMAGE3DPROC __glewCopyTexSubImage3D = NULL;
PFNGLDRAWRANGEELEMENTSPROC __glewDrawRangeElements = NULL;
PFNGLTEXIMAGE3DPROC __glewTexImage3D = NULL;
PFNGLTEXSUBIMAGE3DPROC __glewTexSubImage3D = NULL;

PFNGLACTIVETEXTUREPROC __glewActiveTexture = NULL;
PFNGLCLIENTACTIVETEXTUREPROC __glewClientActiveTexture = NULL;
PFNGLCOMPRESSEDTEXIMAGE1DPROC __glewCompressedTexImage1D = NULL;
PFNGLCOMPRESSEDTEXIMAGE2DPROC __glewCompressedTexImage2D = NULL;
PFNGLCOMPRESSEDTEXIMAGE3DPROC __glewCompressedTexImage3D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC __glewCompressedTexSubImage1D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC __glewCompressedTexSubImage2D = NULL;
PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC __glewCompressedTexSubImage3D = NULL;
PFNGLGETCOMPRESSEDTEXIMAGEPROC __glewGetCompressedTexImage = NULL;
PFNGLLOADTRANSPOSEMATRIXDPROC __glewLoadTransposeMatrixd = NULL;
PFNGLLOADTRANSPOSEMATRIXFPROC __glewLoadTransposeMatrixf = NULL;
PFNGLMULTTRANSPOSEMATRIXDPROC __glewMultTransposeMatrixd = NULL;
PFNGLMULTTRANSPOSEMATRIXFPROC __glewMultTransposeMatrixf = NULL;
PFNGLMULTITEXCOORD1DPROC __glewMultiTexCoord1d = NULL;
PFNGLMULTITEXCOORD1DVPROC __glewMultiTexCoord1dv = NULL;
PFNGLMULTITEXCOORD1FPROC __glewMultiTexCoord1f = NULL;
PFNGLMULTITEXCOORD1FVPROC __glewMultiTexCoord1fv = NULL;
PFNGLMULTITEXCOORD1IPROC __glewMultiTexCoord1i = NULL;
PFNGLMULTITEXCOORD1IVPROC __glewMultiTexCoord1iv = NULL;
PFNGLMULTITEXCOORD1SPROC __glewMultiTexCoord1s = NULL;
PFNGLMULTITEXCOORD1SVPROC __glewMultiTexCoord1sv = NULL;
PFNGLMULTITEXCOORD2DPROC __glewMultiTexCoord2d = NULL;
PFNGLMULTITEXCOORD2DVPROC __glewMultiTexCoord2dv = NULL;
PFNGLMULTITEXCOORD2FPROC __glewMultiTexCoord2f = NULL;
PFNGLMULTITEXCOORD2FVPROC __glewMultiTexCoord2fv = NULL;
PFNGLMULTITEXCOORD2IPROC __glewMultiTexCoord2i = NULL;
PFNGLMULTITEXCOORD2IVPROC __glewMultiTexCoord2iv = NULL;
PFNGLMULTITEXCOORD2SPROC __glewMultiTexCoord2s = NULL;
PFNGLMULTITEXCOORD2SVPROC __glewMultiTexCoord2sv = NULL;
PFNGLMULTITEXCOORD3DPROC __glewMultiTexCoord3d = NULL;
PFNGLMULTITEXCOORD3DVPROC __glewMultiTexCoord3dv = NULL;
PFNGLMULTITEXCOORD3FPROC __glewMultiTexCoord3f = NULL;
PFNGLMULTITEXCOORD3FVPROC __glewMultiTexCoord3fv = NULL;
PFNGLMULTITEXCOORD3IPROC __glewMultiTexCoord3i = NULL;
PFNGLMULTITEXCOORD3IVPROC __glewMultiTexCoord3iv = NULL;
PFNGLMULTITEXCOORD3SPROC __glewMultiTexCoord3s = NULL;
PFNGLMULTITEXCOORD3SVPROC __glewMultiTexCoord3sv = NULL;
PFNGLMULTITEXCOORD4DPROC __glewMultiTexCoord4d = NULL;
PFNGLMULTITEXCOORD4DVPROC __glewMultiTexCoord4dv = NULL;
PFNGLMULTITEXCOORD4FPROC __glewMultiTexCoord4f = NULL;
PFNGLMULTITEXCOORD4FVPROC __glewMultiTexCoord4fv = NULL;
PFNGLMULTITEXCOORD4IPROC __glewMultiTexCoord4i = NULL;
PFNGLMULTITEXCOORD4IVPROC __glewMultiTexCoord4iv = NULL;
PFNGLMULTITEXCOORD4SPROC __glewMultiTexCoord4s = NULL;
PFNGLMULTITEXCOORD4SVPROC __glewMultiTexCoord4sv = NULL;
PFNGLSAMPLECOVERAGEPROC __glewSampleCoverage = NULL;

PFNGLBLENDCOLORPROC __glewBlendColor = NULL;
PFNGLBLENDEQUATIONPROC __glewBlendEquation = NULL;
PFNGLBLENDFUNCSEPARATEPROC __glewBlendFuncSeparate = NULL;
PFNGLFOGCOORDPOINTERPROC __glewFogCoordPointer = NULL;
PFNGLFOGCOORDDPROC __glewFogCoordd = NULL;
PFNGLFOGCOORDDVPROC __glewFogCoorddv = NULL;
PFNGLFOGCOORDFPROC __glewFogCoordf = NULL;
PFNGLFOGCOORDFVPROC __glewFogCoordfv = NULL;
PFNGLMULTIDRAWARRAYSPROC __glewMultiDrawArrays = NULL;
PFNGLMULTIDRAWELEMENTSPROC __glewMultiDrawElements = NULL;
PFNGLPOINTPARAMETERFPROC __glewPointParameterf = NULL;
PFNGLPOINTPARAMETERFVPROC __glewPointParameterfv = NULL;
PFNGLPOINTPARAMETERIPROC __glewPointParameteri = NULL;
PFNGLPOINTPARAMETERIVPROC __glewPointParameteriv = NULL;
PFNGLSECONDARYCOLOR3BPROC __glewSecondaryColor3b = NULL;
PFNGLSECONDARYCOLOR3BVPROC __glewSecondaryColor3bv = NULL;
PFNGLSECONDARYCOLOR3DPROC __glewSecondaryColor3d = NULL;
PFNGLSECONDARYCOLOR3DVPROC __glewSecondaryColor3dv = NULL;
PFNGLSECONDARYCOLOR3FPROC __glewSecondaryColor3f = NULL;
PFNGLSECONDARYCOLOR3FVPROC __glewSecondaryColor3fv = NULL;
PFNGLSECONDARYCOLOR3IPROC __glewSecondaryColor3i = NULL;
PFNGLSECONDARYCOLOR3IVPROC __glewSecondaryColor3iv = NULL;
PFNGLSECONDARYCOLOR3SPROC __glewSecondaryColor3s = NULL;
PFNGLSECONDARYCOLOR3SVPROC __glewSecondaryColor3sv = NULL;
PFNGLSECONDARYCOLOR3UBPROC __glewSecondaryColor3ub = NULL;
PFNGLSECONDARYCOLOR3UBVPROC __glewSecondaryColor3ubv = NULL;
PFNGLSECONDARYCOLOR3UIPROC __glewSecondaryColor3ui = NULL;
PFNGLSECONDARYCOLOR3UIVPROC __glewSecondaryColor3uiv = NULL;
PFNGLSECONDARYCOLOR3USPROC __glewSecondaryColor3us = NULL;
PFNGLSECONDARYCOLOR3USVPROC __glewSecondaryColor3usv = NULL;
PFNGLSECONDARYCOLORPOINTERPROC __glewSecondaryColorPointer = NULL;
PFNGLWINDOWPOS2DPROC __glewWindowPos2d = NULL;
PFNGLWINDOWPOS2DVPROC __glewWindowPos2dv = NULL;
PFNGLWINDOWPOS2FPROC __glewWindowPos2f = NULL;
PFNGLWINDOWPOS2FVPROC __glewWindowPos2fv = NULL;
PFNGLWINDOWPOS2IPROC __glewWindowPos2i = NULL;
PFNGLWINDOWPOS2IVPROC __glewWindowPos2iv = NULL;
PFNGLWINDOWPOS2SPROC __glewWindowPos2s = NULL;
PFNGLWINDOWPOS2SVPROC __glewWindowPos2sv = NULL;
PFNGLWINDOWPOS3DPROC __glewWindowPos3d = NULL;
PFNGLWINDOWPOS3DVPROC __glewWindowPos3dv = NULL;
PFNGLWINDOWPOS3FPROC __glewWindowPos3f = NULL;
PFNGLWINDOWPOS3FVPROC __glewWindowPos3fv = NULL;
PFNGLWINDOWPOS3IPROC __glewWindowPos3i = NULL;
PFNGLWINDOWPOS3IVPROC __glewWindowPos3iv = NULL;
PFNGLWINDOWPOS3SPROC __glewWindowPos3s = NULL;
PFNGLWINDOWPOS3SVPROC __glewWindowPos3sv = NULL;

PFNGLBEGINQUERYPROC __glewBeginQuery = NULL;
PFNGLBINDBUFFERPROC __glewBindBuffer = NULL;
PFNGLBUFFERDATAPROC __glewBufferData = NULL;
PFNGLBUFFERSUBDATAPROC __glewBufferSubData = NULL;
PFNGLDELETEBUFFERSPROC __glewDeleteBuffers = NULL;
PFNGLDELETEQUERIESPROC __glewDeleteQueries = NULL;
PFNGLENDQUERYPROC __glewEndQuery = NULL;
PFNGLGENBUFFERSPROC __glewGenBuffers = NULL;
PFNGLGENQUERIESPROC __glewGenQueries = NULL;
PFNGLGETBUFFERPARAMETERIVPROC __glewGetBufferParameteriv = NULL;
PFNGLGETBUFFERPOINTERVPROC __glewGetBufferPointerv = NULL;
PFNGLGETBUFFERSUBDATAPROC __glewGetBufferSubData = NULL;
PFNGLGETQUERYOBJECTIVPROC __glewGetQueryObjectiv = NULL;
PFNGLGETQUERYOBJECTUIVPROC __glewGetQueryObjectuiv = NULL;
PFNGLGETQUERYIVPROC __glewGetQueryiv = NULL;
PFNGLISBUFFERPROC __glewIsBuffer = NULL;
PFNGLISQUERYPROC __glewIsQuery = NULL;
PFNGLMAPBUFFERPROC __glewMapBuffer = NULL;
PFNGLUNMAPBUFFERPROC __glewUnmapBuffer = NULL;

PFNGLATTACHSHADERPROC __glewAttachShader = NULL;
PFNGLBINDATTRIBLOCATIONPROC __glewBindAttribLocation = NULL;
PFNGLBLENDEQUATIONSEPARATEPROC __glewBlendEquationSeparate = NULL;
PFNGLCOMPILESHADERPROC __glewCompileShader = NULL;
PFNGLCREATEPROGRAMPROC __glewCreateProgram = NULL;
PFNGLCREATESHADERPROC __glewCreateShader = NULL;
PFNGLDELETEPROGRAMPROC __glewDeleteProgram = NULL;
PFNGLDELETESHADERPROC __glewDeleteShader = NULL;
PFNGLDETACHSHADERPROC __glewDetachShader = NULL;
PFNGLDISABLEVERTEXATTRIBARRAYPROC __glewDisableVertexAttribArray = NULL;
PFNGLDRAWBUFFERSPROC __glewDrawBuffers = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC __glewEnableVertexAttribArray = NULL;
PFNGLGETACTIVEATTRIBPROC __glewGetActiveAttrib = NULL;
PFNGLGETACTIVEUNIFORMPROC __glewGetActiveUniform = NULL;
PFNGLGETATTACHEDSHADERSPROC __glewGetAttachedShaders = NULL;
PFNGLGETATTRIBLOCATIONPROC __glewGetAttribLocation = NULL;
PFNGLGETPROGRAMINFOLOGPROC __glewGetProgramInfoLog = NULL;
PFNGLGETPROGRAMIVPROC __glewGetProgramiv = NULL;
PFNGLGETSHADERINFOLOGPROC __glewGetShaderInfoLog = NULL;
PFNGLGETSHADERSOURCEPROC __glewGetShaderSource = NULL;
PFNGLGETSHADERIVPROC __glewGetShaderiv = NULL;
PFNGLGETUNIFORMLOCATIONPROC __glewGetUniformLocation = NULL;
PFNGLGETUNIFORMFVPROC __glewGetUniformfv = NULL;
PFNGLGETUNIFORMIVPROC __glewGetUniformiv = NULL;
PFNGLGETVERTEXATTRIBPOINTERVPROC __glewGetVertexAttribPointerv = NULL;
PFNGLGETVERTEXATTRIBDVPROC __glewGetVertexAttribdv = NULL;
PFNGLGETVERTEXATTRIBFVPROC __glewGetVertexAttribfv = NULL;
PFNGLGETVERTEXATTRIBIVPROC __glewGetVertexAttribiv = NULL;
PFNGLISPROGRAMPROC __glewIsProgram = NULL;
PFNGLISSHADERPROC __glewIsShader = NULL;
PFNGLLINKPROGRAMPROC __glewLinkProgram = NULL;
PFNGLSHADERSOURCEPROC __glewShaderSource = NULL;
PFNGLSTENCILFUNCSEPARATEPROC __glewStencilFuncSeparate = NULL;
PFNGLSTENCILMASKSEPARATEPROC __glewStencilMaskSeparate = NULL;
PFNGLSTENCILOPSEPARATEPROC __glewStencilOpSeparate = NULL;
PFNGLUNIFORM1FPROC __glewUniform1f = NULL;
PFNGLUNIFORM1FVPROC __glewUniform1fv = NULL;
PFNGLUNIFORM1IPROC __glewUniform1i = NULL;
PFNGLUNIFORM1IVPROC __glewUniform1iv = NULL;
PFNGLUNIFORM2FPROC __glewUniform2f = NULL;
PFNGLUNIFORM2FVPROC __glewUniform2fv = NULL;
PFNGLUNIFORM2IPROC __glewUniform2i = NULL;
PFNGLUNIFORM2IVPROC __glewUniform2iv = NULL;
PFNGLUNIFORM3FPROC __glewUniform3f = NULL;
PFNGLUNIFORM3FVPROC __glewUniform3fv = NULL;
PFNGLUNIFORM3IPROC __glewUniform3i = NULL;
PFNGLUNIFORM3IVPROC __glewUniform3iv = NULL;
PFNGLUNIFORM4FPROC __glewUniform4f = NULL;
PFNGLUNIFORM4FVPROC __glewUniform4fv = NULL;
PFNGLUNIFORM4IPROC __glewUniform4i = NULL;
PFNGLUNIFORM4IVPROC __glewUniform4iv = NULL;
PFNGLUNIFORMMATRIX2FVPROC __glewUniformMatrix2fv = NULL;
PFNGLUNIFORMMATRIX3FVPROC __glewUniformMatrix3fv = NULL;
PFNGLUNIFORMMATRIX4FVPROC __glewUniformMatrix4fv = NULL;
PFNGLUSEPROGRAMPROC __glewUseProgram = NULL;
PFNGLVALIDATEPROGRAMPROC __glewValidateProgram = NULL;
PFNGLVERTEXATTRIB1DPROC __glewVertexAttrib1d = NULL;
PFNGLVERTEXATTRIB1DVPROC __glewVertexAttrib1dv = NULL;
PFNGLVERTEXATTRIB1FPROC __glewVertexAttrib1f = NULL;
PFNGLVERTEXATTRIB1FVPROC __glewVertexAttrib1fv = NULL;
PFNGLVERTEXATTRIB1SPROC __glewVertexAttrib1s = NULL;
PFNGLVERTEXATTRIB1SVPROC __glewVertexAttrib1sv = NULL;
PFNGLVERTEXATTRIB2DPROC __glewVertexAttrib2d = NULL;
PFNGLVERTEXATTRIB2DVPROC __glewVertexAttrib2dv = NULL;
PFNGLVERTEXATTRIB2FPROC __glewVertexAttrib2f = NULL;
PFNGLVERTEXATTRIB2FVPROC __glewVertexAttrib2fv = NULL;
PFNGLVERTEXATTRIB2SPROC __glewVertexAttrib2s = NULL;
PFNGLVERTEXATTRIB2SVPROC __glewVertexAttrib2sv = NULL;
PFNGLVERTEXATTRIB3DPROC __glewVertexAttrib3d = NULL;
PFNGLVERTEXATTRIB3DVPROC __glewVertexAttrib3dv = NULL;
PFNGLVERTEXATTRIB3FPROC __glewVertexAttrib3f = NULL;
PFNGLVERTEXATTRIB3FVPROC __glewVertexAttrib3fv = NULL;
PFNGLVERTEXATTRIB3SPROC __glewVertexAttrib3s = NULL;
PFNGLVERTEXATTRIB3SVPROC __glewVertexAttrib3sv = NULL;
PFNGLVERTEXATTRIB4NBVPROC __glewVertexAttrib4Nbv = NULL;
PFNGLVERTEXATTRIB4NIVPROC __glewVertexAttrib4Niv = NULL;
PFNGLVERTEXATTRIB4NSVPROC __glewVertexAttrib4Nsv = NULL;
PFNGLVERTEXATTRIB4NUBPROC __glewVertexAttrib4Nub = NULL;
PFNGLVERTEXATTRIB4NUBVPROC __glewVertexAttrib4Nubv = NULL;
PFNGLVERTEXATTRIB4NUIVPROC __glewVertexAttrib4Nuiv = NULL;
PFNGLVERTEXATTRIB4NUSVPROC __glewVertexAttrib4Nusv = NULL;
PFNGLVERTEXATTRIB4BVPROC __glewVertexAttrib4bv = NULL;
PFNGLVERTEXATTRIB4DPROC __glewVertexAttrib4d = NULL;
PFNGLVERTEXATTRIB4DVPROC __glewVertexAttrib4dv = NULL;
PFNGLVERTEXATTRIB4FPROC __glewVertexAttrib4f = NULL;
PFNGLVERTEXATTRIB4FVPROC __glewVertexAttrib4fv = NULL;
PFNGLVERTEXATTRIB4IVPROC __glewVertexAttrib4iv = NULL;
PFNGLVERTEXATTRIB4SPROC __glewVertexAttrib4s = NULL;
PFNGLVERTEXATTRIB4SVPROC __glewVertexAttrib4sv = NULL;
PFNGLVERTEXATTRIB4UBVPROC __glewVertexAttrib4ubv = NULL;
PFNGLVERTEXATTRIB4UIVPROC __glewVertexAttrib4uiv = NULL;
PFNGLVERTEXATTRIB4USVPROC __glewVertexAttrib4usv = NULL;
PFNGLVERTEXATTRIBPOINTERPROC __glewVertexAttribPointer = NULL;

PFNGLUNIFORMMATRIX2X3FVPROC __glewUniformMatrix2x3fv = NULL;
PFNGLUNIFORMMATRIX2X4FVPROC __glewUniformMatrix2x4fv = NULL;
PFNGLUNIFORMMATRIX3X2FVPROC __glewUniformMatrix3x2fv = NULL;
PFNGLUNIFORMMATRIX3X4FVPROC __glewUniformMatrix3x4fv = NULL;
PFNGLUNIFORMMATRIX4X2FVPROC __glewUniformMatrix4x2fv = NULL;
PFNGLUNIFORMMATRIX4X3FVPROC __glewUniformMatrix4x3fv = NULL;

PFNGLBEGINCONDITIONALRENDERPROC __glewBeginConditionalRender = NULL;
PFNGLBEGINTRANSFORMFEEDBACKPROC __glewBeginTransformFeedback = NULL;
PFNGLBINDFRAGDATALOCATIONPROC __glewBindFragDataLocation = NULL;
PFNGLCLAMPCOLORPROC __glewClampColor = NULL;
PFNGLCLEARBUFFERFIPROC __glewClearBufferfi = NULL;
PFNGLCLEARBUFFERFVPROC __glewClearBufferfv = NULL;
PFNGLCLEARBUFFERIVPROC __glewClearBufferiv = NULL;
PFNGLCLEARBUFFERUIVPROC __glewClearBufferuiv = NULL;
PFNGLCOLORMASKIPROC __glewColorMaski = NULL;
PFNGLDISABLEIPROC __glewDisablei = NULL;
PFNGLENABLEIPROC __glewEnablei = NULL;
PFNGLENDCONDITIONALRENDERPROC __glewEndConditionalRender = NULL;
PFNGLENDTRANSFORMFEEDBACKPROC __glewEndTransformFeedback = NULL;
PFNGLGETBOOLEANI_VPROC __glewGetBooleani_v = NULL;
PFNGLGETFRAGDATALOCATIONPROC __glewGetFragDataLocation = NULL;
PFNGLGETSTRINGIPROC __glewGetStringi = NULL;
PFNGLGETTEXPARAMETERIIVPROC __glewGetTexParameterIiv = NULL;
PFNGLGETTEXPARAMETERIUIVPROC __glewGetTexParameterIuiv = NULL;
PFNGLGETTRANSFORMFEEDBACKVARYINGPROC __glewGetTransformFeedbackVarying = NULL;
PFNGLGETUNIFORMUIVPROC __glewGetUniformuiv = NULL;
PFNGLGETVERTEXATTRIBIIVPROC __glewGetVertexAttribIiv = NULL;
PFNGLGETVERTEXATTRIBIUIVPROC __glewGetVertexAttribIuiv = NULL;
PFNGLISENABLEDIPROC __glewIsEnabledi = NULL;
PFNGLTEXPARAMETERIIVPROC __glewTexParameterIiv = NULL;
PFNGLTEXPARAMETERIUIVPROC __glewTexParameterIuiv = NULL;
PFNGLTRANSFORMFEEDBACKVARYINGSPROC __glewTransformFeedbackVaryings = NULL;
PFNGLUNIFORM1UIPROC __glewUniform1ui = NULL;
PFNGLUNIFORM1UIVPROC __glewUniform1uiv = NULL;
PFNGLUNIFORM2UIPROC __glewUniform2ui = NULL;
PFNGLUNIFORM2UIVPROC __glewUniform2uiv = NULL;
PFNGLUNIFORM3UIPROC __glewUniform3ui = NULL;
PFNGLUNIFORM3UIVPROC __glewUniform3uiv = NULL;
PFNGLUNIFORM4UIPROC __glewUniform4ui = NULL;
PFNGLUNIFORM4UIVPROC __glewUniform4uiv = NULL;
PFNGLVERTEXATTRIBI1IPROC __glewVertexAttribI1i = NULL;
PFNGLVERTEXATTRIBI1IVPROC __glewVertexAttribI1iv = NULL;
PFNGLVERTEXATTRIBI1UIPROC __glewVertexAttribI1ui = NULL;
PFNGLVERTEXATTRIBI1UIVPROC __glewVertexAttribI1uiv = NULL;
PFNGLVERTEXATTRIBI2IPROC __glewVertexAttribI2i = NULL;
PFNGLVERTEXATTRIBI2IVPROC __glewVertexAttribI2iv = NULL;
PFNGLVERTEXATTRIBI2UIPROC __glewVertexAttribI2ui = NULL;
PFNGLVERTEXATTRIBI2UIVPROC __glewVertexAttribI2uiv = NULL;
PFNGLVERTEXATTRIBI3IPROC __glewVertexAttribI3i = NULL;
PFNGLVERTEXATTRIBI3IVPROC __glewVertexAttribI3iv = NULL;
PFNGLVERTEXATTRIBI3UIPROC __glewVertexAttribI3ui = NULL;
PFNGLVERTEXATTRIBI3UIVPROC __glewVertexAttribI3uiv = NULL;
PFNGLVERTEXATTRIBI4BVPROC __glewVertexAttribI4bv = NULL;
PFNGLVERTEXATTRIBI4IPROC __glewVertexAttribI4i = NULL;
PFNGLVERTEXATTRIBI4IVPROC __glewVertexAttribI4iv = NULL;
PFNGLVERTEXATTRIBI4SVPROC __glewVertexAttribI4sv = NULL;
PFNGLVERTEXATTRIBI4UBVPROC __glewVertexAttribI4ubv = NULL;
PFNGLVERTEXATTRIBI4UIPROC __glewVertexAttribI4ui = NULL;
PFNGLVERTEXATTRIBI4UIVPROC __glewVertexAttribI4uiv = NULL;
PFNGLVERTEXATTRIBI4USVPROC __glewVertexAttribI4usv = NULL;
PFNGLVERTEXATTRIBIPOINTERPROC __glewVertexAttribIPointer = NULL;

PFNGLDRAWARRAYSINSTANCEDPROC __glewDrawArraysInstanced = NULL;
PFNGLDRAWELEMENTSINSTANCEDPROC __glewDrawElementsInstanced = NULL;
PFNGLPRIMITIVERESTARTINDEXPROC __glewPrimitiveRestartIndex = NULL;
PFNGLTEXBUFFERPROC __glewTexBuffer = NULL;

PFNGLFRAMEBUFFERTEXTUREPROC __glewFramebufferTexture = NULL;
PFNGLGETBUFFERPARAMETERI64VPROC __glewGetBufferParameteri64v = NULL;
PFNGLGETINTEGER64I_VPROC __glewGetInteger64i_v = NULL;

PFNGLVERTEXATTRIBDIVISORPROC __glewVertexAttribDivisor = NULL;

PFNGLBLENDEQUATIONSEPARATEIPROC __glewBlendEquationSeparatei = NULL;
PFNGLBLENDEQUATIONIPROC __glewBlendEquationi = NULL;
PFNGLBLENDFUNCSEPARATEIPROC __glewBlendFuncSeparatei = NULL;
PFNGLBLENDFUNCIPROC __glewBlendFunci = NULL;
PFNGLMINSAMPLESHADINGPROC __glewMinSampleShading = NULL;

PFNGLGETGRAPHICSRESETSTATUSPROC __glewGetGraphicsResetStatus = NULL;
PFNGLGETNCOMPRESSEDTEXIMAGEPROC __glewGetnCompressedTexImage = NULL;
PFNGLGETNTEXIMAGEPROC __glewGetnTexImage = NULL;
PFNGLGETNUNIFORMDVPROC __glewGetnUniformdv = NULL;

PFNGLMULTIDRAWARRAYSINDIRECTCOUNTPROC __glewMultiDrawArraysIndirectCount = NULL;
PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTPROC __glewMultiDrawElementsIndirectCount = NULL;
PFNGLSPECIALIZESHADERPROC __glewSpecializeShader = NULL;

PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC __glewNamedRenderbufferStorageMultisampleAdvancedAMD = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC __glewRenderbufferStorageMultisampleAdvancedAMD = NULL;

PFNGLBUFFERSTORAGEPROC __glewBufferStorage = NULL;

PFNGLCLEARBUFFERDATAPROC __glewClearBufferData = NULL;
PFNGLCLEARBUFFERSUBDATAPROC __glewClearBufferSubData = NULL;
PFNGLCLEARNAMEDBUFFERDATAEXTPROC __glewClearNamedBufferDataEXT = NULL;
PFNGLCLEARNAMEDBUFFERSUBDATAEXTPROC __glewClearNamedBufferSubDataEXT = NULL;

PFNGLCLEARTEXIMAGEPROC __glewClearTexImage = NULL;
PFNGLCLEARTEXSUBIMAGEPROC __glewClearTexSubImage = NULL;

PFNGLDISPATCHCOMPUTEPROC __glewDispatchCompute = NULL;
PFNGLDISPATCHCOMPUTEINDIRECTPROC __glewDispatchComputeIndirect = NULL;

PFNGLCOPYBUFFERSUBDATAPROC __glewCopyBufferSubData = NULL;

PFNGLCOPYIMAGESUBDATAPROC __glewCopyImageSubData = NULL;

PFNGLBINDTEXTUREUNITPROC __glewBindTextureUnit = NULL;
PFNGLBLITNAMEDFRAMEBUFFERPROC __glewBlitNamedFramebuffer = NULL;
PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC __glewCheckNamedFramebufferStatus = NULL;
PFNGLCLEARNAMEDBUFFERDATAPROC __glewClearNamedBufferData = NULL;
PFNGLCLEARNAMEDBUFFERSUBDATAPROC __glewClearNamedBufferSubData = NULL;
PFNGLCLEARNAMEDFRAMEBUFFERFIPROC __glewClearNamedFramebufferfi = NULL;
PFNGLCLEARNAMEDFRAMEBUFFERFVPROC __glewClearNamedFramebufferfv = NULL;
PFNGLCLEARNAMEDFRAMEBUFFERIVPROC __glewClearNamedFramebufferiv = NULL;
PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC __glewClearNamedFramebufferuiv = NULL;
PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC __glewCompressedTextureSubImage1D = NULL;
PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC __glewCompressedTextureSubImage2D = NULL;
PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC __glewCompressedTextureSubImage3D = NULL;
PFNGLCOPYNAMEDBUFFERSUBDATAPROC __glewCopyNamedBufferSubData = NULL;
PFNGLCOPYTEXTURESUBIMAGE1DPROC __glewCopyTextureSubImage1D = NULL;
PFNGLCOPYTEXTURESUBIMAGE2DPROC __glewCopyTextureSubImage2D = NULL;
PFNGLCOPYTEXTURESUBIMAGE3DPROC __glewCopyTextureSubImage3D = NULL;
PFNGLCREATEBUFFERSPROC __glewCreateBuffers = NULL;
PFNGLCREATEFRAMEBUFFERSPROC __glewCreateFramebuffers = NULL;
PFNGLCREATEPROGRAMPIPELINESPROC __glewCreateProgramPipelines = NULL;
PFNGLCREATEQUERIESPROC __glewCreateQueries = NULL;
PFNGLCREATERENDERBUFFERSPROC __glewCreateRenderbuffers = NULL;
PFNGLCREATESAMPLERSPROC __glewCreateSamplers = NULL;
PFNGLCREATETEXTURESPROC __glewCreateTextures = NULL;
PFNGLCREATETRANSFORMFEEDBACKSPROC __glewCreateTransformFeedbacks = NULL;
PFNGLCREATEVERTEXARRAYSPROC __glewCreateVertexArrays = NULL;
PFNGLDISABLEVERTEXARRAYATTRIBPROC __glewDisableVertexArrayAttrib = NULL;
PFNGLENABLEVERTEXARRAYATTRIBPROC __glewEnableVertexArrayAttrib = NULL;
PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC __glewFlushMappedNamedBufferRange = NULL;
PFNGLGENERATETEXTUREMIPMAPPROC __glewGenerateTextureMipmap = NULL;
PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC __glewGetCompressedTextureImage = NULL;
PFNGLGETNAMEDBUFFERPARAMETERI64VPROC __glewGetNamedBufferParameteri64v = NULL;
PFNGLGETNAMEDBUFFERPARAMETERIVPROC __glewGetNamedBufferParameteriv = NULL;
PFNGLGETNAMEDBUFFERPOINTERVPROC __glewGetNamedBufferPointerv = NULL;
PFNGLGETNAMEDBUFFERSUBDATAPROC __glewGetNamedBufferSubData = NULL;
PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC __glewGetNamedFramebufferAttachmentParameteriv = NULL;
PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC __glewGetNamedFramebufferParameteriv = NULL;
PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC __glewGetNamedRenderbufferParameteriv = NULL;
PFNGLGETQUERYBUFFEROBJECTI64VPROC __glewGetQueryBufferObjecti64v = NULL;
PFNGLGETQUERYBUFFEROBJECTIVPROC __glewGetQueryBufferObjectiv = NULL;
PFNGLGETQUERYBUFFEROBJECTUI64VPROC __glewGetQueryBufferObjectui64v = NULL;
PFNGLGETQUERYBUFFEROBJECTUIVPROC __glewGetQueryBufferObjectuiv = NULL;
PFNGLGETTEXTUREIMAGEPROC __glewGetTextureImage = NULL;
PFNGLGETTEXTURELEVELPARAMETERFVPROC __glewGetTextureLevelParameterfv = NULL;
PFNGLGETTEXTURELEVELPARAMETERIVPROC __glewGetTextureLevelParameteriv = NULL;
PFNGLGETTEXTUREPARAMETERIIVPROC __glewGetTextureParameterIiv = NULL;
PFNGLGETTEXTUREPARAMETERIUIVPROC __glewGetTextureParameterIuiv = NULL;
PFNGLGETTEXTUREPARAMETERFVPROC __glewGetTextureParameterfv = NULL;
PFNGLGETTEXTUREPARAMETERIVPROC __glewGetTextureParameteriv = NULL;
PFNGLGETTRANSFORMFEEDBACKI64_VPROC __glewGetTransformFeedbacki64_v = NULL;
PFNGLGETTRANSFORMFEEDBACKI_VPROC __glewGetTransformFeedbacki_v = NULL;
PFNGLGETTRANSFORMFEEDBACKIVPROC __glewGetTransformFeedbackiv = NULL;
PFNGLGETVERTEXARRAYINDEXED64IVPROC __glewGetVertexArrayIndexed64iv = NULL;
PFNGLGETVERTEXARRAYINDEXEDIVPROC __glewGetVertexArrayIndexediv = NULL;
PFNGLGETVERTEXARRAYIVPROC __glewGetVertexArrayiv = NULL;
PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC __glewInvalidateNamedFramebufferData = NULL;
PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC __glewInvalidateNamedFramebufferSubData = NULL;
PFNGLMAPNAMEDBUFFERPROC __glewMapNamedBuffer = NULL;
PFNGLMAPNAMEDBUFFERRANGEPROC __glewMapNamedBufferRange = NULL;
PFNGLNAMEDBUFFERDATAPROC __glewNamedBufferData = NULL;
PFNGLNAMEDBUFFERSTORAGEPROC __glewNamedBufferStorage = NULL;
PFNGLNAMEDBUFFERSUBDATAPROC __glewNamedBufferSubData = NULL;
PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC __glewNamedFramebufferDrawBuffer = NULL;
PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC __glewNamedFramebufferDrawBuffers = NULL;
PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC __glewNamedFramebufferParameteri = NULL;
PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC __glewNamedFramebufferReadBuffer = NULL;
PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC __glewNamedFramebufferRenderbuffer = NULL;
PFNGLNAMEDFRAMEBUFFERTEXTUREPROC __glewNamedFramebufferTexture = NULL;
PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC __glewNamedFramebufferTextureLayer = NULL;
PFNGLNAMEDRENDERBUFFERSTORAGEPROC __glewNamedRenderbufferStorage = NULL;
PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC __glewNamedRenderbufferStorageMultisample = NULL;
PFNGLTEXTUREBUFFERPROC __glewTextureBuffer = NULL;
PFNGLTEXTUREBUFFERRANGEPROC __glewTextureBufferRange = NULL;
PFNGLTEXTUREPARAMETERIIVPROC __glewTextureParameterIiv = NULL;
PFNGLTEXTUREPARAMETERIUIVPROC __glewTextureParameterIuiv = NULL;
PFNGLTEXTUREPARAMETERFPROC __glewTextureParameterf = NULL;
PFNGLTEXTUREPARAMETERFVPROC __glewTextureParameterfv = NULL;
PFNGLTEXTUREPARAMETERIPROC __glewTextureParameteri = NULL;
PFNGLTEXTUREPARAMETERIVPROC __glewTextureParameteriv = NULL;
PFNGLTEXTURESTORAGE1DPROC __glewTextureStorage1D = NULL;
PFNGLTEXTURESTORAGE2DPROC __glewTextureStorage2D = NULL;
PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC __glewTextureStorage2DMultisample = NULL;
PFNGLTEXTURESTORAGE3DPROC __glewTextureStorage3D = NULL;
PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC __glewTextureStorage3DMultisample = NULL;
PFNGLTEXTURESUBIMAGE1DPROC __glewTextureSubImage1D = NULL;
PFNGLTEXTURESUBIMAGE2DPROC __glewTextureSubImage2D = NULL;
PFNGLTEXTURESUBIMAGE3DPROC __glewTextureSubImage3D = NULL;
PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC __glewTransformFeedbackBufferBase = NULL;
PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC __glewTransformFeedbackBufferRange = NULL;
PFNGLUNMAPNAMEDBUFFERPROC __glewUnmapNamedBuffer = NULL;
PFNGLVERTEXARRAYATTRIBBINDINGPROC __glewVertexArrayAttribBinding = NULL;
PFNGLVERTEXARRAYATTRIBFORMATPROC __glewVertexArrayAttribFormat = NULL;
PFNGLVERTEXARRAYATTRIBIFORMATPROC __glewVertexArrayAttribIFormat = NULL;
PFNGLVERTEXARRAYATTRIBLFORMATPROC __glewVertexArrayAttribLFormat = NULL;
PFNGLVERTEXARRAYBINDINGDIVISORPROC __glewVertexArrayBindingDivisor = NULL;
PFNGLVERTEXARRAYELEMENTBUFFERPROC __glewVertexArrayElementBuffer = NULL;
PFNGLVERTEXARRAYVERTEXBUFFERPROC __glewVertexArrayVertexBuffer = NULL;
PFNGLVERTEXARRAYVERTEXBUFFERSPROC __glewVertexArrayVertexBuffers = NULL;

PFNGLDRAWELEMENTSBASEVERTEXPROC __glewDrawElementsBaseVertex = NULL;
PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC __glewDrawElementsInstancedBaseVertex = NULL;
PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC __glewDrawRangeElementsBaseVertex = NULL;
PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC __glewMultiDrawElementsBaseVertex = NULL;

PFNGLBINDFRAMEBUFFERPROC __glewBindFramebuffer = NULL;
PFNGLBINDRENDERBUFFERPROC __glewBindRenderbuffer = NULL;
PFNGLBLITFRAMEBUFFERPROC __glewBlitFramebuffer = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSPROC __glewCheckFramebufferStatus = NULL;
PFNGLDELETEFRAMEBUFFERSPROC __glewDeleteFramebuffers = NULL;
PFNGLDELETERENDERBUFFERSPROC __glewDeleteRenderbuffers = NULL;
PFNGLFRAMEBUFFERRENDERBUFFERPROC __glewFramebufferRenderbuffer = NULL;
PFNGLFRAMEBUFFERTEXTURE1DPROC __glewFramebufferTexture1D = NULL;
PFNGLFRAMEBUFFERTEXTURE2DPROC __glewFramebufferTexture2D = NULL;
PFNGLFRAMEBUFFERTEXTURE3DPROC __glewFramebufferTexture3D = NULL;
PFNGLFRAMEBUFFERTEXTURELAYERPROC __glewFramebufferTextureLayer = NULL;
PFNGLGENFRAMEBUFFERSPROC __glewGenFramebuffers = NULL;
PFNGLGENRENDERBUFFERSPROC __glewGenRenderbuffers = NULL;
PFNGLGENERATEMIPMAPPROC __glewGenerateMipmap = NULL;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC __glewGetFramebufferAttachmentParameteriv = NULL;
PFNGLGETRENDERBUFFERPARAMETERIVPROC __glewGetRenderbufferParameteriv = NULL;
PFNGLISFRAMEBUFFERPROC __glewIsFramebuffer = NULL;
PFNGLISRENDERBUFFERPROC __glewIsRenderbuffer = NULL;
PFNGLRENDERBUFFERSTORAGEPROC __glewRenderbufferStorage = NULL;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC __glewRenderbufferStorageMultisample = NULL;

PFNGLFRAMEBUFFERTEXTUREARBPROC __glewFramebufferTextureARB = NULL;
PFNGLFRAMEBUFFERTEXTUREFACEARBPROC __glewFramebufferTextureFaceARB = NULL;
PFNGLFRAMEBUFFERTEXTURELAYERARBPROC __glewFramebufferTextureLayerARB = NULL;
PFNGLPROGRAMPARAMETERIARBPROC __glewProgramParameteriARB = NULL;

PFNGLGETPROGRAMBINARYPROC __glewGetProgramBinary = NULL;
PFNGLPROGRAMBINARYPROC __glewProgramBinary = NULL;
PFNGLPROGRAMPARAMETERIPROC __glewProgramParameteri = NULL;

PFNGLCOLORSUBTABLEPROC __glewColorSubTable = NULL;
PFNGLCOLORTABLEPROC __glewColorTable = NULL;
PFNGLCOLORTABLEPARAMETERFVPROC __glewColorTableParameterfv = NULL;
PFNGLCOLORTABLEPARAMETERIVPROC __glewColorTableParameteriv = NULL;
PFNGLCONVOLUTIONFILTER1DPROC __glewConvolutionFilter1D = NULL;
PFNGLCONVOLUTIONFILTER2DPROC __glewConvolutionFilter2D = NULL;
PFNGLCONVOLUTIONPARAMETERFPROC __glewConvolutionParameterf = NULL;
PFNGLCONVOLUTIONPARAMETERFVPROC __glewConvolutionParameterfv = NULL;
PFNGLCONVOLUTIONPARAMETERIPROC __glewConvolutionParameteri = NULL;
PFNGLCONVOLUTIONPARAMETERIVPROC __glewConvolutionParameteriv = NULL;
PFNGLCOPYCOLORSUBTABLEPROC __glewCopyColorSubTable = NULL;
PFNGLCOPYCOLORTABLEPROC __glewCopyColorTable = NULL;
PFNGLCOPYCONVOLUTIONFILTER1DPROC __glewCopyConvolutionFilter1D = NULL;
PFNGLCOPYCONVOLUTIONFILTER2DPROC __glewCopyConvolutionFilter2D = NULL;
PFNGLGETCOLORTABLEPROC __glewGetColorTable = NULL;
PFNGLGETCOLORTABLEPARAMETERFVPROC __glewGetColorTableParameterfv = NULL;
PFNGLGETCOLORTABLEPARAMETERIVPROC __glewGetColorTableParameteriv = NULL;
PFNGLGETCONVOLUTIONFILTERPROC __glewGetConvolutionFilter = NULL;
PFNGLGETCONVOLUTIONPARAMETERFVPROC __glewGetConvolutionParameterfv = NULL;
PFNGLGETCONVOLUTIONPARAMETERIVPROC __glewGetConvolutionParameteriv = NULL;
PFNGLGETHISTOGRAMPROC __glewGetHistogram = NULL;
PFNGLGETHISTOGRAMPARAMETERFVPROC __glewGetHistogramParameterfv = NULL;
PFNGLGETHISTOGRAMPARAMETERIVPROC __glewGetHistogramParameteriv = NULL;
PFNGLGETMINMAXPROC __glewGetMinmax = NULL;
PFNGLGETMINMAXPARAMETERFVPROC __glewGetMinmaxParameterfv = NULL;
PFNGLGETMINMAXPARAMETERIVPROC __glewGetMinmaxParameteriv = NULL;
PFNGLGETSEPARABLEFILTERPROC __glewGetSeparableFilter = NULL;
PFNGLHISTOGRAMPROC __glewHistogram = NULL;
PFNGLMINMAXPROC __glewMinmax = NULL;
PFNGLRESETHISTOGRAMPROC __glewResetHistogram = NULL;
PFNGLRESETMINMAXPROC __glewResetMinmax = NULL;
PFNGLSEPARABLEFILTER2DPROC __glewSeparableFilter2D = NULL;

PFNGLDRAWARRAYSINSTANCEDARBPROC __glewDrawArraysInstancedARB = NULL;
PFNGLDRAWELEMENTSINSTANCEDARBPROC __glewDrawElementsInstancedARB = NULL;
PFNGLVERTEXATTRIBDIVISORARBPROC __glewVertexAttribDivisorARB = NULL;

PFNGLINVALIDATEBUFFERDATAPROC __glewInvalidateBufferData = NULL;
PFNGLINVALIDATEBUFFERSUBDATAPROC __glewInvalidateBufferSubData = NULL;
PFNGLINVALIDATEFRAMEBUFFERPROC __glewInvalidateFramebuffer = NULL;
PFNGLINVALIDATESUBFRAMEBUFFERPROC __glewInvalidateSubFramebuffer = NULL;
PFNGLINVALIDATETEXIMAGEPROC __glewInvalidateTexImage = NULL;
PFNGLINVALIDATETEXSUBIMAGEPROC __glewInvalidateTexSubImage = NULL;

PFNGLFLUSHMAPPEDBUFFERRANGEPROC __glewFlushMappedBufferRange = NULL;
PFNGLMAPBUFFERRANGEPROC __glewMapBufferRange = NULL;

PFNGLBINDBUFFERSBASEPROC __glewBindBuffersBase = NULL;
PFNGLBINDBUFFERSRANGEPROC __glewBindBuffersRange = NULL;
PFNGLBINDIMAGETEXTURESPROC __glewBindImageTextures = NULL;
PFNGLBINDSAMPLERSPROC __glewBindSamplers = NULL;
PFNGLBINDTEXTURESPROC __glewBindTextures = NULL;
PFNGLBINDVERTEXBUFFERSPROC __glewBindVertexBuffers = NULL;

PFNGLMAXSHADERCOMPILERTHREADSARBPROC __glewMaxShaderCompilerThreadsARB = NULL;

PFNGLGETPROGRAMINTERFACEIVPROC __glewGetProgramInterfaceiv = NULL;
PFNGLGETPROGRAMRESOURCEINDEXPROC __glewGetProgramResourceIndex = NULL;
PFNGLGETPROGRAMRESOURCELOCATIONPROC __glewGetProgramResourceLocation = NULL;
PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC __glewGetProgramResourceLocationIndex = NULL;
PFNGLGETPROGRAMRESOURCENAMEPROC __glewGetProgramResourceName = NULL;
PFNGLGETPROGRAMRESOURCEIVPROC __glewGetProgramResourceiv = NULL;

PFNGLPROVOKINGVERTEXPROC __glewProvokingVertex = NULL;

PFNGLGETGRAPHICSRESETSTATUSARBPROC __glewGetGraphicsResetStatusARB = NULL;
PFNGLGETNCOLORTABLEARBPROC __glewGetnColorTableARB = NULL;
PFNGLGETNCOMPRESSEDTEXIMAGEARBPROC __glewGetnCompressedTexImageARB = NULL;
PFNGLGETNCONVOLUTIONFILTERARBPROC __glewGetnConvolutionFilterARB = NULL;
PFNGLGETNHISTOGRAMARBPROC __glewGetnHistogramARB = NULL;
PFNGLGETNMAPDVARBPROC __glewGetnMapdvARB = NULL;
PFNGLGETNMAPFVARBPROC __glewGetnMapfvARB = NULL;
PFNGLGETNMAPIVARBPROC __glewGetnMapivARB = NULL;
PFNGLGETNMINMAXARBPROC __glewGetnMinmaxARB = NULL;
PFNGLGETNPIXELMAPFVARBPROC __glewGetnPixelMapfvARB = NULL;
PFNGLGETNPIXELMAPUIVARBPROC __glewGetnPixelMapuivARB = NULL;
PFNGLGETNPIXELMAPUSVARBPROC __glewGetnPixelMapusvARB = NULL;
PFNGLGETNPOLYGONSTIPPLEARBPROC __glewGetnPolygonStippleARB = NULL;
PFNGLGETNSEPARABLEFILTERARBPROC __glewGetnSeparableFilterARB = NULL;
PFNGLGETNTEXIMAGEARBPROC __glewGetnTexImageARB = NULL;
PFNGLGETNUNIFORMDVARBPROC __glewGetnUniformdvARB = NULL;
PFNGLGETNUNIFORMFVARBPROC __glewGetnUniformfvARB = NULL;
PFNGLGETNUNIFORMIVARBPROC __glewGetnUniformivARB = NULL;
PFNGLGETNUNIFORMUIVARBPROC __glewGetnUniformuivARB = NULL;
PFNGLREADNPIXELSARBPROC __glewReadnPixelsARB = NULL;

PFNGLMINSAMPLESHADINGARBPROC __glewMinSampleShadingARB = NULL;

PFNGLBINDIMAGETEXTUREPROC __glewBindImageTexture = NULL;
PFNGLMEMORYBARRIERPROC __glewMemoryBarrier = NULL;

PFNGLCLIENTWAITSYNCPROC __glewClientWaitSync = NULL;
PFNGLDELETESYNCPROC __glewDeleteSync = NULL;
PFNGLFENCESYNCPROC __glewFenceSync = NULL;
PFNGLGETINTEGER64VPROC __glewGetInteger64v = NULL;
PFNGLGETSYNCIVPROC __glewGetSynciv = NULL;
PFNGLISSYNCPROC __glewIsSync = NULL;
PFNGLWAITSYNCPROC __glewWaitSync = NULL;

PFNGLPATCHPARAMETERFVPROC __glewPatchParameterfv = NULL;
PFNGLPATCHPARAMETERIPROC __glewPatchParameteri = NULL;

PFNGLGETMULTISAMPLEFVPROC __glewGetMultisamplefv = NULL;
PFNGLSAMPLEMASKIPROC __glewSampleMaski = NULL;
PFNGLTEXIMAGE2DMULTISAMPLEPROC __glewTexImage2DMultisample = NULL;
PFNGLTEXIMAGE3DMULTISAMPLEPROC __glewTexImage3DMultisample = NULL;

PFNGLTEXSTORAGE1DPROC __glewTexStorage1D = NULL;
PFNGLTEXSTORAGE2DPROC __glewTexStorage2D = NULL;
PFNGLTEXSTORAGE3DPROC __glewTexStorage3D = NULL;

PFNGLGETQUERYOBJECTI64VPROC __glewGetQueryObjecti64v = NULL;
PFNGLGETQUERYOBJECTUI64VPROC __glewGetQueryObjectui64v = NULL;
PFNGLQUERYCOUNTERPROC __glewQueryCounter = NULL;

PFNGLBINDBUFFERBASEPROC __glewBindBufferBase = NULL;
PFNGLBINDBUFFERRANGEPROC __glewBindBufferRange = NULL;
PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC __glewGetActiveUniformBlockName = NULL;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC __glewGetActiveUniformBlockiv = NULL;
PFNGLGETACTIVEUNIFORMNAMEPROC __glewGetActiveUniformName = NULL;
PFNGLGETACTIVEUNIFORMSIVPROC __glewGetActiveUniformsiv = NULL;
PFNGLGETINTEGERI_VPROC __glewGetIntegeri_v = NULL;
PFNGLGETUNIFORMBLOCKINDEXPROC __glewGetUniformBlockIndex = NULL;
PFNGLGETUNIFORMINDICESPROC __glewGetUniformIndices = NULL;
PFNGLUNIFORMBLOCKBINDINGPROC __glewUniformBlockBinding = NULL;

PFNGLBINDVERTEXARRAYPROC __glewBindVertexArray = NULL;
PFNGLDELETEVERTEXARRAYSPROC __glewDeleteVertexArrays = NULL;
PFNGLGENVERTEXARRAYSPROC __glewGenVertexArrays = NULL;
PFNGLISVERTEXARRAYPROC __glewIsVertexArray = NULL;

PFNGLBINDVERTEXBUFFERPROC __glewBindVertexBuffer = NULL;
PFNGLVERTEXARRAYBINDVERTEXBUFFEREXTPROC __glewVertexArrayBindVertexBufferEXT = NULL;
PFNGLVERTEXARRAYVERTEXATTRIBBINDINGEXTPROC __glewVertexArrayVertexAttribBindingEXT = NULL;
PFNGLVERTEXARRAYVERTEXATTRIBFORMATEXTPROC __glewVertexArrayVertexAttribFormatEXT = NULL;
PFNGLVERTEXARRAYVERTEXATTRIBIFORMATEXTPROC __glewVertexArrayVertexAttribIFormatEXT = NULL;
PFNGLVERTEXARRAYVERTEXATTRIBLFORMATEXTPROC __glewVertexArrayVertexAttribLFormatEXT = NULL;
PFNGLVERTEXARRAYVERTEXBINDINGDIVISOREXTPROC __glewVertexArrayVertexBindingDivisorEXT = NULL;
PFNGLVERTEXATTRIBBINDINGPROC __glewVertexAttribBinding = NULL;
PFNGLVERTEXATTRIBFORMATPROC __glewVertexAttribFormat = NULL;
PFNGLVERTEXATTRIBIFORMATPROC __glewVertexAttribIFormat = NULL;
PFNGLVERTEXATTRIBLFORMATPROC __glewVertexAttribLFormat = NULL;
PFNGLVERTEXBINDINGDIVISORPROC __glewVertexBindingDivisor = NULL;

PFNGLCOLORP3UIPROC __glewColorP3ui = NULL;
PFNGLCOLORP3UIVPROC __glewColorP3uiv = NULL;
PFNGLCOLORP4UIPROC __glewColorP4ui = NULL;
PFNGLCOLORP4UIVPROC __glewColorP4uiv = NULL;
PFNGLMULTITEXCOORDP1UIPROC __glewMultiTexCoordP1ui = NULL;
PFNGLMULTITEXCOORDP1UIVPROC __glewMultiTexCoordP1uiv = NULL;
PFNGLMULTITEXCOORDP2UIPROC __glewMultiTexCoordP2ui = NULL;
PFNGLMULTITEXCOORDP2UIVPROC __glewMultiTexCoordP2uiv = NULL;
PFNGLMULTITEXCOORDP3UIPROC __glewMultiTexCoordP3ui = NULL;
PFNGLMULTITEXCOORDP3UIVPROC __glewMultiTexCoordP3uiv = NULL;
PFNGLMULTITEXCOORDP4UIPROC __glewMultiTexCoordP4ui = NULL;
PFNGLMULTITEXCOORDP4UIVPROC __glewMultiTexCoordP4uiv = NULL;
PFNGLNORMALP3UIPROC __glewNormalP3ui = NULL;
PFNGLNORMALP3UIVPROC __glewNormalP3uiv = NULL;
PFNGLSECONDARYCOLORP3UIPROC __glewSecondaryColorP3ui = NULL;
PFNGLSECONDARYCOLORP3UIVPROC __glewSecondaryColorP3uiv = NULL;
PFNGLTEXCOORDP1UIPROC __glewTexCoordP1ui = NULL;
PFNGLTEXCOORDP1UIVPROC __glewTexCoordP1uiv = NULL;
PFNGLTEXCOORDP2UIPROC __glewTexCoordP2ui = NULL;
PFNGLTEXCOORDP2UIVPROC __glewTexCoordP2uiv = NULL;
PFNGLTEXCOORDP3UIPROC __glewTexCoordP3ui = NULL;
PFNGLTEXCOORDP3UIVPROC __glewTexCoordP3uiv = NULL;
PFNGLTEXCOORDP4UIPROC __glewTexCoordP4ui = NULL;
PFNGLTEXCOORDP4UIVPROC __glewTexCoordP4uiv = NULL;
PFNGLVERTEXATTRIBP1UIPROC __glewVertexAttribP1ui = NULL;
PFNGLVERTEXATTRIBP1UIVPROC __glewVertexAttribP1uiv = NULL;
PFNGLVERTEXATTRIBP2UIPROC __glewVertexAttribP2ui = NULL;
PFNGLVERTEXATTRIBP2UIVPROC __glewVertexAttribP2uiv = NULL;
PFNGLVERTEXATTRIBP3UIPROC __glewVertexAttribP3ui = NULL;
PFNGLVERTEXATTRIBP3UIVPROC __glewVertexAttribP3uiv = NULL;
PFNGLVERTEXATTRIBP4UIPROC __glewVertexAttribP4ui = NULL;
PFNGLVERTEXATTRIBP4UIVPROC __glewVertexAttribP4uiv = NULL;
PFNGLVERTEXP2UIPROC __glewVertexP2ui = NULL;
PFNGLVERTEXP2UIVPROC __glewVertexP2uiv = NULL;
PFNGLVERTEXP3UIPROC __glewVertexP3ui = NULL;
PFNGLVERTEXP3UIVPROC __glewVertexP3uiv = NULL;
PFNGLVERTEXP4UIPROC __glewVertexP4ui = NULL;
PFNGLVERTEXP4UIVPROC __glewVertexP4uiv = NULL;

PFNGLBINDMULTITEXTUREEXTPROC __glewBindMultiTextureEXT = NULL;
PFNGLCHECKNAMEDFRAMEBUFFERSTATUSEXTPROC __glewCheckNamedFramebufferStatusEXT = NULL;
PFNGLCLIENTATTRIBDEFAULTEXTPROC __glewClientAttribDefaultEXT = NULL;
PFNGLCOMPRESSEDMULTITEXIMAGE1DEXTPROC __glewCompressedMultiTexImage1DEXT = NULL;
PFNGLCOMPRESSEDMULTITEXIMAGE2DEXTPROC __glewCompressedMultiTexImage2DEXT = NULL;
PFNGLCOMPRESSEDMULTITEXIMAGE3DEXTPROC __glewCompressedMultiTexImage3DEXT = NULL;
PFNGLCOMPRESSEDMULTITEXSUBIMAGE1DEXTPROC __glewCompressedMultiTexSubImage1DEXT = NULL;
PFNGLCOMPRESSEDMULTITEXSUBIMAGE2DEXTPROC __glewCompressedMultiTexSubImage2DEXT = NULL;
PFNGLCOMPRESSEDMULTITEXSUBIMAGE3DEXTPROC __glewCompressedMultiTexSubImage3DEXT = NULL;
PFNGLCOMPRESSEDTEXTUREIMAGE1DEXTPROC __glewCompressedTextureImage1DEXT = NULL;
PFNGLCOMPRESSEDTEXTUREIMAGE2DEXTPROC __glewCompressedTextureImage2DEXT = NULL;
PFNGLCOMPRESSEDTEXTUREIMAGE3DEXTPROC __glewCompressedTextureImage3DEXT = NULL;
PFNGLCOMPRESSEDTEXTURESUBIMAGE1DEXTPROC __glewCompressedTextureSubImage1DEXT = NULL;
PFNGLCOMPRESSEDTEXTURESUBIMAGE2DEXTPROC __glewCompressedTextureSubImage2DEXT = NULL;
PFNGLCOMPRESSEDTEXTURESUBIMAGE3DEXTPROC __glewCompressedTextureSubImage3DEXT = NULL;
PFNGLCOPYMULTITEXIMAGE1DEXTPROC __glewCopyMultiTexImage1DEXT = NULL;
PFNGLCOPYMULTITEXIMAGE2DEXTPROC __glewCopyMultiTexImage2DEXT = NULL;
PFNGLCOPYMULTITEXSUBIMAGE1DEXTPROC __glewCopyMultiTexSubImage1DEXT = NULL;
PFNGLCOPYMULTITEXSUBIMAGE2DEXTPROC __glewCopyMultiTexSubImage2DEXT = NULL;
PFNGLCOPYMULTITEXSUBIMAGE3DEXTPROC __glewCopyMultiTexSubImage3DEXT = NULL;
PFNGLCOPYTEXTUREIMAGE1DEXTPROC __glewCopyTextureImage1DEXT = NULL;
PFNGLCOPYTEXTUREIMAGE2DEXTPROC __glewCopyTextureImage2DEXT = NULL;
PFNGLCOPYTEXTURESUBIMAGE1DEXTPROC __glewCopyTextureSubImage1DEXT = NULL;
PFNGLCOPYTEXTURESUBIMAGE2DEXTPROC __glewCopyTextureSubImage2DEXT = NULL;
PFNGLCOPYTEXTURESUBIMAGE3DEXTPROC __glewCopyTextureSubImage3DEXT = NULL;
PFNGLDISABLECLIENTSTATEINDEXEDEXTPROC __glewDisableClientStateIndexedEXT = NULL;
PFNGLDISABLECLIENTSTATEIEXTPROC __glewDisableClientStateiEXT = NULL;
PFNGLDISABLEVERTEXARRAYATTRIBEXTPROC __glewDisableVertexArrayAttribEXT = NULL;
PFNGLDISABLEVERTEXARRAYEXTPROC __glewDisableVertexArrayEXT = NULL;
PFNGLENABLECLIENTSTATEINDEXEDEXTPROC __glewEnableClientStateIndexedEXT = NULL;
PFNGLENABLECLIENTSTATEIEXTPROC __glewEnableClientStateiEXT = NULL;
PFNGLENABLEVERTEXARRAYATTRIBEXTPROC __glewEnableVertexArrayAttribEXT = NULL;
PFNGLENABLEVERTEXARRAYEXTPROC __glewEnableVertexArrayEXT = NULL;
PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEEXTPROC __glewFlushMappedNamedBufferRangeEXT = NULL;
PFNGLFRAMEBUFFERDRAWBUFFEREXTPROC __glewFramebufferDrawBufferEXT = NULL;
PFNGLFRAMEBUFFERDRAWBUFFERSEXTPROC __glewFramebufferDrawBuffersEXT = NULL;
PFNGLFRAMEBUFFERREADBUFFEREXTPROC __glewFramebufferReadBufferEXT = NULL;
PFNGLGENERATEMULTITEXMIPMAPEXTPROC __glewGenerateMultiTexMipmapEXT = NULL;
PFNGLGENERATETEXTUREMIPMAPEXTPROC __glewGenerateTextureMipmapEXT = NULL;
PFNGLGETCOMPRESSEDMULTITEXIMAGEEXTPROC __glewGetCompressedMultiTexImageEXT = NULL;
PFNGLGETCOMPRESSEDTEXTUREIMAGEEXTPROC __glewGetCompressedTextureImageEXT = NULL;
PFNGLGETDOUBLEINDEXEDVEXTPROC __glewGetDoubleIndexedvEXT = NULL;
PFNGLGETDOUBLEI_VEXTPROC __glewGetDoublei_vEXT = NULL;
PFNGLGETFLOATINDEXEDVEXTPROC __glewGetFloatIndexedvEXT = NULL;
PFNGLGETFLOATI_VEXTPROC __glewGetFloati_vEXT = NULL;
PFNGLGETFRAMEBUFFERPARAMETERIVEXTPROC __glewGetFramebufferParameterivEXT = NULL;
PFNGLGETMULTITEXENVFVEXTPROC __glewGetMultiTexEnvfvEXT = NULL;
PFNGLGETMULTITEXENVIVEXTPROC __glewGetMultiTexEnvivEXT = NULL;
PFNGLGETMULTITEXGENDVEXTPROC __glewGetMultiTexGendvEXT = NULL;
PFNGLGETMULTITEXGENFVEXTPROC __glewGetMultiTexGenfvEXT = NULL;
PFNGLGETMULTITEXGENIVEXTPROC __glewGetMultiTexGenivEXT = NULL;
PFNGLGETMULTITEXIMAGEEXTPROC __glewGetMultiTexImageEXT = NULL;
PFNGLGETMULTITEXLEVELPARAMETERFVEXTPROC __glewGetMultiTexLevelParameterfvEXT = NULL;
PFNGLGETMULTITEXLEVELPARAMETERIVEXTPROC __glewGetMultiTexLevelParameterivEXT = NULL;
PFNGLGETMULTITEXPARAMETERIIVEXTPROC __glewGetMultiTexParameterIivEXT = NULL;
PFNGLGETMULTITEXPARAMETERIUIVEXTPROC __glewGetMultiTexParameterIuivEXT = NULL;
PFNGLGETMULTITEXPARAMETERFVEXTPROC __glewGetMultiTexParameterfvEXT = NULL;
PFNGLGETMULTITEXPARAMETERIVEXTPROC __glewGetMultiTexParameterivEXT = NULL;
PFNGLGETNAMEDBUFFERPARAMETERIVEXTPROC __glewGetNamedBufferParameterivEXT = NULL;
PFNGLGETNAMEDBUFFERPOINTERVEXTPROC __glewGetNamedBufferPointervEXT = NULL;
PFNGLGETNAMEDBUFFERSUBDATAEXTPROC __glewGetNamedBufferSubDataEXT = NULL;
PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC __glewGetNamedFramebufferAttachmentParameterivEXT = NULL;
PFNGLGETNAMEDPROGRAMLOCALPARAMETERIIVEXTPROC __glewGetNamedProgramLocalParameterIivEXT = NULL;
PFNGLGETNAMEDPROGRAMLOCALPARAMETERIUIVEXTPROC __glewGetNamedProgramLocalParameterIuivEXT = NULL;
PFNGLGETNAMEDPROGRAMLOCALPARAMETERDVEXTPROC __glewGetNamedProgramLocalParameterdvEXT = NULL;
PFNGLGETNAMEDPROGRAMLOCALPARAMETERFVEXTPROC __glewGetNamedProgramLocalParameterfvEXT = NULL;
PFNGLGETNAMEDPROGRAMSTRINGEXTPROC __glewGetNamedProgramStringEXT = NULL;
PFNGLGETNAMEDPROGRAMIVEXTPROC __glewGetNamedProgramivEXT = NULL;
PFNGLGETNAMEDRENDERBUFFERPARAMETERIVEXTPROC __glewGetNamedRenderbufferParameterivEXT = NULL;
PFNGLGETPOINTERINDEXEDVEXTPROC __glewGetPointerIndexedvEXT = NULL;
PFNGLGETPOINTERI_VEXTPROC __glewGetPointeri_vEXT = NULL;
PFNGLGETTEXTUREIMAGEEXTPROC __glewGetTextureImageEXT = NULL;
PFNGLGETTEXTURELEVELPARAMETERFVEXTPROC __glewGetTextureLevelParameterfvEXT = NULL;
PFNGLGETTEXTURELEVELPARAMETERIVEXTPROC __glewGetTextureLevelParameterivEXT = NULL;
PFNGLGETTEXTUREPARAMETERIIVEXTPROC __glewGetTextureParameterIivEXT = NULL;
PFNGLGETTEXTUREPARAMETERIUIVEXTPROC __glewGetTextureParameterIuivEXT = NULL;
PFNGLGETTEXTUREPARAMETERFVEXTPROC __glewGetTextureParameterfvEXT = NULL;
PFNGLGETTEXTUREPARAMETERIVEXTPROC __glewGetTextureParameterivEXT = NULL;
PFNGLGETVERTEXARRAYINTEGERI_VEXTPROC __glewGetVertexArrayIntegeri_vEXT = NULL;
PFNGLGETVERTEXARRAYINTEGERVEXTPROC __glewGetVertexArrayIntegervEXT = NULL;
PFNGLGETVERTEXARRAYPOINTERI_VEXTPROC __glewGetVertexArrayPointeri_vEXT = NULL;
PFNGLGETVERTEXARRAYPOINTERVEXTPROC __glewGetVertexArrayPointervEXT = NULL;
PFNGLMAPNAMEDBUFFEREXTPROC __glewMapNamedBufferEXT = NULL;
PFNGLMAPNAMEDBUFFERRANGEEXTPROC __glewMapNamedBufferRangeEXT = NULL;
PFNGLMATRIXFRUSTUMEXTPROC __glewMatrixFrustumEXT = NULL;
PFNGLMATRIXLOADIDENTITYEXTPROC __glewMatrixLoadIdentityEXT = NULL;
PFNGLMATRIXLOADTRANSPOSEDEXTPROC __glewMatrixLoadTransposedEXT = NULL;
PFNGLMATRIXLOADTRANSPOSEFEXTPROC __glewMatrixLoadTransposefEXT = NULL;
PFNGLMATRIXLOADDEXTPROC __glewMatrixLoaddEXT = NULL;
PFNGLMATRIXLOADFEXTPROC __glewMatrixLoadfEXT = NULL;
PFNGLMATRIXMULTTRANSPOSEDEXTPROC __glewMatrixMultTransposedEXT = NULL;
PFNGLMATRIXMULTTRANSPOSEFEXTPROC __glewMatrixMultTransposefEXT = NULL;
PFNGLMATRIXMULTDEXTPROC __glewMatrixMultdEXT = NULL;
PFNGLMATRIXMULTFEXTPROC __glewMatrixMultfEXT = NULL;
PFNGLMATRIXORTHOEXTPROC __glewMatrixOrthoEXT = NULL;
PFNGLMATRIXPOPEXTPROC __glewMatrixPopEXT = NULL;
PFNGLMATRIXPUSHEXTPROC __glewMatrixPushEXT = NULL;
PFNGLMATRIXROTATEDEXTPROC __glewMatrixRotatedEXT = NULL;
PFNGLMATRIXROTATEFEXTPROC __glewMatrixRotatefEXT = NULL;
PFNGLMATRIXSCALEDEXTPROC __glewMatrixScaledEXT = NULL;
PFNGLMATRIXSCALEFEXTPROC __glewMatrixScalefEXT = NULL;
PFNGLMATRIXTRANSLATEDEXTPROC __glewMatrixTranslatedEXT = NULL;
PFNGLMATRIXTRANSLATEFEXTPROC __glewMatrixTranslatefEXT = NULL;
PFNGLMULTITEXBUFFEREXTPROC __glewMultiTexBufferEXT = NULL;
PFNGLMULTITEXCOORDPOINTEREXTPROC __glewMultiTexCoordPointerEXT = NULL;
PFNGLMULTITEXENVFEXTPROC __glewMultiTexEnvfEXT = NULL;
PFNGLMULTITEXENVFVEXTPROC __glewMultiTexEnvfvEXT = NULL;
PFNGLMULTITEXENVIEXTPROC __glewMultiTexEnviEXT = NULL;
PFNGLMULTITEXENVIVEXTPROC __glewMultiTexEnvivEXT = NULL;
PFNGLMULTITEXGENDEXTPROC __glewMultiTexGendEXT = NULL;
PFNGLMULTITEXGENDVEXTPROC __glewMultiTexGendvEXT = NULL;
PFNGLMULTITEXGENFEXTPROC __glewMultiTexGenfEXT = NULL;
PFNGLMULTITEXGENFVEXTPROC __glewMultiTexGenfvEXT = NULL;
PFNGLMULTITEXGENIEXTPROC __glewMultiTexGeniEXT = NULL;
PFNGLMULTITEXGENIVEXTPROC __glewMultiTexGenivEXT = NULL;
PFNGLMULTITEXIMAGE1DEXTPROC __glewMultiTexImage1DEXT = NULL;
PFNGLMULTITEXIMAGE2DEXTPROC __glewMultiTexImage2DEXT = NULL;
PFNGLMULTITEXIMAGE3DEXTPROC __glewMultiTexImage3DEXT = NULL;
PFNGLMULTITEXPARAMETERIIVEXTPROC __glewMultiTexParameterIivEXT = NULL;
PFNGLMULTITEXPARAMETERIUIVEXTPROC __glewMultiTexParameterIuivEXT = NULL;
PFNGLMULTITEXPARAMETERFEXTPROC __glewMultiTexParameterfEXT = NULL;
PFNGLMULTITEXPARAMETERFVEXTPROC __glewMultiTexParameterfvEXT = NULL;
PFNGLMULTITEXPARAMETERIEXTPROC __glewMultiTexParameteriEXT = NULL;
PFNGLMULTITEXPARAMETERIVEXTPROC __glewMultiTexParameterivEXT = NULL;
PFNGLMULTITEXRENDERBUFFEREXTPROC __glewMultiTexRenderbufferEXT = NULL;
PFNGLMULTITEXSUBIMAGE1DEXTPROC __glewMultiTexSubImage1DEXT = NULL;
PFNGLMULTITEXSUBIMAGE2DEXTPROC __glewMultiTexSubImage2DEXT = NULL;
PFNGLMULTITEXSUBIMAGE3DEXTPROC __glewMultiTexSubImage3DEXT = NULL;
PFNGLNAMEDBUFFERDATAEXTPROC __glewNamedBufferDataEXT = NULL;
PFNGLNAMEDBUFFERSTORAGEEXTPROC __glewNamedBufferStorageEXT = NULL;
PFNGLNAMEDBUFFERSUBDATAEXTPROC __glewNamedBufferSubDataEXT = NULL;
PFNGLNAMEDCOPYBUFFERSUBDATAEXTPROC __glewNamedCopyBufferSubDataEXT = NULL;
PFNGLNAMEDFRAMEBUFFERRENDERBUFFEREXTPROC __glewNamedFramebufferRenderbufferEXT = NULL;
PFNGLNAMEDFRAMEBUFFERTEXTURE1DEXTPROC __glewNamedFramebufferTexture1DEXT = NULL;
PFNGLNAMEDFRAMEBUFFERTEXTURE2DEXTPROC __glewNamedFramebufferTexture2DEXT = NULL;
PFNGLNAMEDFRAMEBUFFERTEXTURE3DEXTPROC __glewNamedFramebufferTexture3DEXT = NULL;
PFNGLNAMEDFRAMEBUFFERTEXTUREEXTPROC __glewNamedFramebufferTextureEXT = NULL;
PFNGLNAMEDFRAMEBUFFERTEXTUREFACEEXTPROC __glewNamedFramebufferTextureFaceEXT = NULL;
PFNGLNAMEDFRAMEBUFFERTEXTURELAYEREXTPROC __glewNamedFramebufferTextureLayerEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETER4DEXTPROC __glewNamedProgramLocalParameter4dEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETER4DVEXTPROC __glewNamedProgramLocalParameter4dvEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETER4FEXTPROC __glewNamedProgramLocalParameter4fEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETER4FVEXTPROC __glewNamedProgramLocalParameter4fvEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETERI4IEXTPROC __glewNamedProgramLocalParameterI4iEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETERI4IVEXTPROC __glewNamedProgramLocalParameterI4ivEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIEXTPROC __glewNamedProgramLocalParameterI4uiEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIVEXTPROC __glewNamedProgramLocalParameterI4uivEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETERS4FVEXTPROC __glewNamedProgramLocalParameters4fvEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETERSI4IVEXTPROC __glewNamedProgramLocalParametersI4ivEXT = NULL;
PFNGLNAMEDPROGRAMLOCALPARAMETERSI4UIVEXTPROC __glewNamedProgramLocalParametersI4uivEXT = NULL;
PFNGLNAMEDPROGRAMSTRINGEXTPROC __glewNamedProgramStringEXT = NULL;
PFNGLNAMEDRENDERBUFFERSTORAGEEXTPROC __glewNamedRenderbufferStorageEXT = NULL;
PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLECOVERAGEEXTPROC __glewNamedRenderbufferStorageMultisampleCoverageEXT = NULL;
PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC __glewNamedRenderbufferStorageMultisampleEXT = NULL;
PFNGLPROGRAMUNIFORM1FEXTPROC __glewProgramUniform1fEXT = NULL;
PFNGLPROGRAMUNIFORM1FVEXTPROC __glewProgramUniform1fvEXT = NULL;
PFNGLPROGRAMUNIFORM1IEXTPROC __glewProgramUniform1iEXT = NULL;
PFNGLPROGRAMUNIFORM1IVEXTPROC __glewProgramUniform1ivEXT = NULL;
PFNGLPROGRAMUNIFORM1UIEXTPROC __glewProgramUniform1uiEXT = NULL;
PFNGLPROGRAMUNIFORM1UIVEXTPROC __glewProgramUniform1uivEXT = NULL;
PFNGLPROGRAMUNIFORM2FEXTPROC __glewProgramUniform2fEXT = NULL;
PFNGLPROGRAMUNIFORM2FVEXTPROC __glewProgramUniform2fvEXT = NULL;
PFNGLPROGRAMUNIFORM2IEXTPROC __glewProgramUniform2iEXT = NULL;
PFNGLPROGRAMUNIFORM2IVEXTPROC __glewProgramUniform2ivEXT = NULL;
PFNGLPROGRAMUNIFORM2UIEXTPROC __glewProgramUniform2uiEXT = NULL;
PFNGLPROGRAMUNIFORM2UIVEXTPROC __glewProgramUniform2uivEXT = NULL;
PFNGLPROGRAMUNIFORM3FEXTPROC __glewProgramUniform3fEXT = NULL;
PFNGLPROGRAMUNIFORM3FVEXTPROC __glewProgramUniform3fvEXT = NULL;
PFNGLPROGRAMUNIFORM3IEXTPROC __glewProgramUniform3iEXT = NULL;
PFNGLPROGRAMUNIFORM3IVEXTPROC __glewProgramUniform3ivEXT = NULL;
PFNGLPROGRAMUNIFORM3UIEXTPROC __glewProgramUniform3uiEXT = NULL;
PFNGLPROGRAMUNIFORM3UIVEXTPROC __glewProgramUniform3uivEXT = NULL;
PFNGLPROGRAMUNIFORM4FEXTPROC __glewProgramUniform4fEXT = NULL;
PFNGLPROGRAMUNIFORM4FVEXTPROC __glewProgramUniform4fvEXT = NULL;
PFNGLPROGRAMUNIFORM4IEXTPROC __glewProgramUniform4iEXT = NULL;
PFNGLPROGRAMUNIFORM4IVEXTPROC __glewProgramUniform4ivEXT = NULL;
PFNGLPROGRAMUNIFORM4UIEXTPROC __glewProgramUniform4uiEXT = NULL;
PFNGLPROGRAMUNIFORM4UIVEXTPROC __glewProgramUniform4uivEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC __glewProgramUniformMatrix2fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC __glewProgramUniformMatrix2x3fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC __glewProgramUniformMatrix2x4fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC __glewProgramUniformMatrix3fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC __glewProgramUniformMatrix3x2fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC __glewProgramUniformMatrix3x4fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC __glewProgramUniformMatrix4fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC __glewProgramUniformMatrix4x2fvEXT = NULL;
PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC __glewProgramUniformMatrix4x3fvEXT = NULL;
PFNGLPUSHCLIENTATTRIBDEFAULTEXTPROC __glewPushClientAttribDefaultEXT = NULL;
PFNGLTEXTUREBUFFEREXTPROC __glewTextureBufferEXT = NULL;
PFNGLTEXTUREIMAGE1DEXTPROC __glewTextureImage1DEXT = NULL;
PFNGLTEXTUREIMAGE2DEXTPROC __glewTextureImage2DEXT = NULL;
PFNGLTEXTUREIMAGE3DEXTPROC __glewTextureImage3DEXT = NULL;
PFNGLTEXTUREPAGECOMMITMENTEXTPROC __glewTexturePageCommitmentEXT = NULL;
PFNGLTEXTUREPARAMETERIIVEXTPROC __glewTextureParameterIivEXT = NULL;
PFNGLTEXTUREPARAMETERIUIVEXTPROC __glewTextureParameterIuivEXT = NULL;
PFNGLTEXTUREPARAMETERFEXTPROC __glewTextureParameterfEXT = NULL;
PFNGLTEXTUREPARAMETERFVEXTPROC __glewTextureParameterfvEXT = NULL;
PFNGLTEXTUREPARAMETERIEXTPROC __glewTextureParameteriEXT = NULL;
PFNGLTEXTUREPARAMETERIVEXTPROC __glewTextureParameterivEXT = NULL;
PFNGLTEXTURERENDERBUFFEREXTPROC __glewTextureRenderbufferEXT = NULL;
PFNGLTEXTURESUBIMAGE1DEXTPROC __glewTextureSubImage1DEXT = NULL;
PFNGLTEXTURESUBIMAGE2DEXTPROC __glewTextureSubImage2DEXT = NULL;
PFNGLTEXTURESUBIMAGE3DEXTPROC __glewTextureSubImage3DEXT = NULL;
PFNGLUNMAPNAMEDBUFFEREXTPROC __glewUnmapNamedBufferEXT = NULL;
PFNGLVERTEXARRAYCOLOROFFSETEXTPROC __glewVertexArrayColorOffsetEXT = NULL;
PFNGLVERTEXARRAYEDGEFLAGOFFSETEXTPROC __glewVertexArrayEdgeFlagOffsetEXT = NULL;
PFNGLVERTEXARRAYFOGCOORDOFFSETEXTPROC __glewVertexArrayFogCoordOffsetEXT = NULL;
PFNGLVERTEXARRAYINDEXOFFSETEXTPROC __glewVertexArrayIndexOffsetEXT = NULL;
PFNGLVERTEXARRAYMULTITEXCOORDOFFSETEXTPROC __glewVertexArrayMultiTexCoordOffsetEXT = NULL;
PFNGLVERTEXARRAYNORMALOFFSETEXTPROC __glewVertexArrayNormalOffsetEXT = NULL;
PFNGLVERTEXARRAYSECONDARYCOLOROFFSETEXTPROC __glewVertexArraySecondaryColorOffsetEXT = NULL;
PFNGLVERTEXARRAYTEXCOORDOFFSETEXTPROC __glewVertexArrayTexCoordOffsetEXT = NULL;
PFNGLVERTEXARRAYVERTEXATTRIBDIVISOREXTPROC __glewVertexArrayVertexAttribDivisorEXT = NULL;
PFNGLVERTEXARRAYVERTEXATTRIBIOFFSETEXTPROC __glewVertexArrayVertexAttribIOffsetEXT = NULL;
PFNGLVERTEXARRAYVERTEXATTRIBOFFSETEXTPROC __glewVertexArrayVertexAttribOffsetEXT = NULL;
PFNGLVERTEXARRAYVERTEXOFFSETEXTPROC __glewVertexArrayVertexOffsetEXT = NULL;

PFNGLBLITFRAMEBUFFEREXTPROC __glewBlitFramebufferEXT = NULL;

PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC __glewRenderbufferStorageMultisampleEXT = NULL;

PFNGLBINDFRAMEBUFFEREXTPROC __glewBindFramebufferEXT = NULL;
PFNGLBINDRENDERBUFFEREXTPROC __glewBindRenderbufferEXT = NULL;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC __glewCheckFramebufferStatusEXT = NULL;
PFNGLDELETEFRAMEBUFFERSEXTPROC __glewDeleteFramebuffersEXT = NULL;
PFNGLDELETERENDERBUFFERSEXTPROC __glewDeleteRenderbuffersEXT = NULL;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC __glewFramebufferRenderbufferEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE1DEXTPROC __glewFramebufferTexture1DEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC __glewFramebufferTexture2DEXT = NULL;
PFNGLFRAMEBUFFERTEXTURE3DEXTPROC __glewFramebufferTexture3DEXT = NULL;
PFNGLGENFRAMEBUFFERSEXTPROC __glewGenFramebuffersEXT = NULL;
PFNGLGENRENDERBUFFERSEXTPROC __glewGenRenderbuffersEXT = NULL;
PFNGLGENERATEMIPMAPEXTPROC __glewGenerateMipmapEXT = NULL;
PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC __glewGetFramebufferAttachmentParameterivEXT = NULL;
PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC __glewGetRenderbufferParameterivEXT = NULL;
PFNGLISFRAMEBUFFEREXTPROC __glewIsFramebufferEXT = NULL;
PFNGLISRENDERBUFFEREXTPROC __glewIsRenderbufferEXT = NULL;
PFNGLRENDERBUFFERSTORAGEEXTPROC __glewRenderbufferStorageEXT = NULL;

PFNGLFRAMEBUFFERTEXTUREEXTPROC __glewFramebufferTextureEXT = NULL;
PFNGLFRAMEBUFFERTEXTUREFACEEXTPROC __glewFramebufferTextureFaceEXT = NULL;
PFNGLPROGRAMPARAMETERIEXTPROC __glewProgramParameteriEXT = NULL;

PFNGLBINDFRAGDATALOCATIONEXTPROC __glewBindFragDataLocationEXT = NULL;
PFNGLGETFRAGDATALOCATIONEXTPROC __glewGetFragDataLocationEXT = NULL;
PFNGLGETUNIFORMUIVEXTPROC __glewGetUniformuivEXT = NULL;
PFNGLGETVERTEXATTRIBIIVEXTPROC __glewGetVertexAttribIivEXT = NULL;
PFNGLGETVERTEXATTRIBIUIVEXTPROC __glewGetVertexAttribIuivEXT = NULL;
PFNGLUNIFORM1UIEXTPROC __glewUniform1uiEXT = NULL;
PFNGLUNIFORM1UIVEXTPROC __glewUniform1uivEXT = NULL;
PFNGLUNIFORM2UIEXTPROC __glewUniform2uiEXT = NULL;
PFNGLUNIFORM2UIVEXTPROC __glewUniform2uivEXT = NULL;
PFNGLUNIFORM3UIEXTPROC __glewUniform3uiEXT = NULL;
PFNGLUNIFORM3UIVEXTPROC __glewUniform3uivEXT = NULL;
PFNGLUNIFORM4UIEXTPROC __glewUniform4uiEXT = NULL;
PFNGLUNIFORM4UIVEXTPROC __glewUniform4uivEXT = NULL;
PFNGLVERTEXATTRIBI1IEXTPROC __glewVertexAttribI1iEXT = NULL;
PFNGLVERTEXATTRIBI1IVEXTPROC __glewVertexAttribI1ivEXT = NULL;
PFNGLVERTEXATTRIBI1UIEXTPROC __glewVertexAttribI1uiEXT = NULL;
PFNGLVERTEXATTRIBI1UIVEXTPROC __glewVertexAttribI1uivEXT = NULL;
PFNGLVERTEXATTRIBI2IEXTPROC __glewVertexAttribI2iEXT = NULL;
PFNGLVERTEXATTRIBI2IVEXTPROC __glewVertexAttribI2ivEXT = NULL;
PFNGLVERTEXATTRIBI2UIEXTPROC __glewVertexAttribI2uiEXT = NULL;
PFNGLVERTEXATTRIBI2UIVEXTPROC __glewVertexAttribI2uivEXT = NULL;
PFNGLVERTEXATTRIBI3IEXTPROC __glewVertexAttribI3iEXT = NULL;
PFNGLVERTEXATTRIBI3IVEXTPROC __glewVertexAttribI3ivEXT = NULL;
PFNGLVERTEXATTRIBI3UIEXTPROC __glewVertexAttribI3uiEXT = NULL;
PFNGLVERTEXATTRIBI3UIVEXTPROC __glewVertexAttribI3uivEXT = NULL;
PFNGLVERTEXATTRIBI4BVEXTPROC __glewVertexAttribI4bvEXT = NULL;
PFNGLVERTEXATTRIBI4IEXTPROC __glewVertexAttribI4iEXT = NULL;
PFNGLVERTEXATTRIBI4IVEXTPROC __glewVertexAttribI4ivEXT = NULL;
PFNGLVERTEXATTRIBI4SVEXTPROC __glewVertexAttribI4svEXT = NULL;
PFNGLVERTEXATTRIBI4UBVEXTPROC __glewVertexAttribI4ubvEXT = NULL;
PFNGLVERTEXATTRIBI4UIEXTPROC __glewVertexAttribI4uiEXT = NULL;
PFNGLVERTEXATTRIBI4UIVEXTPROC __glewVertexAttribI4uivEXT = NULL;
PFNGLVERTEXATTRIBI4USVEXTPROC __glewVertexAttribI4usvEXT = NULL;
PFNGLVERTEXATTRIBIPOINTEREXTPROC __glewVertexAttribIPointerEXT = NULL;

PFNGLBINDIMAGETEXTUREEXTPROC __glewBindImageTextureEXT = NULL;
PFNGLMEMORYBARRIEREXTPROC __glewMemoryBarrierEXT = NULL;

PFNGLTEXSTORAGE1DEXTPROC __glewTexStorage1DEXT = NULL;
PFNGLTEXSTORAGE2DEXTPROC __glewTexStorage2DEXT = NULL;
PFNGLTEXSTORAGE3DEXTPROC __glewTexStorage3DEXT = NULL;
PFNGLTEXTURESTORAGE1DEXTPROC __glewTextureStorage1DEXT = NULL;
PFNGLTEXTURESTORAGE2DEXTPROC __glewTextureStorage2DEXT = NULL;
PFNGLTEXTURESTORAGE3DEXTPROC __glewTextureStorage3DEXT = NULL;

PFNGLAPPLYFRAMEBUFFERATTACHMENTCMAAINTELPROC __glewApplyFramebufferAttachmentCMAAINTEL = NULL;

PFNGLDEBUGMESSAGECALLBACKPROC __glewDebugMessageCallback = NULL;
PFNGLDEBUGMESSAGECONTROLPROC __glewDebugMessageControl = NULL;
PFNGLDEBUGMESSAGEINSERTPROC __glewDebugMessageInsert = NULL;
PFNGLGETDEBUGMESSAGELOGPROC __glewGetDebugMessageLog = NULL;
PFNGLGETOBJECTLABELPROC __glewGetObjectLabel = NULL;
PFNGLGETOBJECTPTRLABELPROC __glewGetObjectPtrLabel = NULL;
PFNGLOBJECTLABELPROC __glewObjectLabel = NULL;
PFNGLOBJECTPTRLABELPROC __glewObjectPtrLabel = NULL;
PFNGLPOPDEBUGGROUPPROC __glewPopDebugGroup = NULL;
PFNGLPUSHDEBUGGROUPPROC __glewPushDebugGroup = NULL;

PFNGLMAXSHADERCOMPILERTHREADSKHRPROC __glewMaxShaderCompilerThreadsKHR = NULL;

PFNGLGETNUNIFORMFVPROC __glewGetnUniformfv = NULL;
PFNGLGETNUNIFORMIVPROC __glewGetnUniformiv = NULL;
PFNGLGETNUNIFORMUIVPROC __glewGetnUniformuiv = NULL;
PFNGLREADNPIXELSPROC __glewReadnPixels = NULL;

PFNGLSUBPIXELPRECISIONBIASNVPROC __glewSubpixelPrecisionBiasNV = NULL;

PFNGLCOPYIMAGESUBDATANVPROC __glewCopyImageSubDataNV = NULL;

PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC __glewRenderbufferStorageMultisampleCoverageNV = NULL;

PFNGLGETUNIFORMI64VNVPROC __glewGetUniformi64vNV = NULL;
PFNGLGETUNIFORMUI64VNVPROC __glewGetUniformui64vNV = NULL;
PFNGLPROGRAMUNIFORM1I64NVPROC __glewProgramUniform1i64NV = NULL;
PFNGLPROGRAMUNIFORM1I64VNVPROC __glewProgramUniform1i64vNV = NULL;
PFNGLPROGRAMUNIFORM1UI64NVPROC __glewProgramUniform1ui64NV = NULL;
PFNGLPROGRAMUNIFORM1UI64VNVPROC __glewProgramUniform1ui64vNV = NULL;
PFNGLPROGRAMUNIFORM2I64NVPROC __glewProgramUniform2i64NV = NULL;
PFNGLPROGRAMUNIFORM2I64VNVPROC __glewProgramUniform2i64vNV = NULL;
PFNGLPROGRAMUNIFORM2UI64NVPROC __glewProgramUniform2ui64NV = NULL;
PFNGLPROGRAMUNIFORM2UI64VNVPROC __glewProgramUniform2ui64vNV = NULL;
PFNGLPROGRAMUNIFORM3I64NVPROC __glewProgramUniform3i64NV = NULL;
PFNGLPROGRAMUNIFORM3I64VNVPROC __glewProgramUniform3i64vNV = NULL;
PFNGLPROGRAMUNIFORM3UI64NVPROC __glewProgramUniform3ui64NV = NULL;
PFNGLPROGRAMUNIFORM3UI64VNVPROC __glewProgramUniform3ui64vNV = NULL;
PFNGLPROGRAMUNIFORM4I64NVPROC __glewProgramUniform4i64NV = NULL;
PFNGLPROGRAMUNIFORM4I64VNVPROC __glewProgramUniform4i64vNV = NULL;
PFNGLPROGRAMUNIFORM4UI64NVPROC __glewProgramUniform4ui64NV = NULL;
PFNGLPROGRAMUNIFORM4UI64VNVPROC __glewProgramUniform4ui64vNV = NULL;
PFNGLUNIFORM1I64NVPROC __glewUniform1i64NV = NULL;
PFNGLUNIFORM1I64VNVPROC __glewUniform1i64vNV = NULL;
PFNGLUNIFORM1UI64NVPROC __glewUniform1ui64NV = NULL;
PFNGLUNIFORM1UI64VNVPROC __glewUniform1ui64vNV = NULL;
PFNGLUNIFORM2I64NVPROC __glewUniform2i64NV = NULL;
PFNGLUNIFORM2I64VNVPROC __glewUniform2i64vNV = NULL;
PFNGLUNIFORM2UI64NVPROC __glewUniform2ui64NV = NULL;
PFNGLUNIFORM2UI64VNVPROC __glewUniform2ui64vNV = NULL;
PFNGLUNIFORM3I64NVPROC __glewUniform3i64NV = NULL;
PFNGLUNIFORM3I64VNVPROC __glewUniform3i64vNV = NULL;
PFNGLUNIFORM3UI64NVPROC __glewUniform3ui64NV = NULL;
PFNGLUNIFORM3UI64VNVPROC __glewUniform3ui64vNV = NULL;
PFNGLUNIFORM4I64NVPROC __glewUniform4i64NV = NULL;
PFNGLUNIFORM4I64VNVPROC __glewUniform4i64vNV = NULL;
PFNGLUNIFORM4UI64NVPROC __glewUniform4ui64NV = NULL;
PFNGLUNIFORM4UI64VNVPROC __glewUniform4ui64vNV = NULL;

GLboolean __GLEW_VERSION_1_1 = GL_FALSE;
GLboolean __GLEW_VERSION_1_2 = GL_FALSE;
GLboolean __GLEW_VERSION_1_2_1 = GL_FALSE;
GLboolean __GLEW_VERSION_1_3 = GL_FALSE;
GLboolean __GLEW_VERSION_1_4 = GL_FALSE;
GLboolean __GLEW_VERSION_1_5 = GL_FALSE;
GLboolean __GLEW_VERSION_2_0 = GL_FALSE;
GLboolean __GLEW_VERSION_2_1 = GL_FALSE;
GLboolean __GLEW_VERSION_3_0 = GL_FALSE;
GLboolean __GLEW_VERSION_3_1 = GL_FALSE;
GLboolean __GLEW_VERSION_3_2 = GL_FALSE;
GLboolean __GLEW_VERSION_3_3 = GL_FALSE;
GLboolean __GLEW_VERSION_4_0 = GL_FALSE;
GLboolean __GLEW_VERSION_4_1 = GL_FALSE;
GLboolean __GLEW_VERSION_4_2 = GL_FALSE;
GLboolean __GLEW_VERSION_4_3 = GL_FALSE;
GLboolean __GLEW_VERSION_4_4 = GL_FALSE;
GLboolean __GLEW_VERSION_4_5 = GL_FALSE;
GLboolean __GLEW_VERSION_4_6 = GL_FALSE;
GLboolean __GLEW_AMD_conservative_depth = GL_FALSE;
GLboolean __GLEW_AMD_framebuffer_multisample_advanced = GL_FALSE;
GLboolean __GLEW_AMD_gpu_shader_half_float = GL_FALSE;
GLboolean __GLEW_AMD_shader_trinary_minmax = GL_FALSE;
GLboolean __GLEW_ARB_buffer_storage = GL_FALSE;
GLboolean __GLEW_ARB_clear_buffer_object = GL_FALSE;
GLboolean __GLEW_ARB_clear_texture = GL_FALSE;
GLboolean __GLEW_ARB_compute_shader = GL_FALSE;
GLboolean __GLEW_ARB_conservative_depth = GL_FALSE;
GLboolean __GLEW_ARB_copy_buffer = GL_FALSE;
GLboolean __GLEW_ARB_copy_image = GL_FALSE;
GLboolean __GLEW_ARB_depth_buffer_float = GL_FALSE;
GLboolean __GLEW_ARB_depth_clamp = GL_FALSE;
GLboolean __GLEW_ARB_direct_state_access = GL_FALSE;
GLboolean __GLEW_ARB_draw_elements_base_vertex = GL_FALSE;
GLboolean __GLEW_ARB_enhanced_layouts = GL_FALSE;
GLboolean __GLEW_ARB_framebuffer_object = GL_FALSE;
GLboolean __GLEW_ARB_framebuffer_sRGB = GL_FALSE;
GLboolean __GLEW_ARB_geometry_shader4 = GL_FALSE;
GLboolean __GLEW_ARB_get_program_binary = GL_FALSE;
GLboolean __GLEW_ARB_gpu_shader5 = GL_FALSE;
GLboolean __GLEW_ARB_half_float_vertex = GL_FALSE;
GLboolean __GLEW_ARB_imaging = GL_FALSE;
GLboolean __GLEW_ARB_instanced_arrays = GL_FALSE;
GLboolean __GLEW_ARB_invalidate_subdata = GL_FALSE;
GLboolean __GLEW_ARB_map_buffer_range = GL_FALSE;
GLboolean __GLEW_ARB_multi_bind = GL_FALSE;
GLboolean __GLEW_ARB_parallel_shader_compile = GL_FALSE;
GLboolean __GLEW_ARB_pipeline_statistics_query = GL_FALSE;
GLboolean __GLEW_ARB_pixel_buffer_object = GL_FALSE;
GLboolean __GLEW_ARB_program_interface_query = GL_FALSE;
GLboolean __GLEW_ARB_provoking_vertex = GL_FALSE;
GLboolean __GLEW_ARB_robustness = GL_FALSE;
GLboolean __GLEW_ARB_sample_shading = GL_FALSE;
GLboolean __GLEW_ARB_seamless_cube_map = GL_FALSE;
GLboolean __GLEW_ARB_shader_group_vote = GL_FALSE;
GLboolean __GLEW_ARB_shader_image_load_store = GL_FALSE;
GLboolean __GLEW_ARB_shader_texture_lod = GL_FALSE;
GLboolean __GLEW_ARB_shading_language_packing = GL_FALSE;
GLboolean __GLEW_ARB_sync = GL_FALSE;
GLboolean __GLEW_ARB_tessellation_shader = GL_FALSE;
GLboolean __GLEW_ARB_texture_compression_bptc = GL_FALSE;
GLboolean __GLEW_ARB_texture_compression_rgtc = GL_FALSE;
GLboolean __GLEW_ARB_texture_filter_anisotropic = GL_FALSE;
GLboolean __GLEW_ARB_texture_float = GL_FALSE;
GLboolean __GLEW_ARB_texture_multisample = GL_FALSE;
GLboolean __GLEW_ARB_texture_rg = GL_FALSE;
GLboolean __GLEW_ARB_texture_stencil8 = GL_FALSE;
GLboolean __GLEW_ARB_texture_storage = GL_FALSE;
GLboolean __GLEW_ARB_timer_query = GL_FALSE;
GLboolean __GLEW_ARB_uniform_buffer_object = GL_FALSE;
GLboolean __GLEW_ARB_vertex_array_object = GL_FALSE;
GLboolean __GLEW_ARB_vertex_attrib_binding = GL_FALSE;
GLboolean __GLEW_ARB_vertex_type_2_10_10_10_rev = GL_FALSE;
GLboolean __GLEW_EXT_demote_to_helper_invocation = GL_FALSE;
GLboolean __GLEW_EXT_direct_state_access = GL_FALSE;
GLboolean __GLEW_EXT_framebuffer_blit = GL_FALSE;
GLboolean __GLEW_EXT_framebuffer_multisample = GL_FALSE;
GLboolean __GLEW_EXT_framebuffer_object = GL_FALSE;
GLboolean __GLEW_EXT_framebuffer_sRGB = GL_FALSE;
GLboolean __GLEW_EXT_geometry_shader4 = GL_FALSE;
GLboolean __GLEW_EXT_gpu_shader4 = GL_FALSE;
GLboolean __GLEW_EXT_packed_depth_stencil = GL_FALSE;
GLboolean __GLEW_EXT_packed_float = GL_FALSE;
GLboolean __GLEW_EXT_pixel_buffer_object = GL_FALSE;
GLboolean __GLEW_EXT_shader_image_load_store = GL_FALSE;
GLboolean __GLEW_EXT_texture_compression_rgtc = GL_FALSE;
GLboolean __GLEW_EXT_texture_compression_s3tc = GL_FALSE;
GLboolean __GLEW_EXT_texture_filter_anisotropic = GL_FALSE;
GLboolean __GLEW_EXT_texture_storage = GL_FALSE;
GLboolean __GLEW_INTEL_conservative_rasterization = GL_FALSE;
GLboolean __GLEW_INTEL_framebuffer_CMAA = GL_FALSE;
GLboolean __GLEW_KHR_debug = GL_FALSE;
GLboolean __GLEW_KHR_no_error = GL_FALSE;
GLboolean __GLEW_KHR_parallel_shader_compile = GL_FALSE;
GLboolean __GLEW_KHR_robustness = GL_FALSE;
GLboolean __GLEW_MESA_pack_invert = GL_FALSE;
GLboolean __GLEW_NVX_gpu_memory_info = GL_FALSE;
GLboolean __GLEW_NV_conservative_raster = GL_FALSE;
GLboolean __GLEW_NV_copy_image = GL_FALSE;
GLboolean __GLEW_NV_depth_clamp = GL_FALSE;
GLboolean __GLEW_NV_framebuffer_multisample_coverage = GL_FALSE;
GLboolean __GLEW_NV_gpu_shader5 = GL_FALSE;
GLboolean __GLEW_NV_multisample_filter_hint = GL_FALSE;

static const char * _glewExtensionLookup[] = {
#ifdef GL_AMD_conservative_depth
  "GL_AMD_conservative_depth",
#endif
#ifdef GL_AMD_framebuffer_multisample_advanced
  "GL_AMD_framebuffer_multisample_advanced",
#endif
#ifdef GL_AMD_gpu_shader_half_float
  "GL_AMD_gpu_shader_half_float",
#endif
#ifdef GL_AMD_shader_trinary_minmax
  "GL_AMD_shader_trinary_minmax",
#endif
#ifdef GL_ARB_buffer_storage
  "GL_ARB_buffer_storage",
#endif
#ifdef GL_ARB_clear_buffer_object
  "GL_ARB_clear_buffer_object",
#endif
#ifdef GL_ARB_clear_texture
  "GL_ARB_clear_texture",
#endif
#ifdef GL_ARB_compute_shader
  "GL_ARB_compute_shader",
#endif
#ifdef GL_ARB_conservative_depth
  "GL_ARB_conservative_depth",
#endif
#ifdef GL_ARB_copy_buffer
  "GL_ARB_copy_buffer",
#endif
#ifdef GL_ARB_copy_image
  "GL_ARB_copy_image",
#endif
#ifdef GL_ARB_depth_buffer_float
  "GL_ARB_depth_buffer_float",
#endif
#ifdef GL_ARB_depth_clamp
  "GL_ARB_depth_clamp",
#endif
#ifdef GL_ARB_direct_state_access
  "GL_ARB_direct_state_access",
#endif
#ifdef GL_ARB_draw_elements_base_vertex
  "GL_ARB_draw_elements_base_vertex",
#endif
#ifdef GL_ARB_enhanced_layouts
  "GL_ARB_enhanced_layouts",
#endif
#ifdef GL_ARB_framebuffer_object
  "GL_ARB_framebuffer_object",
#endif
#ifdef GL_ARB_framebuffer_sRGB
  "GL_ARB_framebuffer_sRGB",
#endif
#ifdef GL_ARB_geometry_shader4
  "GL_ARB_geometry_shader4",
#endif
#ifdef GL_ARB_get_program_binary
  "GL_ARB_get_program_binary",
#endif
#ifdef GL_ARB_gpu_shader5
  "GL_ARB_gpu_shader5",
#endif
#ifdef GL_ARB_half_float_vertex
  "GL_ARB_half_float_vertex",
#endif
#ifdef GL_ARB_imaging
  "GL_ARB_imaging",
#endif
#ifdef GL_ARB_instanced_arrays
  "GL_ARB_instanced_arrays",
#endif
#ifdef GL_ARB_invalidate_subdata
  "GL_ARB_invalidate_subdata",
#endif
#ifdef GL_ARB_map_buffer_range
  "GL_ARB_map_buffer_range",
#endif
#ifdef GL_ARB_multi_bind
  "GL_ARB_multi_bind",
#endif
#ifdef GL_ARB_parallel_shader_compile
  "GL_ARB_parallel_shader_compile",
#endif
#ifdef GL_ARB_pipeline_statistics_query
  "GL_ARB_pipeline_statistics_query",
#endif
#ifdef GL_ARB_pixel_buffer_object
  "GL_ARB_pixel_buffer_object",
#endif
#ifdef GL_ARB_program_interface_query
  "GL_ARB_program_interface_query",
#endif
#ifdef GL_ARB_provoking_vertex
  "GL_ARB_provoking_vertex",
#endif
#ifdef GL_ARB_robustness
  "GL_ARB_robustness",
#endif
#ifdef GL_ARB_sample_shading
  "GL_ARB_sample_shading",
#endif
#ifdef GL_ARB_seamless_cube_map
  "GL_ARB_seamless_cube_map",
#endif
#ifdef GL_ARB_shader_group_vote
  "GL_ARB_shader_group_vote",
#endif
#ifdef GL_ARB_shader_image_load_store
  "GL_ARB_shader_image_load_store",
#endif
#ifdef GL_ARB_shader_texture_lod
  "GL_ARB_shader_texture_lod",
#endif
#ifdef GL_ARB_shading_language_packing
  "GL_ARB_shading_language_packing",
#endif
#ifdef GL_ARB_sync
  "GL_ARB_sync",
#endif
#ifdef GL_ARB_tessellation_shader
  "GL_ARB_tessellation_shader",
#endif
#ifdef GL_ARB_texture_compression_bptc
  "GL_ARB_texture_compression_bptc",
#endif
#ifdef GL_ARB_texture_compression_rgtc
  "GL_ARB_texture_compression_rgtc",
#endif
#ifdef GL_ARB_texture_filter_anisotropic
  "GL_ARB_texture_filter_anisotropic",
#endif
#ifdef GL_ARB_texture_float
  "GL_ARB_texture_float",
#endif
#ifdef GL_ARB_texture_multisample
  "GL_ARB_texture_multisample",
#endif
#ifdef GL_ARB_texture_rg
  "GL_ARB_texture_rg",
#endif
#ifdef GL_ARB_texture_stencil8
  "GL_ARB_texture_stencil8",
#endif
#ifdef GL_ARB_texture_storage
  "GL_ARB_texture_storage",
#endif
#ifdef GL_ARB_timer_query
  "GL_ARB_timer_query",
#endif
#ifdef GL_ARB_uniform_buffer_object
  "GL_ARB_uniform_buffer_object",
#endif
#ifdef GL_ARB_vertex_array_object
  "GL_ARB_vertex_array_object",
#endif
#ifdef GL_ARB_vertex_attrib_binding
  "GL_ARB_vertex_attrib_binding",
#endif
#ifdef GL_ARB_vertex_type_2_10_10_10_rev
  "GL_ARB_vertex_type_2_10_10_10_rev",
#endif
#ifdef GL_EXT_demote_to_helper_invocation
  "GL_EXT_demote_to_helper_invocation",
#endif
#ifdef GL_EXT_direct_state_access
  "GL_EXT_direct_state_access",
#endif
#ifdef GL_EXT_framebuffer_blit
  "GL_EXT_framebuffer_blit",
#endif
#ifdef GL_EXT_framebuffer_multisample
  "GL_EXT_framebuffer_multisample",
#endif
#ifdef GL_EXT_framebuffer_object
  "GL_EXT_framebuffer_object",
#endif
#ifdef GL_EXT_framebuffer_sRGB
  "GL_EXT_framebuffer_sRGB",
#endif
#ifdef GL_EXT_geometry_shader4
  "GL_EXT_geometry_shader4",
#endif
#ifdef GL_EXT_gpu_shader4
  "GL_EXT_gpu_shader4",
#endif
#ifdef GL_EXT_packed_depth_stencil
  "GL_EXT_packed_depth_stencil",
#endif
#ifdef GL_EXT_packed_float
  "GL_EXT_packed_float",
#endif
#ifdef GL_EXT_pixel_buffer_object
  "GL_EXT_pixel_buffer_object",
#endif
#ifdef GL_EXT_shader_image_load_store
  "GL_EXT_shader_image_load_store",
#endif
#ifdef GL_EXT_texture_compression_rgtc
  "GL_EXT_texture_compression_rgtc",
#endif
#ifdef GL_EXT_texture_compression_s3tc
  "GL_EXT_texture_compression_s3tc",
#endif
#ifdef GL_EXT_texture_filter_anisotropic
  "GL_EXT_texture_filter_anisotropic",
#endif
#ifdef GL_EXT_texture_storage
  "GL_EXT_texture_storage",
#endif
#ifdef GL_INTEL_conservative_rasterization
  "GL_INTEL_conservative_rasterization",
#endif
#ifdef GL_INTEL_framebuffer_CMAA
  "GL_INTEL_framebuffer_CMAA",
#endif
#ifdef GL_KHR_debug
  "GL_KHR_debug",
#endif
#ifdef GL_KHR_no_error
  "GL_KHR_no_error",
#endif
#ifdef GL_KHR_parallel_shader_compile
  "GL_KHR_parallel_shader_compile",
#endif
#ifdef GL_KHR_robustness
  "GL_KHR_robustness",
#endif
#ifdef GL_MESA_pack_invert
  "GL_MESA_pack_invert",
#endif
#ifdef GL_NVX_gpu_memory_info
  "GL_NVX_gpu_memory_info",
#endif
#ifdef GL_NV_conservative_raster
  "GL_NV_conservative_raster",
#endif
#ifdef GL_NV_copy_image
  "GL_NV_copy_image",
#endif
#ifdef GL_NV_depth_clamp
  "GL_NV_depth_clamp",
#endif
#ifdef GL_NV_framebuffer_multisample_coverage
  "GL_NV_framebuffer_multisample_coverage",
#endif
#ifdef GL_NV_gpu_shader5
  "GL_NV_gpu_shader5",
#endif
#ifdef GL_NV_multisample_filter_hint
  "GL_NV_multisample_filter_hint",
#endif
#ifdef GL_VERSION_1_2
  "GL_VERSION_1_2",
#endif
#ifdef GL_VERSION_1_2_1
  "GL_VERSION_1_2_1",
#endif
#ifdef GL_VERSION_1_3
  "GL_VERSION_1_3",
#endif
#ifdef GL_VERSION_1_4
  "GL_VERSION_1_4",
#endif
#ifdef GL_VERSION_1_5
  "GL_VERSION_1_5",
#endif
#ifdef GL_VERSION_2_0
  "GL_VERSION_2_0",
#endif
#ifdef GL_VERSION_2_1
  "GL_VERSION_2_1",
#endif
#ifdef GL_VERSION_3_0
  "GL_VERSION_3_0",
#endif
#ifdef GL_VERSION_3_1
  "GL_VERSION_3_1",
#endif
#ifdef GL_VERSION_3_2
  "GL_VERSION_3_2",
#endif
#ifdef GL_VERSION_3_3
  "GL_VERSION_3_3",
#endif
#ifdef GL_VERSION_4_0
  "GL_VERSION_4_0",
#endif
#ifdef GL_VERSION_4_1
  "GL_VERSION_4_1",
#endif
#ifdef GL_VERSION_4_2
  "GL_VERSION_4_2",
#endif
#ifdef GL_VERSION_4_3
  "GL_VERSION_4_3",
#endif
#ifdef GL_VERSION_4_4
  "GL_VERSION_4_4",
#endif
#ifdef GL_VERSION_4_5
  "GL_VERSION_4_5",
#endif
#ifdef GL_VERSION_4_6
  "GL_VERSION_4_6",
#endif
  NULL
};

/* Detected in the extension string or strings */
static GLboolean _glewExtensionString[1];

/* Detected via extension string or experimental mode */
static GLboolean* _glewExtensionEnabled[] = {
#ifdef GL_AMD_conservative_depth
  &__GLEW_AMD_conservative_depth,
#endif
#ifdef GL_AMD_framebuffer_multisample_advanced
  &__GLEW_AMD_framebuffer_multisample_advanced,
#endif
#ifdef GL_AMD_gpu_shader_half_float
  &__GLEW_AMD_gpu_shader_half_float,
#endif
#ifdef GL_AMD_shader_trinary_minmax
  &__GLEW_AMD_shader_trinary_minmax,
#endif
#ifdef GL_ARB_buffer_storage
  &__GLEW_ARB_buffer_storage,
#endif
#ifdef GL_ARB_clear_buffer_object
  &__GLEW_ARB_clear_buffer_object,
#endif
#ifdef GL_ARB_clear_texture
  &__GLEW_ARB_clear_texture,
#endif
#ifdef GL_ARB_compute_shader
  &__GLEW_ARB_compute_shader,
#endif
#ifdef GL_ARB_conservative_depth
  &__GLEW_ARB_conservative_depth,
#endif
#ifdef GL_ARB_copy_buffer
  &__GLEW_ARB_copy_buffer,
#endif
#ifdef GL_ARB_copy_image
  &__GLEW_ARB_copy_image,
#endif
#ifdef GL_ARB_depth_buffer_float
  &__GLEW_ARB_depth_buffer_float,
#endif
#ifdef GL_ARB_depth_clamp
  &__GLEW_ARB_depth_clamp,
#endif
#ifdef GL_ARB_direct_state_access
  &__GLEW_ARB_direct_state_access,
#endif
#ifdef GL_ARB_draw_elements_base_vertex
  &__GLEW_ARB_draw_elements_base_vertex,
#endif
#ifdef GL_ARB_enhanced_layouts
  &__GLEW_ARB_enhanced_layouts,
#endif
#ifdef GL_ARB_framebuffer_object
  &__GLEW_ARB_framebuffer_object,
#endif
#ifdef GL_ARB_framebuffer_sRGB
  &__GLEW_ARB_framebuffer_sRGB,
#endif
#ifdef GL_ARB_geometry_shader4
  &__GLEW_ARB_geometry_shader4,
#endif
#ifdef GL_ARB_get_program_binary
  &__GLEW_ARB_get_program_binary,
#endif
#ifdef GL_ARB_gpu_shader5
  &__GLEW_ARB_gpu_shader5,
#endif
#ifdef GL_ARB_half_float_vertex
  &__GLEW_ARB_half_float_vertex,
#endif
#ifdef GL_ARB_imaging
  &__GLEW_ARB_imaging,
#endif
#ifdef GL_ARB_instanced_arrays
  &__GLEW_ARB_instanced_arrays,
#endif
#ifdef GL_ARB_invalidate_subdata
  &__GLEW_ARB_invalidate_subdata,
#endif
#ifdef GL_ARB_map_buffer_range
  &__GLEW_ARB_map_buffer_range,
#endif
#ifdef GL_ARB_multi_bind
  &__GLEW_ARB_multi_bind,
#endif
#ifdef GL_ARB_parallel_shader_compile
  &__GLEW_ARB_parallel_shader_compile,
#endif
#ifdef GL_ARB_pipeline_statistics_query
  &__GLEW_ARB_pipeline_statistics_query,
#endif
#ifdef GL_ARB_pixel_buffer_object
  &__GLEW_ARB_pixel_buffer_object,
#endif
#ifdef GL_ARB_program_interface_query
  &__GLEW_ARB_program_interface_query,
#endif
#ifdef GL_ARB_provoking_vertex
  &__GLEW_ARB_provoking_vertex,
#endif
#ifdef GL_ARB_robustness
  &__GLEW_ARB_robustness,
#endif
#ifdef GL_ARB_sample_shading
  &__GLEW_ARB_sample_shading,
#endif
#ifdef GL_ARB_seamless_cube_map
  &__GLEW_ARB_seamless_cube_map,
#endif
#ifdef GL_ARB_shader_group_vote
  &__GLEW_ARB_shader_group_vote,
#endif
#ifdef GL_ARB_shader_image_load_store
  &__GLEW_ARB_shader_image_load_store,
#endif
#ifdef GL_ARB_shader_texture_lod
  &__GLEW_ARB_shader_texture_lod,
#endif
#ifdef GL_ARB_shading_language_packing
  &__GLEW_ARB_shading_language_packing,
#endif
#ifdef GL_ARB_sync
  &__GLEW_ARB_sync,
#endif
#ifdef GL_ARB_tessellation_shader
  &__GLEW_ARB_tessellation_shader,
#endif
#ifdef GL_ARB_texture_compression_bptc
  &__GLEW_ARB_texture_compression_bptc,
#endif
#ifdef GL_ARB_texture_compression_rgtc
  &__GLEW_ARB_texture_compression_rgtc,
#endif
#ifdef GL_ARB_texture_filter_anisotropic
  &__GLEW_ARB_texture_filter_anisotropic,
#endif
#ifdef GL_ARB_texture_float
  &__GLEW_ARB_texture_float,
#endif
#ifdef GL_ARB_texture_multisample
  &__GLEW_ARB_texture_multisample,
#endif
#ifdef GL_ARB_texture_rg
  &__GLEW_ARB_texture_rg,
#endif
#ifdef GL_ARB_texture_stencil8
  &__GLEW_ARB_texture_stencil8,
#endif
#ifdef GL_ARB_texture_storage
  &__GLEW_ARB_texture_storage,
#endif
#ifdef GL_ARB_timer_query
  &__GLEW_ARB_timer_query,
#endif
#ifdef GL_ARB_uniform_buffer_object
  &__GLEW_ARB_uniform_buffer_object,
#endif
#ifdef GL_ARB_vertex_array_object
  &__GLEW_ARB_vertex_array_object,
#endif
#ifdef GL_ARB_vertex_attrib_binding
  &__GLEW_ARB_vertex_attrib_binding,
#endif
#ifdef GL_ARB_vertex_type_2_10_10_10_rev
  &__GLEW_ARB_vertex_type_2_10_10_10_rev,
#endif
#ifdef GL_EXT_demote_to_helper_invocation
  &__GLEW_EXT_demote_to_helper_invocation,
#endif
#ifdef GL_EXT_direct_state_access
  &__GLEW_EXT_direct_state_access,
#endif
#ifdef GL_EXT_framebuffer_blit
  &__GLEW_EXT_framebuffer_blit,
#endif
#ifdef GL_EXT_framebuffer_multisample
  &__GLEW_EXT_framebuffer_multisample,
#endif
#ifdef GL_EXT_framebuffer_object
  &__GLEW_EXT_framebuffer_object,
#endif
#ifdef GL_EXT_framebuffer_sRGB
  &__GLEW_EXT_framebuffer_sRGB,
#endif
#ifdef GL_EXT_geometry_shader4
  &__GLEW_EXT_geometry_shader4,
#endif
#ifdef GL_EXT_gpu_shader4
  &__GLEW_EXT_gpu_shader4,
#endif
#ifdef GL_EXT_packed_depth_stencil
  &__GLEW_EXT_packed_depth_stencil,
#endif
#ifdef GL_EXT_packed_float
  &__GLEW_EXT_packed_float,
#endif
#ifdef GL_EXT_pixel_buffer_object
  &__GLEW_EXT_pixel_buffer_object,
#endif
#ifdef GL_EXT_shader_image_load_store
  &__GLEW_EXT_shader_image_load_store,
#endif
#ifdef GL_EXT_texture_compression_rgtc
  &__GLEW_EXT_texture_compression_rgtc,
#endif
#ifdef GL_EXT_texture_compression_s3tc
  &__GLEW_EXT_texture_compression_s3tc,
#endif
#ifdef GL_EXT_texture_filter_anisotropic
  &__GLEW_EXT_texture_filter_anisotropic,
#endif
#ifdef GL_EXT_texture_storage
  &__GLEW_EXT_texture_storage,
#endif
#ifdef GL_INTEL_conservative_rasterization
  &__GLEW_INTEL_conservative_rasterization,
#endif
#ifdef GL_INTEL_framebuffer_CMAA
  &__GLEW_INTEL_framebuffer_CMAA,
#endif
#ifdef GL_KHR_debug
  &__GLEW_KHR_debug,
#endif
#ifdef GL_KHR_no_error
  &__GLEW_KHR_no_error,
#endif
#ifdef GL_KHR_parallel_shader_compile
  &__GLEW_KHR_parallel_shader_compile,
#endif
#ifdef GL_KHR_robustness
  &__GLEW_KHR_robustness,
#endif
#ifdef GL_MESA_pack_invert
  &__GLEW_MESA_pack_invert,
#endif
#ifdef GL_NVX_gpu_memory_info
  &__GLEW_NVX_gpu_memory_info,
#endif
#ifdef GL_NV_conservative_raster
  &__GLEW_NV_conservative_raster,
#endif
#ifdef GL_NV_copy_image
  &__GLEW_NV_copy_image,
#endif
#ifdef GL_NV_depth_clamp
  &__GLEW_NV_depth_clamp,
#endif
#ifdef GL_NV_framebuffer_multisample_coverage
  &__GLEW_NV_framebuffer_multisample_coverage,
#endif
#ifdef GL_NV_gpu_shader5
  &__GLEW_NV_gpu_shader5,
#endif
#ifdef GL_NV_multisample_filter_hint
  &__GLEW_NV_multisample_filter_hint,
#endif
#ifdef GL_VERSION_1_2
  &__GLEW_VERSION_1_2,
#endif
#ifdef GL_VERSION_1_2_1
  &__GLEW_VERSION_1_2_1,
#endif
#ifdef GL_VERSION_1_3
  &__GLEW_VERSION_1_3,
#endif
#ifdef GL_VERSION_1_4
  &__GLEW_VERSION_1_4,
#endif
#ifdef GL_VERSION_1_5
  &__GLEW_VERSION_1_5,
#endif
#ifdef GL_VERSION_2_0
  &__GLEW_VERSION_2_0,
#endif
#ifdef GL_VERSION_2_1
  &__GLEW_VERSION_2_1,
#endif
#ifdef GL_VERSION_3_0
  &__GLEW_VERSION_3_0,
#endif
#ifdef GL_VERSION_3_1
  &__GLEW_VERSION_3_1,
#endif
#ifdef GL_VERSION_3_2
  &__GLEW_VERSION_3_2,
#endif
#ifdef GL_VERSION_3_3
  &__GLEW_VERSION_3_3,
#endif
#ifdef GL_VERSION_4_0
  &__GLEW_VERSION_4_0,
#endif
#ifdef GL_VERSION_4_1
  &__GLEW_VERSION_4_1,
#endif
#ifdef GL_VERSION_4_2
  &__GLEW_VERSION_4_2,
#endif
#ifdef GL_VERSION_4_3
  &__GLEW_VERSION_4_3,
#endif
#ifdef GL_VERSION_4_4
  &__GLEW_VERSION_4_4,
#endif
#ifdef GL_VERSION_4_5
  &__GLEW_VERSION_4_5,
#endif
#ifdef GL_VERSION_4_6
  &__GLEW_VERSION_4_6,
#endif
  NULL
};

static GLboolean _glewInit_GL_VERSION_1_2 (void);
static GLboolean _glewInit_GL_VERSION_1_3 (void);
static GLboolean _glewInit_GL_VERSION_1_4 (void);
static GLboolean _glewInit_GL_VERSION_1_5 (void);
static GLboolean _glewInit_GL_VERSION_2_0 (void);
static GLboolean _glewInit_GL_VERSION_2_1 (void);
static GLboolean _glewInit_GL_VERSION_3_0 (void);
static GLboolean _glewInit_GL_VERSION_3_1 (void);
static GLboolean _glewInit_GL_VERSION_3_2 (void);
static GLboolean _glewInit_GL_VERSION_3_3 (void);
static GLboolean _glewInit_GL_VERSION_4_0 (void);
static GLboolean _glewInit_GL_VERSION_4_1 (void);
static GLboolean _glewInit_GL_VERSION_4_2 (void);
static GLboolean _glewInit_GL_VERSION_4_3 (void);
static GLboolean _glewInit_GL_VERSION_4_4 (void);
static GLboolean _glewInit_GL_VERSION_4_5 (void);
static GLboolean _glewInit_GL_VERSION_4_6 (void);
static GLboolean _glewInit_GL_AMD_framebuffer_multisample_advanced (void);
static GLboolean _glewInit_GL_ARB_buffer_storage (void);
static GLboolean _glewInit_GL_ARB_clear_buffer_object (void);
static GLboolean _glewInit_GL_ARB_clear_texture (void);
static GLboolean _glewInit_GL_ARB_compute_shader (void);
static GLboolean _glewInit_GL_ARB_copy_buffer (void);
static GLboolean _glewInit_GL_ARB_copy_image (void);
static GLboolean _glewInit_GL_ARB_direct_state_access (void);
static GLboolean _glewInit_GL_ARB_draw_elements_base_vertex (void);
static GLboolean _glewInit_GL_ARB_framebuffer_object (void);
static GLboolean _glewInit_GL_ARB_geometry_shader4 (void);
static GLboolean _glewInit_GL_ARB_get_program_binary (void);
static GLboolean _glewInit_GL_ARB_imaging (void);
static GLboolean _glewInit_GL_ARB_instanced_arrays (void);
static GLboolean _glewInit_GL_ARB_invalidate_subdata (void);
static GLboolean _glewInit_GL_ARB_map_buffer_range (void);
static GLboolean _glewInit_GL_ARB_multi_bind (void);
static GLboolean _glewInit_GL_ARB_parallel_shader_compile (void);
static GLboolean _glewInit_GL_ARB_program_interface_query (void);
static GLboolean _glewInit_GL_ARB_provoking_vertex (void);
static GLboolean _glewInit_GL_ARB_robustness (void);
static GLboolean _glewInit_GL_ARB_sample_shading (void);
static GLboolean _glewInit_GL_ARB_shader_image_load_store (void);
static GLboolean _glewInit_GL_ARB_sync (void);
static GLboolean _glewInit_GL_ARB_tessellation_shader (void);
static GLboolean _glewInit_GL_ARB_texture_multisample (void);
static GLboolean _glewInit_GL_ARB_texture_storage (void);
static GLboolean _glewInit_GL_ARB_timer_query (void);
static GLboolean _glewInit_GL_ARB_uniform_buffer_object (void);
static GLboolean _glewInit_GL_ARB_vertex_array_object (void);
static GLboolean _glewInit_GL_ARB_vertex_attrib_binding (void);
static GLboolean _glewInit_GL_ARB_vertex_type_2_10_10_10_rev (void);
static GLboolean _glewInit_GL_EXT_direct_state_access (void);
static GLboolean _glewInit_GL_EXT_framebuffer_blit (void);
static GLboolean _glewInit_GL_EXT_framebuffer_multisample (void);
static GLboolean _glewInit_GL_EXT_framebuffer_object (void);
static GLboolean _glewInit_GL_EXT_geometry_shader4 (void);
static GLboolean _glewInit_GL_EXT_gpu_shader4 (void);
static GLboolean _glewInit_GL_EXT_shader_image_load_store (void);
static GLboolean _glewInit_GL_EXT_texture_storage (void);
static GLboolean _glewInit_GL_INTEL_framebuffer_CMAA (void);
static GLboolean _glewInit_GL_KHR_debug (void);
static GLboolean _glewInit_GL_KHR_parallel_shader_compile (void);
static GLboolean _glewInit_GL_KHR_robustness (void);
static GLboolean _glewInit_GL_NV_conservative_raster (void);
static GLboolean _glewInit_GL_NV_copy_image (void);
static GLboolean _glewInit_GL_NV_framebuffer_multisample_coverage (void);
static GLboolean _glewInit_GL_NV_gpu_shader5 (void);

#ifdef GL_VERSION_1_2

static GLboolean _glewInit_GL_VERSION_1_2 (void)
{
  GLboolean r = GL_FALSE;

#if !defined(GLEW_STATIC_MINIMUM)

  r = ((glCopyTexSubImage3D = (PFNGLCOPYTEXSUBIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glCopyTexSubImage3D")) == NULL) || r;
  r = ((glDrawRangeElements = (PFNGLDRAWRANGEELEMENTSPROC)glewGetProcAddress((const GLubyte*)"glDrawRangeElements")) == NULL) || r;
  r = ((glTexImage3D = (PFNGLTEXIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glTexImage3D")) == NULL) || r;
  r = ((glTexSubImage3D = (PFNGLTEXSUBIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glTexSubImage3D")) == NULL) || r;

#endif

  return r;
}

#endif /* GL_VERSION_1_2 */

#ifdef GL_VERSION_1_3

static GLboolean _glewInit_GL_VERSION_1_3 (void)
{
  GLboolean r = GL_FALSE;

#if !defined(GLEW_STATIC_MINIMUM)

  r = ((glActiveTexture = (PFNGLACTIVETEXTUREPROC)glewGetProcAddress((const GLubyte*)"glActiveTexture")) == NULL) || r;
  r = ((glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)glewGetProcAddress((const GLubyte*)"glClientActiveTexture")) == NULL) || r;
  r = ((glCompressedTexImage1D = (PFNGLCOMPRESSEDTEXIMAGE1DPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexImage1D")) == NULL) || r;
  r = ((glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexImage2D")) == NULL) || r;
  r = ((glCompressedTexImage3D = (PFNGLCOMPRESSEDTEXIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexImage3D")) == NULL) || r;
  r = ((glCompressedTexSubImage1D = (PFNGLCOMPRESSEDTEXSUBIMAGE1DPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexSubImage1D")) == NULL) || r;
  r = ((glCompressedTexSubImage2D = (PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexSubImage2D")) == NULL) || r;
  r = ((glCompressedTexSubImage3D = (PFNGLCOMPRESSEDTEXSUBIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glCompressedTexSubImage3D")) == NULL) || r;
  r = ((glGetCompressedTexImage = (PFNGLGETCOMPRESSEDTEXIMAGEPROC)glewGetProcAddress((const GLubyte*)"glGetCompressedTexImage")) == NULL) || r;
  r = ((glLoadTransposeMatrixd = (PFNGLLOADTRANSPOSEMATRIXDPROC)glewGetProcAddress((const GLubyte*)"glLoadTransposeMatrixd")) == NULL) || r;
  r = ((glLoadTransposeMatrixf = (PFNGLLOADTRANSPOSEMATRIXFPROC)glewGetProcAddress((const GLubyte*)"glLoadTransposeMatrixf")) == NULL) || r;
  r = ((glMultTransposeMatrixd = (PFNGLMULTTRANSPOSEMATRIXDPROC)glewGetProcAddress((const GLubyte*)"glMultTransposeMatrixd")) == NULL) || r;
  r = ((glMultTransposeMatrixf = (PFNGLMULTTRANSPOSEMATRIXFPROC)glewGetProcAddress((const GLubyte*)"glMultTransposeMatrixf")) == NULL) || r;
  r = ((glMultiTexCoord1d = (PFNGLMULTITEXCOORD1DPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1d")) == NULL) || r;
  r = ((glMultiTexCoord1dv = (PFNGLMULTITEXCOORD1DVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1dv")) == NULL) || r;
  r = ((glMultiTexCoord1f = (PFNGLMULTITEXCOORD1FPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1f")) == NULL) || r;
  r = ((glMultiTexCoord1fv = (PFNGLMULTITEXCOORD1FVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1fv")) == NULL) || r;
  r = ((glMultiTexCoord1i = (PFNGLMULTITEXCOORD1IPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1i")) == NULL) || r;
  r = ((glMultiTexCoord1iv = (PFNGLMULTITEXCOORD1IVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1iv")) == NULL) || r;
  r = ((glMultiTexCoord1s = (PFNGLMULTITEXCOORD1SPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1s")) == NULL) || r;
  r = ((glMultiTexCoord1sv = (PFNGLMULTITEXCOORD1SVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord1sv")) == NULL) || r;
  r = ((glMultiTexCoord2d = (PFNGLMULTITEXCOORD2DPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2d")) == NULL) || r;
  r = ((glMultiTexCoord2dv = (PFNGLMULTITEXCOORD2DVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2dv")) == NULL) || r;
  r = ((glMultiTexCoord2f = (PFNGLMULTITEXCOORD2FPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2f")) == NULL) || r;
  r = ((glMultiTexCoord2fv = (PFNGLMULTITEXCOORD2FVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2fv")) == NULL) || r;
  r = ((glMultiTexCoord2i = (PFNGLMULTITEXCOORD2IPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2i")) == NULL) || r;
  r = ((glMultiTexCoord2iv = (PFNGLMULTITEXCOORD2IVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2iv")) == NULL) || r;
  r = ((glMultiTexCoord2s = (PFNGLMULTITEXCOORD2SPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2s")) == NULL) || r;
  r = ((glMultiTexCoord2sv = (PFNGLMULTITEXCOORD2SVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord2sv")) == NULL) || r;
  r = ((glMultiTexCoord3d = (PFNGLMULTITEXCOORD3DPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3d")) == NULL) || r;
  r = ((glMultiTexCoord3dv = (PFNGLMULTITEXCOORD3DVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3dv")) == NULL) || r;
  r = ((glMultiTexCoord3f = (PFNGLMULTITEXCOORD3FPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3f")) == NULL) || r;
  r = ((glMultiTexCoord3fv = (PFNGLMULTITEXCOORD3FVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3fv")) == NULL) || r;
  r = ((glMultiTexCoord3i = (PFNGLMULTITEXCOORD3IPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3i")) == NULL) || r;
  r = ((glMultiTexCoord3iv = (PFNGLMULTITEXCOORD3IVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3iv")) == NULL) || r;
  r = ((glMultiTexCoord3s = (PFNGLMULTITEXCOORD3SPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3s")) == NULL) || r;
  r = ((glMultiTexCoord3sv = (PFNGLMULTITEXCOORD3SVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord3sv")) == NULL) || r;
  r = ((glMultiTexCoord4d = (PFNGLMULTITEXCOORD4DPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4d")) == NULL) || r;
  r = ((glMultiTexCoord4dv = (PFNGLMULTITEXCOORD4DVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4dv")) == NULL) || r;
  r = ((glMultiTexCoord4f = (PFNGLMULTITEXCOORD4FPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4f")) == NULL) || r;
  r = ((glMultiTexCoord4fv = (PFNGLMULTITEXCOORD4FVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4fv")) == NULL) || r;
  r = ((glMultiTexCoord4i = (PFNGLMULTITEXCOORD4IPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4i")) == NULL) || r;
  r = ((glMultiTexCoord4iv = (PFNGLMULTITEXCOORD4IVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4iv")) == NULL) || r;
  r = ((glMultiTexCoord4s = (PFNGLMULTITEXCOORD4SPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4s")) == NULL) || r;
  r = ((glMultiTexCoord4sv = (PFNGLMULTITEXCOORD4SVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoord4sv")) == NULL) || r;
  r = ((glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC)glewGetProcAddress((const GLubyte*)"glSampleCoverage")) == NULL) || r;

#endif

  return r;
}

#endif /* GL_VERSION_1_3 */

#ifdef GL_VERSION_1_4

static GLboolean _glewInit_GL_VERSION_1_4 (void)
{
  GLboolean r = GL_FALSE;

#if !defined(GLEW_STATIC_MINIMUM)

  r = ((glBlendColor = (PFNGLBLENDCOLORPROC)glewGetProcAddress((const GLubyte*)"glBlendColor")) == NULL) || r;
  r = ((glBlendEquation = (PFNGLBLENDEQUATIONPROC)glewGetProcAddress((const GLubyte*)"glBlendEquation")) == NULL) || r;
  r = ((glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC)glewGetProcAddress((const GLubyte*)"glBlendFuncSeparate")) == NULL) || r;
  r = ((glFogCoordPointer = (PFNGLFOGCOORDPOINTERPROC)glewGetProcAddress((const GLubyte*)"glFogCoordPointer")) == NULL) || r;
  r = ((glFogCoordd = (PFNGLFOGCOORDDPROC)glewGetProcAddress((const GLubyte*)"glFogCoordd")) == NULL) || r;
  r = ((glFogCoorddv = (PFNGLFOGCOORDDVPROC)glewGetProcAddress((const GLubyte*)"glFogCoorddv")) == NULL) || r;
  r = ((glFogCoordf = (PFNGLFOGCOORDFPROC)glewGetProcAddress((const GLubyte*)"glFogCoordf")) == NULL) || r;
  r = ((glFogCoordfv = (PFNGLFOGCOORDFVPROC)glewGetProcAddress((const GLubyte*)"glFogCoordfv")) == NULL) || r;
  r = ((glMultiDrawArrays = (PFNGLMULTIDRAWARRAYSPROC)glewGetProcAddress((const GLubyte*)"glMultiDrawArrays")) == NULL) || r;
  r = ((glMultiDrawElements = (PFNGLMULTIDRAWELEMENTSPROC)glewGetProcAddress((const GLubyte*)"glMultiDrawElements")) == NULL) || r;
  r = ((glPointParameterf = (PFNGLPOINTPARAMETERFPROC)glewGetProcAddress((const GLubyte*)"glPointParameterf")) == NULL) || r;
  r = ((glPointParameterfv = (PFNGLPOINTPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glPointParameterfv")) == NULL) || r;
  r = ((glPointParameteri = (PFNGLPOINTPARAMETERIPROC)glewGetProcAddress((const GLubyte*)"glPointParameteri")) == NULL) || r;
  r = ((glPointParameteriv = (PFNGLPOINTPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glPointParameteriv")) == NULL) || r;
  r = ((glSecondaryColor3b = (PFNGLSECONDARYCOLOR3BPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3b")) == NULL) || r;
  r = ((glSecondaryColor3bv = (PFNGLSECONDARYCOLOR3BVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3bv")) == NULL) || r;
  r = ((glSecondaryColor3d = (PFNGLSECONDARYCOLOR3DPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3d")) == NULL) || r;
  r = ((glSecondaryColor3dv = (PFNGLSECONDARYCOLOR3DVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3dv")) == NULL) || r;
  r = ((glSecondaryColor3f = (PFNGLSECONDARYCOLOR3FPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3f")) == NULL) || r;
  r = ((glSecondaryColor3fv = (PFNGLSECONDARYCOLOR3FVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3fv")) == NULL) || r;
  r = ((glSecondaryColor3i = (PFNGLSECONDARYCOLOR3IPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3i")) == NULL) || r;
  r = ((glSecondaryColor3iv = (PFNGLSECONDARYCOLOR3IVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3iv")) == NULL) || r;
  r = ((glSecondaryColor3s = (PFNGLSECONDARYCOLOR3SPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3s")) == NULL) || r;
  r = ((glSecondaryColor3sv = (PFNGLSECONDARYCOLOR3SVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3sv")) == NULL) || r;
  r = ((glSecondaryColor3ub = (PFNGLSECONDARYCOLOR3UBPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3ub")) == NULL) || r;
  r = ((glSecondaryColor3ubv = (PFNGLSECONDARYCOLOR3UBVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3ubv")) == NULL) || r;
  r = ((glSecondaryColor3ui = (PFNGLSECONDARYCOLOR3UIPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3ui")) == NULL) || r;
  r = ((glSecondaryColor3uiv = (PFNGLSECONDARYCOLOR3UIVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3uiv")) == NULL) || r;
  r = ((glSecondaryColor3us = (PFNGLSECONDARYCOLOR3USPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3us")) == NULL) || r;
  r = ((glSecondaryColor3usv = (PFNGLSECONDARYCOLOR3USVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColor3usv")) == NULL) || r;
  r = ((glSecondaryColorPointer = (PFNGLSECONDARYCOLORPOINTERPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColorPointer")) == NULL) || r;
  r = ((glWindowPos2d = (PFNGLWINDOWPOS2DPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2d")) == NULL) || r;
  r = ((glWindowPos2dv = (PFNGLWINDOWPOS2DVPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2dv")) == NULL) || r;
  r = ((glWindowPos2f = (PFNGLWINDOWPOS2FPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2f")) == NULL) || r;
  r = ((glWindowPos2fv = (PFNGLWINDOWPOS2FVPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2fv")) == NULL) || r;
  r = ((glWindowPos2i = (PFNGLWINDOWPOS2IPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2i")) == NULL) || r;
  r = ((glWindowPos2iv = (PFNGLWINDOWPOS2IVPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2iv")) == NULL) || r;
  r = ((glWindowPos2s = (PFNGLWINDOWPOS2SPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2s")) == NULL) || r;
  r = ((glWindowPos2sv = (PFNGLWINDOWPOS2SVPROC)glewGetProcAddress((const GLubyte*)"glWindowPos2sv")) == NULL) || r;
  r = ((glWindowPos3d = (PFNGLWINDOWPOS3DPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3d")) == NULL) || r;
  r = ((glWindowPos3dv = (PFNGLWINDOWPOS3DVPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3dv")) == NULL) || r;
  r = ((glWindowPos3f = (PFNGLWINDOWPOS3FPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3f")) == NULL) || r;
  r = ((glWindowPos3fv = (PFNGLWINDOWPOS3FVPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3fv")) == NULL) || r;
  r = ((glWindowPos3i = (PFNGLWINDOWPOS3IPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3i")) == NULL) || r;
  r = ((glWindowPos3iv = (PFNGLWINDOWPOS3IVPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3iv")) == NULL) || r;
  r = ((glWindowPos3s = (PFNGLWINDOWPOS3SPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3s")) == NULL) || r;
  r = ((glWindowPos3sv = (PFNGLWINDOWPOS3SVPROC)glewGetProcAddress((const GLubyte*)"glWindowPos3sv")) == NULL) || r;

#endif

  return r;
}

#endif /* GL_VERSION_1_4 */

#ifdef GL_VERSION_1_5

static GLboolean _glewInit_GL_VERSION_1_5 (void)
{
  GLboolean r = GL_FALSE;

#if !defined(GLEW_STATIC_MINIMUM)

  r = ((glBeginQuery = (PFNGLBEGINQUERYPROC)glewGetProcAddress((const GLubyte*)"glBeginQuery")) == NULL) || r;
  r = ((glBindBuffer = (PFNGLBINDBUFFERPROC)glewGetProcAddress((const GLubyte*)"glBindBuffer")) == NULL) || r;
  r = ((glBufferData = (PFNGLBUFFERDATAPROC)glewGetProcAddress((const GLubyte*)"glBufferData")) == NULL) || r;
  r = ((glBufferSubData = (PFNGLBUFFERSUBDATAPROC)glewGetProcAddress((const GLubyte*)"glBufferSubData")) == NULL) || r;
  r = ((glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glDeleteBuffers")) == NULL) || r;
  r = ((glDeleteQueries = (PFNGLDELETEQUERIESPROC)glewGetProcAddress((const GLubyte*)"glDeleteQueries")) == NULL) || r;
  r = ((glEndQuery = (PFNGLENDQUERYPROC)glewGetProcAddress((const GLubyte*)"glEndQuery")) == NULL) || r;
  r = ((glGenBuffers = (PFNGLGENBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glGenBuffers")) == NULL) || r;
  r = ((glGenQueries = (PFNGLGENQUERIESPROC)glewGetProcAddress((const GLubyte*)"glGenQueries")) == NULL) || r;
  r = ((glGetBufferParameteriv = (PFNGLGETBUFFERPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetBufferParameteriv")) == NULL) || r;
  r = ((glGetBufferPointerv = (PFNGLGETBUFFERPOINTERVPROC)glewGetProcAddress((const GLubyte*)"glGetBufferPointerv")) == NULL) || r;
  r = ((glGetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC)glewGetProcAddress((const GLubyte*)"glGetBufferSubData")) == NULL) || r;
  r = ((glGetQueryObjectiv = (PFNGLGETQUERYOBJECTIVPROC)glewGetProcAddress((const GLubyte*)"glGetQueryObjectiv")) == NULL) || r;
  r = ((glGetQueryObjectuiv = (PFNGLGETQUERYOBJECTUIVPROC)glewGetProcAddress((const GLubyte*)"glGetQueryObjectuiv")) == NULL) || r;
  r = ((glGetQueryiv = (PFNGLGETQUERYIVPROC)glewGetProcAddress((const GLubyte*)"glGetQueryiv")) == NULL) || r;
  r = ((glIsBuffer = (PFNGLISBUFFERPROC)glewGetProcAddress((const GLubyte*)"glIsBuffer")) == NULL) || r;
  r = ((glIsQuery = (PFNGLISQUERYPROC)glewGetProcAddress((const GLubyte*)"glIsQuery")) == NULL) || r;
  r = ((glMapBuffer = (PFNGLMAPBUFFERPROC)glewGetProcAddress((const GLubyte*)"glMapBuffer")) == NULL) || r;
  r = ((glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)glewGetProcAddress((const GLubyte*)"glUnmapBuffer")) == NULL) || r;

#endif

  return r;
}

#endif /* GL_VERSION_1_5 */

#ifdef GL_VERSION_2_0

static GLboolean _glewInit_GL_VERSION_2_0 (void)
{
  GLboolean r = GL_FALSE;

#if !defined(GLEW_STATIC_MINIMUM)

  r = ((glAttachShader = (PFNGLATTACHSHADERPROC)glewGetProcAddress((const GLubyte*)"glAttachShader")) == NULL) || r;
  r = ((glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)glewGetProcAddress((const GLubyte*)"glBindAttribLocation")) == NULL) || r;
  r = ((glBlendEquationSeparate = (PFNGLBLENDEQUATIONSEPARATEPROC)glewGetProcAddress((const GLubyte*)"glBlendEquationSeparate")) == NULL) || r;
  r = ((glCompileShader = (PFNGLCOMPILESHADERPROC)glewGetProcAddress((const GLubyte*)"glCompileShader")) == NULL) || r;
  r = ((glCreateProgram = (PFNGLCREATEPROGRAMPROC)glewGetProcAddress((const GLubyte*)"glCreateProgram")) == NULL) || r;
  r = ((glCreateShader = (PFNGLCREATESHADERPROC)glewGetProcAddress((const GLubyte*)"glCreateShader")) == NULL) || r;
  r = ((glDeleteProgram = (PFNGLDELETEPROGRAMPROC)glewGetProcAddress((const GLubyte*)"glDeleteProgram")) == NULL) || r;
  r = ((glDeleteShader = (PFNGLDELETESHADERPROC)glewGetProcAddress((const GLubyte*)"glDeleteShader")) == NULL) || r;
  r = ((glDetachShader = (PFNGLDETACHSHADERPROC)glewGetProcAddress((const GLubyte*)"glDetachShader")) == NULL) || r;
  r = ((glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC)glewGetProcAddress((const GLubyte*)"glDisableVertexAttribArray")) == NULL) || r;
  r = ((glDrawBuffers = (PFNGLDRAWBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glDrawBuffers")) == NULL) || r;
  r = ((glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glewGetProcAddress((const GLubyte*)"glEnableVertexAttribArray")) == NULL) || r;
  r = ((glGetActiveAttrib = (PFNGLGETACTIVEATTRIBPROC)glewGetProcAddress((const GLubyte*)"glGetActiveAttrib")) == NULL) || r;
  r = ((glGetActiveUniform = (PFNGLGETACTIVEUNIFORMPROC)glewGetProcAddress((const GLubyte*)"glGetActiveUniform")) == NULL) || r;
  r = ((glGetAttachedShaders = (PFNGLGETATTACHEDSHADERSPROC)glewGetProcAddress((const GLubyte*)"glGetAttachedShaders")) == NULL) || r;
  r = ((glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC)glewGetProcAddress((const GLubyte*)"glGetAttribLocation")) == NULL) || r;
  r = ((glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)glewGetProcAddress((const GLubyte*)"glGetProgramInfoLog")) == NULL) || r;
  r = ((glGetProgramiv = (PFNGLGETPROGRAMIVPROC)glewGetProcAddress((const GLubyte*)"glGetProgramiv")) == NULL) || r;
  r = ((glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)glewGetProcAddress((const GLubyte*)"glGetShaderInfoLog")) == NULL) || r;
  r = ((glGetShaderSource = (PFNGLGETSHADERSOURCEPROC)glewGetProcAddress((const GLubyte*)"glGetShaderSource")) == NULL) || r;
  r = ((glGetShaderiv = (PFNGLGETSHADERIVPROC)glewGetProcAddress((const GLubyte*)"glGetShaderiv")) == NULL) || r;
  r = ((glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)glewGetProcAddress((const GLubyte*)"glGetUniformLocation")) == NULL) || r;
  r = ((glGetUniformfv = (PFNGLGETUNIFORMFVPROC)glewGetProcAddress((const GLubyte*)"glGetUniformfv")) == NULL) || r;
  r = ((glGetUniformiv = (PFNGLGETUNIFORMIVPROC)glewGetProcAddress((const GLubyte*)"glGetUniformiv")) == NULL) || r;
  r = ((glGetVertexAttribPointerv = (PFNGLGETVERTEXATTRIBPOINTERVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribPointerv")) == NULL) || r;
  r = ((glGetVertexAttribdv = (PFNGLGETVERTEXATTRIBDVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribdv")) == NULL) || r;
  r = ((glGetVertexAttribfv = (PFNGLGETVERTEXATTRIBFVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribfv")) == NULL) || r;
  r = ((glGetVertexAttribiv = (PFNGLGETVERTEXATTRIBIVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribiv")) == NULL) || r;
  r = ((glIsProgram = (PFNGLISPROGRAMPROC)glewGetProcAddress((const GLubyte*)"glIsProgram")) == NULL) || r;
  r = ((glIsShader = (PFNGLISSHADERPROC)glewGetProcAddress((const GLubyte*)"glIsShader")) == NULL) || r;
  r = ((glLinkProgram = (PFNGLLINKPROGRAMPROC)glewGetProcAddress((const GLubyte*)"glLinkProgram")) == NULL) || r;
  r = ((glShaderSource = (PFNGLSHADERSOURCEPROC)glewGetProcAddress((const GLubyte*)"glShaderSource")) == NULL) || r;
  r = ((glStencilFuncSeparate = (PFNGLSTENCILFUNCSEPARATEPROC)glewGetProcAddress((const GLubyte*)"glStencilFuncSeparate")) == NULL) || r;
  r = ((glStencilMaskSeparate = (PFNGLSTENCILMASKSEPARATEPROC)glewGetProcAddress((const GLubyte*)"glStencilMaskSeparate")) == NULL) || r;
  r = ((glStencilOpSeparate = (PFNGLSTENCILOPSEPARATEPROC)glewGetProcAddress((const GLubyte*)"glStencilOpSeparate")) == NULL) || r;
  r = ((glUniform1f = (PFNGLUNIFORM1FPROC)glewGetProcAddress((const GLubyte*)"glUniform1f")) == NULL) || r;
  r = ((glUniform1fv = (PFNGLUNIFORM1FVPROC)glewGetProcAddress((const GLubyte*)"glUniform1fv")) == NULL) || r;
  r = ((glUniform1i = (PFNGLUNIFORM1IPROC)glewGetProcAddress((const GLubyte*)"glUniform1i")) == NULL) || r;
  r = ((glUniform1iv = (PFNGLUNIFORM1IVPROC)glewGetProcAddress((const GLubyte*)"glUniform1iv")) == NULL) || r;
  r = ((glUniform2f = (PFNGLUNIFORM2FPROC)glewGetProcAddress((const GLubyte*)"glUniform2f")) == NULL) || r;
  r = ((glUniform2fv = (PFNGLUNIFORM2FVPROC)glewGetProcAddress((const GLubyte*)"glUniform2fv")) == NULL) || r;
  r = ((glUniform2i = (PFNGLUNIFORM2IPROC)glewGetProcAddress((const GLubyte*)"glUniform2i")) == NULL) || r;
  r = ((glUniform2iv = (PFNGLUNIFORM2IVPROC)glewGetProcAddress((const GLubyte*)"glUniform2iv")) == NULL) || r;
  r = ((glUniform3f = (PFNGLUNIFORM3FPROC)glewGetProcAddress((const GLubyte*)"glUniform3f")) == NULL) || r;
  r = ((glUniform3fv = (PFNGLUNIFORM3FVPROC)glewGetProcAddress((const GLubyte*)"glUniform3fv")) == NULL) || r;
  r = ((glUniform3i = (PFNGLUNIFORM3IPROC)glewGetProcAddress((const GLubyte*)"glUniform3i")) == NULL) || r;
  r = ((glUniform3iv = (PFNGLUNIFORM3IVPROC)glewGetProcAddress((const GLubyte*)"glUniform3iv")) == NULL) || r;
  r = ((glUniform4f = (PFNGLUNIFORM4FPROC)glewGetProcAddress((const GLubyte*)"glUniform4f")) == NULL) || r;
  r = ((glUniform4fv = (PFNGLUNIFORM4FVPROC)glewGetProcAddress((const GLubyte*)"glUniform4fv")) == NULL) || r;
  r = ((glUniform4i = (PFNGLUNIFORM4IPROC)glewGetProcAddress((const GLubyte*)"glUniform4i")) == NULL) || r;
  r = ((glUniform4iv = (PFNGLUNIFORM4IVPROC)glewGetProcAddress((const GLubyte*)"glUniform4iv")) == NULL) || r;
  r = ((glUniformMatrix2fv = (PFNGLUNIFORMMATRIX2FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix2fv")) == NULL) || r;
  r = ((glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix3fv")) == NULL) || r;
  r = ((glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix4fv")) == NULL) || r;
  r = ((glUseProgram = (PFNGLUSEPROGRAMPROC)glewGetProcAddress((const GLubyte*)"glUseProgram")) == NULL) || r;
  r = ((glValidateProgram = (PFNGLVALIDATEPROGRAMPROC)glewGetProcAddress((const GLubyte*)"glValidateProgram")) == NULL) || r;
  r = ((glVertexAttrib1d = (PFNGLVERTEXATTRIB1DPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1d")) == NULL) || r;
  r = ((glVertexAttrib1dv = (PFNGLVERTEXATTRIB1DVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1dv")) == NULL) || r;
  r = ((glVertexAttrib1f = (PFNGLVERTEXATTRIB1FPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1f")) == NULL) || r;
  r = ((glVertexAttrib1fv = (PFNGLVERTEXATTRIB1FVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1fv")) == NULL) || r;
  r = ((glVertexAttrib1s = (PFNGLVERTEXATTRIB1SPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1s")) == NULL) || r;
  r = ((glVertexAttrib1sv = (PFNGLVERTEXATTRIB1SVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib1sv")) == NULL) || r;
  r = ((glVertexAttrib2d = (PFNGLVERTEXATTRIB2DPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2d")) == NULL) || r;
  r = ((glVertexAttrib2dv = (PFNGLVERTEXATTRIB2DVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2dv")) == NULL) || r;
  r = ((glVertexAttrib2f = (PFNGLVERTEXATTRIB2FPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2f")) == NULL) || r;
  r = ((glVertexAttrib2fv = (PFNGLVERTEXATTRIB2FVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2fv")) == NULL) || r;
  r = ((glVertexAttrib2s = (PFNGLVERTEXATTRIB2SPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2s")) == NULL) || r;
  r = ((glVertexAttrib2sv = (PFNGLVERTEXATTRIB2SVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib2sv")) == NULL) || r;
  r = ((glVertexAttrib3d = (PFNGLVERTEXATTRIB3DPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3d")) == NULL) || r;
  r = ((glVertexAttrib3dv = (PFNGLVERTEXATTRIB3DVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3dv")) == NULL) || r;
  r = ((glVertexAttrib3f = (PFNGLVERTEXATTRIB3FPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3f")) == NULL) || r;
  r = ((glVertexAttrib3fv = (PFNGLVERTEXATTRIB3FVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3fv")) == NULL) || r;
  r = ((glVertexAttrib3s = (PFNGLVERTEXATTRIB3SPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3s")) == NULL) || r;
  r = ((glVertexAttrib3sv = (PFNGLVERTEXATTRIB3SVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib3sv")) == NULL) || r;
  r = ((glVertexAttrib4Nbv = (PFNGLVERTEXATTRIB4NBVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4Nbv")) == NULL) || r;
  r = ((glVertexAttrib4Niv = (PFNGLVERTEXATTRIB4NIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4Niv")) == NULL) || r;
  r = ((glVertexAttrib4Nsv = (PFNGLVERTEXATTRIB4NSVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4Nsv")) == NULL) || r;
  r = ((glVertexAttrib4Nub = (PFNGLVERTEXATTRIB4NUBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4Nub")) == NULL) || r;
  r = ((glVertexAttrib4Nubv = (PFNGLVERTEXATTRIB4NUBVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4Nubv")) == NULL) || r;
  r = ((glVertexAttrib4Nuiv = (PFNGLVERTEXATTRIB4NUIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4Nuiv")) == NULL) || r;
  r = ((glVertexAttrib4Nusv = (PFNGLVERTEXATTRIB4NUSVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4Nusv")) == NULL) || r;
  r = ((glVertexAttrib4bv = (PFNGLVERTEXATTRIB4BVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4bv")) == NULL) || r;
  r = ((glVertexAttrib4d = (PFNGLVERTEXATTRIB4DPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4d")) == NULL) || r;
  r = ((glVertexAttrib4dv = (PFNGLVERTEXATTRIB4DVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4dv")) == NULL) || r;
  r = ((glVertexAttrib4f = (PFNGLVERTEXATTRIB4FPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4f")) == NULL) || r;
  r = ((glVertexAttrib4fv = (PFNGLVERTEXATTRIB4FVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4fv")) == NULL) || r;
  r = ((glVertexAttrib4iv = (PFNGLVERTEXATTRIB4IVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4iv")) == NULL) || r;
  r = ((glVertexAttrib4s = (PFNGLVERTEXATTRIB4SPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4s")) == NULL) || r;
  r = ((glVertexAttrib4sv = (PFNGLVERTEXATTRIB4SVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4sv")) == NULL) || r;
  r = ((glVertexAttrib4ubv = (PFNGLVERTEXATTRIB4UBVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4ubv")) == NULL) || r;
  r = ((glVertexAttrib4uiv = (PFNGLVERTEXATTRIB4UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4uiv")) == NULL) || r;
  r = ((glVertexAttrib4usv = (PFNGLVERTEXATTRIB4USVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttrib4usv")) == NULL) || r;
  r = ((glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribPointer")) == NULL) || r;

#endif

  return r;
}

#endif /* GL_VERSION_2_0 */

#ifdef GL_VERSION_2_1

static GLboolean _glewInit_GL_VERSION_2_1 (void)
{
  GLboolean r = GL_FALSE;

  r = ((glUniformMatrix2x3fv = (PFNGLUNIFORMMATRIX2X3FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix2x3fv")) == NULL) || r;
  r = ((glUniformMatrix2x4fv = (PFNGLUNIFORMMATRIX2X4FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix2x4fv")) == NULL) || r;
  r = ((glUniformMatrix3x2fv = (PFNGLUNIFORMMATRIX3X2FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix3x2fv")) == NULL) || r;
  r = ((glUniformMatrix3x4fv = (PFNGLUNIFORMMATRIX3X4FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix3x4fv")) == NULL) || r;
  r = ((glUniformMatrix4x2fv = (PFNGLUNIFORMMATRIX4X2FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix4x2fv")) == NULL) || r;
  r = ((glUniformMatrix4x3fv = (PFNGLUNIFORMMATRIX4X3FVPROC)glewGetProcAddress((const GLubyte*)"glUniformMatrix4x3fv")) == NULL) || r;

  return r;
}

#endif /* GL_VERSION_2_1 */

#ifdef GL_VERSION_3_0

static GLboolean _glewInit_GL_VERSION_3_0 (void)
{
  GLboolean r = GL_FALSE;

  r = _glewInit_GL_ARB_framebuffer_object() || r;
  r = _glewInit_GL_ARB_map_buffer_range() || r;
  r = _glewInit_GL_ARB_vertex_array_object() || r;

  r = ((glBeginConditionalRender = (PFNGLBEGINCONDITIONALRENDERPROC)glewGetProcAddress((const GLubyte*)"glBeginConditionalRender")) == NULL) || r;
  r = ((glBeginTransformFeedback = (PFNGLBEGINTRANSFORMFEEDBACKPROC)glewGetProcAddress((const GLubyte*)"glBeginTransformFeedback")) == NULL) || r;
  r = ((glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC)glewGetProcAddress((const GLubyte*)"glBindFragDataLocation")) == NULL) || r;
  r = ((glClampColor = (PFNGLCLAMPCOLORPROC)glewGetProcAddress((const GLubyte*)"glClampColor")) == NULL) || r;
  r = ((glClearBufferfi = (PFNGLCLEARBUFFERFIPROC)glewGetProcAddress((const GLubyte*)"glClearBufferfi")) == NULL) || r;
  r = ((glClearBufferfv = (PFNGLCLEARBUFFERFVPROC)glewGetProcAddress((const GLubyte*)"glClearBufferfv")) == NULL) || r;
  r = ((glClearBufferiv = (PFNGLCLEARBUFFERIVPROC)glewGetProcAddress((const GLubyte*)"glClearBufferiv")) == NULL) || r;
  r = ((glClearBufferuiv = (PFNGLCLEARBUFFERUIVPROC)glewGetProcAddress((const GLubyte*)"glClearBufferuiv")) == NULL) || r;
  r = ((glColorMaski = (PFNGLCOLORMASKIPROC)glewGetProcAddress((const GLubyte*)"glColorMaski")) == NULL) || r;
  r = ((glDisablei = (PFNGLDISABLEIPROC)glewGetProcAddress((const GLubyte*)"glDisablei")) == NULL) || r;
  r = ((glEnablei = (PFNGLENABLEIPROC)glewGetProcAddress((const GLubyte*)"glEnablei")) == NULL) || r;
  r = ((glEndConditionalRender = (PFNGLENDCONDITIONALRENDERPROC)glewGetProcAddress((const GLubyte*)"glEndConditionalRender")) == NULL) || r;
  r = ((glEndTransformFeedback = (PFNGLENDTRANSFORMFEEDBACKPROC)glewGetProcAddress((const GLubyte*)"glEndTransformFeedback")) == NULL) || r;
  r = ((glGetBooleani_v = (PFNGLGETBOOLEANI_VPROC)glewGetProcAddress((const GLubyte*)"glGetBooleani_v")) == NULL) || r;
  r = ((glGetFragDataLocation = (PFNGLGETFRAGDATALOCATIONPROC)glewGetProcAddress((const GLubyte*)"glGetFragDataLocation")) == NULL) || r;
  r = ((glGetStringi = (PFNGLGETSTRINGIPROC)glewGetProcAddress((const GLubyte*)"glGetStringi")) == NULL) || r;
  r = ((glGetTexParameterIiv = (PFNGLGETTEXPARAMETERIIVPROC)glewGetProcAddress((const GLubyte*)"glGetTexParameterIiv")) == NULL) || r;
  r = ((glGetTexParameterIuiv = (PFNGLGETTEXPARAMETERIUIVPROC)glewGetProcAddress((const GLubyte*)"glGetTexParameterIuiv")) == NULL) || r;
  r = ((glGetTransformFeedbackVarying = (PFNGLGETTRANSFORMFEEDBACKVARYINGPROC)glewGetProcAddress((const GLubyte*)"glGetTransformFeedbackVarying")) == NULL) || r;
  r = ((glGetUniformuiv = (PFNGLGETUNIFORMUIVPROC)glewGetProcAddress((const GLubyte*)"glGetUniformuiv")) == NULL) || r;
  r = ((glGetVertexAttribIiv = (PFNGLGETVERTEXATTRIBIIVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribIiv")) == NULL) || r;
  r = ((glGetVertexAttribIuiv = (PFNGLGETVERTEXATTRIBIUIVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribIuiv")) == NULL) || r;
  r = ((glIsEnabledi = (PFNGLISENABLEDIPROC)glewGetProcAddress((const GLubyte*)"glIsEnabledi")) == NULL) || r;
  r = ((glTexParameterIiv = (PFNGLTEXPARAMETERIIVPROC)glewGetProcAddress((const GLubyte*)"glTexParameterIiv")) == NULL) || r;
  r = ((glTexParameterIuiv = (PFNGLTEXPARAMETERIUIVPROC)glewGetProcAddress((const GLubyte*)"glTexParameterIuiv")) == NULL) || r;
  r = ((glTransformFeedbackVaryings = (PFNGLTRANSFORMFEEDBACKVARYINGSPROC)glewGetProcAddress((const GLubyte*)"glTransformFeedbackVaryings")) == NULL) || r;
  r = ((glUniform1ui = (PFNGLUNIFORM1UIPROC)glewGetProcAddress((const GLubyte*)"glUniform1ui")) == NULL) || r;
  r = ((glUniform1uiv = (PFNGLUNIFORM1UIVPROC)glewGetProcAddress((const GLubyte*)"glUniform1uiv")) == NULL) || r;
  r = ((glUniform2ui = (PFNGLUNIFORM2UIPROC)glewGetProcAddress((const GLubyte*)"glUniform2ui")) == NULL) || r;
  r = ((glUniform2uiv = (PFNGLUNIFORM2UIVPROC)glewGetProcAddress((const GLubyte*)"glUniform2uiv")) == NULL) || r;
  r = ((glUniform3ui = (PFNGLUNIFORM3UIPROC)glewGetProcAddress((const GLubyte*)"glUniform3ui")) == NULL) || r;
  r = ((glUniform3uiv = (PFNGLUNIFORM3UIVPROC)glewGetProcAddress((const GLubyte*)"glUniform3uiv")) == NULL) || r;
  r = ((glUniform4ui = (PFNGLUNIFORM4UIPROC)glewGetProcAddress((const GLubyte*)"glUniform4ui")) == NULL) || r;
  r = ((glUniform4uiv = (PFNGLUNIFORM4UIVPROC)glewGetProcAddress((const GLubyte*)"glUniform4uiv")) == NULL) || r;
  r = ((glVertexAttribI1i = (PFNGLVERTEXATTRIBI1IPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI1i")) == NULL) || r;
  r = ((glVertexAttribI1iv = (PFNGLVERTEXATTRIBI1IVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI1iv")) == NULL) || r;
  r = ((glVertexAttribI1ui = (PFNGLVERTEXATTRIBI1UIPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI1ui")) == NULL) || r;
  r = ((glVertexAttribI1uiv = (PFNGLVERTEXATTRIBI1UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI1uiv")) == NULL) || r;
  r = ((glVertexAttribI2i = (PFNGLVERTEXATTRIBI2IPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI2i")) == NULL) || r;
  r = ((glVertexAttribI2iv = (PFNGLVERTEXATTRIBI2IVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI2iv")) == NULL) || r;
  r = ((glVertexAttribI2ui = (PFNGLVERTEXATTRIBI2UIPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI2ui")) == NULL) || r;
  r = ((glVertexAttribI2uiv = (PFNGLVERTEXATTRIBI2UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI2uiv")) == NULL) || r;
  r = ((glVertexAttribI3i = (PFNGLVERTEXATTRIBI3IPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI3i")) == NULL) || r;
  r = ((glVertexAttribI3iv = (PFNGLVERTEXATTRIBI3IVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI3iv")) == NULL) || r;
  r = ((glVertexAttribI3ui = (PFNGLVERTEXATTRIBI3UIPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI3ui")) == NULL) || r;
  r = ((glVertexAttribI3uiv = (PFNGLVERTEXATTRIBI3UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI3uiv")) == NULL) || r;
  r = ((glVertexAttribI4bv = (PFNGLVERTEXATTRIBI4BVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4bv")) == NULL) || r;
  r = ((glVertexAttribI4i = (PFNGLVERTEXATTRIBI4IPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4i")) == NULL) || r;
  r = ((glVertexAttribI4iv = (PFNGLVERTEXATTRIBI4IVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4iv")) == NULL) || r;
  r = ((glVertexAttribI4sv = (PFNGLVERTEXATTRIBI4SVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4sv")) == NULL) || r;
  r = ((glVertexAttribI4ubv = (PFNGLVERTEXATTRIBI4UBVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4ubv")) == NULL) || r;
  r = ((glVertexAttribI4ui = (PFNGLVERTEXATTRIBI4UIPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4ui")) == NULL) || r;
  r = ((glVertexAttribI4uiv = (PFNGLVERTEXATTRIBI4UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4uiv")) == NULL) || r;
  r = ((glVertexAttribI4usv = (PFNGLVERTEXATTRIBI4USVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4usv")) == NULL) || r;
  r = ((glVertexAttribIPointer = (PFNGLVERTEXATTRIBIPOINTERPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribIPointer")) == NULL) || r;

  // additional
  _glewInit_GL_EXT_framebuffer_blit();
  _glewInit_GL_EXT_framebuffer_multisample();
  _glewInit_GL_EXT_framebuffer_object();
  _glewInit_GL_EXT_gpu_shader4();

  return r;
}

#endif /* GL_VERSION_3_0 */

#ifdef GL_VERSION_3_1

static GLboolean _glewInit_GL_VERSION_3_1 (void)
{
  GLboolean r = GL_FALSE;

  r = _glewInit_GL_ARB_copy_buffer() || r;
  r = _glewInit_GL_ARB_uniform_buffer_object() || r;

  r = ((glDrawArraysInstanced = (PFNGLDRAWARRAYSINSTANCEDPROC)glewGetProcAddress((const GLubyte*)"glDrawArraysInstanced")) == NULL) || r;
  r = ((glDrawElementsInstanced = (PFNGLDRAWELEMENTSINSTANCEDPROC)glewGetProcAddress((const GLubyte*)"glDrawElementsInstanced")) == NULL) || r;
  r = ((glPrimitiveRestartIndex = (PFNGLPRIMITIVERESTARTINDEXPROC)glewGetProcAddress((const GLubyte*)"glPrimitiveRestartIndex")) == NULL) || r;
  r = ((glTexBuffer = (PFNGLTEXBUFFERPROC)glewGetProcAddress((const GLubyte*)"glTexBuffer")) == NULL) || r;

  return r;
}

#endif /* GL_VERSION_3_1 */

#ifdef GL_VERSION_3_2

static GLboolean _glewInit_GL_VERSION_3_2 (void)
{
  GLboolean r = GL_FALSE;

  r = _glewInit_GL_ARB_draw_elements_base_vertex() || r;
  r = _glewInit_GL_ARB_provoking_vertex() || r;
  r = _glewInit_GL_ARB_sync() || r;
  r = _glewInit_GL_ARB_texture_multisample() || r;

  r = ((glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTexture")) == NULL) || r;
  r = ((glGetBufferParameteri64v = (PFNGLGETBUFFERPARAMETERI64VPROC)glewGetProcAddress((const GLubyte*)"glGetBufferParameteri64v")) == NULL) || r;
  r = ((glGetInteger64i_v = (PFNGLGETINTEGER64I_VPROC)glewGetProcAddress((const GLubyte*)"glGetInteger64i_v")) == NULL) || r;

  // additional
  _glewInit_GL_ARB_geometry_shader4();
  _glewInit_GL_ARB_timer_query();
  _glewInit_GL_EXT_geometry_shader4();

  return r;
}

#endif /* GL_VERSION_3_2 */

#ifdef GL_VERSION_3_3

static GLboolean _glewInit_GL_VERSION_3_3 (void)
{
  GLboolean r = GL_FALSE;

  r = ((glVertexAttribDivisor = (PFNGLVERTEXATTRIBDIVISORPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribDivisor")) == NULL) || r;

  // additional
  _glewInit_GL_ARB_instanced_arrays();
  _glewInit_GL_ARB_vertex_type_2_10_10_10_rev();

  return r;
}

#endif /* GL_VERSION_3_3 */

#ifdef GL_VERSION_4_0

static GLboolean _glewInit_GL_VERSION_4_0 (void)
{
  GLboolean r = GL_FALSE;

  r = ((glBlendEquationSeparatei = (PFNGLBLENDEQUATIONSEPARATEIPROC)glewGetProcAddress((const GLubyte*)"glBlendEquationSeparatei")) == NULL) || r;
  r = ((glBlendEquationi = (PFNGLBLENDEQUATIONIPROC)glewGetProcAddress((const GLubyte*)"glBlendEquationi")) == NULL) || r;
  r = ((glBlendFuncSeparatei = (PFNGLBLENDFUNCSEPARATEIPROC)glewGetProcAddress((const GLubyte*)"glBlendFuncSeparatei")) == NULL) || r;
  r = ((glBlendFunci = (PFNGLBLENDFUNCIPROC)glewGetProcAddress((const GLubyte*)"glBlendFunci")) == NULL) || r;
  r = ((glMinSampleShading = (PFNGLMINSAMPLESHADINGPROC)glewGetProcAddress((const GLubyte*)"glMinSampleShading")) == NULL) || r;

  // additional
  _glewInit_GL_ARB_sample_shading();
  _glewInit_GL_ARB_tessellation_shader();

  return r;
}

#endif /* GL_VERSION_4_0 */

#ifdef GL_VERSION_4_1

static GLboolean _glewInit_GL_VERSION_4_1 (void)
{
  GLboolean r = GL_FALSE;

  // additional
  _glewInit_GL_ARB_get_program_binary();

  return r;
}

#endif /* GL_VERSION_4_1 */

#ifdef GL_VERSION_4_2

static GLboolean _glewInit_GL_VERSION_4_2 (void)
{
  GLboolean r = GL_FALSE;

  // additional
  _glewInit_GL_ARB_shader_image_load_store();
  _glewInit_GL_ARB_texture_storage();
  _glewInit_GL_EXT_shader_image_load_store();
  _glewInit_GL_EXT_texture_storage();

  return r;
}

#endif /* GL_VERSION_4_2 */

#ifdef GL_VERSION_4_3

static GLboolean _glewInit_GL_VERSION_4_3 (void)
{
  GLboolean r = GL_FALSE;

  // additional
  _glewInit_GL_ARB_clear_buffer_object();
  _glewInit_GL_ARB_compute_shader();
  _glewInit_GL_ARB_copy_image();
  _glewInit_GL_ARB_invalidate_subdata();
  _glewInit_GL_ARB_program_interface_query();
  _glewInit_GL_ARB_vertex_attrib_binding();
  _glewInit_GL_KHR_debug();
  _glewInit_GL_NV_copy_image();

  return r;
}

#endif /* GL_VERSION_4_3 */

#ifdef GL_VERSION_4_4

static GLboolean _glewInit_GL_VERSION_4_4 (void)
{
  GLboolean r = GL_FALSE;

  // additional
  _glewInit_GL_ARB_buffer_storage();
  _glewInit_GL_ARB_clear_texture();
  _glewInit_GL_ARB_multi_bind();

  return r;
}

#endif /* GL_VERSION_4_4 */

#ifdef GL_VERSION_4_5

static GLboolean _glewInit_GL_VERSION_4_5 (void)
{
  GLboolean r = GL_FALSE;

  r = ((glGetGraphicsResetStatus = (PFNGLGETGRAPHICSRESETSTATUSPROC)glewGetProcAddress((const GLubyte*)"glGetGraphicsResetStatus")) == NULL) || r;
  r = ((glGetnCompressedTexImage = (PFNGLGETNCOMPRESSEDTEXIMAGEPROC)glewGetProcAddress((const GLubyte*)"glGetnCompressedTexImage")) == NULL) || r;
  r = ((glGetnTexImage = (PFNGLGETNTEXIMAGEPROC)glewGetProcAddress((const GLubyte*)"glGetnTexImage")) == NULL) || r;
  r = ((glGetnUniformdv = (PFNGLGETNUNIFORMDVPROC)glewGetProcAddress((const GLubyte*)"glGetnUniformdv")) == NULL) || r;

  // additional
  _glewInit_GL_ARB_direct_state_access();
  _glewInit_GL_ARB_robustness();
  _glewInit_GL_KHR_robustness();

  return r;
}

#endif /* GL_VERSION_4_5 */

#ifdef GL_VERSION_4_6

static GLboolean _glewInit_GL_VERSION_4_6 (void)
{
  GLboolean r = GL_FALSE;

  r = ((glMultiDrawArraysIndirectCount = (PFNGLMULTIDRAWARRAYSINDIRECTCOUNTPROC)glewGetProcAddress((const GLubyte*)"glMultiDrawArraysIndirectCount")) == NULL) || r;
  r = ((glMultiDrawElementsIndirectCount = (PFNGLMULTIDRAWELEMENTSINDIRECTCOUNTPROC)glewGetProcAddress((const GLubyte*)"glMultiDrawElementsIndirectCount")) == NULL) || r;
  r = ((glSpecializeShader = (PFNGLSPECIALIZESHADERPROC)glewGetProcAddress((const GLubyte*)"glSpecializeShader")) == NULL) || r;

  return r;
}

#endif /* GL_VERSION_4_6 */

#ifdef GL_AMD_framebuffer_multisample_advanced

static GLboolean _glewInit_GL_AMD_framebuffer_multisample_advanced (void)
{
  GLboolean r = GL_FALSE;

  r = ((glNamedRenderbufferStorageMultisampleAdvancedAMD = (PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC)glewGetProcAddress((const GLubyte*)"glNamedRenderbufferStorageMultisampleAdvancedAMD")) == NULL) || r;
  r = ((glRenderbufferStorageMultisampleAdvancedAMD = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC)glewGetProcAddress((const GLubyte*)"glRenderbufferStorageMultisampleAdvancedAMD")) == NULL) || r;

  return r;
}

#endif /* GL_AMD_framebuffer_multisample_advanced */

#ifdef GL_ARB_buffer_storage

static GLboolean _glewInit_GL_ARB_buffer_storage (void)
{
  GLboolean r = GL_FALSE;

  r = ((glBufferStorage = (PFNGLBUFFERSTORAGEPROC)glewGetProcAddress((const GLubyte*)"glBufferStorage")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_buffer_storage */

#ifdef GL_ARB_clear_buffer_object

static GLboolean _glewInit_GL_ARB_clear_buffer_object (void)
{
  GLboolean r = GL_FALSE;

  r = ((glClearBufferData = (PFNGLCLEARBUFFERDATAPROC)glewGetProcAddress((const GLubyte*)"glClearBufferData")) == NULL) || r;
  r = ((glClearBufferSubData = (PFNGLCLEARBUFFERSUBDATAPROC)glewGetProcAddress((const GLubyte*)"glClearBufferSubData")) == NULL) || r;
  r = ((glClearNamedBufferDataEXT = (PFNGLCLEARNAMEDBUFFERDATAEXTPROC)glewGetProcAddress((const GLubyte*)"glClearNamedBufferDataEXT")) == NULL) || r;
  r = ((glClearNamedBufferSubDataEXT = (PFNGLCLEARNAMEDBUFFERSUBDATAEXTPROC)glewGetProcAddress((const GLubyte*)"glClearNamedBufferSubDataEXT")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_clear_buffer_object */

#ifdef GL_ARB_clear_texture

static GLboolean _glewInit_GL_ARB_clear_texture (void)
{
  GLboolean r = GL_FALSE;

  r = ((glClearTexImage = (PFNGLCLEARTEXIMAGEPROC)glewGetProcAddress((const GLubyte*)"glClearTexImage")) == NULL) || r;
  r = ((glClearTexSubImage = (PFNGLCLEARTEXSUBIMAGEPROC)glewGetProcAddress((const GLubyte*)"glClearTexSubImage")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_clear_texture */

#ifdef GL_ARB_compute_shader

static GLboolean _glewInit_GL_ARB_compute_shader (void)
{
  GLboolean r = GL_FALSE;

  r = ((glDispatchCompute = (PFNGLDISPATCHCOMPUTEPROC)glewGetProcAddress((const GLubyte*)"glDispatchCompute")) == NULL) || r;
  r = ((glDispatchComputeIndirect = (PFNGLDISPATCHCOMPUTEINDIRECTPROC)glewGetProcAddress((const GLubyte*)"glDispatchComputeIndirect")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_compute_shader */

#ifdef GL_ARB_copy_buffer

static GLboolean _glewInit_GL_ARB_copy_buffer (void)
{
  GLboolean r = GL_FALSE;

  r = ((glCopyBufferSubData = (PFNGLCOPYBUFFERSUBDATAPROC)glewGetProcAddress((const GLubyte*)"glCopyBufferSubData")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_copy_buffer */

#ifdef GL_ARB_copy_image

static GLboolean _glewInit_GL_ARB_copy_image (void)
{
  GLboolean r = GL_FALSE;

  r = ((glCopyImageSubData = (PFNGLCOPYIMAGESUBDATAPROC)glewGetProcAddress((const GLubyte*)"glCopyImageSubData")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_copy_image */

#ifdef GL_ARB_direct_state_access

static GLboolean _glewInit_GL_ARB_direct_state_access (void)
{
  GLboolean r = GL_FALSE;

  r = ((glBindTextureUnit = (PFNGLBINDTEXTUREUNITPROC)glewGetProcAddress((const GLubyte*)"glBindTextureUnit")) == NULL) || r;
  r = ((glBlitNamedFramebuffer = (PFNGLBLITNAMEDFRAMEBUFFERPROC)glewGetProcAddress((const GLubyte*)"glBlitNamedFramebuffer")) == NULL) || r;
  r = ((glCheckNamedFramebufferStatus = (PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC)glewGetProcAddress((const GLubyte*)"glCheckNamedFramebufferStatus")) == NULL) || r;
  r = ((glClearNamedBufferData = (PFNGLCLEARNAMEDBUFFERDATAPROC)glewGetProcAddress((const GLubyte*)"glClearNamedBufferData")) == NULL) || r;
  r = ((glClearNamedBufferSubData = (PFNGLCLEARNAMEDBUFFERSUBDATAPROC)glewGetProcAddress((const GLubyte*)"glClearNamedBufferSubData")) == NULL) || r;
  r = ((glClearNamedFramebufferfi = (PFNGLCLEARNAMEDFRAMEBUFFERFIPROC)glewGetProcAddress((const GLubyte*)"glClearNamedFramebufferfi")) == NULL) || r;
  r = ((glClearNamedFramebufferfv = (PFNGLCLEARNAMEDFRAMEBUFFERFVPROC)glewGetProcAddress((const GLubyte*)"glClearNamedFramebufferfv")) == NULL) || r;
  r = ((glClearNamedFramebufferiv = (PFNGLCLEARNAMEDFRAMEBUFFERIVPROC)glewGetProcAddress((const GLubyte*)"glClearNamedFramebufferiv")) == NULL) || r;
  r = ((glClearNamedFramebufferuiv = (PFNGLCLEARNAMEDFRAMEBUFFERUIVPROC)glewGetProcAddress((const GLubyte*)"glClearNamedFramebufferuiv")) == NULL) || r;
  r = ((glCompressedTextureSubImage1D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE1DPROC)glewGetProcAddress((const GLubyte*)"glCompressedTextureSubImage1D")) == NULL) || r;
  r = ((glCompressedTextureSubImage2D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE2DPROC)glewGetProcAddress((const GLubyte*)"glCompressedTextureSubImage2D")) == NULL) || r;
  r = ((glCompressedTextureSubImage3D = (PFNGLCOMPRESSEDTEXTURESUBIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glCompressedTextureSubImage3D")) == NULL) || r;
  r = ((glCopyNamedBufferSubData = (PFNGLCOPYNAMEDBUFFERSUBDATAPROC)glewGetProcAddress((const GLubyte*)"glCopyNamedBufferSubData")) == NULL) || r;
  r = ((glCopyTextureSubImage1D = (PFNGLCOPYTEXTURESUBIMAGE1DPROC)glewGetProcAddress((const GLubyte*)"glCopyTextureSubImage1D")) == NULL) || r;
  r = ((glCopyTextureSubImage2D = (PFNGLCOPYTEXTURESUBIMAGE2DPROC)glewGetProcAddress((const GLubyte*)"glCopyTextureSubImage2D")) == NULL) || r;
  r = ((glCopyTextureSubImage3D = (PFNGLCOPYTEXTURESUBIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glCopyTextureSubImage3D")) == NULL) || r;
  r = ((glCreateBuffers = (PFNGLCREATEBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glCreateBuffers")) == NULL) || r;
  r = ((glCreateFramebuffers = (PFNGLCREATEFRAMEBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glCreateFramebuffers")) == NULL) || r;
  r = ((glCreateProgramPipelines = (PFNGLCREATEPROGRAMPIPELINESPROC)glewGetProcAddress((const GLubyte*)"glCreateProgramPipelines")) == NULL) || r;
  r = ((glCreateQueries = (PFNGLCREATEQUERIESPROC)glewGetProcAddress((const GLubyte*)"glCreateQueries")) == NULL) || r;
  r = ((glCreateRenderbuffers = (PFNGLCREATERENDERBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glCreateRenderbuffers")) == NULL) || r;
  r = ((glCreateSamplers = (PFNGLCREATESAMPLERSPROC)glewGetProcAddress((const GLubyte*)"glCreateSamplers")) == NULL) || r;
  r = ((glCreateTextures = (PFNGLCREATETEXTURESPROC)glewGetProcAddress((const GLubyte*)"glCreateTextures")) == NULL) || r;
  r = ((glCreateTransformFeedbacks = (PFNGLCREATETRANSFORMFEEDBACKSPROC)glewGetProcAddress((const GLubyte*)"glCreateTransformFeedbacks")) == NULL) || r;
  r = ((glCreateVertexArrays = (PFNGLCREATEVERTEXARRAYSPROC)glewGetProcAddress((const GLubyte*)"glCreateVertexArrays")) == NULL) || r;
  r = ((glDisableVertexArrayAttrib = (PFNGLDISABLEVERTEXARRAYATTRIBPROC)glewGetProcAddress((const GLubyte*)"glDisableVertexArrayAttrib")) == NULL) || r;
  r = ((glEnableVertexArrayAttrib = (PFNGLENABLEVERTEXARRAYATTRIBPROC)glewGetProcAddress((const GLubyte*)"glEnableVertexArrayAttrib")) == NULL) || r;
  r = ((glFlushMappedNamedBufferRange = (PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEPROC)glewGetProcAddress((const GLubyte*)"glFlushMappedNamedBufferRange")) == NULL) || r;
  r = ((glGenerateTextureMipmap = (PFNGLGENERATETEXTUREMIPMAPPROC)glewGetProcAddress((const GLubyte*)"glGenerateTextureMipmap")) == NULL) || r;
  r = ((glGetCompressedTextureImage = (PFNGLGETCOMPRESSEDTEXTUREIMAGEPROC)glewGetProcAddress((const GLubyte*)"glGetCompressedTextureImage")) == NULL) || r;
  r = ((glGetNamedBufferParameteri64v = (PFNGLGETNAMEDBUFFERPARAMETERI64VPROC)glewGetProcAddress((const GLubyte*)"glGetNamedBufferParameteri64v")) == NULL) || r;
  r = ((glGetNamedBufferParameteriv = (PFNGLGETNAMEDBUFFERPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetNamedBufferParameteriv")) == NULL) || r;
  r = ((glGetNamedBufferPointerv = (PFNGLGETNAMEDBUFFERPOINTERVPROC)glewGetProcAddress((const GLubyte*)"glGetNamedBufferPointerv")) == NULL) || r;
  r = ((glGetNamedBufferSubData = (PFNGLGETNAMEDBUFFERSUBDATAPROC)glewGetProcAddress((const GLubyte*)"glGetNamedBufferSubData")) == NULL) || r;
  r = ((glGetNamedFramebufferAttachmentParameteriv = (PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetNamedFramebufferAttachmentParameteriv")) == NULL) || r;
  r = ((glGetNamedFramebufferParameteriv = (PFNGLGETNAMEDFRAMEBUFFERPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetNamedFramebufferParameteriv")) == NULL) || r;
  r = ((glGetNamedRenderbufferParameteriv = (PFNGLGETNAMEDRENDERBUFFERPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetNamedRenderbufferParameteriv")) == NULL) || r;
  r = ((glGetQueryBufferObjecti64v = (PFNGLGETQUERYBUFFEROBJECTI64VPROC)glewGetProcAddress((const GLubyte*)"glGetQueryBufferObjecti64v")) == NULL) || r;
  r = ((glGetQueryBufferObjectiv = (PFNGLGETQUERYBUFFEROBJECTIVPROC)glewGetProcAddress((const GLubyte*)"glGetQueryBufferObjectiv")) == NULL) || r;
  r = ((glGetQueryBufferObjectui64v = (PFNGLGETQUERYBUFFEROBJECTUI64VPROC)glewGetProcAddress((const GLubyte*)"glGetQueryBufferObjectui64v")) == NULL) || r;
  r = ((glGetQueryBufferObjectuiv = (PFNGLGETQUERYBUFFEROBJECTUIVPROC)glewGetProcAddress((const GLubyte*)"glGetQueryBufferObjectuiv")) == NULL) || r;
  r = ((glGetTextureImage = (PFNGLGETTEXTUREIMAGEPROC)glewGetProcAddress((const GLubyte*)"glGetTextureImage")) == NULL) || r;
  r = ((glGetTextureLevelParameterfv = (PFNGLGETTEXTURELEVELPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glGetTextureLevelParameterfv")) == NULL) || r;
  r = ((glGetTextureLevelParameteriv = (PFNGLGETTEXTURELEVELPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetTextureLevelParameteriv")) == NULL) || r;
  r = ((glGetTextureParameterIiv = (PFNGLGETTEXTUREPARAMETERIIVPROC)glewGetProcAddress((const GLubyte*)"glGetTextureParameterIiv")) == NULL) || r;
  r = ((glGetTextureParameterIuiv = (PFNGLGETTEXTUREPARAMETERIUIVPROC)glewGetProcAddress((const GLubyte*)"glGetTextureParameterIuiv")) == NULL) || r;
  r = ((glGetTextureParameterfv = (PFNGLGETTEXTUREPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glGetTextureParameterfv")) == NULL) || r;
  r = ((glGetTextureParameteriv = (PFNGLGETTEXTUREPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetTextureParameteriv")) == NULL) || r;
  r = ((glGetTransformFeedbacki64_v = (PFNGLGETTRANSFORMFEEDBACKI64_VPROC)glewGetProcAddress((const GLubyte*)"glGetTransformFeedbacki64_v")) == NULL) || r;
  r = ((glGetTransformFeedbacki_v = (PFNGLGETTRANSFORMFEEDBACKI_VPROC)glewGetProcAddress((const GLubyte*)"glGetTransformFeedbacki_v")) == NULL) || r;
  r = ((glGetTransformFeedbackiv = (PFNGLGETTRANSFORMFEEDBACKIVPROC)glewGetProcAddress((const GLubyte*)"glGetTransformFeedbackiv")) == NULL) || r;
  r = ((glGetVertexArrayIndexed64iv = (PFNGLGETVERTEXARRAYINDEXED64IVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexArrayIndexed64iv")) == NULL) || r;
  r = ((glGetVertexArrayIndexediv = (PFNGLGETVERTEXARRAYINDEXEDIVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexArrayIndexediv")) == NULL) || r;
  r = ((glGetVertexArrayiv = (PFNGLGETVERTEXARRAYIVPROC)glewGetProcAddress((const GLubyte*)"glGetVertexArrayiv")) == NULL) || r;
  r = ((glInvalidateNamedFramebufferData = (PFNGLINVALIDATENAMEDFRAMEBUFFERDATAPROC)glewGetProcAddress((const GLubyte*)"glInvalidateNamedFramebufferData")) == NULL) || r;
  r = ((glInvalidateNamedFramebufferSubData = (PFNGLINVALIDATENAMEDFRAMEBUFFERSUBDATAPROC)glewGetProcAddress((const GLubyte*)"glInvalidateNamedFramebufferSubData")) == NULL) || r;
  r = ((glMapNamedBuffer = (PFNGLMAPNAMEDBUFFERPROC)glewGetProcAddress((const GLubyte*)"glMapNamedBuffer")) == NULL) || r;
  r = ((glMapNamedBufferRange = (PFNGLMAPNAMEDBUFFERRANGEPROC)glewGetProcAddress((const GLubyte*)"glMapNamedBufferRange")) == NULL) || r;
  r = ((glNamedBufferData = (PFNGLNAMEDBUFFERDATAPROC)glewGetProcAddress((const GLubyte*)"glNamedBufferData")) == NULL) || r;
  r = ((glNamedBufferStorage = (PFNGLNAMEDBUFFERSTORAGEPROC)glewGetProcAddress((const GLubyte*)"glNamedBufferStorage")) == NULL) || r;
  r = ((glNamedBufferSubData = (PFNGLNAMEDBUFFERSUBDATAPROC)glewGetProcAddress((const GLubyte*)"glNamedBufferSubData")) == NULL) || r;
  r = ((glNamedFramebufferDrawBuffer = (PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferDrawBuffer")) == NULL) || r;
  r = ((glNamedFramebufferDrawBuffers = (PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferDrawBuffers")) == NULL) || r;
  r = ((glNamedFramebufferParameteri = (PFNGLNAMEDFRAMEBUFFERPARAMETERIPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferParameteri")) == NULL) || r;
  r = ((glNamedFramebufferReadBuffer = (PFNGLNAMEDFRAMEBUFFERREADBUFFERPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferReadBuffer")) == NULL) || r;
  r = ((glNamedFramebufferRenderbuffer = (PFNGLNAMEDFRAMEBUFFERRENDERBUFFERPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferRenderbuffer")) == NULL) || r;
  r = ((glNamedFramebufferTexture = (PFNGLNAMEDFRAMEBUFFERTEXTUREPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferTexture")) == NULL) || r;
  r = ((glNamedFramebufferTextureLayer = (PFNGLNAMEDFRAMEBUFFERTEXTURELAYERPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferTextureLayer")) == NULL) || r;
  r = ((glNamedRenderbufferStorage = (PFNGLNAMEDRENDERBUFFERSTORAGEPROC)glewGetProcAddress((const GLubyte*)"glNamedRenderbufferStorage")) == NULL) || r;
  r = ((glNamedRenderbufferStorageMultisample = (PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEPROC)glewGetProcAddress((const GLubyte*)"glNamedRenderbufferStorageMultisample")) == NULL) || r;
  r = ((glTextureBuffer = (PFNGLTEXTUREBUFFERPROC)glewGetProcAddress((const GLubyte*)"glTextureBuffer")) == NULL) || r;
  r = ((glTextureBufferRange = (PFNGLTEXTUREBUFFERRANGEPROC)glewGetProcAddress((const GLubyte*)"glTextureBufferRange")) == NULL) || r;
  r = ((glTextureParameterIiv = (PFNGLTEXTUREPARAMETERIIVPROC)glewGetProcAddress((const GLubyte*)"glTextureParameterIiv")) == NULL) || r;
  r = ((glTextureParameterIuiv = (PFNGLTEXTUREPARAMETERIUIVPROC)glewGetProcAddress((const GLubyte*)"glTextureParameterIuiv")) == NULL) || r;
  r = ((glTextureParameterf = (PFNGLTEXTUREPARAMETERFPROC)glewGetProcAddress((const GLubyte*)"glTextureParameterf")) == NULL) || r;
  r = ((glTextureParameterfv = (PFNGLTEXTUREPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glTextureParameterfv")) == NULL) || r;
  r = ((glTextureParameteri = (PFNGLTEXTUREPARAMETERIPROC)glewGetProcAddress((const GLubyte*)"glTextureParameteri")) == NULL) || r;
  r = ((glTextureParameteriv = (PFNGLTEXTUREPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glTextureParameteriv")) == NULL) || r;
  r = ((glTextureStorage1D = (PFNGLTEXTURESTORAGE1DPROC)glewGetProcAddress((const GLubyte*)"glTextureStorage1D")) == NULL) || r;
  r = ((glTextureStorage2D = (PFNGLTEXTURESTORAGE2DPROC)glewGetProcAddress((const GLubyte*)"glTextureStorage2D")) == NULL) || r;
  r = ((glTextureStorage2DMultisample = (PFNGLTEXTURESTORAGE2DMULTISAMPLEPROC)glewGetProcAddress((const GLubyte*)"glTextureStorage2DMultisample")) == NULL) || r;
  r = ((glTextureStorage3D = (PFNGLTEXTURESTORAGE3DPROC)glewGetProcAddress((const GLubyte*)"glTextureStorage3D")) == NULL) || r;
  r = ((glTextureStorage3DMultisample = (PFNGLTEXTURESTORAGE3DMULTISAMPLEPROC)glewGetProcAddress((const GLubyte*)"glTextureStorage3DMultisample")) == NULL) || r;
  r = ((glTextureSubImage1D = (PFNGLTEXTURESUBIMAGE1DPROC)glewGetProcAddress((const GLubyte*)"glTextureSubImage1D")) == NULL) || r;
  r = ((glTextureSubImage2D = (PFNGLTEXTURESUBIMAGE2DPROC)glewGetProcAddress((const GLubyte*)"glTextureSubImage2D")) == NULL) || r;
  r = ((glTextureSubImage3D = (PFNGLTEXTURESUBIMAGE3DPROC)glewGetProcAddress((const GLubyte*)"glTextureSubImage3D")) == NULL) || r;
  r = ((glTransformFeedbackBufferBase = (PFNGLTRANSFORMFEEDBACKBUFFERBASEPROC)glewGetProcAddress((const GLubyte*)"glTransformFeedbackBufferBase")) == NULL) || r;
  r = ((glTransformFeedbackBufferRange = (PFNGLTRANSFORMFEEDBACKBUFFERRANGEPROC)glewGetProcAddress((const GLubyte*)"glTransformFeedbackBufferRange")) == NULL) || r;
  r = ((glUnmapNamedBuffer = (PFNGLUNMAPNAMEDBUFFERPROC)glewGetProcAddress((const GLubyte*)"glUnmapNamedBuffer")) == NULL) || r;
  r = ((glVertexArrayAttribBinding = (PFNGLVERTEXARRAYATTRIBBINDINGPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayAttribBinding")) == NULL) || r;
  r = ((glVertexArrayAttribFormat = (PFNGLVERTEXARRAYATTRIBFORMATPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayAttribFormat")) == NULL) || r;
  r = ((glVertexArrayAttribIFormat = (PFNGLVERTEXARRAYATTRIBIFORMATPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayAttribIFormat")) == NULL) || r;
  r = ((glVertexArrayAttribLFormat = (PFNGLVERTEXARRAYATTRIBLFORMATPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayAttribLFormat")) == NULL) || r;
  r = ((glVertexArrayBindingDivisor = (PFNGLVERTEXARRAYBINDINGDIVISORPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayBindingDivisor")) == NULL) || r;
  r = ((glVertexArrayElementBuffer = (PFNGLVERTEXARRAYELEMENTBUFFERPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayElementBuffer")) == NULL) || r;
  r = ((glVertexArrayVertexBuffer = (PFNGLVERTEXARRAYVERTEXBUFFERPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayVertexBuffer")) == NULL) || r;
  r = ((glVertexArrayVertexBuffers = (PFNGLVERTEXARRAYVERTEXBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayVertexBuffers")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_direct_state_access */

#ifdef GL_ARB_draw_elements_base_vertex

static GLboolean _glewInit_GL_ARB_draw_elements_base_vertex (void)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawElementsBaseVertex = (PFNGLDRAWELEMENTSBASEVERTEXPROC)glewGetProcAddress((const GLubyte*)"glDrawElementsBaseVertex")) == NULL) || r;
  r = ((glDrawElementsInstancedBaseVertex = (PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC)glewGetProcAddress((const GLubyte*)"glDrawElementsInstancedBaseVertex")) == NULL) || r;
  r = ((glDrawRangeElementsBaseVertex = (PFNGLDRAWRANGEELEMENTSBASEVERTEXPROC)glewGetProcAddress((const GLubyte*)"glDrawRangeElementsBaseVertex")) == NULL) || r;
  r = ((glMultiDrawElementsBaseVertex = (PFNGLMULTIDRAWELEMENTSBASEVERTEXPROC)glewGetProcAddress((const GLubyte*)"glMultiDrawElementsBaseVertex")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_draw_elements_base_vertex */

#ifdef GL_ARB_framebuffer_object

static GLboolean _glewInit_GL_ARB_framebuffer_object (void)
{
  GLboolean r = GL_FALSE;

  r = ((glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)glewGetProcAddress((const GLubyte*)"glBindFramebuffer")) == NULL) || r;
  r = ((glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC)glewGetProcAddress((const GLubyte*)"glBindRenderbuffer")) == NULL) || r;
  r = ((glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC)glewGetProcAddress((const GLubyte*)"glBlitFramebuffer")) == NULL) || r;
  r = ((glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)glewGetProcAddress((const GLubyte*)"glCheckFramebufferStatus")) == NULL) || r;
  r = ((glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glDeleteFramebuffers")) == NULL) || r;
  r = ((glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glDeleteRenderbuffers")) == NULL) || r;
  r = ((glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)glewGetProcAddress((const GLubyte*)"glFramebufferRenderbuffer")) == NULL) || r;
  r = ((glFramebufferTexture1D = (PFNGLFRAMEBUFFERTEXTURE1DPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTexture1D")) == NULL) || r;
  r = ((glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTexture2D")) == NULL) || r;
  r = ((glFramebufferTexture3D = (PFNGLFRAMEBUFFERTEXTURE3DPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTexture3D")) == NULL) || r;
  r = ((glFramebufferTextureLayer = (PFNGLFRAMEBUFFERTEXTURELAYERPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTextureLayer")) == NULL) || r;
  r = ((glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glGenFramebuffers")) == NULL) || r;
  r = ((glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glGenRenderbuffers")) == NULL) || r;
  r = ((glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)glewGetProcAddress((const GLubyte*)"glGenerateMipmap")) == NULL) || r;
  r = ((glGetFramebufferAttachmentParameteriv = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetFramebufferAttachmentParameteriv")) == NULL) || r;
  r = ((glGetRenderbufferParameteriv = (PFNGLGETRENDERBUFFERPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetRenderbufferParameteriv")) == NULL) || r;
  r = ((glIsFramebuffer = (PFNGLISFRAMEBUFFERPROC)glewGetProcAddress((const GLubyte*)"glIsFramebuffer")) == NULL) || r;
  r = ((glIsRenderbuffer = (PFNGLISRENDERBUFFERPROC)glewGetProcAddress((const GLubyte*)"glIsRenderbuffer")) == NULL) || r;
  r = ((glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC)glewGetProcAddress((const GLubyte*)"glRenderbufferStorage")) == NULL) || r;
  r = ((glRenderbufferStorageMultisample = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC)glewGetProcAddress((const GLubyte*)"glRenderbufferStorageMultisample")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_framebuffer_object */

#ifdef GL_ARB_geometry_shader4

static GLboolean _glewInit_GL_ARB_geometry_shader4 (void)
{
  GLboolean r = GL_FALSE;

  r = ((glFramebufferTextureARB = (PFNGLFRAMEBUFFERTEXTUREARBPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTextureARB")) == NULL) || r;
  r = ((glFramebufferTextureFaceARB = (PFNGLFRAMEBUFFERTEXTUREFACEARBPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTextureFaceARB")) == NULL) || r;
  r = ((glFramebufferTextureLayerARB = (PFNGLFRAMEBUFFERTEXTURELAYERARBPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTextureLayerARB")) == NULL) || r;
  r = ((glProgramParameteriARB = (PFNGLPROGRAMPARAMETERIARBPROC)glewGetProcAddress((const GLubyte*)"glProgramParameteriARB")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_geometry_shader4 */

#ifdef GL_ARB_get_program_binary

static GLboolean _glewInit_GL_ARB_get_program_binary (void)
{
  GLboolean r = GL_FALSE;

  r = ((glGetProgramBinary = (PFNGLGETPROGRAMBINARYPROC)glewGetProcAddress((const GLubyte*)"glGetProgramBinary")) == NULL) || r;
  r = ((glProgramBinary = (PFNGLPROGRAMBINARYPROC)glewGetProcAddress((const GLubyte*)"glProgramBinary")) == NULL) || r;
  r = ((glProgramParameteri = (PFNGLPROGRAMPARAMETERIPROC)glewGetProcAddress((const GLubyte*)"glProgramParameteri")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_get_program_binary */

#ifdef GL_ARB_imaging

static GLboolean _glewInit_GL_ARB_imaging (void)
{
  GLboolean r = GL_FALSE;

  r = ((glColorSubTable = (PFNGLCOLORSUBTABLEPROC)glewGetProcAddress((const GLubyte*)"glColorSubTable")) == NULL) || r;
  r = ((glColorTable = (PFNGLCOLORTABLEPROC)glewGetProcAddress((const GLubyte*)"glColorTable")) == NULL) || r;
  r = ((glColorTableParameterfv = (PFNGLCOLORTABLEPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glColorTableParameterfv")) == NULL) || r;
  r = ((glColorTableParameteriv = (PFNGLCOLORTABLEPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glColorTableParameteriv")) == NULL) || r;
  r = ((glConvolutionFilter1D = (PFNGLCONVOLUTIONFILTER1DPROC)glewGetProcAddress((const GLubyte*)"glConvolutionFilter1D")) == NULL) || r;
  r = ((glConvolutionFilter2D = (PFNGLCONVOLUTIONFILTER2DPROC)glewGetProcAddress((const GLubyte*)"glConvolutionFilter2D")) == NULL) || r;
  r = ((glConvolutionParameterf = (PFNGLCONVOLUTIONPARAMETERFPROC)glewGetProcAddress((const GLubyte*)"glConvolutionParameterf")) == NULL) || r;
  r = ((glConvolutionParameterfv = (PFNGLCONVOLUTIONPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glConvolutionParameterfv")) == NULL) || r;
  r = ((glConvolutionParameteri = (PFNGLCONVOLUTIONPARAMETERIPROC)glewGetProcAddress((const GLubyte*)"glConvolutionParameteri")) == NULL) || r;
  r = ((glConvolutionParameteriv = (PFNGLCONVOLUTIONPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glConvolutionParameteriv")) == NULL) || r;
  r = ((glCopyColorSubTable = (PFNGLCOPYCOLORSUBTABLEPROC)glewGetProcAddress((const GLubyte*)"glCopyColorSubTable")) == NULL) || r;
  r = ((glCopyColorTable = (PFNGLCOPYCOLORTABLEPROC)glewGetProcAddress((const GLubyte*)"glCopyColorTable")) == NULL) || r;
  r = ((glCopyConvolutionFilter1D = (PFNGLCOPYCONVOLUTIONFILTER1DPROC)glewGetProcAddress((const GLubyte*)"glCopyConvolutionFilter1D")) == NULL) || r;
  r = ((glCopyConvolutionFilter2D = (PFNGLCOPYCONVOLUTIONFILTER2DPROC)glewGetProcAddress((const GLubyte*)"glCopyConvolutionFilter2D")) == NULL) || r;
  r = ((glGetColorTable = (PFNGLGETCOLORTABLEPROC)glewGetProcAddress((const GLubyte*)"glGetColorTable")) == NULL) || r;
  r = ((glGetColorTableParameterfv = (PFNGLGETCOLORTABLEPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glGetColorTableParameterfv")) == NULL) || r;
  r = ((glGetColorTableParameteriv = (PFNGLGETCOLORTABLEPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetColorTableParameteriv")) == NULL) || r;
  r = ((glGetConvolutionFilter = (PFNGLGETCONVOLUTIONFILTERPROC)glewGetProcAddress((const GLubyte*)"glGetConvolutionFilter")) == NULL) || r;
  r = ((glGetConvolutionParameterfv = (PFNGLGETCONVOLUTIONPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glGetConvolutionParameterfv")) == NULL) || r;
  r = ((glGetConvolutionParameteriv = (PFNGLGETCONVOLUTIONPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetConvolutionParameteriv")) == NULL) || r;
  r = ((glGetHistogram = (PFNGLGETHISTOGRAMPROC)glewGetProcAddress((const GLubyte*)"glGetHistogram")) == NULL) || r;
  r = ((glGetHistogramParameterfv = (PFNGLGETHISTOGRAMPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glGetHistogramParameterfv")) == NULL) || r;
  r = ((glGetHistogramParameteriv = (PFNGLGETHISTOGRAMPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetHistogramParameteriv")) == NULL) || r;
  r = ((glGetMinmax = (PFNGLGETMINMAXPROC)glewGetProcAddress((const GLubyte*)"glGetMinmax")) == NULL) || r;
  r = ((glGetMinmaxParameterfv = (PFNGLGETMINMAXPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glGetMinmaxParameterfv")) == NULL) || r;
  r = ((glGetMinmaxParameteriv = (PFNGLGETMINMAXPARAMETERIVPROC)glewGetProcAddress((const GLubyte*)"glGetMinmaxParameteriv")) == NULL) || r;
  r = ((glGetSeparableFilter = (PFNGLGETSEPARABLEFILTERPROC)glewGetProcAddress((const GLubyte*)"glGetSeparableFilter")) == NULL) || r;
  r = ((glHistogram = (PFNGLHISTOGRAMPROC)glewGetProcAddress((const GLubyte*)"glHistogram")) == NULL) || r;
  r = ((glMinmax = (PFNGLMINMAXPROC)glewGetProcAddress((const GLubyte*)"glMinmax")) == NULL) || r;
  r = ((glResetHistogram = (PFNGLRESETHISTOGRAMPROC)glewGetProcAddress((const GLubyte*)"glResetHistogram")) == NULL) || r;
  r = ((glResetMinmax = (PFNGLRESETMINMAXPROC)glewGetProcAddress((const GLubyte*)"glResetMinmax")) == NULL) || r;
  r = ((glSeparableFilter2D = (PFNGLSEPARABLEFILTER2DPROC)glewGetProcAddress((const GLubyte*)"glSeparableFilter2D")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_imaging */

#ifdef GL_ARB_instanced_arrays

static GLboolean _glewInit_GL_ARB_instanced_arrays (void)
{
  GLboolean r = GL_FALSE;

  r = ((glDrawArraysInstancedARB = (PFNGLDRAWARRAYSINSTANCEDARBPROC)glewGetProcAddress((const GLubyte*)"glDrawArraysInstancedARB")) == NULL) || r;
  r = ((glDrawElementsInstancedARB = (PFNGLDRAWELEMENTSINSTANCEDARBPROC)glewGetProcAddress((const GLubyte*)"glDrawElementsInstancedARB")) == NULL) || r;
  r = ((glVertexAttribDivisorARB = (PFNGLVERTEXATTRIBDIVISORARBPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribDivisorARB")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_instanced_arrays */

#ifdef GL_ARB_invalidate_subdata

static GLboolean _glewInit_GL_ARB_invalidate_subdata (void)
{
  GLboolean r = GL_FALSE;

  r = ((glInvalidateBufferData = (PFNGLINVALIDATEBUFFERDATAPROC)glewGetProcAddress((const GLubyte*)"glInvalidateBufferData")) == NULL) || r;
  r = ((glInvalidateBufferSubData = (PFNGLINVALIDATEBUFFERSUBDATAPROC)glewGetProcAddress((const GLubyte*)"glInvalidateBufferSubData")) == NULL) || r;
  r = ((glInvalidateFramebuffer = (PFNGLINVALIDATEFRAMEBUFFERPROC)glewGetProcAddress((const GLubyte*)"glInvalidateFramebuffer")) == NULL) || r;
  r = ((glInvalidateSubFramebuffer = (PFNGLINVALIDATESUBFRAMEBUFFERPROC)glewGetProcAddress((const GLubyte*)"glInvalidateSubFramebuffer")) == NULL) || r;
  r = ((glInvalidateTexImage = (PFNGLINVALIDATETEXIMAGEPROC)glewGetProcAddress((const GLubyte*)"glInvalidateTexImage")) == NULL) || r;
  r = ((glInvalidateTexSubImage = (PFNGLINVALIDATETEXSUBIMAGEPROC)glewGetProcAddress((const GLubyte*)"glInvalidateTexSubImage")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_invalidate_subdata */

#ifdef GL_ARB_map_buffer_range

static GLboolean _glewInit_GL_ARB_map_buffer_range (void)
{
  GLboolean r = GL_FALSE;

  r = ((glFlushMappedBufferRange = (PFNGLFLUSHMAPPEDBUFFERRANGEPROC)glewGetProcAddress((const GLubyte*)"glFlushMappedBufferRange")) == NULL) || r;
  r = ((glMapBufferRange = (PFNGLMAPBUFFERRANGEPROC)glewGetProcAddress((const GLubyte*)"glMapBufferRange")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_map_buffer_range */

#ifdef GL_ARB_multi_bind

static GLboolean _glewInit_GL_ARB_multi_bind (void)
{
  GLboolean r = GL_FALSE;

  r = ((glBindBuffersBase = (PFNGLBINDBUFFERSBASEPROC)glewGetProcAddress((const GLubyte*)"glBindBuffersBase")) == NULL) || r;
  r = ((glBindBuffersRange = (PFNGLBINDBUFFERSRANGEPROC)glewGetProcAddress((const GLubyte*)"glBindBuffersRange")) == NULL) || r;
  r = ((glBindImageTextures = (PFNGLBINDIMAGETEXTURESPROC)glewGetProcAddress((const GLubyte*)"glBindImageTextures")) == NULL) || r;
  r = ((glBindSamplers = (PFNGLBINDSAMPLERSPROC)glewGetProcAddress((const GLubyte*)"glBindSamplers")) == NULL) || r;
  r = ((glBindTextures = (PFNGLBINDTEXTURESPROC)glewGetProcAddress((const GLubyte*)"glBindTextures")) == NULL) || r;
  r = ((glBindVertexBuffers = (PFNGLBINDVERTEXBUFFERSPROC)glewGetProcAddress((const GLubyte*)"glBindVertexBuffers")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_multi_bind */

#ifdef GL_ARB_parallel_shader_compile

static GLboolean _glewInit_GL_ARB_parallel_shader_compile (void)
{
  GLboolean r = GL_FALSE;

  r = ((glMaxShaderCompilerThreadsARB = (PFNGLMAXSHADERCOMPILERTHREADSARBPROC)glewGetProcAddress((const GLubyte*)"glMaxShaderCompilerThreadsARB")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_parallel_shader_compile */

#ifdef GL_ARB_program_interface_query

static GLboolean _glewInit_GL_ARB_program_interface_query (void)
{
  GLboolean r = GL_FALSE;

  r = ((glGetProgramInterfaceiv = (PFNGLGETPROGRAMINTERFACEIVPROC)glewGetProcAddress((const GLubyte*)"glGetProgramInterfaceiv")) == NULL) || r;
  r = ((glGetProgramResourceIndex = (PFNGLGETPROGRAMRESOURCEINDEXPROC)glewGetProcAddress((const GLubyte*)"glGetProgramResourceIndex")) == NULL) || r;
  r = ((glGetProgramResourceLocation = (PFNGLGETPROGRAMRESOURCELOCATIONPROC)glewGetProcAddress((const GLubyte*)"glGetProgramResourceLocation")) == NULL) || r;
  r = ((glGetProgramResourceLocationIndex = (PFNGLGETPROGRAMRESOURCELOCATIONINDEXPROC)glewGetProcAddress((const GLubyte*)"glGetProgramResourceLocationIndex")) == NULL) || r;
  r = ((glGetProgramResourceName = (PFNGLGETPROGRAMRESOURCENAMEPROC)glewGetProcAddress((const GLubyte*)"glGetProgramResourceName")) == NULL) || r;
  r = ((glGetProgramResourceiv = (PFNGLGETPROGRAMRESOURCEIVPROC)glewGetProcAddress((const GLubyte*)"glGetProgramResourceiv")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_program_interface_query */

#ifdef GL_ARB_provoking_vertex

static GLboolean _glewInit_GL_ARB_provoking_vertex (void)
{
  GLboolean r = GL_FALSE;

  r = ((glProvokingVertex = (PFNGLPROVOKINGVERTEXPROC)glewGetProcAddress((const GLubyte*)"glProvokingVertex")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_provoking_vertex */

#ifdef GL_ARB_robustness

static GLboolean _glewInit_GL_ARB_robustness (void)
{
  GLboolean r = GL_FALSE;

  r = ((glGetGraphicsResetStatusARB = (PFNGLGETGRAPHICSRESETSTATUSARBPROC)glewGetProcAddress((const GLubyte*)"glGetGraphicsResetStatusARB")) == NULL) || r;
  r = ((glGetnColorTableARB = (PFNGLGETNCOLORTABLEARBPROC)glewGetProcAddress((const GLubyte*)"glGetnColorTableARB")) == NULL) || r;
  r = ((glGetnCompressedTexImageARB = (PFNGLGETNCOMPRESSEDTEXIMAGEARBPROC)glewGetProcAddress((const GLubyte*)"glGetnCompressedTexImageARB")) == NULL) || r;
  r = ((glGetnConvolutionFilterARB = (PFNGLGETNCONVOLUTIONFILTERARBPROC)glewGetProcAddress((const GLubyte*)"glGetnConvolutionFilterARB")) == NULL) || r;
  r = ((glGetnHistogramARB = (PFNGLGETNHISTOGRAMARBPROC)glewGetProcAddress((const GLubyte*)"glGetnHistogramARB")) == NULL) || r;
  r = ((glGetnMapdvARB = (PFNGLGETNMAPDVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnMapdvARB")) == NULL) || r;
  r = ((glGetnMapfvARB = (PFNGLGETNMAPFVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnMapfvARB")) == NULL) || r;
  r = ((glGetnMapivARB = (PFNGLGETNMAPIVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnMapivARB")) == NULL) || r;
  r = ((glGetnMinmaxARB = (PFNGLGETNMINMAXARBPROC)glewGetProcAddress((const GLubyte*)"glGetnMinmaxARB")) == NULL) || r;
  r = ((glGetnPixelMapfvARB = (PFNGLGETNPIXELMAPFVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnPixelMapfvARB")) == NULL) || r;
  r = ((glGetnPixelMapuivARB = (PFNGLGETNPIXELMAPUIVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnPixelMapuivARB")) == NULL) || r;
  r = ((glGetnPixelMapusvARB = (PFNGLGETNPIXELMAPUSVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnPixelMapusvARB")) == NULL) || r;
  r = ((glGetnPolygonStippleARB = (PFNGLGETNPOLYGONSTIPPLEARBPROC)glewGetProcAddress((const GLubyte*)"glGetnPolygonStippleARB")) == NULL) || r;
  r = ((glGetnSeparableFilterARB = (PFNGLGETNSEPARABLEFILTERARBPROC)glewGetProcAddress((const GLubyte*)"glGetnSeparableFilterARB")) == NULL) || r;
  r = ((glGetnTexImageARB = (PFNGLGETNTEXIMAGEARBPROC)glewGetProcAddress((const GLubyte*)"glGetnTexImageARB")) == NULL) || r;
  r = ((glGetnUniformdvARB = (PFNGLGETNUNIFORMDVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnUniformdvARB")) == NULL) || r;
  r = ((glGetnUniformfvARB = (PFNGLGETNUNIFORMFVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnUniformfvARB")) == NULL) || r;
  r = ((glGetnUniformivARB = (PFNGLGETNUNIFORMIVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnUniformivARB")) == NULL) || r;
  r = ((glGetnUniformuivARB = (PFNGLGETNUNIFORMUIVARBPROC)glewGetProcAddress((const GLubyte*)"glGetnUniformuivARB")) == NULL) || r;
  r = ((glReadnPixelsARB = (PFNGLREADNPIXELSARBPROC)glewGetProcAddress((const GLubyte*)"glReadnPixelsARB")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_robustness */

#ifdef GL_ARB_sample_shading

static GLboolean _glewInit_GL_ARB_sample_shading (void)
{
  GLboolean r = GL_FALSE;

  r = ((glMinSampleShadingARB = (PFNGLMINSAMPLESHADINGARBPROC)glewGetProcAddress((const GLubyte*)"glMinSampleShadingARB")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_sample_shading */

#ifdef GL_ARB_shader_image_load_store

static GLboolean _glewInit_GL_ARB_shader_image_load_store (void)
{
  GLboolean r = GL_FALSE;

  r = ((glBindImageTexture = (PFNGLBINDIMAGETEXTUREPROC)glewGetProcAddress((const GLubyte*)"glBindImageTexture")) == NULL) || r;
  r = ((glMemoryBarrier = (PFNGLMEMORYBARRIERPROC)glewGetProcAddress((const GLubyte*)"glMemoryBarrier")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_shader_image_load_store */

#ifdef GL_ARB_sync

static GLboolean _glewInit_GL_ARB_sync (void)
{
  GLboolean r = GL_FALSE;

  r = ((glClientWaitSync = (PFNGLCLIENTWAITSYNCPROC)glewGetProcAddress((const GLubyte*)"glClientWaitSync")) == NULL) || r;
  r = ((glDeleteSync = (PFNGLDELETESYNCPROC)glewGetProcAddress((const GLubyte*)"glDeleteSync")) == NULL) || r;
  r = ((glFenceSync = (PFNGLFENCESYNCPROC)glewGetProcAddress((const GLubyte*)"glFenceSync")) == NULL) || r;
  r = ((glGetInteger64v = (PFNGLGETINTEGER64VPROC)glewGetProcAddress((const GLubyte*)"glGetInteger64v")) == NULL) || r;
  r = ((glGetSynciv = (PFNGLGETSYNCIVPROC)glewGetProcAddress((const GLubyte*)"glGetSynciv")) == NULL) || r;
  r = ((glIsSync = (PFNGLISSYNCPROC)glewGetProcAddress((const GLubyte*)"glIsSync")) == NULL) || r;
  r = ((glWaitSync = (PFNGLWAITSYNCPROC)glewGetProcAddress((const GLubyte*)"glWaitSync")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_sync */

#ifdef GL_ARB_tessellation_shader

static GLboolean _glewInit_GL_ARB_tessellation_shader (void)
{
  GLboolean r = GL_FALSE;

  r = ((glPatchParameterfv = (PFNGLPATCHPARAMETERFVPROC)glewGetProcAddress((const GLubyte*)"glPatchParameterfv")) == NULL) || r;
  r = ((glPatchParameteri = (PFNGLPATCHPARAMETERIPROC)glewGetProcAddress((const GLubyte*)"glPatchParameteri")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_tessellation_shader */

#ifdef GL_ARB_texture_multisample

static GLboolean _glewInit_GL_ARB_texture_multisample (void)
{
  GLboolean r = GL_FALSE;

  r = ((glGetMultisamplefv = (PFNGLGETMULTISAMPLEFVPROC)glewGetProcAddress((const GLubyte*)"glGetMultisamplefv")) == NULL) || r;
  r = ((glSampleMaski = (PFNGLSAMPLEMASKIPROC)glewGetProcAddress((const GLubyte*)"glSampleMaski")) == NULL) || r;
  r = ((glTexImage2DMultisample = (PFNGLTEXIMAGE2DMULTISAMPLEPROC)glewGetProcAddress((const GLubyte*)"glTexImage2DMultisample")) == NULL) || r;
  r = ((glTexImage3DMultisample = (PFNGLTEXIMAGE3DMULTISAMPLEPROC)glewGetProcAddress((const GLubyte*)"glTexImage3DMultisample")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_texture_multisample */

#ifdef GL_ARB_texture_storage

static GLboolean _glewInit_GL_ARB_texture_storage (void)
{
  GLboolean r = GL_FALSE;

  r = ((glTexStorage1D = (PFNGLTEXSTORAGE1DPROC)glewGetProcAddress((const GLubyte*)"glTexStorage1D")) == NULL) || r;
  r = ((glTexStorage2D = (PFNGLTEXSTORAGE2DPROC)glewGetProcAddress((const GLubyte*)"glTexStorage2D")) == NULL) || r;
  r = ((glTexStorage3D = (PFNGLTEXSTORAGE3DPROC)glewGetProcAddress((const GLubyte*)"glTexStorage3D")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_texture_storage */

#ifdef GL_ARB_timer_query

static GLboolean _glewInit_GL_ARB_timer_query (void)
{
  GLboolean r = GL_FALSE;

  r = ((glGetQueryObjecti64v = (PFNGLGETQUERYOBJECTI64VPROC)glewGetProcAddress((const GLubyte*)"glGetQueryObjecti64v")) == NULL) || r;
  r = ((glGetQueryObjectui64v = (PFNGLGETQUERYOBJECTUI64VPROC)glewGetProcAddress((const GLubyte*)"glGetQueryObjectui64v")) == NULL) || r;
  r = ((glQueryCounter = (PFNGLQUERYCOUNTERPROC)glewGetProcAddress((const GLubyte*)"glQueryCounter")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_timer_query */

#ifdef GL_ARB_uniform_buffer_object

static GLboolean _glewInit_GL_ARB_uniform_buffer_object (void)
{
  GLboolean r = GL_FALSE;

  r = ((glBindBufferBase = (PFNGLBINDBUFFERBASEPROC)glewGetProcAddress((const GLubyte*)"glBindBufferBase")) == NULL) || r;
  r = ((glBindBufferRange = (PFNGLBINDBUFFERRANGEPROC)glewGetProcAddress((const GLubyte*)"glBindBufferRange")) == NULL) || r;
  r = ((glGetActiveUniformBlockName = (PFNGLGETACTIVEUNIFORMBLOCKNAMEPROC)glewGetProcAddress((const GLubyte*)"glGetActiveUniformBlockName")) == NULL) || r;
  r = ((glGetActiveUniformBlockiv = (PFNGLGETACTIVEUNIFORMBLOCKIVPROC)glewGetProcAddress((const GLubyte*)"glGetActiveUniformBlockiv")) == NULL) || r;
  r = ((glGetActiveUniformName = (PFNGLGETACTIVEUNIFORMNAMEPROC)glewGetProcAddress((const GLubyte*)"glGetActiveUniformName")) == NULL) || r;
  r = ((glGetActiveUniformsiv = (PFNGLGETACTIVEUNIFORMSIVPROC)glewGetProcAddress((const GLubyte*)"glGetActiveUniformsiv")) == NULL) || r;
  r = ((glGetIntegeri_v = (PFNGLGETINTEGERI_VPROC)glewGetProcAddress((const GLubyte*)"glGetIntegeri_v")) == NULL) || r;
  r = ((glGetUniformBlockIndex = (PFNGLGETUNIFORMBLOCKINDEXPROC)glewGetProcAddress((const GLubyte*)"glGetUniformBlockIndex")) == NULL) || r;
  r = ((glGetUniformIndices = (PFNGLGETUNIFORMINDICESPROC)glewGetProcAddress((const GLubyte*)"glGetUniformIndices")) == NULL) || r;
  r = ((glUniformBlockBinding = (PFNGLUNIFORMBLOCKBINDINGPROC)glewGetProcAddress((const GLubyte*)"glUniformBlockBinding")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_uniform_buffer_object */

#ifdef GL_ARB_vertex_array_object

static GLboolean _glewInit_GL_ARB_vertex_array_object (void)
{
  GLboolean r = GL_FALSE;

  r = ((glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)glewGetProcAddress((const GLubyte*)"glBindVertexArray")) == NULL) || r;
  r = ((glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC)glewGetProcAddress((const GLubyte*)"glDeleteVertexArrays")) == NULL) || r;
  r = ((glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)glewGetProcAddress((const GLubyte*)"glGenVertexArrays")) == NULL) || r;
  r = ((glIsVertexArray = (PFNGLISVERTEXARRAYPROC)glewGetProcAddress((const GLubyte*)"glIsVertexArray")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_vertex_array_object */

#ifdef GL_ARB_vertex_attrib_binding

static GLboolean _glewInit_GL_ARB_vertex_attrib_binding (void)
{
  GLboolean r = GL_FALSE;

  r = ((glBindVertexBuffer = (PFNGLBINDVERTEXBUFFERPROC)glewGetProcAddress((const GLubyte*)"glBindVertexBuffer")) == NULL) || r;
  r = ((glVertexArrayBindVertexBufferEXT = (PFNGLVERTEXARRAYBINDVERTEXBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayBindVertexBufferEXT")) == NULL) || r;
  r = ((glVertexArrayVertexAttribBindingEXT = (PFNGLVERTEXARRAYVERTEXATTRIBBINDINGEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayVertexAttribBindingEXT")) == NULL) || r;
  r = ((glVertexArrayVertexAttribFormatEXT = (PFNGLVERTEXARRAYVERTEXATTRIBFORMATEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayVertexAttribFormatEXT")) == NULL) || r;
  r = ((glVertexArrayVertexAttribIFormatEXT = (PFNGLVERTEXARRAYVERTEXATTRIBIFORMATEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayVertexAttribIFormatEXT")) == NULL) || r;
  r = ((glVertexArrayVertexAttribLFormatEXT = (PFNGLVERTEXARRAYVERTEXATTRIBLFORMATEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayVertexAttribLFormatEXT")) == NULL) || r;
  r = ((glVertexArrayVertexBindingDivisorEXT = (PFNGLVERTEXARRAYVERTEXBINDINGDIVISOREXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayVertexBindingDivisorEXT")) == NULL) || r;
  r = ((glVertexAttribBinding = (PFNGLVERTEXATTRIBBINDINGPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribBinding")) == NULL) || r;
  r = ((glVertexAttribFormat = (PFNGLVERTEXATTRIBFORMATPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribFormat")) == NULL) || r;
  r = ((glVertexAttribIFormat = (PFNGLVERTEXATTRIBIFORMATPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribIFormat")) == NULL) || r;
  r = ((glVertexAttribLFormat = (PFNGLVERTEXATTRIBLFORMATPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribLFormat")) == NULL) || r;
  r = ((glVertexBindingDivisor = (PFNGLVERTEXBINDINGDIVISORPROC)glewGetProcAddress((const GLubyte*)"glVertexBindingDivisor")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_vertex_attrib_binding */

#ifdef GL_ARB_vertex_type_2_10_10_10_rev

static GLboolean _glewInit_GL_ARB_vertex_type_2_10_10_10_rev (void)
{
  GLboolean r = GL_FALSE;

  r = ((glColorP3ui = (PFNGLCOLORP3UIPROC)glewGetProcAddress((const GLubyte*)"glColorP3ui")) == NULL) || r;
  r = ((glColorP3uiv = (PFNGLCOLORP3UIVPROC)glewGetProcAddress((const GLubyte*)"glColorP3uiv")) == NULL) || r;
  r = ((glColorP4ui = (PFNGLCOLORP4UIPROC)glewGetProcAddress((const GLubyte*)"glColorP4ui")) == NULL) || r;
  r = ((glColorP4uiv = (PFNGLCOLORP4UIVPROC)glewGetProcAddress((const GLubyte*)"glColorP4uiv")) == NULL) || r;
  r = ((glMultiTexCoordP1ui = (PFNGLMULTITEXCOORDP1UIPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordP1ui")) == NULL) || r;
  r = ((glMultiTexCoordP1uiv = (PFNGLMULTITEXCOORDP1UIVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordP1uiv")) == NULL) || r;
  r = ((glMultiTexCoordP2ui = (PFNGLMULTITEXCOORDP2UIPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordP2ui")) == NULL) || r;
  r = ((glMultiTexCoordP2uiv = (PFNGLMULTITEXCOORDP2UIVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordP2uiv")) == NULL) || r;
  r = ((glMultiTexCoordP3ui = (PFNGLMULTITEXCOORDP3UIPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordP3ui")) == NULL) || r;
  r = ((glMultiTexCoordP3uiv = (PFNGLMULTITEXCOORDP3UIVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordP3uiv")) == NULL) || r;
  r = ((glMultiTexCoordP4ui = (PFNGLMULTITEXCOORDP4UIPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordP4ui")) == NULL) || r;
  r = ((glMultiTexCoordP4uiv = (PFNGLMULTITEXCOORDP4UIVPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordP4uiv")) == NULL) || r;
  r = ((glNormalP3ui = (PFNGLNORMALP3UIPROC)glewGetProcAddress((const GLubyte*)"glNormalP3ui")) == NULL) || r;
  r = ((glNormalP3uiv = (PFNGLNORMALP3UIVPROC)glewGetProcAddress((const GLubyte*)"glNormalP3uiv")) == NULL) || r;
  r = ((glSecondaryColorP3ui = (PFNGLSECONDARYCOLORP3UIPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColorP3ui")) == NULL) || r;
  r = ((glSecondaryColorP3uiv = (PFNGLSECONDARYCOLORP3UIVPROC)glewGetProcAddress((const GLubyte*)"glSecondaryColorP3uiv")) == NULL) || r;
  r = ((glTexCoordP1ui = (PFNGLTEXCOORDP1UIPROC)glewGetProcAddress((const GLubyte*)"glTexCoordP1ui")) == NULL) || r;
  r = ((glTexCoordP1uiv = (PFNGLTEXCOORDP1UIVPROC)glewGetProcAddress((const GLubyte*)"glTexCoordP1uiv")) == NULL) || r;
  r = ((glTexCoordP2ui = (PFNGLTEXCOORDP2UIPROC)glewGetProcAddress((const GLubyte*)"glTexCoordP2ui")) == NULL) || r;
  r = ((glTexCoordP2uiv = (PFNGLTEXCOORDP2UIVPROC)glewGetProcAddress((const GLubyte*)"glTexCoordP2uiv")) == NULL) || r;
  r = ((glTexCoordP3ui = (PFNGLTEXCOORDP3UIPROC)glewGetProcAddress((const GLubyte*)"glTexCoordP3ui")) == NULL) || r;
  r = ((glTexCoordP3uiv = (PFNGLTEXCOORDP3UIVPROC)glewGetProcAddress((const GLubyte*)"glTexCoordP3uiv")) == NULL) || r;
  r = ((glTexCoordP4ui = (PFNGLTEXCOORDP4UIPROC)glewGetProcAddress((const GLubyte*)"glTexCoordP4ui")) == NULL) || r;
  r = ((glTexCoordP4uiv = (PFNGLTEXCOORDP4UIVPROC)glewGetProcAddress((const GLubyte*)"glTexCoordP4uiv")) == NULL) || r;
  r = ((glVertexAttribP1ui = (PFNGLVERTEXATTRIBP1UIPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribP1ui")) == NULL) || r;
  r = ((glVertexAttribP1uiv = (PFNGLVERTEXATTRIBP1UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribP1uiv")) == NULL) || r;
  r = ((glVertexAttribP2ui = (PFNGLVERTEXATTRIBP2UIPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribP2ui")) == NULL) || r;
  r = ((glVertexAttribP2uiv = (PFNGLVERTEXATTRIBP2UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribP2uiv")) == NULL) || r;
  r = ((glVertexAttribP3ui = (PFNGLVERTEXATTRIBP3UIPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribP3ui")) == NULL) || r;
  r = ((glVertexAttribP3uiv = (PFNGLVERTEXATTRIBP3UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribP3uiv")) == NULL) || r;
  r = ((glVertexAttribP4ui = (PFNGLVERTEXATTRIBP4UIPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribP4ui")) == NULL) || r;
  r = ((glVertexAttribP4uiv = (PFNGLVERTEXATTRIBP4UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribP4uiv")) == NULL) || r;
  r = ((glVertexP2ui = (PFNGLVERTEXP2UIPROC)glewGetProcAddress((const GLubyte*)"glVertexP2ui")) == NULL) || r;
  r = ((glVertexP2uiv = (PFNGLVERTEXP2UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexP2uiv")) == NULL) || r;
  r = ((glVertexP3ui = (PFNGLVERTEXP3UIPROC)glewGetProcAddress((const GLubyte*)"glVertexP3ui")) == NULL) || r;
  r = ((glVertexP3uiv = (PFNGLVERTEXP3UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexP3uiv")) == NULL) || r;
  r = ((glVertexP4ui = (PFNGLVERTEXP4UIPROC)glewGetProcAddress((const GLubyte*)"glVertexP4ui")) == NULL) || r;
  r = ((glVertexP4uiv = (PFNGLVERTEXP4UIVPROC)glewGetProcAddress((const GLubyte*)"glVertexP4uiv")) == NULL) || r;

  return r;
}

#endif /* GL_ARB_vertex_type_2_10_10_10_rev */

#ifdef GL_EXT_direct_state_access

static GLboolean _glewInit_GL_EXT_direct_state_access (void)
{
  GLboolean r = GL_FALSE;

  r = ((glBindMultiTextureEXT = (PFNGLBINDMULTITEXTUREEXTPROC)glewGetProcAddress((const GLubyte*)"glBindMultiTextureEXT")) == NULL) || r;
  r = ((glCheckNamedFramebufferStatusEXT = (PFNGLCHECKNAMEDFRAMEBUFFERSTATUSEXTPROC)glewGetProcAddress((const GLubyte*)"glCheckNamedFramebufferStatusEXT")) == NULL) || r;
  r = ((glClientAttribDefaultEXT = (PFNGLCLIENTATTRIBDEFAULTEXTPROC)glewGetProcAddress((const GLubyte*)"glClientAttribDefaultEXT")) == NULL) || r;
  r = ((glCompressedMultiTexImage1DEXT = (PFNGLCOMPRESSEDMULTITEXIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedMultiTexImage1DEXT")) == NULL) || r;
  r = ((glCompressedMultiTexImage2DEXT = (PFNGLCOMPRESSEDMULTITEXIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedMultiTexImage2DEXT")) == NULL) || r;
  r = ((glCompressedMultiTexImage3DEXT = (PFNGLCOMPRESSEDMULTITEXIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedMultiTexImage3DEXT")) == NULL) || r;
  r = ((glCompressedMultiTexSubImage1DEXT = (PFNGLCOMPRESSEDMULTITEXSUBIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedMultiTexSubImage1DEXT")) == NULL) || r;
  r = ((glCompressedMultiTexSubImage2DEXT = (PFNGLCOMPRESSEDMULTITEXSUBIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedMultiTexSubImage2DEXT")) == NULL) || r;
  r = ((glCompressedMultiTexSubImage3DEXT = (PFNGLCOMPRESSEDMULTITEXSUBIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedMultiTexSubImage3DEXT")) == NULL) || r;
  r = ((glCompressedTextureImage1DEXT = (PFNGLCOMPRESSEDTEXTUREIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedTextureImage1DEXT")) == NULL) || r;
  r = ((glCompressedTextureImage2DEXT = (PFNGLCOMPRESSEDTEXTUREIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedTextureImage2DEXT")) == NULL) || r;
  r = ((glCompressedTextureImage3DEXT = (PFNGLCOMPRESSEDTEXTUREIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedTextureImage3DEXT")) == NULL) || r;
  r = ((glCompressedTextureSubImage1DEXT = (PFNGLCOMPRESSEDTEXTURESUBIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedTextureSubImage1DEXT")) == NULL) || r;
  r = ((glCompressedTextureSubImage2DEXT = (PFNGLCOMPRESSEDTEXTURESUBIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedTextureSubImage2DEXT")) == NULL) || r;
  r = ((glCompressedTextureSubImage3DEXT = (PFNGLCOMPRESSEDTEXTURESUBIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glCompressedTextureSubImage3DEXT")) == NULL) || r;
  r = ((glCopyMultiTexImage1DEXT = (PFNGLCOPYMULTITEXIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyMultiTexImage1DEXT")) == NULL) || r;
  r = ((glCopyMultiTexImage2DEXT = (PFNGLCOPYMULTITEXIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyMultiTexImage2DEXT")) == NULL) || r;
  r = ((glCopyMultiTexSubImage1DEXT = (PFNGLCOPYMULTITEXSUBIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyMultiTexSubImage1DEXT")) == NULL) || r;
  r = ((glCopyMultiTexSubImage2DEXT = (PFNGLCOPYMULTITEXSUBIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyMultiTexSubImage2DEXT")) == NULL) || r;
  r = ((glCopyMultiTexSubImage3DEXT = (PFNGLCOPYMULTITEXSUBIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyMultiTexSubImage3DEXT")) == NULL) || r;
  r = ((glCopyTextureImage1DEXT = (PFNGLCOPYTEXTUREIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyTextureImage1DEXT")) == NULL) || r;
  r = ((glCopyTextureImage2DEXT = (PFNGLCOPYTEXTUREIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyTextureImage2DEXT")) == NULL) || r;
  r = ((glCopyTextureSubImage1DEXT = (PFNGLCOPYTEXTURESUBIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyTextureSubImage1DEXT")) == NULL) || r;
  r = ((glCopyTextureSubImage2DEXT = (PFNGLCOPYTEXTURESUBIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyTextureSubImage2DEXT")) == NULL) || r;
  r = ((glCopyTextureSubImage3DEXT = (PFNGLCOPYTEXTURESUBIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glCopyTextureSubImage3DEXT")) == NULL) || r;
  r = ((glDisableClientStateIndexedEXT = (PFNGLDISABLECLIENTSTATEINDEXEDEXTPROC)glewGetProcAddress((const GLubyte*)"glDisableClientStateIndexedEXT")) == NULL) || r;
  r = ((glDisableClientStateiEXT = (PFNGLDISABLECLIENTSTATEIEXTPROC)glewGetProcAddress((const GLubyte*)"glDisableClientStateiEXT")) == NULL) || r;
  r = ((glDisableVertexArrayAttribEXT = (PFNGLDISABLEVERTEXARRAYATTRIBEXTPROC)glewGetProcAddress((const GLubyte*)"glDisableVertexArrayAttribEXT")) == NULL) || r;
  r = ((glDisableVertexArrayEXT = (PFNGLDISABLEVERTEXARRAYEXTPROC)glewGetProcAddress((const GLubyte*)"glDisableVertexArrayEXT")) == NULL) || r;
  r = ((glEnableClientStateIndexedEXT = (PFNGLENABLECLIENTSTATEINDEXEDEXTPROC)glewGetProcAddress((const GLubyte*)"glEnableClientStateIndexedEXT")) == NULL) || r;
  r = ((glEnableClientStateiEXT = (PFNGLENABLECLIENTSTATEIEXTPROC)glewGetProcAddress((const GLubyte*)"glEnableClientStateiEXT")) == NULL) || r;
  r = ((glEnableVertexArrayAttribEXT = (PFNGLENABLEVERTEXARRAYATTRIBEXTPROC)glewGetProcAddress((const GLubyte*)"glEnableVertexArrayAttribEXT")) == NULL) || r;
  r = ((glEnableVertexArrayEXT = (PFNGLENABLEVERTEXARRAYEXTPROC)glewGetProcAddress((const GLubyte*)"glEnableVertexArrayEXT")) == NULL) || r;
  r = ((glFlushMappedNamedBufferRangeEXT = (PFNGLFLUSHMAPPEDNAMEDBUFFERRANGEEXTPROC)glewGetProcAddress((const GLubyte*)"glFlushMappedNamedBufferRangeEXT")) == NULL) || r;
  r = ((glFramebufferDrawBufferEXT = (PFNGLFRAMEBUFFERDRAWBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferDrawBufferEXT")) == NULL) || r;
  r = ((glFramebufferDrawBuffersEXT = (PFNGLFRAMEBUFFERDRAWBUFFERSEXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferDrawBuffersEXT")) == NULL) || r;
  r = ((glFramebufferReadBufferEXT = (PFNGLFRAMEBUFFERREADBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferReadBufferEXT")) == NULL) || r;
  r = ((glGenerateMultiTexMipmapEXT = (PFNGLGENERATEMULTITEXMIPMAPEXTPROC)glewGetProcAddress((const GLubyte*)"glGenerateMultiTexMipmapEXT")) == NULL) || r;
  r = ((glGenerateTextureMipmapEXT = (PFNGLGENERATETEXTUREMIPMAPEXTPROC)glewGetProcAddress((const GLubyte*)"glGenerateTextureMipmapEXT")) == NULL) || r;
  r = ((glGetCompressedMultiTexImageEXT = (PFNGLGETCOMPRESSEDMULTITEXIMAGEEXTPROC)glewGetProcAddress((const GLubyte*)"glGetCompressedMultiTexImageEXT")) == NULL) || r;
  r = ((glGetCompressedTextureImageEXT = (PFNGLGETCOMPRESSEDTEXTUREIMAGEEXTPROC)glewGetProcAddress((const GLubyte*)"glGetCompressedTextureImageEXT")) == NULL) || r;
  r = ((glGetDoubleIndexedvEXT = (PFNGLGETDOUBLEINDEXEDVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetDoubleIndexedvEXT")) == NULL) || r;
  r = ((glGetDoublei_vEXT = (PFNGLGETDOUBLEI_VEXTPROC)glewGetProcAddress((const GLubyte*)"glGetDoublei_vEXT")) == NULL) || r;
  r = ((glGetFloatIndexedvEXT = (PFNGLGETFLOATINDEXEDVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetFloatIndexedvEXT")) == NULL) || r;
  r = ((glGetFloati_vEXT = (PFNGLGETFLOATI_VEXTPROC)glewGetProcAddress((const GLubyte*)"glGetFloati_vEXT")) == NULL) || r;
  r = ((glGetFramebufferParameterivEXT = (PFNGLGETFRAMEBUFFERPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetFramebufferParameterivEXT")) == NULL) || r;
  r = ((glGetMultiTexEnvfvEXT = (PFNGLGETMULTITEXENVFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexEnvfvEXT")) == NULL) || r;
  r = ((glGetMultiTexEnvivEXT = (PFNGLGETMULTITEXENVIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexEnvivEXT")) == NULL) || r;
  r = ((glGetMultiTexGendvEXT = (PFNGLGETMULTITEXGENDVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexGendvEXT")) == NULL) || r;
  r = ((glGetMultiTexGenfvEXT = (PFNGLGETMULTITEXGENFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexGenfvEXT")) == NULL) || r;
  r = ((glGetMultiTexGenivEXT = (PFNGLGETMULTITEXGENIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexGenivEXT")) == NULL) || r;
  r = ((glGetMultiTexImageEXT = (PFNGLGETMULTITEXIMAGEEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexImageEXT")) == NULL) || r;
  r = ((glGetMultiTexLevelParameterfvEXT = (PFNGLGETMULTITEXLEVELPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexLevelParameterfvEXT")) == NULL) || r;
  r = ((glGetMultiTexLevelParameterivEXT = (PFNGLGETMULTITEXLEVELPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexLevelParameterivEXT")) == NULL) || r;
  r = ((glGetMultiTexParameterIivEXT = (PFNGLGETMULTITEXPARAMETERIIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexParameterIivEXT")) == NULL) || r;
  r = ((glGetMultiTexParameterIuivEXT = (PFNGLGETMULTITEXPARAMETERIUIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexParameterIuivEXT")) == NULL) || r;
  r = ((glGetMultiTexParameterfvEXT = (PFNGLGETMULTITEXPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexParameterfvEXT")) == NULL) || r;
  r = ((glGetMultiTexParameterivEXT = (PFNGLGETMULTITEXPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetMultiTexParameterivEXT")) == NULL) || r;
  r = ((glGetNamedBufferParameterivEXT = (PFNGLGETNAMEDBUFFERPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedBufferParameterivEXT")) == NULL) || r;
  r = ((glGetNamedBufferPointervEXT = (PFNGLGETNAMEDBUFFERPOINTERVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedBufferPointervEXT")) == NULL) || r;
  r = ((glGetNamedBufferSubDataEXT = (PFNGLGETNAMEDBUFFERSUBDATAEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedBufferSubDataEXT")) == NULL) || r;
  r = ((glGetNamedFramebufferAttachmentParameterivEXT = (PFNGLGETNAMEDFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedFramebufferAttachmentParameterivEXT")) == NULL) || r;
  r = ((glGetNamedProgramLocalParameterIivEXT = (PFNGLGETNAMEDPROGRAMLOCALPARAMETERIIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedProgramLocalParameterIivEXT")) == NULL) || r;
  r = ((glGetNamedProgramLocalParameterIuivEXT = (PFNGLGETNAMEDPROGRAMLOCALPARAMETERIUIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedProgramLocalParameterIuivEXT")) == NULL) || r;
  r = ((glGetNamedProgramLocalParameterdvEXT = (PFNGLGETNAMEDPROGRAMLOCALPARAMETERDVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedProgramLocalParameterdvEXT")) == NULL) || r;
  r = ((glGetNamedProgramLocalParameterfvEXT = (PFNGLGETNAMEDPROGRAMLOCALPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedProgramLocalParameterfvEXT")) == NULL) || r;
  r = ((glGetNamedProgramStringEXT = (PFNGLGETNAMEDPROGRAMSTRINGEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedProgramStringEXT")) == NULL) || r;
  r = ((glGetNamedProgramivEXT = (PFNGLGETNAMEDPROGRAMIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedProgramivEXT")) == NULL) || r;
  r = ((glGetNamedRenderbufferParameterivEXT = (PFNGLGETNAMEDRENDERBUFFERPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetNamedRenderbufferParameterivEXT")) == NULL) || r;
  r = ((glGetPointerIndexedvEXT = (PFNGLGETPOINTERINDEXEDVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetPointerIndexedvEXT")) == NULL) || r;
  r = ((glGetPointeri_vEXT = (PFNGLGETPOINTERI_VEXTPROC)glewGetProcAddress((const GLubyte*)"glGetPointeri_vEXT")) == NULL) || r;
  r = ((glGetTextureImageEXT = (PFNGLGETTEXTUREIMAGEEXTPROC)glewGetProcAddress((const GLubyte*)"glGetTextureImageEXT")) == NULL) || r;
  r = ((glGetTextureLevelParameterfvEXT = (PFNGLGETTEXTURELEVELPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetTextureLevelParameterfvEXT")) == NULL) || r;
  r = ((glGetTextureLevelParameterivEXT = (PFNGLGETTEXTURELEVELPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetTextureLevelParameterivEXT")) == NULL) || r;
  r = ((glGetTextureParameterIivEXT = (PFNGLGETTEXTUREPARAMETERIIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetTextureParameterIivEXT")) == NULL) || r;
  r = ((glGetTextureParameterIuivEXT = (PFNGLGETTEXTUREPARAMETERIUIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetTextureParameterIuivEXT")) == NULL) || r;
  r = ((glGetTextureParameterfvEXT = (PFNGLGETTEXTUREPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetTextureParameterfvEXT")) == NULL) || r;
  r = ((glGetTextureParameterivEXT = (PFNGLGETTEXTUREPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetTextureParameterivEXT")) == NULL) || r;
  r = ((glGetVertexArrayIntegeri_vEXT = (PFNGLGETVERTEXARRAYINTEGERI_VEXTPROC)glewGetProcAddress((const GLubyte*)"glGetVertexArrayIntegeri_vEXT")) == NULL) || r;
  r = ((glGetVertexArrayIntegervEXT = (PFNGLGETVERTEXARRAYINTEGERVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetVertexArrayIntegervEXT")) == NULL) || r;
  r = ((glGetVertexArrayPointeri_vEXT = (PFNGLGETVERTEXARRAYPOINTERI_VEXTPROC)glewGetProcAddress((const GLubyte*)"glGetVertexArrayPointeri_vEXT")) == NULL) || r;
  r = ((glGetVertexArrayPointervEXT = (PFNGLGETVERTEXARRAYPOINTERVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetVertexArrayPointervEXT")) == NULL) || r;
  r = ((glMapNamedBufferEXT = (PFNGLMAPNAMEDBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glMapNamedBufferEXT")) == NULL) || r;
  r = ((glMapNamedBufferRangeEXT = (PFNGLMAPNAMEDBUFFERRANGEEXTPROC)glewGetProcAddress((const GLubyte*)"glMapNamedBufferRangeEXT")) == NULL) || r;
  r = ((glMatrixFrustumEXT = (PFNGLMATRIXFRUSTUMEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixFrustumEXT")) == NULL) || r;
  r = ((glMatrixLoadIdentityEXT = (PFNGLMATRIXLOADIDENTITYEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixLoadIdentityEXT")) == NULL) || r;
  r = ((glMatrixLoadTransposedEXT = (PFNGLMATRIXLOADTRANSPOSEDEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixLoadTransposedEXT")) == NULL) || r;
  r = ((glMatrixLoadTransposefEXT = (PFNGLMATRIXLOADTRANSPOSEFEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixLoadTransposefEXT")) == NULL) || r;
  r = ((glMatrixLoaddEXT = (PFNGLMATRIXLOADDEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixLoaddEXT")) == NULL) || r;
  r = ((glMatrixLoadfEXT = (PFNGLMATRIXLOADFEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixLoadfEXT")) == NULL) || r;
  r = ((glMatrixMultTransposedEXT = (PFNGLMATRIXMULTTRANSPOSEDEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixMultTransposedEXT")) == NULL) || r;
  r = ((glMatrixMultTransposefEXT = (PFNGLMATRIXMULTTRANSPOSEFEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixMultTransposefEXT")) == NULL) || r;
  r = ((glMatrixMultdEXT = (PFNGLMATRIXMULTDEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixMultdEXT")) == NULL) || r;
  r = ((glMatrixMultfEXT = (PFNGLMATRIXMULTFEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixMultfEXT")) == NULL) || r;
  r = ((glMatrixOrthoEXT = (PFNGLMATRIXORTHOEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixOrthoEXT")) == NULL) || r;
  r = ((glMatrixPopEXT = (PFNGLMATRIXPOPEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixPopEXT")) == NULL) || r;
  r = ((glMatrixPushEXT = (PFNGLMATRIXPUSHEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixPushEXT")) == NULL) || r;
  r = ((glMatrixRotatedEXT = (PFNGLMATRIXROTATEDEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixRotatedEXT")) == NULL) || r;
  r = ((glMatrixRotatefEXT = (PFNGLMATRIXROTATEFEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixRotatefEXT")) == NULL) || r;
  r = ((glMatrixScaledEXT = (PFNGLMATRIXSCALEDEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixScaledEXT")) == NULL) || r;
  r = ((glMatrixScalefEXT = (PFNGLMATRIXSCALEFEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixScalefEXT")) == NULL) || r;
  r = ((glMatrixTranslatedEXT = (PFNGLMATRIXTRANSLATEDEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixTranslatedEXT")) == NULL) || r;
  r = ((glMatrixTranslatefEXT = (PFNGLMATRIXTRANSLATEFEXTPROC)glewGetProcAddress((const GLubyte*)"glMatrixTranslatefEXT")) == NULL) || r;
  r = ((glMultiTexBufferEXT = (PFNGLMULTITEXBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexBufferEXT")) == NULL) || r;
  r = ((glMultiTexCoordPointerEXT = (PFNGLMULTITEXCOORDPOINTEREXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexCoordPointerEXT")) == NULL) || r;
  r = ((glMultiTexEnvfEXT = (PFNGLMULTITEXENVFEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexEnvfEXT")) == NULL) || r;
  r = ((glMultiTexEnvfvEXT = (PFNGLMULTITEXENVFVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexEnvfvEXT")) == NULL) || r;
  r = ((glMultiTexEnviEXT = (PFNGLMULTITEXENVIEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexEnviEXT")) == NULL) || r;
  r = ((glMultiTexEnvivEXT = (PFNGLMULTITEXENVIVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexEnvivEXT")) == NULL) || r;
  r = ((glMultiTexGendEXT = (PFNGLMULTITEXGENDEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexGendEXT")) == NULL) || r;
  r = ((glMultiTexGendvEXT = (PFNGLMULTITEXGENDVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexGendvEXT")) == NULL) || r;
  r = ((glMultiTexGenfEXT = (PFNGLMULTITEXGENFEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexGenfEXT")) == NULL) || r;
  r = ((glMultiTexGenfvEXT = (PFNGLMULTITEXGENFVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexGenfvEXT")) == NULL) || r;
  r = ((glMultiTexGeniEXT = (PFNGLMULTITEXGENIEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexGeniEXT")) == NULL) || r;
  r = ((glMultiTexGenivEXT = (PFNGLMULTITEXGENIVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexGenivEXT")) == NULL) || r;
  r = ((glMultiTexImage1DEXT = (PFNGLMULTITEXIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexImage1DEXT")) == NULL) || r;
  r = ((glMultiTexImage2DEXT = (PFNGLMULTITEXIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexImage2DEXT")) == NULL) || r;
  r = ((glMultiTexImage3DEXT = (PFNGLMULTITEXIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexImage3DEXT")) == NULL) || r;
  r = ((glMultiTexParameterIivEXT = (PFNGLMULTITEXPARAMETERIIVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexParameterIivEXT")) == NULL) || r;
  r = ((glMultiTexParameterIuivEXT = (PFNGLMULTITEXPARAMETERIUIVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexParameterIuivEXT")) == NULL) || r;
  r = ((glMultiTexParameterfEXT = (PFNGLMULTITEXPARAMETERFEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexParameterfEXT")) == NULL) || r;
  r = ((glMultiTexParameterfvEXT = (PFNGLMULTITEXPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexParameterfvEXT")) == NULL) || r;
  r = ((glMultiTexParameteriEXT = (PFNGLMULTITEXPARAMETERIEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexParameteriEXT")) == NULL) || r;
  r = ((glMultiTexParameterivEXT = (PFNGLMULTITEXPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexParameterivEXT")) == NULL) || r;
  r = ((glMultiTexRenderbufferEXT = (PFNGLMULTITEXRENDERBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexRenderbufferEXT")) == NULL) || r;
  r = ((glMultiTexSubImage1DEXT = (PFNGLMULTITEXSUBIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexSubImage1DEXT")) == NULL) || r;
  r = ((glMultiTexSubImage2DEXT = (PFNGLMULTITEXSUBIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexSubImage2DEXT")) == NULL) || r;
  r = ((glMultiTexSubImage3DEXT = (PFNGLMULTITEXSUBIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glMultiTexSubImage3DEXT")) == NULL) || r;
  r = ((glNamedBufferDataEXT = (PFNGLNAMEDBUFFERDATAEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedBufferDataEXT")) == NULL) || r;
  r = ((glNamedBufferStorageEXT = (PFNGLNAMEDBUFFERSTORAGEEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedBufferStorageEXT")) == NULL) || r;
  r = ((glNamedBufferSubDataEXT = (PFNGLNAMEDBUFFERSUBDATAEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedBufferSubDataEXT")) == NULL) || r;
  r = ((glNamedCopyBufferSubDataEXT = (PFNGLNAMEDCOPYBUFFERSUBDATAEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedCopyBufferSubDataEXT")) == NULL) || r;
  r = ((glNamedFramebufferRenderbufferEXT = (PFNGLNAMEDFRAMEBUFFERRENDERBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferRenderbufferEXT")) == NULL) || r;
  r = ((glNamedFramebufferTexture1DEXT = (PFNGLNAMEDFRAMEBUFFERTEXTURE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferTexture1DEXT")) == NULL) || r;
  r = ((glNamedFramebufferTexture2DEXT = (PFNGLNAMEDFRAMEBUFFERTEXTURE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferTexture2DEXT")) == NULL) || r;
  r = ((glNamedFramebufferTexture3DEXT = (PFNGLNAMEDFRAMEBUFFERTEXTURE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferTexture3DEXT")) == NULL) || r;
  r = ((glNamedFramebufferTextureEXT = (PFNGLNAMEDFRAMEBUFFERTEXTUREEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferTextureEXT")) == NULL) || r;
  r = ((glNamedFramebufferTextureFaceEXT = (PFNGLNAMEDFRAMEBUFFERTEXTUREFACEEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferTextureFaceEXT")) == NULL) || r;
  r = ((glNamedFramebufferTextureLayerEXT = (PFNGLNAMEDFRAMEBUFFERTEXTURELAYEREXTPROC)glewGetProcAddress((const GLubyte*)"glNamedFramebufferTextureLayerEXT")) == NULL) || r;
  r = ((glNamedProgramLocalParameter4dEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETER4DEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameter4dEXT")) == NULL) || r;
  r = ((glNamedProgramLocalParameter4dvEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETER4DVEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameter4dvEXT")) == NULL) || r;
  r = ((glNamedProgramLocalParameter4fEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETER4FEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameter4fEXT")) == NULL) || r;
  r = ((glNamedProgramLocalParameter4fvEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETER4FVEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameter4fvEXT")) == NULL) || r;
  r = ((glNamedProgramLocalParameterI4iEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERI4IEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameterI4iEXT")) == NULL) || r;
  r = ((glNamedProgramLocalParameterI4ivEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERI4IVEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameterI4ivEXT")) == NULL) || r;
  r = ((glNamedProgramLocalParameterI4uiEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameterI4uiEXT")) == NULL) || r;
  r = ((glNamedProgramLocalParameterI4uivEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERI4UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameterI4uivEXT")) == NULL) || r;
  r = ((glNamedProgramLocalParameters4fvEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERS4FVEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParameters4fvEXT")) == NULL) || r;
  r = ((glNamedProgramLocalParametersI4ivEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERSI4IVEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParametersI4ivEXT")) == NULL) || r;
  r = ((glNamedProgramLocalParametersI4uivEXT = (PFNGLNAMEDPROGRAMLOCALPARAMETERSI4UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramLocalParametersI4uivEXT")) == NULL) || r;
  r = ((glNamedProgramStringEXT = (PFNGLNAMEDPROGRAMSTRINGEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedProgramStringEXT")) == NULL) || r;
  r = ((glNamedRenderbufferStorageEXT = (PFNGLNAMEDRENDERBUFFERSTORAGEEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedRenderbufferStorageEXT")) == NULL) || r;
  r = ((glNamedRenderbufferStorageMultisampleCoverageEXT = (PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLECOVERAGEEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedRenderbufferStorageMultisampleCoverageEXT")) == NULL) || r;
  r = ((glNamedRenderbufferStorageMultisampleEXT = (PFNGLNAMEDRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)glewGetProcAddress((const GLubyte*)"glNamedRenderbufferStorageMultisampleEXT")) == NULL) || r;
  r = ((glProgramUniform1fEXT = (PFNGLPROGRAMUNIFORM1FEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1fEXT")) == NULL) || r;
  r = ((glProgramUniform1fvEXT = (PFNGLPROGRAMUNIFORM1FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1fvEXT")) == NULL) || r;
  r = ((glProgramUniform1iEXT = (PFNGLPROGRAMUNIFORM1IEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1iEXT")) == NULL) || r;
  r = ((glProgramUniform1ivEXT = (PFNGLPROGRAMUNIFORM1IVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1ivEXT")) == NULL) || r;
  r = ((glProgramUniform1uiEXT = (PFNGLPROGRAMUNIFORM1UIEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1uiEXT")) == NULL) || r;
  r = ((glProgramUniform1uivEXT = (PFNGLPROGRAMUNIFORM1UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1uivEXT")) == NULL) || r;
  r = ((glProgramUniform2fEXT = (PFNGLPROGRAMUNIFORM2FEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2fEXT")) == NULL) || r;
  r = ((glProgramUniform2fvEXT = (PFNGLPROGRAMUNIFORM2FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2fvEXT")) == NULL) || r;
  r = ((glProgramUniform2iEXT = (PFNGLPROGRAMUNIFORM2IEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2iEXT")) == NULL) || r;
  r = ((glProgramUniform2ivEXT = (PFNGLPROGRAMUNIFORM2IVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2ivEXT")) == NULL) || r;
  r = ((glProgramUniform2uiEXT = (PFNGLPROGRAMUNIFORM2UIEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2uiEXT")) == NULL) || r;
  r = ((glProgramUniform2uivEXT = (PFNGLPROGRAMUNIFORM2UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2uivEXT")) == NULL) || r;
  r = ((glProgramUniform3fEXT = (PFNGLPROGRAMUNIFORM3FEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3fEXT")) == NULL) || r;
  r = ((glProgramUniform3fvEXT = (PFNGLPROGRAMUNIFORM3FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3fvEXT")) == NULL) || r;
  r = ((glProgramUniform3iEXT = (PFNGLPROGRAMUNIFORM3IEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3iEXT")) == NULL) || r;
  r = ((glProgramUniform3ivEXT = (PFNGLPROGRAMUNIFORM3IVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3ivEXT")) == NULL) || r;
  r = ((glProgramUniform3uiEXT = (PFNGLPROGRAMUNIFORM3UIEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3uiEXT")) == NULL) || r;
  r = ((glProgramUniform3uivEXT = (PFNGLPROGRAMUNIFORM3UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3uivEXT")) == NULL) || r;
  r = ((glProgramUniform4fEXT = (PFNGLPROGRAMUNIFORM4FEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4fEXT")) == NULL) || r;
  r = ((glProgramUniform4fvEXT = (PFNGLPROGRAMUNIFORM4FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4fvEXT")) == NULL) || r;
  r = ((glProgramUniform4iEXT = (PFNGLPROGRAMUNIFORM4IEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4iEXT")) == NULL) || r;
  r = ((glProgramUniform4ivEXT = (PFNGLPROGRAMUNIFORM4IVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4ivEXT")) == NULL) || r;
  r = ((glProgramUniform4uiEXT = (PFNGLPROGRAMUNIFORM4UIEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4uiEXT")) == NULL) || r;
  r = ((glProgramUniform4uivEXT = (PFNGLPROGRAMUNIFORM4UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4uivEXT")) == NULL) || r;
  r = ((glProgramUniformMatrix2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix2fvEXT")) == NULL) || r;
  r = ((glProgramUniformMatrix2x3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X3FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix2x3fvEXT")) == NULL) || r;
  r = ((glProgramUniformMatrix2x4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX2X4FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix2x4fvEXT")) == NULL) || r;
  r = ((glProgramUniformMatrix3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix3fvEXT")) == NULL) || r;
  r = ((glProgramUniformMatrix3x2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X2FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix3x2fvEXT")) == NULL) || r;
  r = ((glProgramUniformMatrix3x4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX3X4FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix3x4fvEXT")) == NULL) || r;
  r = ((glProgramUniformMatrix4fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix4fvEXT")) == NULL) || r;
  r = ((glProgramUniformMatrix4x2fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X2FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix4x2fvEXT")) == NULL) || r;
  r = ((glProgramUniformMatrix4x3fvEXT = (PFNGLPROGRAMUNIFORMMATRIX4X3FVEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramUniformMatrix4x3fvEXT")) == NULL) || r;
  r = ((glPushClientAttribDefaultEXT = (PFNGLPUSHCLIENTATTRIBDEFAULTEXTPROC)glewGetProcAddress((const GLubyte*)"glPushClientAttribDefaultEXT")) == NULL) || r;
  r = ((glTextureBufferEXT = (PFNGLTEXTUREBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glTextureBufferEXT")) == NULL) || r;
  r = ((glTextureImage1DEXT = (PFNGLTEXTUREIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureImage1DEXT")) == NULL) || r;
  r = ((glTextureImage2DEXT = (PFNGLTEXTUREIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureImage2DEXT")) == NULL) || r;
  r = ((glTextureImage3DEXT = (PFNGLTEXTUREIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureImage3DEXT")) == NULL) || r;
  r = ((glTexturePageCommitmentEXT = (PFNGLTEXTUREPAGECOMMITMENTEXTPROC)glewGetProcAddress((const GLubyte*)"glTexturePageCommitmentEXT")) == NULL) || r;
  r = ((glTextureParameterIivEXT = (PFNGLTEXTUREPARAMETERIIVEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureParameterIivEXT")) == NULL) || r;
  r = ((glTextureParameterIuivEXT = (PFNGLTEXTUREPARAMETERIUIVEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureParameterIuivEXT")) == NULL) || r;
  r = ((glTextureParameterfEXT = (PFNGLTEXTUREPARAMETERFEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureParameterfEXT")) == NULL) || r;
  r = ((glTextureParameterfvEXT = (PFNGLTEXTUREPARAMETERFVEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureParameterfvEXT")) == NULL) || r;
  r = ((glTextureParameteriEXT = (PFNGLTEXTUREPARAMETERIEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureParameteriEXT")) == NULL) || r;
  r = ((glTextureParameterivEXT = (PFNGLTEXTUREPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureParameterivEXT")) == NULL) || r;
  r = ((glTextureRenderbufferEXT = (PFNGLTEXTURERENDERBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glTextureRenderbufferEXT")) == NULL) || r;
  r = ((glTextureSubImage1DEXT = (PFNGLTEXTURESUBIMAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureSubImage1DEXT")) == NULL) || r;
  r = ((glTextureSubImage2DEXT = (PFNGLTEXTURESUBIMAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureSubImage2DEXT")) == NULL) || r;
  r = ((glTextureSubImage3DEXT = (PFNGLTEXTURESUBIMAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureSubImage3DEXT")) == NULL) || r;
  r = ((glUnmapNamedBufferEXT = (PFNGLUNMAPNAMEDBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glUnmapNamedBufferEXT")) == NULL) || r;
  r = ((glVertexArrayColorOffsetEXT = (PFNGLVERTEXARRAYCOLOROFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayColorOffsetEXT")) == NULL) || r;
  r = ((glVertexArrayEdgeFlagOffsetEXT = (PFNGLVERTEXARRAYEDGEFLAGOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayEdgeFlagOffsetEXT")) == NULL) || r;
  r = ((glVertexArrayFogCoordOffsetEXT = (PFNGLVERTEXARRAYFOGCOORDOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayFogCoordOffsetEXT")) == NULL) || r;
  r = ((glVertexArrayIndexOffsetEXT = (PFNGLVERTEXARRAYINDEXOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayIndexOffsetEXT")) == NULL) || r;
  r = ((glVertexArrayMultiTexCoordOffsetEXT = (PFNGLVERTEXARRAYMULTITEXCOORDOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayMultiTexCoordOffsetEXT")) == NULL) || r;
  r = ((glVertexArrayNormalOffsetEXT = (PFNGLVERTEXARRAYNORMALOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayNormalOffsetEXT")) == NULL) || r;
  r = ((glVertexArraySecondaryColorOffsetEXT = (PFNGLVERTEXARRAYSECONDARYCOLOROFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArraySecondaryColorOffsetEXT")) == NULL) || r;
  r = ((glVertexArrayTexCoordOffsetEXT = (PFNGLVERTEXARRAYTEXCOORDOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayTexCoordOffsetEXT")) == NULL) || r;
  r = ((glVertexArrayVertexAttribDivisorEXT = (PFNGLVERTEXARRAYVERTEXATTRIBDIVISOREXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayVertexAttribDivisorEXT")) == NULL) || r;
  r = ((glVertexArrayVertexAttribIOffsetEXT = (PFNGLVERTEXARRAYVERTEXATTRIBIOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayVertexAttribIOffsetEXT")) == NULL) || r;
  r = ((glVertexArrayVertexAttribOffsetEXT = (PFNGLVERTEXARRAYVERTEXATTRIBOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayVertexAttribOffsetEXT")) == NULL) || r;
  r = ((glVertexArrayVertexOffsetEXT = (PFNGLVERTEXARRAYVERTEXOFFSETEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexArrayVertexOffsetEXT")) == NULL) || r;

  return r;
}

#endif /* GL_EXT_direct_state_access */

#ifdef GL_EXT_framebuffer_blit

static GLboolean _glewInit_GL_EXT_framebuffer_blit (void)
{
  GLboolean r = GL_FALSE;

  r = ((glBlitFramebufferEXT = (PFNGLBLITFRAMEBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glBlitFramebufferEXT")) == NULL) || r;

  return r;
}

#endif /* GL_EXT_framebuffer_blit */

#ifdef GL_EXT_framebuffer_multisample

static GLboolean _glewInit_GL_EXT_framebuffer_multisample (void)
{
  GLboolean r = GL_FALSE;

  r = ((glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)glewGetProcAddress((const GLubyte*)"glRenderbufferStorageMultisampleEXT")) == NULL) || r;

  return r;
}

#endif /* GL_EXT_framebuffer_multisample */

#ifdef GL_EXT_framebuffer_object

static GLboolean _glewInit_GL_EXT_framebuffer_object (void)
{
  GLboolean r = GL_FALSE;

  r = ((glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glBindFramebufferEXT")) == NULL) || r;
  r = ((glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glBindRenderbufferEXT")) == NULL) || r;
  r = ((glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)glewGetProcAddress((const GLubyte*)"glCheckFramebufferStatusEXT")) == NULL) || r;
  r = ((glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)glewGetProcAddress((const GLubyte*)"glDeleteFramebuffersEXT")) == NULL) || r;
  r = ((glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)glewGetProcAddress((const GLubyte*)"glDeleteRenderbuffersEXT")) == NULL) || r;
  r = ((glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferRenderbufferEXT")) == NULL) || r;
  r = ((glFramebufferTexture1DEXT = (PFNGLFRAMEBUFFERTEXTURE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTexture1DEXT")) == NULL) || r;
  r = ((glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTexture2DEXT")) == NULL) || r;
  r = ((glFramebufferTexture3DEXT = (PFNGLFRAMEBUFFERTEXTURE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTexture3DEXT")) == NULL) || r;
  r = ((glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)glewGetProcAddress((const GLubyte*)"glGenFramebuffersEXT")) == NULL) || r;
  r = ((glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)glewGetProcAddress((const GLubyte*)"glGenRenderbuffersEXT")) == NULL) || r;
  r = ((glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC)glewGetProcAddress((const GLubyte*)"glGenerateMipmapEXT")) == NULL) || r;
  r = ((glGetFramebufferAttachmentParameterivEXT = (PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetFramebufferAttachmentParameterivEXT")) == NULL) || r;
  r = ((glGetRenderbufferParameterivEXT = (PFNGLGETRENDERBUFFERPARAMETERIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetRenderbufferParameterivEXT")) == NULL) || r;
  r = ((glIsFramebufferEXT = (PFNGLISFRAMEBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glIsFramebufferEXT")) == NULL) || r;
  r = ((glIsRenderbufferEXT = (PFNGLISRENDERBUFFEREXTPROC)glewGetProcAddress((const GLubyte*)"glIsRenderbufferEXT")) == NULL) || r;
  r = ((glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)glewGetProcAddress((const GLubyte*)"glRenderbufferStorageEXT")) == NULL) || r;

  return r;
}

#endif /* GL_EXT_framebuffer_object */

#ifdef GL_EXT_geometry_shader4

static GLboolean _glewInit_GL_EXT_geometry_shader4 (void)
{
  GLboolean r = GL_FALSE;

  r = ((glFramebufferTextureEXT = (PFNGLFRAMEBUFFERTEXTUREEXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTextureEXT")) == NULL) || r;
  r = ((glFramebufferTextureFaceEXT = (PFNGLFRAMEBUFFERTEXTUREFACEEXTPROC)glewGetProcAddress((const GLubyte*)"glFramebufferTextureFaceEXT")) == NULL) || r;
  r = ((glProgramParameteriEXT = (PFNGLPROGRAMPARAMETERIEXTPROC)glewGetProcAddress((const GLubyte*)"glProgramParameteriEXT")) == NULL) || r;

  return r;
}

#endif /* GL_EXT_geometry_shader4 */

#ifdef GL_EXT_gpu_shader4

static GLboolean _glewInit_GL_EXT_gpu_shader4 (void)
{
  GLboolean r = GL_FALSE;

  r = ((glBindFragDataLocationEXT = (PFNGLBINDFRAGDATALOCATIONEXTPROC)glewGetProcAddress((const GLubyte*)"glBindFragDataLocationEXT")) == NULL) || r;
  r = ((glGetFragDataLocationEXT = (PFNGLGETFRAGDATALOCATIONEXTPROC)glewGetProcAddress((const GLubyte*)"glGetFragDataLocationEXT")) == NULL) || r;
  r = ((glGetUniformuivEXT = (PFNGLGETUNIFORMUIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetUniformuivEXT")) == NULL) || r;
  r = ((glGetVertexAttribIivEXT = (PFNGLGETVERTEXATTRIBIIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribIivEXT")) == NULL) || r;
  r = ((glGetVertexAttribIuivEXT = (PFNGLGETVERTEXATTRIBIUIVEXTPROC)glewGetProcAddress((const GLubyte*)"glGetVertexAttribIuivEXT")) == NULL) || r;
  r = ((glUniform1uiEXT = (PFNGLUNIFORM1UIEXTPROC)glewGetProcAddress((const GLubyte*)"glUniform1uiEXT")) == NULL) || r;
  r = ((glUniform1uivEXT = (PFNGLUNIFORM1UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glUniform1uivEXT")) == NULL) || r;
  r = ((glUniform2uiEXT = (PFNGLUNIFORM2UIEXTPROC)glewGetProcAddress((const GLubyte*)"glUniform2uiEXT")) == NULL) || r;
  r = ((glUniform2uivEXT = (PFNGLUNIFORM2UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glUniform2uivEXT")) == NULL) || r;
  r = ((glUniform3uiEXT = (PFNGLUNIFORM3UIEXTPROC)glewGetProcAddress((const GLubyte*)"glUniform3uiEXT")) == NULL) || r;
  r = ((glUniform3uivEXT = (PFNGLUNIFORM3UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glUniform3uivEXT")) == NULL) || r;
  r = ((glUniform4uiEXT = (PFNGLUNIFORM4UIEXTPROC)glewGetProcAddress((const GLubyte*)"glUniform4uiEXT")) == NULL) || r;
  r = ((glUniform4uivEXT = (PFNGLUNIFORM4UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glUniform4uivEXT")) == NULL) || r;
  r = ((glVertexAttribI1iEXT = (PFNGLVERTEXATTRIBI1IEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI1iEXT")) == NULL) || r;
  r = ((glVertexAttribI1ivEXT = (PFNGLVERTEXATTRIBI1IVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI1ivEXT")) == NULL) || r;
  r = ((glVertexAttribI1uiEXT = (PFNGLVERTEXATTRIBI1UIEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI1uiEXT")) == NULL) || r;
  r = ((glVertexAttribI1uivEXT = (PFNGLVERTEXATTRIBI1UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI1uivEXT")) == NULL) || r;
  r = ((glVertexAttribI2iEXT = (PFNGLVERTEXATTRIBI2IEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI2iEXT")) == NULL) || r;
  r = ((glVertexAttribI2ivEXT = (PFNGLVERTEXATTRIBI2IVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI2ivEXT")) == NULL) || r;
  r = ((glVertexAttribI2uiEXT = (PFNGLVERTEXATTRIBI2UIEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI2uiEXT")) == NULL) || r;
  r = ((glVertexAttribI2uivEXT = (PFNGLVERTEXATTRIBI2UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI2uivEXT")) == NULL) || r;
  r = ((glVertexAttribI3iEXT = (PFNGLVERTEXATTRIBI3IEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI3iEXT")) == NULL) || r;
  r = ((glVertexAttribI3ivEXT = (PFNGLVERTEXATTRIBI3IVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI3ivEXT")) == NULL) || r;
  r = ((glVertexAttribI3uiEXT = (PFNGLVERTEXATTRIBI3UIEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI3uiEXT")) == NULL) || r;
  r = ((glVertexAttribI3uivEXT = (PFNGLVERTEXATTRIBI3UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI3uivEXT")) == NULL) || r;
  r = ((glVertexAttribI4bvEXT = (PFNGLVERTEXATTRIBI4BVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4bvEXT")) == NULL) || r;
  r = ((glVertexAttribI4iEXT = (PFNGLVERTEXATTRIBI4IEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4iEXT")) == NULL) || r;
  r = ((glVertexAttribI4ivEXT = (PFNGLVERTEXATTRIBI4IVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4ivEXT")) == NULL) || r;
  r = ((glVertexAttribI4svEXT = (PFNGLVERTEXATTRIBI4SVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4svEXT")) == NULL) || r;
  r = ((glVertexAttribI4ubvEXT = (PFNGLVERTEXATTRIBI4UBVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4ubvEXT")) == NULL) || r;
  r = ((glVertexAttribI4uiEXT = (PFNGLVERTEXATTRIBI4UIEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4uiEXT")) == NULL) || r;
  r = ((glVertexAttribI4uivEXT = (PFNGLVERTEXATTRIBI4UIVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4uivEXT")) == NULL) || r;
  r = ((glVertexAttribI4usvEXT = (PFNGLVERTEXATTRIBI4USVEXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribI4usvEXT")) == NULL) || r;
  r = ((glVertexAttribIPointerEXT = (PFNGLVERTEXATTRIBIPOINTEREXTPROC)glewGetProcAddress((const GLubyte*)"glVertexAttribIPointerEXT")) == NULL) || r;

  return r;
}

#endif /* GL_EXT_gpu_shader4 */

#ifdef GL_EXT_shader_image_load_store

static GLboolean _glewInit_GL_EXT_shader_image_load_store (void)
{
  GLboolean r = GL_FALSE;

  r = ((glBindImageTextureEXT = (PFNGLBINDIMAGETEXTUREEXTPROC)glewGetProcAddress((const GLubyte*)"glBindImageTextureEXT")) == NULL) || r;
  r = ((glMemoryBarrierEXT = (PFNGLMEMORYBARRIEREXTPROC)glewGetProcAddress((const GLubyte*)"glMemoryBarrierEXT")) == NULL) || r;

  return r;
}

#endif /* GL_EXT_shader_image_load_store */

#ifdef GL_EXT_texture_storage

static GLboolean _glewInit_GL_EXT_texture_storage (void)
{
  GLboolean r = GL_FALSE;

  r = ((glTexStorage1DEXT = (PFNGLTEXSTORAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glTexStorage1DEXT")) == NULL) || r;
  r = ((glTexStorage2DEXT = (PFNGLTEXSTORAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glTexStorage2DEXT")) == NULL) || r;
  r = ((glTexStorage3DEXT = (PFNGLTEXSTORAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glTexStorage3DEXT")) == NULL) || r;
  r = ((glTextureStorage1DEXT = (PFNGLTEXTURESTORAGE1DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureStorage1DEXT")) == NULL) || r;
  r = ((glTextureStorage2DEXT = (PFNGLTEXTURESTORAGE2DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureStorage2DEXT")) == NULL) || r;
  r = ((glTextureStorage3DEXT = (PFNGLTEXTURESTORAGE3DEXTPROC)glewGetProcAddress((const GLubyte*)"glTextureStorage3DEXT")) == NULL) || r;

  return r;
}

#endif /* GL_EXT_texture_storage */

#ifdef GL_INTEL_framebuffer_CMAA

static GLboolean _glewInit_GL_INTEL_framebuffer_CMAA (void)
{
  GLboolean r = GL_FALSE;

  r = ((glApplyFramebufferAttachmentCMAAINTEL = (PFNGLAPPLYFRAMEBUFFERATTACHMENTCMAAINTELPROC)glewGetProcAddress((const GLubyte*)"glApplyFramebufferAttachmentCMAAINTEL")) == NULL) || r;

  return r;
}

#endif /* GL_INTEL_framebuffer_CMAA */

#ifdef GL_KHR_debug

static GLboolean _glewInit_GL_KHR_debug (void)
{
  GLboolean r = GL_FALSE;

  r = ((glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)glewGetProcAddress((const GLubyte*)"glDebugMessageCallback")) == NULL) || r;
  r = ((glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROC)glewGetProcAddress((const GLubyte*)"glDebugMessageControl")) == NULL) || r;
  r = ((glDebugMessageInsert = (PFNGLDEBUGMESSAGEINSERTPROC)glewGetProcAddress((const GLubyte*)"glDebugMessageInsert")) == NULL) || r;
  r = ((glGetDebugMessageLog = (PFNGLGETDEBUGMESSAGELOGPROC)glewGetProcAddress((const GLubyte*)"glGetDebugMessageLog")) == NULL) || r;
  r = ((glGetObjectLabel = (PFNGLGETOBJECTLABELPROC)glewGetProcAddress((const GLubyte*)"glGetObjectLabel")) == NULL) || r;
  r = ((glGetObjectPtrLabel = (PFNGLGETOBJECTPTRLABELPROC)glewGetProcAddress((const GLubyte*)"glGetObjectPtrLabel")) == NULL) || r;
  r = ((glObjectLabel = (PFNGLOBJECTLABELPROC)glewGetProcAddress((const GLubyte*)"glObjectLabel")) == NULL) || r;
  r = ((glObjectPtrLabel = (PFNGLOBJECTPTRLABELPROC)glewGetProcAddress((const GLubyte*)"glObjectPtrLabel")) == NULL) || r;
  r = ((glPopDebugGroup = (PFNGLPOPDEBUGGROUPPROC)glewGetProcAddress((const GLubyte*)"glPopDebugGroup")) == NULL) || r;
  r = ((glPushDebugGroup = (PFNGLPUSHDEBUGGROUPPROC)glewGetProcAddress((const GLubyte*)"glPushDebugGroup")) == NULL) || r;

  return r;
}

#endif /* GL_KHR_debug */

#ifdef GL_KHR_parallel_shader_compile

static GLboolean _glewInit_GL_KHR_parallel_shader_compile (void)
{
  GLboolean r = GL_FALSE;

  r = ((glMaxShaderCompilerThreadsKHR = (PFNGLMAXSHADERCOMPILERTHREADSKHRPROC)glewGetProcAddress((const GLubyte*)"glMaxShaderCompilerThreadsKHR")) == NULL) || r;

  return r;
}

#endif /* GL_KHR_parallel_shader_compile */

#ifdef GL_KHR_robustness

static GLboolean _glewInit_GL_KHR_robustness (void)
{
  GLboolean r = GL_FALSE;

  r = ((glGetnUniformfv = (PFNGLGETNUNIFORMFVPROC)glewGetProcAddress((const GLubyte*)"glGetnUniformfv")) == NULL) || r;
  r = ((glGetnUniformiv = (PFNGLGETNUNIFORMIVPROC)glewGetProcAddress((const GLubyte*)"glGetnUniformiv")) == NULL) || r;
  r = ((glGetnUniformuiv = (PFNGLGETNUNIFORMUIVPROC)glewGetProcAddress((const GLubyte*)"glGetnUniformuiv")) == NULL) || r;
  r = ((glReadnPixels = (PFNGLREADNPIXELSPROC)glewGetProcAddress((const GLubyte*)"glReadnPixels")) == NULL) || r;

  return r;
}

#endif /* GL_KHR_robustness */

#ifdef GL_NV_conservative_raster

static GLboolean _glewInit_GL_NV_conservative_raster (void)
{
  GLboolean r = GL_FALSE;

  r = ((glSubpixelPrecisionBiasNV = (PFNGLSUBPIXELPRECISIONBIASNVPROC)glewGetProcAddress((const GLubyte*)"glSubpixelPrecisionBiasNV")) == NULL) || r;

  return r;
}

#endif /* GL_NV_conservative_raster */

#ifdef GL_NV_copy_image

static GLboolean _glewInit_GL_NV_copy_image (void)
{
  GLboolean r = GL_FALSE;

  r = ((glCopyImageSubDataNV = (PFNGLCOPYIMAGESUBDATANVPROC)glewGetProcAddress((const GLubyte*)"glCopyImageSubDataNV")) == NULL) || r;

  return r;
}

#endif /* GL_NV_copy_image */

#ifdef GL_NV_framebuffer_multisample_coverage

static GLboolean _glewInit_GL_NV_framebuffer_multisample_coverage (void)
{
  GLboolean r = GL_FALSE;

  r = ((glRenderbufferStorageMultisampleCoverageNV = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC)glewGetProcAddress((const GLubyte*)"glRenderbufferStorageMultisampleCoverageNV")) == NULL) || r;

  return r;
}

#endif /* GL_NV_framebuffer_multisample_coverage */

#ifdef GL_NV_gpu_shader5

static GLboolean _glewInit_GL_NV_gpu_shader5 (void)
{
  GLboolean r = GL_FALSE;

  r = ((glGetUniformi64vNV = (PFNGLGETUNIFORMI64VNVPROC)glewGetProcAddress((const GLubyte*)"glGetUniformi64vNV")) == NULL) || r;
  r = ((glGetUniformui64vNV = (PFNGLGETUNIFORMUI64VNVPROC)glewGetProcAddress((const GLubyte*)"glGetUniformui64vNV")) == NULL) || r;
  r = ((glProgramUniform1i64NV = (PFNGLPROGRAMUNIFORM1I64NVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1i64NV")) == NULL) || r;
  r = ((glProgramUniform1i64vNV = (PFNGLPROGRAMUNIFORM1I64VNVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1i64vNV")) == NULL) || r;
  r = ((glProgramUniform1ui64NV = (PFNGLPROGRAMUNIFORM1UI64NVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1ui64NV")) == NULL) || r;
  r = ((glProgramUniform1ui64vNV = (PFNGLPROGRAMUNIFORM1UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform1ui64vNV")) == NULL) || r;
  r = ((glProgramUniform2i64NV = (PFNGLPROGRAMUNIFORM2I64NVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2i64NV")) == NULL) || r;
  r = ((glProgramUniform2i64vNV = (PFNGLPROGRAMUNIFORM2I64VNVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2i64vNV")) == NULL) || r;
  r = ((glProgramUniform2ui64NV = (PFNGLPROGRAMUNIFORM2UI64NVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2ui64NV")) == NULL) || r;
  r = ((glProgramUniform2ui64vNV = (PFNGLPROGRAMUNIFORM2UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform2ui64vNV")) == NULL) || r;
  r = ((glProgramUniform3i64NV = (PFNGLPROGRAMUNIFORM3I64NVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3i64NV")) == NULL) || r;
  r = ((glProgramUniform3i64vNV = (PFNGLPROGRAMUNIFORM3I64VNVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3i64vNV")) == NULL) || r;
  r = ((glProgramUniform3ui64NV = (PFNGLPROGRAMUNIFORM3UI64NVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3ui64NV")) == NULL) || r;
  r = ((glProgramUniform3ui64vNV = (PFNGLPROGRAMUNIFORM3UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform3ui64vNV")) == NULL) || r;
  r = ((glProgramUniform4i64NV = (PFNGLPROGRAMUNIFORM4I64NVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4i64NV")) == NULL) || r;
  r = ((glProgramUniform4i64vNV = (PFNGLPROGRAMUNIFORM4I64VNVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4i64vNV")) == NULL) || r;
  r = ((glProgramUniform4ui64NV = (PFNGLPROGRAMUNIFORM4UI64NVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4ui64NV")) == NULL) || r;
  r = ((glProgramUniform4ui64vNV = (PFNGLPROGRAMUNIFORM4UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glProgramUniform4ui64vNV")) == NULL) || r;
  r = ((glUniform1i64NV = (PFNGLUNIFORM1I64NVPROC)glewGetProcAddress((const GLubyte*)"glUniform1i64NV")) == NULL) || r;
  r = ((glUniform1i64vNV = (PFNGLUNIFORM1I64VNVPROC)glewGetProcAddress((const GLubyte*)"glUniform1i64vNV")) == NULL) || r;
  r = ((glUniform1ui64NV = (PFNGLUNIFORM1UI64NVPROC)glewGetProcAddress((const GLubyte*)"glUniform1ui64NV")) == NULL) || r;
  r = ((glUniform1ui64vNV = (PFNGLUNIFORM1UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glUniform1ui64vNV")) == NULL) || r;
  r = ((glUniform2i64NV = (PFNGLUNIFORM2I64NVPROC)glewGetProcAddress((const GLubyte*)"glUniform2i64NV")) == NULL) || r;
  r = ((glUniform2i64vNV = (PFNGLUNIFORM2I64VNVPROC)glewGetProcAddress((const GLubyte*)"glUniform2i64vNV")) == NULL) || r;
  r = ((glUniform2ui64NV = (PFNGLUNIFORM2UI64NVPROC)glewGetProcAddress((const GLubyte*)"glUniform2ui64NV")) == NULL) || r;
  r = ((glUniform2ui64vNV = (PFNGLUNIFORM2UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glUniform2ui64vNV")) == NULL) || r;
  r = ((glUniform3i64NV = (PFNGLUNIFORM3I64NVPROC)glewGetProcAddress((const GLubyte*)"glUniform3i64NV")) == NULL) || r;
  r = ((glUniform3i64vNV = (PFNGLUNIFORM3I64VNVPROC)glewGetProcAddress((const GLubyte*)"glUniform3i64vNV")) == NULL) || r;
  r = ((glUniform3ui64NV = (PFNGLUNIFORM3UI64NVPROC)glewGetProcAddress((const GLubyte*)"glUniform3ui64NV")) == NULL) || r;
  r = ((glUniform3ui64vNV = (PFNGLUNIFORM3UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glUniform3ui64vNV")) == NULL) || r;
  r = ((glUniform4i64NV = (PFNGLUNIFORM4I64NVPROC)glewGetProcAddress((const GLubyte*)"glUniform4i64NV")) == NULL) || r;
  r = ((glUniform4i64vNV = (PFNGLUNIFORM4I64VNVPROC)glewGetProcAddress((const GLubyte*)"glUniform4i64vNV")) == NULL) || r;
  r = ((glUniform4ui64NV = (PFNGLUNIFORM4UI64NVPROC)glewGetProcAddress((const GLubyte*)"glUniform4ui64NV")) == NULL) || r;
  r = ((glUniform4ui64vNV = (PFNGLUNIFORM4UI64VNVPROC)glewGetProcAddress((const GLubyte*)"glUniform4ui64vNV")) == NULL) || r;

  return r;
}

#endif /* GL_NV_gpu_shader5 */

/* ------------------------------------------------------------------------- */

static int _glewExtensionCompare (const char *s1, const char *s2)
{
  /* http://www.chanduthedev.com/2012/07/strcmp-implementation-in-c.html */
  while (*s1 || *s2)
  {
    if (*s1 > *s2)
      return 1;
    if (*s1 < *s2)
      return -1;
    s1++;
    s2++;
  }
  return 0;
}

static ptrdiff_t _glewBsearchExtension (const char* name)
{
  ptrdiff_t lo = 0, hi = sizeof(_glewExtensionLookup) / sizeof(char*) - 2;

  while (lo <= hi)
  {
    ptrdiff_t mid = (lo + hi) / 2;
    const int cmp = _glewExtensionCompare(name, _glewExtensionLookup[mid]);
    if (cmp < 0) hi = mid - 1;
    else if (cmp > 0) lo = mid + 1;
    else return mid;
  }
  return -1;
}

static GLboolean *_glewGetExtensionString (const char *name)
{
  ptrdiff_t n = _glewBsearchExtension(name);
  if (n >= 0) return &_glewExtensionString[n];
  return NULL;
}

static GLboolean *_glewGetExtensionEnable (const char *name)
{
  ptrdiff_t n = _glewBsearchExtension(name);
  if (n >= 0) return _glewExtensionEnabled[n];
  return NULL;
}

static const char *_glewNextSpace (const char *i)
{
  const char *j = i;
  if (j)
    while (*j!=' ' && *j) ++j;
  return j;
}

static const char *_glewNextNonSpace (const char *i)
{
  const char *j = i;
  if (j)
    while (*j==' ') ++j;
  return j;
}

GLboolean GLEWAPIENTRY glewGetExtension (const char* name)
{
  GLboolean *enable = _glewGetExtensionString(name);
  if (enable)
    return *enable;
  return GL_FALSE;
}

GLboolean GLEWAPIENTRY glewEnableExtension (const char* name)
{
  GLboolean* pBool = _glewGetExtensionEnable(name);
  if(pBool)
  {
    (*pBool) = GL_TRUE;
    return GL_TRUE;
  }
  return GL_FALSE;
}

GLboolean GLEWAPIENTRY glewDisableExtension (const char* name)
{
  GLboolean* pBool = _glewGetExtensionEnable(name);
  if(pBool)
  {
    (*pBool) = GL_FALSE;
    return GL_TRUE;
  }
  return GL_FALSE;
}

/* ------------------------------------------------------------------------- */

typedef const GLubyte* (GLAPIENTRY * PFNGLGETSTRINGPROC) (GLenum name);
typedef void (GLAPIENTRY * PFNGLGETINTEGERVPROC) (GLenum pname, GLint *params);

static GLenum GLEWAPIENTRY glewContextInit (void)
{
  PFNGLGETSTRINGPROC getString;
  const GLubyte* s;
  GLuint dot;
  GLint major, minor;

  #ifdef _WIN32
  getString = glGetString;
  #else
  getString = (PFNGLGETSTRINGPROC) glewGetProcAddress((const GLubyte*)"glGetString");
  if (!getString)
    return GLEW_ERROR_NO_GL_VERSION;
  #endif

  /* query opengl version */
  s = getString(GL_VERSION);
  dot = _glewStrCLen(s, '.');
  if (dot == 0)
    return GLEW_ERROR_NO_GL_VERSION;

  major = s[dot-1]-'0';
  minor = s[dot+1]-'0';

  if (minor < 0 || minor > 9)
    minor = 0;
  if (major<0 || major>9)
    return GLEW_ERROR_NO_GL_VERSION;

  if (major == 1 && minor == 0)
  {
    return GLEW_ERROR_GL_VERSION_10_ONLY;
  }
  else
  {
    GLEW_VERSION_4_6   = ( major > 4 )                 || ( major == 4 && minor >= 6 ) ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_4_5   = GLEW_VERSION_4_6   == GL_TRUE || ( major == 4 && minor >= 5 ) ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_4_4   = GLEW_VERSION_4_5   == GL_TRUE || ( major == 4 && minor >= 4 ) ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_4_3   = GLEW_VERSION_4_4   == GL_TRUE || ( major == 4 && minor >= 3 ) ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_4_2   = GLEW_VERSION_4_3   == GL_TRUE || ( major == 4 && minor >= 2 ) ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_4_1   = GLEW_VERSION_4_2   == GL_TRUE || ( major == 4 && minor >= 1 ) ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_4_0   = GLEW_VERSION_4_1   == GL_TRUE || ( major == 4               ) ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_3_3   = GLEW_VERSION_4_0   == GL_TRUE || ( major == 3 && minor >= 3 ) ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_3_2   = GLEW_VERSION_3_3   == GL_TRUE || ( major == 3 && minor >= 2 ) ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_3_1   = GLEW_VERSION_3_2   == GL_TRUE || ( major == 3 && minor >= 1 ) ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_3_0   = GLEW_VERSION_3_1   == GL_TRUE || ( major == 3               ) ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_2_1   = GLEW_VERSION_3_0   == GL_TRUE || ( major == 2 && minor >= 1 ) ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_2_0   = GLEW_VERSION_2_1   == GL_TRUE || ( major == 2               ) ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_1_5   = GLEW_VERSION_2_0   == GL_TRUE || ( major == 1 && minor >= 5 ) ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_1_4   = GLEW_VERSION_1_5   == GL_TRUE || ( major == 1 && minor >= 4 ) ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_1_3   = GLEW_VERSION_1_4   == GL_TRUE || ( major == 1 && minor >= 3 ) ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_1_2_1 = GLEW_VERSION_1_3   == GL_TRUE                                 ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_1_2   = GLEW_VERSION_1_2_1 == GL_TRUE || ( major == 1 && minor >= 2 ) ? GL_TRUE : GL_FALSE;
    GLEW_VERSION_1_1   = GLEW_VERSION_1_2   == GL_TRUE || ( major == 1 && minor >= 1 ) ? GL_TRUE : GL_FALSE;
  }

  {
    size_t i;
    for (i = 0; i < sizeof(_glewExtensionString) / sizeof(_glewExtensionString[0]); ++i)
      _glewExtensionString[i] = GL_FALSE;
  }

  if (GLEW_VERSION_3_0)
  {
    GLint n = 0;
    GLint i;
    PFNGLGETINTEGERVPROC getIntegerv;
    PFNGLGETSTRINGIPROC getStringi;
    const char *ext;
    GLboolean *enable;

    #ifdef _WIN32
    getIntegerv = glGetIntegerv;
    #else
    getIntegerv = (PFNGLGETINTEGERVPROC) glewGetProcAddress((const GLubyte*)"glGetIntegerv");
    #endif

    if (getIntegerv)
      getIntegerv(GL_NUM_EXTENSIONS, &n);

    /* glGetStringi is OpenGL 3.0 */
    getStringi = (PFNGLGETSTRINGIPROC) glewGetProcAddress((const GLubyte*)"glGetStringi");
    if (getStringi)
      for (i = 0; i<n; ++i)
      {
        ext = (const char *) getStringi(GL_EXTENSIONS, i);

        /* Based on extension string(s), experimental mode, glewIsSupported purposes */
        enable = _glewGetExtensionEnable(ext);
        if (enable)
          *enable = GL_TRUE;
      }
  }
  else
  {
    const char *extensions;
    const char *end;
    const char *i;
    const char *j;
    char ext[128];
    GLboolean *enable;

    extensions = (const char *) getString(GL_EXTENSIONS);

    if (extensions)
    {
      end = extensions + _glewStrLen((const GLubyte *) extensions);
      for (i=extensions; i<end; i = j + 1)
      {
        i = _glewNextNonSpace(i);
        j = _glewNextSpace(i);

        /* Copy extension into NUL terminated string */
        if (j-i >= (ptrdiff_t) sizeof(ext))
          continue;
        _glewStrCopy(ext, i, ' ');

        /* Based on extension string(s), experimental mode, glewIsSupported purposes */
        enable = _glewGetExtensionEnable(ext);
        if (enable)
          *enable = GL_TRUE;
      }
    }
  }
#ifdef GL_VERSION_1_2
  if (glewExperimental || GLEW_VERSION_1_2) GLEW_VERSION_1_2 = !_glewInit_GL_VERSION_1_2();
#endif /* GL_VERSION_1_2 */
#ifdef GL_VERSION_1_3
  if (glewExperimental || GLEW_VERSION_1_3) GLEW_VERSION_1_3 = !_glewInit_GL_VERSION_1_3();
#endif /* GL_VERSION_1_3 */
#ifdef GL_VERSION_1_4
  if (glewExperimental || GLEW_VERSION_1_4) GLEW_VERSION_1_4 = !_glewInit_GL_VERSION_1_4();
#endif /* GL_VERSION_1_4 */
#ifdef GL_VERSION_1_5
  if (glewExperimental || GLEW_VERSION_1_5) GLEW_VERSION_1_5 = !_glewInit_GL_VERSION_1_5();
#endif /* GL_VERSION_1_5 */
#ifdef GL_VERSION_2_0
  if (glewExperimental || GLEW_VERSION_2_0) GLEW_VERSION_2_0 = !_glewInit_GL_VERSION_2_0();
#endif /* GL_VERSION_2_0 */
#ifdef GL_VERSION_2_1
  if (glewExperimental || GLEW_VERSION_2_1) GLEW_VERSION_2_1 = !_glewInit_GL_VERSION_2_1();
#endif /* GL_VERSION_2_1 */
#ifdef GL_VERSION_3_0
  if (glewExperimental || GLEW_VERSION_3_0) GLEW_VERSION_3_0 = !_glewInit_GL_VERSION_3_0();
#endif /* GL_VERSION_3_0 */
#ifdef GL_VERSION_3_1
  if (glewExperimental || GLEW_VERSION_3_1) GLEW_VERSION_3_1 = !_glewInit_GL_VERSION_3_1();
#endif /* GL_VERSION_3_1 */
#ifdef GL_VERSION_3_2
  if (glewExperimental || GLEW_VERSION_3_2) GLEW_VERSION_3_2 = !_glewInit_GL_VERSION_3_2();
#endif /* GL_VERSION_3_2 */
#ifdef GL_VERSION_3_3
  if (glewExperimental || GLEW_VERSION_3_3) GLEW_VERSION_3_3 = !_glewInit_GL_VERSION_3_3();
#endif /* GL_VERSION_3_3 */
#ifdef GL_VERSION_4_0
  if (glewExperimental || GLEW_VERSION_4_0) GLEW_VERSION_4_0 = !_glewInit_GL_VERSION_4_0();
#endif /* GL_VERSION_4_0 */
#ifdef GL_VERSION_4_1
  if (glewExperimental || GLEW_VERSION_4_1) GLEW_VERSION_4_1 = !_glewInit_GL_VERSION_4_1();
#endif /* GL_VERSION_4_1 */
#ifdef GL_VERSION_4_2
  if (glewExperimental || GLEW_VERSION_4_2) GLEW_VERSION_4_2 = !_glewInit_GL_VERSION_4_2();
#endif /* GL_VERSION_4_2 */
#ifdef GL_VERSION_4_3
  if (glewExperimental || GLEW_VERSION_4_3) GLEW_VERSION_4_3 = !_glewInit_GL_VERSION_4_3();
#endif /* GL_VERSION_4_3 */
#ifdef GL_VERSION_4_4
  if (glewExperimental || GLEW_VERSION_4_4) GLEW_VERSION_4_4 = !_glewInit_GL_VERSION_4_4();
#endif /* GL_VERSION_4_4 */
#ifdef GL_VERSION_4_5
  if (glewExperimental || GLEW_VERSION_4_5) GLEW_VERSION_4_5 = !_glewInit_GL_VERSION_4_5();
#endif /* GL_VERSION_4_5 */
#ifdef GL_VERSION_4_6
  if (glewExperimental || GLEW_VERSION_4_6) GLEW_VERSION_4_6 = !_glewInit_GL_VERSION_4_6();
#endif /* GL_VERSION_4_6 */
#ifdef GL_AMD_framebuffer_multisample_advanced
  if (glewExperimental || GLEW_AMD_framebuffer_multisample_advanced) GLEW_AMD_framebuffer_multisample_advanced = !_glewInit_GL_AMD_framebuffer_multisample_advanced();
#endif /* GL_AMD_framebuffer_multisample_advanced */
#ifdef GL_ARB_buffer_storage
  if (glewExperimental || GLEW_ARB_buffer_storage) GLEW_ARB_buffer_storage = !_glewInit_GL_ARB_buffer_storage();
#endif /* GL_ARB_buffer_storage */
#ifdef GL_ARB_clear_buffer_object
  if (glewExperimental || GLEW_ARB_clear_buffer_object) GLEW_ARB_clear_buffer_object = !_glewInit_GL_ARB_clear_buffer_object();
#endif /* GL_ARB_clear_buffer_object */
#ifdef GL_ARB_clear_texture
  if (glewExperimental || GLEW_ARB_clear_texture) GLEW_ARB_clear_texture = !_glewInit_GL_ARB_clear_texture();
#endif /* GL_ARB_clear_texture */
#ifdef GL_ARB_compute_shader
  if (glewExperimental || GLEW_ARB_compute_shader) GLEW_ARB_compute_shader = !_glewInit_GL_ARB_compute_shader();
#endif /* GL_ARB_compute_shader */
#ifdef GL_ARB_copy_buffer
  if (glewExperimental || GLEW_ARB_copy_buffer) GLEW_ARB_copy_buffer = !_glewInit_GL_ARB_copy_buffer();
#endif /* GL_ARB_copy_buffer */
#ifdef GL_ARB_copy_image
  if (glewExperimental || GLEW_ARB_copy_image) GLEW_ARB_copy_image = !_glewInit_GL_ARB_copy_image();
#endif /* GL_ARB_copy_image */
#ifdef GL_ARB_direct_state_access
  if (glewExperimental || GLEW_ARB_direct_state_access) GLEW_ARB_direct_state_access = !_glewInit_GL_ARB_direct_state_access();
#endif /* GL_ARB_direct_state_access */
#ifdef GL_ARB_draw_elements_base_vertex
  if (glewExperimental || GLEW_ARB_draw_elements_base_vertex) GLEW_ARB_draw_elements_base_vertex = !_glewInit_GL_ARB_draw_elements_base_vertex();
#endif /* GL_ARB_draw_elements_base_vertex */
#ifdef GL_ARB_framebuffer_object
  if (glewExperimental || GLEW_ARB_framebuffer_object) GLEW_ARB_framebuffer_object = !_glewInit_GL_ARB_framebuffer_object();
#endif /* GL_ARB_framebuffer_object */
#ifdef GL_ARB_geometry_shader4
  if (glewExperimental || GLEW_ARB_geometry_shader4) GLEW_ARB_geometry_shader4 = !_glewInit_GL_ARB_geometry_shader4();
#endif /* GL_ARB_geometry_shader4 */
#ifdef GL_ARB_get_program_binary
  if (glewExperimental || GLEW_ARB_get_program_binary) GLEW_ARB_get_program_binary = !_glewInit_GL_ARB_get_program_binary();
#endif /* GL_ARB_get_program_binary */
#ifdef GL_ARB_imaging
  if (glewExperimental || GLEW_ARB_imaging) GLEW_ARB_imaging = !_glewInit_GL_ARB_imaging();
#endif /* GL_ARB_imaging */
#ifdef GL_ARB_instanced_arrays
  if (glewExperimental || GLEW_ARB_instanced_arrays) GLEW_ARB_instanced_arrays = !_glewInit_GL_ARB_instanced_arrays();
#endif /* GL_ARB_instanced_arrays */
#ifdef GL_ARB_invalidate_subdata
  if (glewExperimental || GLEW_ARB_invalidate_subdata) GLEW_ARB_invalidate_subdata = !_glewInit_GL_ARB_invalidate_subdata();
#endif /* GL_ARB_invalidate_subdata */
#ifdef GL_ARB_map_buffer_range
  if (glewExperimental || GLEW_ARB_map_buffer_range) GLEW_ARB_map_buffer_range = !_glewInit_GL_ARB_map_buffer_range();
#endif /* GL_ARB_map_buffer_range */
#ifdef GL_ARB_multi_bind
  if (glewExperimental || GLEW_ARB_multi_bind) GLEW_ARB_multi_bind = !_glewInit_GL_ARB_multi_bind();
#endif /* GL_ARB_multi_bind */
#ifdef GL_ARB_parallel_shader_compile
  if (glewExperimental || GLEW_ARB_parallel_shader_compile) GLEW_ARB_parallel_shader_compile = !_glewInit_GL_ARB_parallel_shader_compile();
#endif /* GL_ARB_parallel_shader_compile */
#ifdef GL_ARB_program_interface_query
  if (glewExperimental || GLEW_ARB_program_interface_query) GLEW_ARB_program_interface_query = !_glewInit_GL_ARB_program_interface_query();
#endif /* GL_ARB_program_interface_query */
#ifdef GL_ARB_provoking_vertex
  if (glewExperimental || GLEW_ARB_provoking_vertex) GLEW_ARB_provoking_vertex = !_glewInit_GL_ARB_provoking_vertex();
#endif /* GL_ARB_provoking_vertex */
#ifdef GL_ARB_robustness
  if (glewExperimental || GLEW_ARB_robustness) GLEW_ARB_robustness = !_glewInit_GL_ARB_robustness();
#endif /* GL_ARB_robustness */
#ifdef GL_ARB_sample_shading
  if (glewExperimental || GLEW_ARB_sample_shading) GLEW_ARB_sample_shading = !_glewInit_GL_ARB_sample_shading();
#endif /* GL_ARB_sample_shading */
#ifdef GL_ARB_shader_image_load_store
  if (glewExperimental || GLEW_ARB_shader_image_load_store) GLEW_ARB_shader_image_load_store = !_glewInit_GL_ARB_shader_image_load_store();
#endif /* GL_ARB_shader_image_load_store */
#ifdef GL_ARB_sync
  if (glewExperimental || GLEW_ARB_sync) GLEW_ARB_sync = !_glewInit_GL_ARB_sync();
#endif /* GL_ARB_sync */
#ifdef GL_ARB_tessellation_shader
  if (glewExperimental || GLEW_ARB_tessellation_shader) GLEW_ARB_tessellation_shader = !_glewInit_GL_ARB_tessellation_shader();
#endif /* GL_ARB_tessellation_shader */
#ifdef GL_ARB_texture_multisample
  if (glewExperimental || GLEW_ARB_texture_multisample) GLEW_ARB_texture_multisample = !_glewInit_GL_ARB_texture_multisample();
#endif /* GL_ARB_texture_multisample */
#ifdef GL_ARB_texture_storage
  if (glewExperimental || GLEW_ARB_texture_storage) GLEW_ARB_texture_storage = !_glewInit_GL_ARB_texture_storage();
#endif /* GL_ARB_texture_storage */
#ifdef GL_ARB_timer_query
  if (glewExperimental || GLEW_ARB_timer_query) GLEW_ARB_timer_query = !_glewInit_GL_ARB_timer_query();
#endif /* GL_ARB_timer_query */
#ifdef GL_ARB_uniform_buffer_object
  if (glewExperimental || GLEW_ARB_uniform_buffer_object) GLEW_ARB_uniform_buffer_object = !_glewInit_GL_ARB_uniform_buffer_object();
#endif /* GL_ARB_uniform_buffer_object */
#ifdef GL_ARB_vertex_array_object
  if (glewExperimental || GLEW_ARB_vertex_array_object) GLEW_ARB_vertex_array_object = !_glewInit_GL_ARB_vertex_array_object();
#endif /* GL_ARB_vertex_array_object */
#ifdef GL_ARB_vertex_attrib_binding
  if (glewExperimental || GLEW_ARB_vertex_attrib_binding) GLEW_ARB_vertex_attrib_binding = !_glewInit_GL_ARB_vertex_attrib_binding();
#endif /* GL_ARB_vertex_attrib_binding */
#ifdef GL_ARB_vertex_type_2_10_10_10_rev
  if (glewExperimental || GLEW_ARB_vertex_type_2_10_10_10_rev) GLEW_ARB_vertex_type_2_10_10_10_rev = !_glewInit_GL_ARB_vertex_type_2_10_10_10_rev();
#endif /* GL_ARB_vertex_type_2_10_10_10_rev */
#ifdef GL_EXT_direct_state_access
  if (glewExperimental || GLEW_EXT_direct_state_access) GLEW_EXT_direct_state_access = !_glewInit_GL_EXT_direct_state_access();
#endif /* GL_EXT_direct_state_access */
#ifdef GL_EXT_framebuffer_blit
  if (glewExperimental || GLEW_EXT_framebuffer_blit) GLEW_EXT_framebuffer_blit = !_glewInit_GL_EXT_framebuffer_blit();
#endif /* GL_EXT_framebuffer_blit */
#ifdef GL_EXT_framebuffer_multisample
  if (glewExperimental || GLEW_EXT_framebuffer_multisample) GLEW_EXT_framebuffer_multisample = !_glewInit_GL_EXT_framebuffer_multisample();
#endif /* GL_EXT_framebuffer_multisample */
#ifdef GL_EXT_framebuffer_object
  if (glewExperimental || GLEW_EXT_framebuffer_object) GLEW_EXT_framebuffer_object = !_glewInit_GL_EXT_framebuffer_object();
#endif /* GL_EXT_framebuffer_object */
#ifdef GL_EXT_geometry_shader4
  if (glewExperimental || GLEW_EXT_geometry_shader4) GLEW_EXT_geometry_shader4 = !_glewInit_GL_EXT_geometry_shader4();
#endif /* GL_EXT_geometry_shader4 */
#ifdef GL_EXT_gpu_shader4
  if (glewExperimental || GLEW_EXT_gpu_shader4) GLEW_EXT_gpu_shader4 = !_glewInit_GL_EXT_gpu_shader4();
#endif /* GL_EXT_gpu_shader4 */
#ifdef GL_EXT_shader_image_load_store
  if (glewExperimental || GLEW_EXT_shader_image_load_store) GLEW_EXT_shader_image_load_store = !_glewInit_GL_EXT_shader_image_load_store();
#endif /* GL_EXT_shader_image_load_store */
#ifdef GL_EXT_texture_storage
  if (glewExperimental || GLEW_EXT_texture_storage) GLEW_EXT_texture_storage = !_glewInit_GL_EXT_texture_storage();
#endif /* GL_EXT_texture_storage */
#ifdef GL_INTEL_framebuffer_CMAA
  if (glewExperimental || GLEW_INTEL_framebuffer_CMAA) GLEW_INTEL_framebuffer_CMAA = !_glewInit_GL_INTEL_framebuffer_CMAA();
#endif /* GL_INTEL_framebuffer_CMAA */
#ifdef GL_KHR_debug
  if (glewExperimental || GLEW_KHR_debug) GLEW_KHR_debug = !_glewInit_GL_KHR_debug();
#endif /* GL_KHR_debug */
#ifdef GL_KHR_parallel_shader_compile
  if (glewExperimental || GLEW_KHR_parallel_shader_compile) GLEW_KHR_parallel_shader_compile = !_glewInit_GL_KHR_parallel_shader_compile();
#endif /* GL_KHR_parallel_shader_compile */
#ifdef GL_KHR_robustness
  if (glewExperimental || GLEW_KHR_robustness) GLEW_KHR_robustness = !_glewInit_GL_KHR_robustness();
#endif /* GL_KHR_robustness */
#ifdef GL_NV_conservative_raster
  if (glewExperimental || GLEW_NV_conservative_raster) GLEW_NV_conservative_raster = !_glewInit_GL_NV_conservative_raster();
#endif /* GL_NV_conservative_raster */
#ifdef GL_NV_copy_image
  if (glewExperimental || GLEW_NV_copy_image) GLEW_NV_copy_image = !_glewInit_GL_NV_copy_image();
#endif /* GL_NV_copy_image */
#ifdef GL_NV_framebuffer_multisample_coverage
  if (glewExperimental || GLEW_NV_framebuffer_multisample_coverage) GLEW_NV_framebuffer_multisample_coverage = !_glewInit_GL_NV_framebuffer_multisample_coverage();
#endif /* GL_NV_framebuffer_multisample_coverage */
#ifdef GL_NV_gpu_shader5
  if (glewExperimental || GLEW_NV_gpu_shader5) GLEW_NV_gpu_shader5 = !_glewInit_GL_NV_gpu_shader5();
#endif /* GL_NV_gpu_shader5 */

  return GLEW_OK;
}

#if defined(_WIN32)

PFNWGLGETEXTENSIONSSTRINGARBPROC __wglewGetExtensionsStringARB = NULL;

PFNWGLGETEXTENSIONSSTRINGEXTPROC __wglewGetExtensionsStringEXT = NULL;

GLboolean __WGLEW_ARB_extensions_string = GL_FALSE;
GLboolean __WGLEW_EXT_extensions_string = GL_FALSE;

#ifdef WGL_ARB_extensions_string

static GLboolean _glewInit_WGL_ARB_extensions_string (void)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)glewGetProcAddress((const GLubyte*)"wglGetExtensionsStringARB")) == NULL) || r;

  return r;
}

#endif /* WGL_ARB_extensions_string */

#ifdef WGL_EXT_extensions_string

static GLboolean _glewInit_WGL_EXT_extensions_string (void)
{
  GLboolean r = GL_FALSE;

  r = ((wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)glewGetProcAddress((const GLubyte*)"wglGetExtensionsStringEXT")) == NULL) || r;

  return r;
}

#endif /* WGL_EXT_extensions_string */

/* ------------------------------------------------------------------------- */

static PFNWGLGETEXTENSIONSSTRINGARBPROC _wglewGetExtensionsStringARB = NULL;
static PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglewGetExtensionsStringEXT = NULL;

GLboolean GLEWAPIENTRY wglewGetExtension (const char* name)
{
  const GLubyte* start;
  const GLubyte* end;
  if (_wglewGetExtensionsStringARB == NULL)
    if (_wglewGetExtensionsStringEXT == NULL)
      return GL_FALSE;
    else
      start = (const GLubyte*)_wglewGetExtensionsStringEXT();
  else
    start = (const GLubyte*)_wglewGetExtensionsStringARB(wglGetCurrentDC());
  if (start == 0)
    return GL_FALSE;
  end = start + _glewStrLen(start);
  return _glewSearchExtension(name, start, end);
}

GLenum GLEWAPIENTRY wglewInit (void)
{
  const GLubyte* extStart;
  const GLubyte* extEnd;
  /* find wgl extension string query functions */
  _wglewGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)glewGetProcAddress((const GLubyte*)"wglGetExtensionsStringARB");
  _wglewGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)glewGetProcAddress((const GLubyte*)"wglGetExtensionsStringEXT");
  /* query wgl extension string */
  if (_wglewGetExtensionsStringARB == NULL)
    if (_wglewGetExtensionsStringEXT == NULL)
      extStart = (const GLubyte*)"";
    else
      extStart = (const GLubyte*)_wglewGetExtensionsStringEXT();
  else
    extStart = (const GLubyte*)_wglewGetExtensionsStringARB(wglGetCurrentDC());
  extEnd = extStart + _glewStrLen(extStart);
  /* initialize extensions */
#ifdef WGL_ARB_extensions_string
  WGLEW_ARB_extensions_string = !_glewInit_WGL_ARB_extensions_string();
#endif /* WGL_ARB_extensions_string */
#ifdef WGL_EXT_extensions_string
  WGLEW_EXT_extensions_string = !_glewInit_WGL_EXT_extensions_string();
#endif /* WGL_EXT_extensions_string */

  return GLEW_OK;
}

#endif

/* ------------------------------------------------------------------------ */

const GLubyte * GLEWAPIENTRY glewGetErrorString (GLenum error)
{
  static const GLubyte* _glewErrorString[] =
  {
    (const GLubyte*)"No error",
    (const GLubyte*)"Missing GL version",
    (const GLubyte*)"GL 1.1 and up are not supported",
    (const GLubyte*)"GLX 1.2 and up are not supported",
    (const GLubyte*)"No GLX display",
    (const GLubyte*)"Unknown error"
  };
  const size_t max_error = sizeof(_glewErrorString)/sizeof(*_glewErrorString) - 1;
  return _glewErrorString[(size_t)error > max_error ? max_error : (size_t)error];
}

const GLubyte * GLEWAPIENTRY glewGetString (GLenum name)
{
  static const GLubyte* _glewString[] =
  {
    (const GLubyte*)NULL,
    (const GLubyte*)"2.3.1 custom",
    (const GLubyte*)"2",
    (const GLubyte*)"3",
    (const GLubyte*)"1"
  };
  const size_t max_string = sizeof(_glewString)/sizeof(*_glewString) - 1;
  return _glewString[(size_t)name > max_string ? 0 : (size_t)name];
}

/* ------------------------------------------------------------------------ */

GLboolean glewExperimental = GL_FALSE;

GLenum GLEWAPIENTRY glewInit (void)
{
  GLenum r = glewContextInit();
  if ( r != 0 ) return r;
#if defined(_WIN32)
  return wglewInit();
#else
  return r;
#endif /* _WIN32 */
}

GLboolean GLEWAPIENTRY glewIsSupported (const char* name)
{
  const GLubyte* pos = (const GLubyte*)name;
  GLuint len = _glewStrLen(pos);
  GLboolean ret = GL_TRUE;
  while (ret && len > 0)
  {
    if (_glewStrSame1(&pos, &len, (const GLubyte*)"GL_", 3))
    {
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"VERSION_", 8))
      {
#ifdef GL_VERSION_1_2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_2", 3))
        {
          ret = GLEW_VERSION_1_2;
          continue;
        }
#endif
#ifdef GL_VERSION_1_2_1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_2_1", 5))
        {
          ret = GLEW_VERSION_1_2_1;
          continue;
        }
#endif
#ifdef GL_VERSION_1_3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_3", 3))
        {
          ret = GLEW_VERSION_1_3;
          continue;
        }
#endif
#ifdef GL_VERSION_1_4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_4", 3))
        {
          ret = GLEW_VERSION_1_4;
          continue;
        }
#endif
#ifdef GL_VERSION_1_5
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"1_5", 3))
        {
          ret = GLEW_VERSION_1_5;
          continue;
        }
#endif
#ifdef GL_VERSION_2_0
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"2_0", 3))
        {
          ret = GLEW_VERSION_2_0;
          continue;
        }
#endif
#ifdef GL_VERSION_2_1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"2_1", 3))
        {
          ret = GLEW_VERSION_2_1;
          continue;
        }
#endif
#ifdef GL_VERSION_3_0
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"3_0", 3))
        {
          ret = GLEW_VERSION_3_0;
          continue;
        }
#endif
#ifdef GL_VERSION_3_1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"3_1", 3))
        {
          ret = GLEW_VERSION_3_1;
          continue;
        }
#endif
#ifdef GL_VERSION_3_2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"3_2", 3))
        {
          ret = GLEW_VERSION_3_2;
          continue;
        }
#endif
#ifdef GL_VERSION_3_3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"3_3", 3))
        {
          ret = GLEW_VERSION_3_3;
          continue;
        }
#endif
#ifdef GL_VERSION_4_0
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"4_0", 3))
        {
          ret = GLEW_VERSION_4_0;
          continue;
        }
#endif
#ifdef GL_VERSION_4_1
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"4_1", 3))
        {
          ret = GLEW_VERSION_4_1;
          continue;
        }
#endif
#ifdef GL_VERSION_4_2
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"4_2", 3))
        {
          ret = GLEW_VERSION_4_2;
          continue;
        }
#endif
#ifdef GL_VERSION_4_3
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"4_3", 3))
        {
          ret = GLEW_VERSION_4_3;
          continue;
        }
#endif
#ifdef GL_VERSION_4_4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"4_4", 3))
        {
          ret = GLEW_VERSION_4_4;
          continue;
        }
#endif
#ifdef GL_VERSION_4_5
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"4_5", 3))
        {
          ret = GLEW_VERSION_4_5;
          continue;
        }
#endif
#ifdef GL_VERSION_4_6
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"4_6", 3))
        {
          ret = GLEW_VERSION_4_6;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"AMD_", 4))
      {
#ifdef GL_AMD_conservative_depth
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"conservative_depth", 18))
        {
          ret = GLEW_AMD_conservative_depth;
          continue;
        }
#endif
#ifdef GL_AMD_framebuffer_multisample_advanced
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_multisample_advanced", 32))
        {
          ret = GLEW_AMD_framebuffer_multisample_advanced;
          continue;
        }
#endif
#ifdef GL_AMD_gpu_shader_half_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_shader_half_float", 21))
        {
          ret = GLEW_AMD_gpu_shader_half_float;
          continue;
        }
#endif
#ifdef GL_AMD_shader_trinary_minmax
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_trinary_minmax", 21))
        {
          ret = GLEW_AMD_shader_trinary_minmax;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"ARB_", 4))
      {
#ifdef GL_ARB_buffer_storage
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"buffer_storage", 14))
        {
          ret = GLEW_ARB_buffer_storage;
          continue;
        }
#endif
#ifdef GL_ARB_clear_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"clear_buffer_object", 19))
        {
          ret = GLEW_ARB_clear_buffer_object;
          continue;
        }
#endif
#ifdef GL_ARB_clear_texture
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"clear_texture", 13))
        {
          ret = GLEW_ARB_clear_texture;
          continue;
        }
#endif
#ifdef GL_ARB_compute_shader
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"compute_shader", 14))
        {
          ret = GLEW_ARB_compute_shader;
          continue;
        }
#endif
#ifdef GL_ARB_conservative_depth
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"conservative_depth", 18))
        {
          ret = GLEW_ARB_conservative_depth;
          continue;
        }
#endif
#ifdef GL_ARB_copy_buffer
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"copy_buffer", 11))
        {
          ret = GLEW_ARB_copy_buffer;
          continue;
        }
#endif
#ifdef GL_ARB_copy_image
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"copy_image", 10))
        {
          ret = GLEW_ARB_copy_image;
          continue;
        }
#endif
#ifdef GL_ARB_depth_buffer_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_buffer_float", 18))
        {
          ret = GLEW_ARB_depth_buffer_float;
          continue;
        }
#endif
#ifdef GL_ARB_depth_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_clamp", 11))
        {
          ret = GLEW_ARB_depth_clamp;
          continue;
        }
#endif
#ifdef GL_ARB_direct_state_access
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"direct_state_access", 19))
        {
          ret = GLEW_ARB_direct_state_access;
          continue;
        }
#endif
#ifdef GL_ARB_draw_elements_base_vertex
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"draw_elements_base_vertex", 25))
        {
          ret = GLEW_ARB_draw_elements_base_vertex;
          continue;
        }
#endif
#ifdef GL_ARB_enhanced_layouts
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"enhanced_layouts", 16))
        {
          ret = GLEW_ARB_enhanced_layouts;
          continue;
        }
#endif
#ifdef GL_ARB_framebuffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_object", 18))
        {
          ret = GLEW_ARB_framebuffer_object;
          continue;
        }
#endif
#ifdef GL_ARB_framebuffer_sRGB
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_sRGB", 16))
        {
          ret = GLEW_ARB_framebuffer_sRGB;
          continue;
        }
#endif
#ifdef GL_ARB_geometry_shader4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"geometry_shader4", 16))
        {
          ret = GLEW_ARB_geometry_shader4;
          continue;
        }
#endif
#ifdef GL_ARB_get_program_binary
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"get_program_binary", 18))
        {
          ret = GLEW_ARB_get_program_binary;
          continue;
        }
#endif
#ifdef GL_ARB_gpu_shader5
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_shader5", 11))
        {
          ret = GLEW_ARB_gpu_shader5;
          continue;
        }
#endif
#ifdef GL_ARB_half_float_vertex
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"half_float_vertex", 17))
        {
          ret = GLEW_ARB_half_float_vertex;
          continue;
        }
#endif
#ifdef GL_ARB_imaging
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"imaging", 7))
        {
          ret = GLEW_ARB_imaging;
          continue;
        }
#endif
#ifdef GL_ARB_instanced_arrays
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"instanced_arrays", 16))
        {
          ret = GLEW_ARB_instanced_arrays;
          continue;
        }
#endif
#ifdef GL_ARB_invalidate_subdata
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"invalidate_subdata", 18))
        {
          ret = GLEW_ARB_invalidate_subdata;
          continue;
        }
#endif
#ifdef GL_ARB_map_buffer_range
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"map_buffer_range", 16))
        {
          ret = GLEW_ARB_map_buffer_range;
          continue;
        }
#endif
#ifdef GL_ARB_multi_bind
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multi_bind", 10))
        {
          ret = GLEW_ARB_multi_bind;
          continue;
        }
#endif
#ifdef GL_ARB_parallel_shader_compile
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"parallel_shader_compile", 23))
        {
          ret = GLEW_ARB_parallel_shader_compile;
          continue;
        }
#endif
#ifdef GL_ARB_pipeline_statistics_query
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pipeline_statistics_query", 25))
        {
          ret = GLEW_ARB_pipeline_statistics_query;
          continue;
        }
#endif
#ifdef GL_ARB_pixel_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_buffer_object", 19))
        {
          ret = GLEW_ARB_pixel_buffer_object;
          continue;
        }
#endif
#ifdef GL_ARB_program_interface_query
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"program_interface_query", 23))
        {
          ret = GLEW_ARB_program_interface_query;
          continue;
        }
#endif
#ifdef GL_ARB_provoking_vertex
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"provoking_vertex", 16))
        {
          ret = GLEW_ARB_provoking_vertex;
          continue;
        }
#endif
#ifdef GL_ARB_robustness
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"robustness", 10))
        {
          ret = GLEW_ARB_robustness;
          continue;
        }
#endif
#ifdef GL_ARB_sample_shading
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"sample_shading", 14))
        {
          ret = GLEW_ARB_sample_shading;
          continue;
        }
#endif
#ifdef GL_ARB_seamless_cube_map
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"seamless_cube_map", 17))
        {
          ret = GLEW_ARB_seamless_cube_map;
          continue;
        }
#endif
#ifdef GL_ARB_shader_group_vote
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_group_vote", 17))
        {
          ret = GLEW_ARB_shader_group_vote;
          continue;
        }
#endif
#ifdef GL_ARB_shader_image_load_store
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_image_load_store", 23))
        {
          ret = GLEW_ARB_shader_image_load_store;
          continue;
        }
#endif
#ifdef GL_ARB_shader_texture_lod
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_texture_lod", 18))
        {
          ret = GLEW_ARB_shader_texture_lod;
          continue;
        }
#endif
#ifdef GL_ARB_shading_language_packing
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shading_language_packing", 24))
        {
          ret = GLEW_ARB_shading_language_packing;
          continue;
        }
#endif
#ifdef GL_ARB_sync
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"sync", 4))
        {
          ret = GLEW_ARB_sync;
          continue;
        }
#endif
#ifdef GL_ARB_tessellation_shader
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"tessellation_shader", 19))
        {
          ret = GLEW_ARB_tessellation_shader;
          continue;
        }
#endif
#ifdef GL_ARB_texture_compression_bptc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_bptc", 24))
        {
          ret = GLEW_ARB_texture_compression_bptc;
          continue;
        }
#endif
#ifdef GL_ARB_texture_compression_rgtc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_rgtc", 24))
        {
          ret = GLEW_ARB_texture_compression_rgtc;
          continue;
        }
#endif
#ifdef GL_ARB_texture_filter_anisotropic
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_filter_anisotropic", 26))
        {
          ret = GLEW_ARB_texture_filter_anisotropic;
          continue;
        }
#endif
#ifdef GL_ARB_texture_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_float", 13))
        {
          ret = GLEW_ARB_texture_float;
          continue;
        }
#endif
#ifdef GL_ARB_texture_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_multisample", 19))
        {
          ret = GLEW_ARB_texture_multisample;
          continue;
        }
#endif
#ifdef GL_ARB_texture_rg
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_rg", 10))
        {
          ret = GLEW_ARB_texture_rg;
          continue;
        }
#endif
#ifdef GL_ARB_texture_stencil8
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_stencil8", 16))
        {
          ret = GLEW_ARB_texture_stencil8;
          continue;
        }
#endif
#ifdef GL_ARB_texture_storage
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_storage", 15))
        {
          ret = GLEW_ARB_texture_storage;
          continue;
        }
#endif
#ifdef GL_ARB_timer_query
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"timer_query", 11))
        {
          ret = GLEW_ARB_timer_query;
          continue;
        }
#endif
#ifdef GL_ARB_uniform_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"uniform_buffer_object", 21))
        {
          ret = GLEW_ARB_uniform_buffer_object;
          continue;
        }
#endif
#ifdef GL_ARB_vertex_array_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_array_object", 19))
        {
          ret = GLEW_ARB_vertex_array_object;
          continue;
        }
#endif
#ifdef GL_ARB_vertex_attrib_binding
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_attrib_binding", 21))
        {
          ret = GLEW_ARB_vertex_attrib_binding;
          continue;
        }
#endif
#ifdef GL_ARB_vertex_type_2_10_10_10_rev
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"vertex_type_2_10_10_10_rev", 26))
        {
          ret = GLEW_ARB_vertex_type_2_10_10_10_rev;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"EXT_", 4))
      {
#ifdef GL_EXT_demote_to_helper_invocation
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"demote_to_helper_invocation", 27))
        {
          ret = GLEW_EXT_demote_to_helper_invocation;
          continue;
        }
#endif
#ifdef GL_EXT_direct_state_access
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"direct_state_access", 19))
        {
          ret = GLEW_EXT_direct_state_access;
          continue;
        }
#endif
#ifdef GL_EXT_framebuffer_blit
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_blit", 16))
        {
          ret = GLEW_EXT_framebuffer_blit;
          continue;
        }
#endif
#ifdef GL_EXT_framebuffer_multisample
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_multisample", 23))
        {
          ret = GLEW_EXT_framebuffer_multisample;
          continue;
        }
#endif
#ifdef GL_EXT_framebuffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_object", 18))
        {
          ret = GLEW_EXT_framebuffer_object;
          continue;
        }
#endif
#ifdef GL_EXT_framebuffer_sRGB
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_sRGB", 16))
        {
          ret = GLEW_EXT_framebuffer_sRGB;
          continue;
        }
#endif
#ifdef GL_EXT_geometry_shader4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"geometry_shader4", 16))
        {
          ret = GLEW_EXT_geometry_shader4;
          continue;
        }
#endif
#ifdef GL_EXT_gpu_shader4
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_shader4", 11))
        {
          ret = GLEW_EXT_gpu_shader4;
          continue;
        }
#endif
#ifdef GL_EXT_packed_depth_stencil
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"packed_depth_stencil", 20))
        {
          ret = GLEW_EXT_packed_depth_stencil;
          continue;
        }
#endif
#ifdef GL_EXT_packed_float
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"packed_float", 12))
        {
          ret = GLEW_EXT_packed_float;
          continue;
        }
#endif
#ifdef GL_EXT_pixel_buffer_object
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pixel_buffer_object", 19))
        {
          ret = GLEW_EXT_pixel_buffer_object;
          continue;
        }
#endif
#ifdef GL_EXT_shader_image_load_store
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"shader_image_load_store", 23))
        {
          ret = GLEW_EXT_shader_image_load_store;
          continue;
        }
#endif
#ifdef GL_EXT_texture_compression_rgtc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_rgtc", 24))
        {
          ret = GLEW_EXT_texture_compression_rgtc;
          continue;
        }
#endif
#ifdef GL_EXT_texture_compression_s3tc
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_compression_s3tc", 24))
        {
          ret = GLEW_EXT_texture_compression_s3tc;
          continue;
        }
#endif
#ifdef GL_EXT_texture_filter_anisotropic
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_filter_anisotropic", 26))
        {
          ret = GLEW_EXT_texture_filter_anisotropic;
          continue;
        }
#endif
#ifdef GL_EXT_texture_storage
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"texture_storage", 15))
        {
          ret = GLEW_EXT_texture_storage;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"INTEL_", 6))
      {
#ifdef GL_INTEL_conservative_rasterization
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"conservative_rasterization", 26))
        {
          ret = GLEW_INTEL_conservative_rasterization;
          continue;
        }
#endif
#ifdef GL_INTEL_framebuffer_CMAA
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_CMAA", 16))
        {
          ret = GLEW_INTEL_framebuffer_CMAA;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"KHR_", 4))
      {
#ifdef GL_KHR_debug
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"debug", 5))
        {
          ret = GLEW_KHR_debug;
          continue;
        }
#endif
#ifdef GL_KHR_no_error
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"no_error", 8))
        {
          ret = GLEW_KHR_no_error;
          continue;
        }
#endif
#ifdef GL_KHR_parallel_shader_compile
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"parallel_shader_compile", 23))
        {
          ret = GLEW_KHR_parallel_shader_compile;
          continue;
        }
#endif
#ifdef GL_KHR_robustness
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"robustness", 10))
        {
          ret = GLEW_KHR_robustness;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"MESA_", 5))
      {
#ifdef GL_MESA_pack_invert
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"pack_invert", 11))
        {
          ret = GLEW_MESA_pack_invert;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"NVX_", 4))
      {
#ifdef GL_NVX_gpu_memory_info
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_memory_info", 15))
        {
          ret = GLEW_NVX_gpu_memory_info;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"NV_", 3))
      {
#ifdef GL_NV_conservative_raster
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"conservative_raster", 19))
        {
          ret = GLEW_NV_conservative_raster;
          continue;
        }
#endif
#ifdef GL_NV_copy_image
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"copy_image", 10))
        {
          ret = GLEW_NV_copy_image;
          continue;
        }
#endif
#ifdef GL_NV_depth_clamp
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"depth_clamp", 11))
        {
          ret = GLEW_NV_depth_clamp;
          continue;
        }
#endif
#ifdef GL_NV_framebuffer_multisample_coverage
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"framebuffer_multisample_coverage", 32))
        {
          ret = GLEW_NV_framebuffer_multisample_coverage;
          continue;
        }
#endif
#ifdef GL_NV_gpu_shader5
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"gpu_shader5", 11))
        {
          ret = GLEW_NV_gpu_shader5;
          continue;
        }
#endif
#ifdef GL_NV_multisample_filter_hint
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"multisample_filter_hint", 23))
        {
          ret = GLEW_NV_multisample_filter_hint;
          continue;
        }
#endif
      }
    }
    ret = (len == 0);
  }
  return ret;
}

#if defined(_WIN32)

GLboolean GLEWAPIENTRY wglewIsSupported (const char* name)
{
  const GLubyte* pos = (const GLubyte*)name;
  GLuint len = _glewStrLen(pos);
  GLboolean ret = GL_TRUE;
  while (ret && len > 0)
  {
    if (_glewStrSame1(&pos, &len, (const GLubyte*)"WGL_", 4))
    {
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"ARB_", 4))
      {
#ifdef WGL_ARB_extensions_string
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"extensions_string", 17))
        {
          ret = WGLEW_ARB_extensions_string;
          continue;
        }
#endif
      }
      if (_glewStrSame2(&pos, &len, (const GLubyte*)"EXT_", 4))
      {
#ifdef WGL_EXT_extensions_string
        if (_glewStrSame3(&pos, &len, (const GLubyte*)"extensions_string", 17))
        {
          ret = WGLEW_EXT_extensions_string;
          continue;
        }
#endif
      }
    }
    ret = (len == 0);
  }
  return ret;
}

#endif /* _WIN32 */
