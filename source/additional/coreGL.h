///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_GL_H_
#define _CORE_GUARD_GL_H_

// TODO 3: also log function remapping (similar to extension override) (4. line in load-message)
// TODO 3: reuse (pooled) resource names


// ****************************************************************
/* support deprecated features */
extern coreBool GLEW_V2_compatibility;


// ****************************************************************
/* generate resource names */
extern void coreGenTextures2D  (coreUintW iCount, GLuint* OUTPUT pNames);
extern void coreGenBuffers     (coreUintW iCount, GLuint* OUTPUT pNames);
extern void coreGenVertexArrays(coreUintW iCount, GLuint* OUTPUT pNames);


// ****************************************************************
/* init and exit OpenGL */
extern void __coreInitOpenGL();
extern void __coreExitOpenGL();
#define coreInitOpenGL __coreInitOpenGL
#define coreExitOpenGL __coreExitOpenGL


// ****************************************************************
/* check for extensions */
extern void coreExtensions        (coreString* OUTPUT psOutput);
extern void corePlatformExtensions(coreString* OUTPUT psOutput);
#define CORE_GL_SUPPORT(e) ((GLEW_ ## e) ? true : false)


#endif /* _CORE_GUARD_GL_H_ */