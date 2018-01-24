//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_GL_H_
#define _CORE_GUARD_GL_H_

// TODO: check <GL 2.0 mappings and GLSL 1.0 (both on my notebook)
// TODO: maybe add pools for renderbuffer, framebuffer, program, shader, queries
// TODO: GL_ARB_texture_filter_anisotropic (+ES)
// TODO: GL_KHR_parallel_shader_compile (+ES)


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
extern void coreExtensions        (std::string* OUTPUT psOutput);
extern void corePlatformExtensions(std::string* OUTPUT psOutput);
#define CORE_GL_SUPPORT(e) ((GLEW_ ## e) ? true : false)


#endif /* _CORE_GUARD_GL_H_ */