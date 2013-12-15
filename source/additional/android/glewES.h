//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
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

#define GL_WRITE_ONLY 0x88B9

#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE

#define GL_SYNC_FLUSH_COMMANDS_BIT 0x00000001
#define GL_SYNC_GPU_COMMANDS_COMPLETE 0x9117
#define GL_TIMEOUT_EXPIRED 0x911B

#define GL_RGBA8 GL_RGBA
#define GL_RGB8  GL_RGB


typedef int* GLsync;
typedef uint64_t GLuint64;

extern GLenum glewInit();

extern const char* glewGetString(GLenum name);
extern bool glewIsSupported(const char* pcName);
extern const char* glewGetErrorString(GLenum error);

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

extern GLenum glClientWaitSync(GLsync GLsync,GLbitfield flags,GLuint64 timeout);
extern void glDeleteSync(GLsync GLsync);
extern GLsync glFenceSync(GLenum condition,GLbitfield flags);


#ifdef __cplusplus
}
#endif

#endif // _GLEWES_H_