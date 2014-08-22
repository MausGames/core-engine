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

#if defined(__cplusplus)
    extern "C" {
#endif


// ****************************************************************   
/* default extensions */
#define GLEW_VERSION_3_2               false
#define GLEW_ARB_buffer_storage        false
#define GLEW_ARB_clear_buffer_object   false
#define GLEW_ARB_vertex_attrib_binding false
#define GLEW_ARB_uniform_buffer_object false
#define GLEW_ARB_tessellation_shader   false
#define GLEW_ARB_sync                  false
#define GLEW_ARB_pixel_buffer_object   false
#define GLEW_ARB_instanced_arrays      false
#define GLEW_ARB_direct_state_access   false
#define GLEW_KHR_debug                 false
#define GLEW_ARB_framebuffer_object    true


// ****************************************************************   
/* handle implemented extensions */
extern std::string g_sExtensions;
extern bool m_bGL_EXT_discard_framebuffer;
extern bool m_bGL_EXT_texture_storage;
extern bool m_bGL_EXT_map_buffer_range;
extern bool m_bGL_OES_vertex_array_object;
extern bool m_bGL_EXT_texture_filter_anisotropic;

extern int coreInitES();

#define __CORE_ES_CHECK(x)   (g_sExtensions.find(x) != std::string::npos)
#define __CORE_ES_FETCH(t,x) {x = (t)eglGetProcAddress(#x);}


// ****************************************************************   
/* implemented GL_EXT_discard_framebuffer (mapped on GL_ARB_invalidate_subdata) */
#define GLEW_ARB_invalidate_subdata m_bGL_EXT_discard_framebuffer

#define GL_DEPTH 0x1801

#define glInvalidateBufferData(x)
#define glInvalidateFramebuffer glDiscardFramebufferEXT

extern PFNGLDISCARDFRAMEBUFFEREXTPROC glDiscardFramebufferEXT;


// ****************************************************************   
/* implemented GL_EXT_texture_storage (mapped on GL_ARB_texture_storage) */
#define GLEW_ARB_texture_storage m_bGL_EXT_texture_storage

extern GLenum GL_RGBA8;
extern GLenum GL_RGB8;

#define glTexStorage2D glTexStorage2DEXT

typedef void (GL_APIENTRYP PFNGLTEXSTORAGE2DEXTPROC) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
extern PFNGLTEXSTORAGE2DEXTPROC glTexStorage2DEXT;


// ****************************************************************   
/* implemented GL_EXT_map_buffer_range (mapped on GL_ARB_map_buffer_range) */
#define GLEW_ARB_map_buffer_range m_bGL_EXT_map_buffer_range

#define GL_MAP_WRITE_BIT             0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT  0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT 0x0008
#define GL_MAP_UNSYNCHRONIZED_BIT    0x0020

#define glUnmapBuffer    glUnmapBufferOES
#define glMapBufferRange glMapBufferRangeEXT

typedef GLvoid* (GL_APIENTRYP PFNGLMAPBUFFERRANGEEXTPROC) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
extern PFNGLUNMAPBUFFEROESPROC    glUnmapBufferOES;
extern PFNGLMAPBUFFERRANGEEXTPROC glMapBufferRangeEXT;


// ****************************************************************   
/* implemented GL_OES_vertex_array_object (mapped on GL_ARB_vertex_array_object) */
#define GLEW_ARB_vertex_array_object m_bGL_OES_vertex_array_object

#define glBindVertexArray    glBindVertexArrayOES
#define glDeleteVertexArrays glDeleteVertexArraysOES
#define glGenVertexArrays    glGenVertexArraysOES

extern PFNGLBINDVERTEXARRAYOESPROC    glBindVertexArrayOES;
extern PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES;
extern PFNGLGENVERTEXARRAYSOESPROC    glGenVertexArraysOES;


// ****************************************************************   
/* implemented GL_EXT_texture_filter_anisotropic */
#define GLEW_EXT_texture_filter_anisotropic m_bGL_EXT_texture_filter_anisotropic


// ****************************************************************   
/* other rerouted functions */
#define glewInit                         coreInitES
#define glClearDepth                     glClearDepthf
#define glDrawRangeElements(a,b,c,d,e,f) glDrawElements(a, d, e, f)


// ****************************************************************  
/* unused definitions and functions */
#define GLEW_OK      0
#define GLEW_VERSION 1
#define GLAPIENTRY   GL_APIENTRY
inline const char* glewGetString     (GLenum name)  {return "";}
inline const char* glewGetErrorString(GLenum error) {return "";}

typedef int*     GLsync;
typedef uint64_t GLuint64;
#define GL_DEBUG_OUTPUT                 0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS     0x8242
#define GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB 0x8242
#define GL_UNIFORM_BUFFER               0x8A11
#define GL_PIXEL_UNPACK_BUFFER          0x88EC
#define GL_STATIC_READ                  0x88E5
#define GL_STATIC_COPY                  0x88E6
#define GL_MAP_PERSISTENT_BIT           0x00000040
#define GL_MAP_COHERENT_BIT             0x00000080
#define GL_SYNC_FLUSH_COMMANDS_BIT      0x00000001
#define GL_SYNC_GPU_COMMANDS_COMPLETE   0x9117
#define GL_TIMEOUT_EXPIRED              0x911B
#define GL_GEOMETRY_SHADER              0x8DD9
#define GL_TESS_EVALUATION_SHADER       0x8E87
#define GL_TESS_CONTROL_SHADER          0x8E88

typedef void (GL_APIENTRYP GLDEBUGPROC) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
inline void      glDebugMessageCallback          (GLDEBUGPROC callback, const GLvoid *userParam)                                                    {}
inline void      glDebugMessageControl           (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled) {}
inline void      glDebugMessageCallbackARB       (GLDEBUGPROC callback, const GLvoid *userParam)                                                    {}
inline void      glDebugMessageControlARB        (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled) {}
inline void      glBindBufferBase                (GLenum target, GLuint index, GLuint buffer)                                                       {}
inline void      glBindFragDataLocation          (GLuint program, GLuint colorNumber, const GLchar* name)                                           {}
inline GLuint    glGetUniformBlockIndex          (GLuint program, const GLchar* uniformBlockName)                                                   {return -1;}
inline void      glUniformBlockBinding           (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)                             {}
inline GLvoid*   glMapBuffer                     (GLenum target, GLenum access)                                                                     {return NULL;}
inline void      glBufferStorage                 (GLenum target, GLsizeiptr size, const GLvoid* data, GLbitfield flags)                             {}
inline void      glBindVertexBuffer              (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)                              {}
inline void      glVertexAttribBinding           (GLuint attribindex, GLuint bindingindex)                                                          {}
inline void      glVertexAttribFormat            (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)         {}
inline void      glVertexAttribIFormat           (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)                               {}
inline void      glVertexAttribIPointer          (GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)                     {}
inline void      glVertexBindingDivisor          (GLuint bindingindex, GLuint divisor)                                                              {}
inline void      glVertexAttribDivisorARB        (GLuint index, GLuint divisor)                                                                     {}
inline void      glDrawArraysInstanced           (GLenum, GLint, GLsizei, GLsizei)                                                                  {}
inline void      glDrawElementsInstanced         (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei primcount)                {}
inline GLenum    glClientWaitSync                (GLsync GLsync, GLbitfield flags, GLuint64 timeout)                                                {return 0;}
inline void      glDeleteSync                    (GLsync GLsync)                                                                                    {}
inline GLsync    glFenceSync                     (GLenum condition, GLbitfield flags)                                                               {return NULL;}
inline void      glBindTextureUnit               (GLuint unit, GLuint texture)                                                                      {}
inline GLvoid*   glMapNamedBufferRange           (GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access)                             {return NULL;}
inline GLboolean glUnmapNamedBuffer              (GLuint buffer)                                                                                    {return false;}
inline void      glClearNamedBufferData          (GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void* data)               {}
inline void      glBlitNamedFramebuffer          (GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {}
inline void      glInvalidateNamedFramebufferData(GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments)                            {}


#if defined(__cplusplus)
    }
#endif

#endif /* _CORE_ES_H_ */