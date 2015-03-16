//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GLES_H_
#define	_CORE_GLES_H_


// ****************************************************************
/* default extensions */
#define CORE_GL_ARB_buffer_storage             false
#define CORE_GL_ARB_clear_buffer_object        false
#define CORE_GL_ARB_vertex_attrib_binding      false
#define CORE_GL_ARB_uniform_buffer_object      false   // controls shader-handling
#define CORE_GL_ARB_tessellation_shader        false
#define CORE_GL_ARB_geometry_shader4           false
#define CORE_GL_ARB_sync                       false
#define CORE_GL_ARB_direct_state_access        false
#define CORE_GL_ARB_vertex_type_2_10_10_10_rev false
#define CORE_GL_ARB_timer_query                false
#define CORE_GL_KHR_debug                      false
#define CORE_GL_EXT_framebuffer_object         true


// ****************************************************************
/* GL_EXT_discard_framebuffer (mapped on GL_ARB_invalidate_subdata) */
#define CORE_GL_ARB_invalidate_subdata __CORE_GLES_VAR(GL_EXT_discard_framebuffer)

#define GL_COLOR   0x1800
#define GL_DEPTH   0x1801
#define GL_STENCIL 0x1802

typedef void (GL_APIENTRY * PFNGLDISCARDFRAMEBUFFEREXTPROC) (GLenum target, GLsizei numAttachments, const GLenum *attachments);
#define glInvalidateBufferData(x)
#define glInvalidateFramebuffer __CORE_GLES_FUNC(glDiscardFramebufferEXT)
#define glInvalidateTexImage(x,y)


// ****************************************************************
/* GL_EXT_texture_storage (mapped on GL_ARB_texture_storage) */
#define CORE_GL_ARB_texture_storage __CORE_GLES_VAR(GL_EXT_texture_storage)

typedef void (GL_APIENTRY * PFNGLTEXSTORAGE2DEXTPROC) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
#define glTexStorage2D __CORE_GLES_FUNC(glTexStorage2DEXT)


// ****************************************************************
/* GL_EXT_map_buffer_range (mapped on GL_ARB_map_buffer_range) */
#define CORE_GL_ARB_map_buffer_range __CORE_GLES_VAR(GL_EXT_map_buffer_range)

#define GL_MAP_WRITE_BIT             0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT  0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT 0x0008
#define GL_MAP_UNSYNCHRONIZED_BIT    0x0020

typedef GLvoid*   (GL_APIENTRY * PFNGLMAPBUFFERRANGEEXTPROC) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef GLboolean (GL_APIENTRY * PFNGLUNMAPBUFFEROESPROC)    (GLenum target);
#define glMapBufferRange __CORE_GLES_FUNC(glMapBufferRangeEXT)
#define glUnmapBuffer    __CORE_GLES_FUNC(glUnmapBufferOES)


// ****************************************************************
/* GL_EXT_instanced_arrays (mapped on GL_ARB_instanced_arrays) */
#define CORE_GL_ARB_instanced_arrays __CORE_GLES_VAR(GL_EXT_instanced_arrays)

