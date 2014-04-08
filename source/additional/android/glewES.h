//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
/* Only required for OpenGL ES 2.0 applications.
 * Implements unavailable definitions and dummy-functions. */
#pragma once
#ifndef _GLEWES_H_
#define	_GLEWES_H_

#ifdef __cplusplus
extern "C"
{
#endif


#define GLEW_OK 0

#define GLEW_VERSION 1

#define GL_DEBUG_OUTPUT 0x92E0

#define GL_UNIFORM_BUFFER 0x8A11
#define GL_PIXEL_UNPACK_BUFFER 0x88EC

#define GL_MAP_WRITE_BIT 0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT 0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT 0x0008
#define GL_MAP_UNSYNCHRONIZED_BIT 0x0020

#define GL_STATIC_READ 0x88E5
#define GL_STATIC_COPY 0x88E6

#define GL_WRITE_ONLY 0x88B9

#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE

#define GL_SYNC_FLUSH_COMMANDS_BIT 0x00000001
#define GL_SYNC_GPU_COMMANDS_COMPLETE 0x9117
#define GL_TIMEOUT_EXPIRED 0x911B

#define GL_RGBA8 GL_RGBA
#define GL_RGB8  GL_RGB

#define GL_DEPTH 0x1801

#define GL_GEOMETRY_SHADER 0x8DD9
#define GL_TESS_EVALUATION_SHADER 0x8E87
#define GL_TESS_CONTROL_SHADER 0x8E88


typedef int* GLsync;
typedef uint64_t GLuint64;

extern GLenum glewInit();

extern const char* glewGetString(GLenum name);
extern bool glewIsSupported(const char* pcName);
extern const char* glewGetErrorString(GLenum error);

#define GLEW_ARB_map_buffer_range           false
#define GLEW_ARB_buffer_storage             false
#define GLEW_ARB_clear_buffer_object        false
#define GLEW_ARB_invalidate_subdata         false
#define GLEW_ARB_vertex_attrib_binding      false
#define GLEW_ARB_texture_storage            false
#define GLEW_ARB_framebuffer_object         true
#define GLEW_ARB_uniform_buffer_object      false
#define GLEW_ARB_vertex_array_object        false
#define GLEW_ARB_tessellation_shader        false
#define GLEW_ARB_sync                       false
#define GLEW_ARB_pixel_buffer_object        false
#define GLEW_ARB_instanced_arrays           false
#define GLEW_EXT_texture_filter_anisotropic false
#define GLEW_KHR_debug                      false
#define GLEW_VERSION_3_2                    false


#define glClearDepth glClearDepthf
#define glDrawRangeElements(a, b, c, d, e, f) glDrawElements(a, d, e, f)


typedef void (*GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam);
extern void glDebugMessageCallback(GLDEBUGPROC callback, const GLvoid *userParam);
extern void glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);

extern void glBindVertexArray(GLuint array);
extern void glDeleteVertexArrays(GLsizei n, const GLuint* arrays);
extern void glGenVertexArrays(GLsizei n, GLuint* arrays);

extern void glBindBufferBase(GLenum target, GLuint index, GLuint buffer);
extern void glBindFragDataLocation(GLuint, GLuint, const GLchar*);

extern GLuint glGetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName);
extern void glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);

extern GLvoid* glMapBuffer(GLenum target, GLenum access);
extern void* glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
extern bool glUnmapBuffer(GLenum target);

static void glBufferStorage(GLenum target, GLsizeiptr size, const GLvoid* data, GLbitfield flags) {}
static void glInvalidateBufferData(GLuint buffer) {}
static void glInvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments) {}

static void glBindVertexBuffer(GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) {}
static void glVertexAttribBinding(GLuint attribindex, GLuint bindingindex) {}
static void glVertexAttribFormat(GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset) {}
static void glVertexAttribIFormat(GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset) {}

static void glVertexAttribIPointer(GLuint, GLint, GLenum, GLsizei, const GLvoid*) {}
static void glVertexBindingDivisor(GLuint bindingindex, GLuint divisor) {}
static void glVertexAttribDivisorARB(GLuint index, GLuint divisor) {}

static void glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) {}

static void glDrawArraysInstanced(GLenum, GLint, GLsizei, GLsizei) {}
static void glDrawElementsInstanced(GLenum, GLsizei, GLenum, const GLvoid*, GLsizei) {}

extern GLenum glClientWaitSync(GLsync GLsync,GLbitfield flags,GLuint64 timeout);
extern void glDeleteSync(GLsync GLsync);
extern GLsync glFenceSync(GLenum condition,GLbitfield flags);


#ifdef __cplusplus
}
#endif

#endif // _GLEWES_H_