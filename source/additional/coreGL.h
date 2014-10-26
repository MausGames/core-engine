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

#if defined(__cplusplus)
    extern "C" {
#endif

// TODO: functionality to ignore and force arbitrary extensions through the config file


// ****************************************************************   
/* GLEW context structure */
extern thread_local GLEWContext g_GlewContext;
#define glewGetContext() (g_GlewContext)


// ****************************************************************   
/* init OpenGL */
extern void __coreInitOpenGL();
#define coreInitOpenGL __coreInitOpenGL


// ****************************************************************   
/* check for extensions */
#define CORE_GL_SUPPORT(e) ((GLEW_ ## e) ? true : false)


#if defined(__cplusplus)
    }
#endif

#endif /* _CORE_GL_H_ */