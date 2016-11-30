//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GL_H_
#define	_CORE_GL_H_

// TODO: functionality to ignore and force arbitrary extensions through the config file
// TODO: check <GL 2.0 mappings and GLSL 1.0


// ****************************************************************
/* init and exit OpenGL */
extern void __coreInitOpenGL();
extern void __coreExitOpenGL();
#define coreInitOpenGL __coreInitOpenGL
#define coreExitOpenGL __coreExitOpenGL


// ****************************************************************
/* generate resource names */
extern void coreGenTextures2D  (coreUintW iCount, GLuint* OUTPUT pNames);
extern void coreGenBuffers     (coreUintW iCount, GLuint* OUTPUT pNames);
extern void coreGenVertexArrays(coreUintW iCount, GLuint* OUTPUT pNames);


// ****************************************************************
/* check for extensions */
extern void coreExtensions(std::string* OUTPUT psOutput);
#define CORE_GL_SUPPORT(e) ((GLEW_ ## e) ? true : false)


#endif /* _CORE_GL_H_ */