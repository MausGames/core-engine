//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
/* Only required for OpenGL ES 2.0 applications.\n
 * Implements unavailable definitions and dummy-functions. */
#pragma once
#ifndef _GLEWES_H_
#define	_GLEWES_H_

#ifdef __cplusplus
extern "C"
{
#endif


#define GLEW_OK 0

#define GL_PIXEL_UNPACK_BUFFER 0x88EC

#define GL_MAP_WRITE_BIT 0x0002
#define GL_MAP_INVALIDATE_BUFFER_BIT 0x0008

#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE

#define GL_SYNC_GPU_COMMANDS_COMPLETE 0x9117
#define GL_TIMEOUT_EXPIRED 0x911B


typedef int* GLsync;
typedef uint64_t GLuint64;


extern GLenum glewInit();

extern bool glewIsSupported(const char* pcName);
extern const char* glewGetErrorString(GLenum error);

extern void glBindVertexArray(GLuint array);
extern void glDeleteVertexArrays(GLsizei n, const GLuint* arrays);
extern void glGenVertexArrays(GLsizei n, GLuint* arrays);

extern void glBindFragDataLocation(GLuint, GLuint, const GLchar*);

extern void* glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
extern bool glUnmapBuffer(GLenum target);

extern GLenum glClientWaitSync(GLsync GLsync,GLbitfield flags,GLuint64 timeout);
extern void glDeleteSync(GLsync GLsync);
extern GLsync glFenceSync(GLenum condition,GLbitfield flags);


#ifdef __cplusplus
}
#endif

#endif // _GLEWES_H_