typedef void (GL_APIENTRY * PFNGLDRAWARRAYSINSTANCEDEXTPROC)   (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
typedef void (GL_APIENTRY * PFNGLDRAWELEMENTSINSTANCEDEXTPROC) (GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount);
typedef void (GL_APIENTRY * PFNGLVERTEXATTRIBDIVISOREXTPROC)   (GLuint index, GLuint divisor);
#define glDrawArraysInstanced   __CORE_GLES_FUNC(glDrawArraysInstancedEXT)
#define glDrawElementsInstanced __CORE_GLES_FUNC(glDrawElementsInstancedEXT)
#define glVertexAttribDivisor   __CORE_GLES_FUNC(glVertexAttribDivisorEXT)


// ****************************************************************
/* GL_EXT_texture_filter_anisotropic */
#define CORE_GL_EXT_texture_filter_anisotropic __CORE_GLES_VAR(GL_EXT_texture_filter_anisotropic)

#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE


// ****************************************************************
/* GL_NV_pixel_buffer_object (mapped on GL_ARB_pixel_buffer_object) */
#define CORE_GL_ARB_pixel_buffer_object __CORE_GLES_VAR(GL_NV_pixel_buffer_object)

#define GL_PIXEL_PACK_BUFFER   0x88EB
#define GL_PIXEL_UNPACK_BUFFER 0x88EC


// ****************************************************************
/* GL_NV_framebuffer_blit (mapped on GL_EXT_framebuffer_blit) */
#define CORE_GL_EXT_framebuffer_blit __CORE_GLES_VAR(GL_NV_framebuffer_blit)

#define GL_READ_FRAMEBUFFER __CORE_GLES_VAR(GL_READ_FRAMEBUFFER)
#define GL_DRAW_FRAMEBUFFER __CORE_GLES_VAR(GL_DRAW_FRAMEBUFFER)

typedef void (GL_APIENTRY * PFNGLBLITFRAMEBUFFERNVPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
#define glBlitFramebuffer __CORE_GLES_FUNC(glBlitFramebufferNV)


// ****************************************************************
/* GL_NV_framebuffer_multisample (mapped on GL_EXT_framebuffer_multisample) */
#define CORE_GL_EXT_framebuffer_multisample __CORE_GLES_VAR(GL_NV_framebuffer_multisample)

typedef void (GL_APIENTRY * PFNGLRENDERBUFFERSTORAGEMULTISAMPLENVPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
#define glRenderbufferStorageMultisample __CORE_GLES_FUNC(glRenderbufferStorageMultisampleNV)


// ****************************************************************
/* GL_OES_vertex_array_object (mapped on GL_ARB_vertex_array_object) */
#define CORE_GL_ARB_vertex_array_object __CORE_GLES_VAR(GL_OES_vertex_array_object)

typedef void (GL_APIENTRY * PFNGLBINDVERTEXARRAYOESPROC)    (GLuint array);
typedef void (GL_APIENTRY * PFNGLDELETEVERTEXARRAYSOESPROC) (GLsizei n, const GLuint *arrays);
typedef void (GL_APIENTRY * PFNGLGENVERTEXARRAYSOESPROC)    (GLsizei n, GLuint *arrays);
#define glBindVertexArray    __CORE_GLES_FUNC(glBindVertexArrayOES)
#define glDeleteVertexArrays __CORE_GLES_FUNC(glDeleteVertexArraysOES)
#define glGenVertexArrays    __CORE_GLES_FUNC(glGenVertexArraysOES)


// ****************************************************************
/* GL_OES_depth_texture (mapped on GL_ARB_depth_texture) */
#define CORE_GL_ARB_depth_texture __CORE_GLES_VAR(GL_OES_depth_texture)


// ****************************************************************
/* GL_OES_texture_stencil8 (mapped on GL_ARB_texture_stencil8) */
#define CORE_GL_ARB_texture_stencil8 __CORE_GLES_VAR(GL_OES_texture_stencil8)


// ****************************************************************
/* GL_OES_rgb8_rgba8 (silent) */
#define GL_RGB8  __CORE_GLES_VAR(GL_RGB8)
#define GL_RGBA8 __CORE_GLES_VAR(GL_RGBA8)


// ****************************************************************
/* other remapped identifiers */
#define glClearDepth                     glClearDepthf
#define glDrawRangeElements(a,b,c,d,e,f) glDrawElements(a, d, e, f)


// ****************************************************************
/* unused definitions and functions */
typedef int*     GLsync;
typedef uint64_t GLuint64;
#define GL_NONE                        0
#define GL_MULTISAMPLE                 0x809D
#define GL_TEXTURE_COMPARE_MODE        0x884C
#define GL_TEXTURE_COMPARE_FUNC        0x884D
#define GL_COMPARE_REF_TO_TEXTURE      0x884E
#define GL_DEBUG_OUTPUT                0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS    0x8242
#define GL_UNIFORM_BUFFER              0x8A11
#define GL_MAP_PERSISTENT_BIT          0x00000040
#define GL_MAP_COHERENT_BIT            0x00000080
#define GL_MAP_FLUSH_EXPLICIT_BIT      0x0010
#define GL_SYNC_FLUSH_COMMANDS_BIT     0x00000001
#define GL_SYNC_GPU_COMMANDS_COMPLETE  0x9117
#define GL_TIMEOUT_EXPIRED             0x911B
#define GL_TIMEOUT_IGNORED             0xFFFFFFFFFFFFFFFF
#define GL_TESS_EVALUATION_SHADER      0x8E87
#define GL_TESS_CONTROL_SHADER         0x8E88
#define GL_GEOMETRY_SHADER             0x8DD9
#define GL_UNSIGNED_INT_2_10_10_10_REV 0x8368
#define GL_INT_2_10_10_10_REV          0x8D9F
#define GL_TIMESTAMP                   0x8E28
#define GL_QUERY_RESULT                0x8866

typedef void (GL_APIENTRY * GLDEBUGPROC) (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
inline void      glDrawBuffer                    (GLenum mode)                                                                                                                                                                            {}
inline void      glDebugMessageCallback          (GLDEBUGPROC callback, const GLvoid *userParam)                                                                                                                                          {}
inline void      glDebugMessageControl           (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)                                                                                       {}
inline void      glBindBufferBase                (GLenum target, GLuint index, GLuint buffer)                                                                                                                                             {}
inline void      glBindFragDataLocation          (GLuint program, GLuint colorNumber, const GLchar* name)                                                                                                                                 {}
inline GLuint    glGetUniformBlockIndex          (GLuint program, const GLchar* uniformBlockName)                                                                                                                                         {return -1;}
inline void      glUniformBlockBinding           (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)                                                                                                                   {}
inline void      glBufferStorage                 (GLenum target, GLsizeiptr size, const GLvoid* data, GLbitfield flags)                                                                                                                   {}
inline void      glClearBufferData               (GLenum target, GLenum internalformat, GLenum format, GLenum type, const void *data)                                                                                                     {}
inline void      glBindVertexBuffer              (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)                                                                                                                    {}
inline void      glVertexAttribBinding           (GLuint attribindex, GLuint bindingindex)                                                                                                                                                {}
inline void      glVertexAttribFormat            (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)                                                                                               {}
inline void      glVertexBindingDivisor          (GLuint bindingindex, GLuint divisor)                                                                                                                                                    {}
inline void      glFlushMappedBufferRange        (GLenum target, GLintptr offset, GLsizeiptr length)                                                                                                                                      {}
inline GLenum    glClientWaitSync                (GLsync GLsync, GLbitfield flags, GLuint64 timeout)                                                                                                                                      {return 0;}
inline void      glDeleteSync                    (GLsync GLsync)                                                                                                                                                                          {}
inline GLsync    glFenceSync                     (GLenum condition, GLbitfield flags)                                                                                                                                                     {return NULL;}
inline void      glBindTextureUnit               (GLuint unit, GLuint texture)                                                                                                                                                            {}
inline void      glTextureSubImage2D             (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)                                               {}
inline void      glGenerateTextureMipmap         (GLuint texture)                                                                                                                                                                         {}
inline GLvoid*   glMapNamedBufferRange           (GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access)                                                                                                                   {return NULL;}
inline GLboolean glUnmapNamedBuffer              (GLuint buffer)                                                                                                                                                                          {return false;}
inline void      glFlushMappedNamedBufferRange   (GLuint buffer, GLintptr offset, GLsizeiptr length)                                                                                                                                      {}
inline void      glClearNamedBufferData          (GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void* data)                                                                                                     {}
inline void      glBlitNamedFramebuffer          (GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {}
inline void      glInvalidateNamedFramebufferData(GLuint framebuffer, GLsizei numAttachments, const GLenum* attachments)                                                                                                                  {}
inline void      glGenQueries                    (GLsizei n, GLuint* ids)                                                                                                                                                                 {}
inline void      glGetQueryObjectui64v           (GLuint id, GLenum pname, GLuint64* params)                                                                                                                                              {}
inline void      glQueryCounter                  (GLuint id, GLenum target)                                                                                                                                                               {}


// ****************************************************************
/* context structure */
struct coreContext
{
    coreFloat __fVersion;
    coreBool  __bES3;

    GLboolean __GL_EXT_discard_framebuffer;
    GLboolean __GL_EXT_texture_storage;
    GLboolean __GL_EXT_map_buffer_range;
    GLboolean __GL_EXT_instanced_arrays;
    GLboolean __GL_EXT_texture_filter_anisotropic;
    GLboolean __GL_NV_pixel_buffer_object;
    GLboolean __GL_NV_framebuffer_blit;
    GLboolean __GL_NV_framebuffer_multisample;
    GLboolean __GL_OES_vertex_array_object;
    GLboolean __GL_OES_depth_texture;
    GLboolean __GL_OES_texture_stencil8;
    GLboolean __GL_OES_rgb8_rgba8;   // silent
    GLboolean __GL_OES_mapbuffer;    // silent

    GLenum __GL_READ_FRAMEBUFFER;
    GLenum __GL_DRAW_FRAMEBUFFER;
    GLenum __GL_RGB8;
    GLenum __GL_RGBA8;

    PFNGLDISCARDFRAMEBUFFEREXTPROC            __glDiscardFramebufferEXT;
    PFNGLTEXSTORAGE2DEXTPROC                  __glTexStorage2DEXT;
    PFNGLMAPBUFFERRANGEEXTPROC                __glMapBufferRangeEXT;
    PFNGLUNMAPBUFFEROESPROC                   __glUnmapBufferOES;
    PFNGLDRAWARRAYSINSTANCEDEXTPROC           __glDrawArraysInstancedEXT;
    PFNGLDRAWELEMENTSINSTANCEDEXTPROC         __glDrawElementsInstancedEXT;
    PFNGLVERTEXATTRIBDIVISOREXTPROC           __glVertexAttribDivisorEXT;
    PFNGLBLITFRAMEBUFFERNVPROC                __glBlitFramebufferNV;
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLENVPROC __glRenderbufferStorageMultisampleNV;
    PFNGLBINDVERTEXARRAYOESPROC               __glBindVertexArrayOES;
    PFNGLDELETEVERTEXARRAYSOESPROC            __glDeleteVertexArraysOES;
    PFNGLGENVERTEXARRAYSOESPROC               __glGenVertexArraysOES;
};

extern std::string              g_sExtensions;   //!< full extension string
extern thread_local coreContext g_CoreContext;   //!< thread local context structure

#define __CORE_GLES_CHECK(x,b)        (g_CoreContext.__ ## x = ((g_sExtensions.find(#x) != std::string::npos) || b))
#define __CORE_GLES_FUNC(f)           (g_CoreContext.__ ## f)
#define __CORE_GLES_FUNC_FETCH(f,a,b) {g_CoreContext.__ ## f ## a = (decltype(g_CoreContext.__ ## f ## a))eglGetProcAddress(b ? #f : #f #a);}
#define __CORE_GLES_VAR(v)            (g_CoreContext.__ ## v)
#define __CORE_GLES_VAR_SET(v,a)      {g_CoreContext.__ ## v = (a);}


// ****************************************************************
/* init OpenGL ES */
extern void __coreInitOpenGLES();
#define coreInitOpenGL __coreInitOpenGLES


// ****************************************************************
/* check for extensions */
#define CORE_GL_SUPPORT(e) (CORE_GL_ ## e)


#endif /* _CORE_GLES_H_ */