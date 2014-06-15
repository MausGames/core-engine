//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_ES_H_
#define	_CORE_ES_H_

#ifdef __cplusplus
    extern "C" {
#endif


// ****************************************************************   
/* default extensions */
#define GLEW_VERSION_3_2               false
#define GLEW_ARB_map_buffer_range      false
#define GLEW_ARB_buffer_storage        false
#define GLEW_ARB_clear_buffer_object   false
#define GLEW_ARB_vertex_attrib_binding false
#define GLEW_ARB_uniform_buffer_object false
#define GLEW_ARB_vertex_array_object   false
#define GLEW_ARB_tessellation_shader   false
#define GLEW_ARB_sync                  false
#define GLEW_ARB_pixel_buffer_object   false
#define GLEW_ARB_instanced_arrays      false
#define GLEW_ARB_debug_output          false
#define GLEW_KHR_debug                 false
#define GLEW_ARB_framebuffer_object    true


// ****************************************************************   
/* handle implemented extensions */
extern std::string g_sExtensions;
extern bool m_bGL_EXT_discard_framebuffer;
extern bool m_bGL_EXT_texture_storage;
extern bool m_bGL_EXT_texture_filter_anisotropic;

extern int coreInitES();

#define __CORE_ES_CHECK(x)   (g_sExtensions.find(x) != std::string::npos)
#define __CORE_ES_FETCH(t,x) {x = (t)eglGetProcAddress(#x);}


// ****************************************************************   
/* implemented GL_EXT_discard_framebuffer (mapped on GL_ARB_invalidate_subdata) */
#define GLEW_ARB_invalidate_subdata m_bGL_EXT_discard_framebuffer

#define GL_DEPTH GL_DEPTH_EXT
#define glInvalidateBufferData(x)
#define glInvalidateFramebuffer glDiscardFramebufferEXT
extern PFNGLDISCARDFRAMEBUFFEREXTPROC glDiscardFramebufferEXT;


// ****************************************************************   
/* implemented GL_EXT_texture_storage (mapped on GL_ARB_texture_storage) */
#define GLEW_ARB_texture_storage m_bGL_EXT_texture_storage

#define glTexStorage2D glTexStorage2DEXT
typedef void (GL_APIENTRYP PFNGLTEXSTORAGE2DEXT) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
extern PFNGLTEXSTORAGE2DEXT glTexStorage2DEXT;
extern GLenum GL_RGBA8;
extern GLenum GL_RGB8;


// ****************************************************************   
/* implemented GL_EXT_texture_filter_anisotropic */
#define GLEW_EXT_texture_filter_anisotropic m_bGL_EXT_texture_filter_anisotropic


// ****************************************************************   
/* other rerouted functions */
#define glewInit                         coreInitES
#define glClearDepth                     glClearDepthf
#define glDrawRangeElements(a,b,c,d,e,f) glDrawElements(a, d, e, f)


// ****************************************************************  
/* unused types, definitions and functions */
#define GLEW_OK      0
#define GLEW_VERSION 1          
static const char* glewGetString(GLenum name)       {return "";}
static const char* glewGetErrorString(GLenum error) {return "";}

typedef int*     GLsync;
typedef uint64_t GLuint64;
typedef void (*GLDEBUGPROC) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);

#define GL_DEBUG_OUTPUT                 0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS     0x8242
#define GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB 0x8242
#define GL_UNIFORM_BUFFER               0x8A11
#define GL_PIXEL_UNPACK_BUFFER          0x88EC
#define GL_MAP_WRITE_BIT                0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT     0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT    0x0008
#define GL_MAP_UNSYNCHRONIZED_BIT       0x0020
#define GL_STATIC_READ                  0x88E5
#define GL_STATIC_COPY                  0x88E6
#define GL_WRITE_ONLY                   0x88B9
#define GL_SYNC_FLUSH_COMMANDS_BIT      0x00000001
#define GL_SYNC_GPU_COMMANDS_COMPLETE   0x9117
#define GL_TIMEOUT_EXPIRED              0x911B
#define GL_GEOMETRY_SHADER              0x8DD9
#define GL_TESS_EVALUATION_SHADER       0x8E87
#define GL_TESS_CONTROL_SHADER          0x8E88

static void    glDebugMessageCallback(GLDEBUGPROC callback, const GLvoid *userParam)                                                      {}
static void    glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)    {}
static void    glDebugMessageCallbackARB(GLDEBUGPROC callback, const GLvoid *userParam)                                                   {}
static void    glDebugMessageControlARB(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled) {}
static void    glBindVertexArray(GLuint array)                                                                                            {}
static void    glDeleteVertexArrays(GLsizei n, const GLuint* arrays)                                                                      {}
static void    glGenVertexArrays(GLsizei n, GLuint* arrays)                                                                               {}
static void    glBindBufferBase(GLenum target, GLuint index, GLuint buffer)                                                               {}
static void    glBindFragDataLocation(GLuint program, GLuint colorNumber, const GLchar* name)                                             {}
static GLuint  glGetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName)                                                     {return -1;}
static void    glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)                                {}
static GLvoid* glMapBuffer(GLenum target, GLenum access)                                                                                  {return NULL;}
static GLvoid* glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)                                     {return NULL;}
static bool    glUnmapBuffer(GLenum target)                                                                                               {return true;}
static void    glBufferStorage(GLenum target, GLsizeiptr size, const GLvoid* data, GLbitfield flags)                                      {}
static void    glBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)                                    {}
static void    glVertexAttribBinding(GLuint attribindex, GLuint bindingindex)                                                             {}
static void    glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)             {}
static void    glVertexAttribIFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)                                  {}
static void    glVertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)                       {}
static void    glVertexBindingDivisor(GLuint bindingindex, GLuint divisor)                                                                {}
static void    glVertexAttribDivisorARB(GLuint index, GLuint divisor)                                                                     {}
static void    glDrawArraysInstanced(GLenum, GLint, GLsizei, GLsizei)                                                                     {}
static void    glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei primcount)                 {}
static GLenum  glClientWaitSync(GLsync GLsync, GLbitfield flags, GLuint64 timeout)                                                        {return 0;}
static void    glDeleteSync(GLsync GLsync)                                                                                                {}
static GLsync  glFenceSync(GLenum condition, GLbitfield flags)                                                                            {return NULL;}


#ifdef __cplusplus
    }
#endif

#endif /* _CORE_ES_H_ */