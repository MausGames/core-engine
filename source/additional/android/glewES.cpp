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
#include "Core.h"


GLenum glewInit() {return GLEW_OK;}

const char* glewGetString(GLenum name)       {return "";}
bool glewIsSupported(const char* pcName)     {return false;}
const char* glewGetErrorString(GLenum error) {return "";}

void glDebugMessageCallback(GLDEBUGPROC callback, const GLvoid *userParam)                                                   {}
void glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled) {}

void glBindVertexArray(GLuint array)                       {}
void glDeleteVertexArrays(GLsizei n, const GLuint* arrays) {}
void glGenVertexArrays(GLsizei n, GLuint* arrays)          {}

void glBindBufferBase(GLenum target, GLuint index, GLuint buffer) {}
void glBindFragDataLocation(GLuint, GLuint, const GLchar*)        {}

GLuint glGetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName)                    {return -1;}
void glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) {}

GLvoid* glMapBuffer(GLenum target, GLenum access)                                            {return NULL;}
void* glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) {return NULL;}
bool glUnmapBuffer(GLenum target)                                                            {return true;}

GLenum glClientWaitSync(GLsync GLsync,GLbitfield flags,GLuint64 timeout) {return 0;}
void glDeleteSync(GLsync GLsync)                                         {}
GLsync glFenceSync(GLenum condition,GLbitfield flags)                    {return NULL;}