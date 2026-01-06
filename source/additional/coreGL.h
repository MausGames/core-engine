///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_GL_H_
#define _CORE_GUARD_GL_H_

// TODO 3: CORE_CONFIG_GRAPHICS_DISABLEEXTENSIONS does not work for shader extensions (also on GLES)


// ****************************************************************
/* handle custom extensions */
extern coreBool GLEW_CORE_shared_context;      // general support for shared contexts
extern coreBool GLEW_CORE_gl2_compatibility;   // handle deprecated OpenGL features
#define GLEW_CORE_es2_restriction false        // handle certain OpenGL ES limitations


// ****************************************************************
/* handle OpenGL ES extensions */
#define GLEW_ANGLE_pack_reverse_row_order    false
#define GLEW_ANGLE_polygon_mode              true
#define GLEW_ANGLE_provoking_vertex          false
#define GLEW_ANGLE_texture_usage             false
#define GLEW_EXT_draw_buffers                true
#define GLEW_EXT_texture_norm16              true
#define GLEW_EXT_texture_type_2_10_10_10_rev true

#define GL_FRAMEBUFFER_ATTACHMENT_ANGLE 0x93A3
#define GL_PACK_REVERSE_ROW_ORDER_ANGLE 0x93A4
#define GL_TEXTURE_USAGE_ANGLE          0x93A2


// ****************************************************************
/* handle OpenGL extensions below minimum */
#define GLEW_ARB_depth_texture true
#define GLEW_ARB_multisample   true
#define GLEW_EXT_texture3D     true


// ****************************************************************
/* generate and delete resource names */
extern void coreGenTextures2D  (coreUintW iCount, GLuint* OUTPUT pNames);
extern void coreGenBuffers     (coreUintW iCount, GLuint* OUTPUT pNames);
extern void coreGenVertexArrays(coreUintW iCount, GLuint* OUTPUT pNames);
extern void coreDelTextures2D  (coreUintW iCount, const GLuint*  pNames);
extern void coreDelBuffers     (coreUintW iCount, const GLuint*  pNames);
extern void coreDelVertexArrays(coreUintW iCount, const GLuint*  pNames);


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