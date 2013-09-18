#include "Core.h"

#if defined(_CORE_GLES_)

GLenum glewInit()
{

}



bool glewIsSupported(const char* pcName)     {return false;}
const char* glewGetErrorString(GLenum error) {return "";}




void glBindVertexArray(GLuint array)                       {}
void glDeleteVertexArrays(GLsizei n, const GLuint* arrays) {}
void glGenVertexArrays(GLsizei n, GLuint* arrays)          {}


void glBindFragDataLocation(GLuint, GLuint, const GLchar*) {}



void* glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) {return NULL;}
bool glUnmapBuffer(GLenum target)                                                            {return true;}


GLenum glClientWaitSync(GLsync GLsync,GLbitfield flags,GLuint64 timeout) {return 0;}
void glDeleteSync(GLsync GLsync)                                         {}
GLsync glFenceSync(GLenum condition,GLbitfield flags)                    {return NULL;}


#endif