///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_GLES_H_
#define _CORE_GUARD_GLES_H_

// TODO 3: add name-pooling to OpenGL ES
// TODO 2: extensions should only be enabled, when all related functions are successfully retrieved
// TODO 5: GL_EXT_buffer_storage, GL_EXT_clear_texture, GL_EXT_copy_image, GL_EXT_draw_buffers, GL_EXT_geometry_shader, GL_EXT_gpu_shader5(?), GL_EXT_shader_group_vote, GL_EXT_texture_rg
// TODO 5: GL_OES_copy_image, GL_EXT_tessellation_shader, GL_OES_tessellation_shader, GL_OES_framebuffer_object, GL_NV_packed_float, GL_EXT_map_buffer_range, GL_EXT_sRGB_write_control
// TODO 5: GL_NV_copy_buffer, GL_OES_sample_shading, GL_EXT_instanced_arrays, GL_NV_instanced_arrays, GL_EXT_disjoint_timer_query, GL_EXT_disjoint_timer_query_webgl2
// TODO 5: GL_ANGLE_texture_usage (set before allocation), GL_AMD_framebuffer_multisample_advanced, GL_OES_geometry_shader, GL_OES_tessellation_shader, GL_WEBGL_draw_buffers, GL_OES_vertex_half_float (token value different)
// TODO 5: ANDROID_extension_pack_es31a
// TODO 5: WEBGL float textures: OES_texture_float(ES3), OES_texture_float_linear(not ES3!), OES_texture_half_float(ES3), OES_texture_half_float_linear(ES3), EXT_color_buffer_float(?), EXT_color_buffer_half_float(?), WEBGL_color_buffer_float(?)


// ****************************************************************
/* default extensions */
#define CORE_GL_AMD_framebuffer_multisample_advanced false
#define CORE_GL_ARB_buffer_storage                   false
#define CORE_GL_ARB_clear_buffer_object              false
#define CORE_GL_ARB_clear_texture                    false
#define CORE_GL_ARB_compute_shader                   __CORE_GLES_VAR(bES31)
#define CORE_GL_ARB_copy_buffer                      __CORE_GLES_VAR(bES30)
#define CORE_GL_ARB_copy_image                       false
#define CORE_GL_ARB_depth_buffer_float               __CORE_GLES_VAR(bES30)
#define CORE_GL_ARB_direct_state_access              false
#define CORE_GL_ARB_framebuffer_sRGB                 false
#define CORE_GL_ARB_geometry_shader4                 __CORE_GLES_VAR(bES32)
#define CORE_GL_ARB_half_float_vertex                __CORE_GLES_VAR(bES30)
#define CORE_GL_ARB_instanced_arrays                 __CORE_GLES_VAR(bES30)
#define CORE_GL_ARB_multi_bind                       false
#define CORE_GL_ARB_multisample                      false
#define CORE_GL_ARB_pipeline_statistics_query        false
#define CORE_GL_ARB_sample_shading                   false
#define CORE_GL_ARB_shader_image_load_store          __CORE_GLES_VAR(bES31)
#define CORE_GL_ARB_sync                             __CORE_GLES_VAR(bES30)
#define CORE_GL_ARB_tessellation_shader              __CORE_GLES_VAR(bES32)
#define CORE_GL_ARB_texture_float                    __CORE_GLES_VAR(bES30)
#define CORE_GL_ARB_texture_rg                       __CORE_GLES_VAR(bES30)
#define CORE_GL_ARB_timer_query                      false
#define CORE_GL_ARB_uniform_buffer_object            __CORE_GLES_VAR(bES30)   // controls shader-handling
#define CORE_GL_EXT_demote_to_helper_invocation      false
#define CORE_GL_EXT_direct_state_access              false
#define CORE_GL_EXT_framebuffer_object               true
#define CORE_GL_EXT_gpu_shader4                      __CORE_GLES_VAR(bES30)
#define CORE_GL_EXT_packed_float                     __CORE_GLES_VAR(bES30)
#define CORE_GL_KHR_debug                            false
#define CORE_GL_NVX_gpu_memory_info                  false
#define CORE_GL_NV_framebuffer_multisample_coverage  false
#define CORE_GL_NV_multisample_filter_hint           false
#define CORE_GL_NV_shader_buffer_load                false

#if defined(_CORE_EMSCRIPTEN_)
    #define CORE_GL_ARB_map_buffer_range             false
    #define CORE_GL_ARB_program_interface_query      false
    #define CORE_GL_ARB_vertex_attrib_binding        false
#else
    #define CORE_GL_ARB_map_buffer_range             __CORE_GLES_VAR(bES30)
    #define CORE_GL_ARB_program_interface_query      __CORE_GLES_VAR(bES31)
    #define CORE_GL_ARB_vertex_attrib_binding        __CORE_GLES_VAR(bES30)
#endif


// ****************************************************************
/* GL_EXT_discard_framebuffer (mapped on GL_ARB_invalidate_subdata) */
#define CORE_GL_ARB_invalidate_subdata __CORE_GLES_VAR(GL_EXT_discard_framebuffer)

using PFNGLDISCARDFRAMEBUFFEREXTPROC = void (GL_APIENTRY *) (GLenum target, GLsizei numAttachments, const GLenum* attachments);
#define glInvalidateBufferData(...)
#define glInvalidateBufferSubData(...) __CORE_GLES_UNUSED_ARGS(__VA_ARGS__)
#define glInvalidateFramebuffer        __CORE_GLES_FUNC(glDiscardFramebufferEXT)
#define glInvalidateSubFramebuffer(...)
#define glInvalidateTexImage(...)
#define glInvalidateTexSubImage(...)


// ****************************************************************
/* GL_EXT_texture_storage (mapped on GL_ARB_texture_storage)  */
#define CORE_GL_ARB_texture_storage __CORE_GLES_VAR(GL_EXT_texture_storage)

using PFNGLTEXSTORAGE2DEXTPROC = void (GL_APIENTRY *) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
#define glTexStorage2D __CORE_GLES_FUNC(glTexStorage2DEXT)


// ****************************************************************
/* GL_EXT_texture_filter_anisotropic (mapped on GL_ARB_texture_filter_anisotropic) */
#define CORE_GL_ARB_texture_filter_anisotropic __CORE_GLES_VAR(GL_EXT_texture_filter_anisotropic)

#define GL_TEXTURE_MAX_ANISOTROPY     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY 0x84FF


// ****************************************************************
/* GL_EXT_texture_compression_rgtc (mapped on GL_ARB_texture_compression_rgtc) */
#define CORE_GL_ARB_texture_compression_rgtc __CORE_GLES_VAR(GL_EXT_texture_compression_rgtc)

#define GL_COMPRESSED_RED_RGTC1 0x8DBB
#define GL_COMPRESSED_RG_RGTC2  0x8DBD


// ****************************************************************
/* GL_EXT_texture_compression_s3tc */
#define CORE_GL_EXT_texture_compression_s3tc __CORE_GLES_VAR(GL_EXT_texture_compression_s3tc)

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT  0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3


// ****************************************************************
/* GL_EXT_texture_norm16 */
#define CORE_GL_EXT_texture_norm16 __CORE_GLES_VAR(GL_EXT_texture_norm16)

#define GL_R16    0x822A
#define GL_RG16   0x822C
#define GL_RGB16  0x8054
#define GL_RGBA16 0x805B


// ****************************************************************
/* GL_EXT_texture_type_2_10_10_10_rev */
#define CORE_GL_EXT_texture_type_2_10_10_10_rev __CORE_GLES_VAR(GL_EXT_texture_type_2_10_10_10_rev)


// ****************************************************************
/* GL_KHR_parallel_shader_compile (mapped on GL_ARB_parallel_shader_compile) */
#define CORE_GL_ARB_parallel_shader_compile __CORE_GLES_VAR(GL_KHR_parallel_shader_compile)

#define GL_COMPLETION_STATUS_ARB 0x91B1

using PFNGLMAXSHADERCOMPILERTHREADSARBPROC = void (GL_APIENTRY *) (GLuint count);
#define glMaxShaderCompilerThreadsARB __CORE_GLES_FUNC(glMaxShaderCompilerThreadsKHR)


// ****************************************************************
/* GL_NV_pixel_buffer_object (mapped on GL_ARB_pixel_buffer_object) */
#define CORE_GL_ARB_pixel_buffer_object __CORE_GLES_VAR(GL_NV_pixel_buffer_object)


// ****************************************************************
/* GL_NV_framebuffer_blit (mapped on GL_EXT_framebuffer_blit) */
#define CORE_GL_EXT_framebuffer_blit __CORE_GLES_VAR(GL_NV_framebuffer_blit)

using PFNGLBLITFRAMEBUFFERNVPROC = void (GL_APIENTRY *) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
#define glBlitFramebuffer __CORE_GLES_FUNC(glBlitFramebufferNV)


// ****************************************************************
/* GL_NV_framebuffer_multisample (mapped on GL_EXT_framebuffer_multisample) */
#define CORE_GL_EXT_framebuffer_multisample __CORE_GLES_VAR(GL_NV_framebuffer_multisample)

using PFNGLRENDERBUFFERSTORAGEMULTISAMPLENVPROC = void (GL_APIENTRY *) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
#define glRenderbufferStorageMultisample __CORE_GLES_FUNC(glRenderbufferStorageMultisampleNV)


// ****************************************************************
/* GL_OES_vertex_array_object (mapped on GL_ARB_vertex_array_object) */
#define CORE_GL_ARB_vertex_array_object __CORE_GLES_VAR(GL_OES_vertex_array_object)

using PFNGLBINDVERTEXARRAYOESPROC    = void (GL_APIENTRY *) (GLuint array);
using PFNGLDELETEVERTEXARRAYSOESPROC = void (GL_APIENTRY *) (GLsizei n, const GLuint* arrays);
using PFNGLGENVERTEXARRAYSOESPROC    = void (GL_APIENTRY *) (GLsizei n, GLuint* arrays);
#define glBindVertexArray    __CORE_GLES_FUNC(glBindVertexArrayOES)
#define glDeleteVertexArrays __CORE_GLES_FUNC(glDeleteVertexArraysOES)
#define glGenVertexArrays    __CORE_GLES_FUNC(glGenVertexArraysOES)


// ****************************************************************
/* GL_OES_vertex_type_2_10_10_10_rev (mapped on GL_ARB_vertex_type_2_10_10_10_rev) */
#define CORE_GL_ARB_vertex_type_2_10_10_10_rev __CORE_GLES_VAR(GL_OES_vertex_type_2_10_10_10_rev)


// ****************************************************************
/* GL_OES_depth_texture (mapped on GL_ARB_depth_texture) */
#define CORE_GL_ARB_depth_texture __CORE_GLES_VAR(GL_OES_depth_texture)


// ****************************************************************
/* GL_OES_texture_stencil8 (mapped on GL_ARB_texture_stencil8) */
#define CORE_GL_ARB_texture_stencil8 __CORE_GLES_VAR(GL_OES_texture_stencil8)


// ****************************************************************
/* GL_OES_packed_depth_stencil (mapped on GL_EXT_packed_depth_stencil) */
#define CORE_GL_EXT_packed_depth_stencil __CORE_GLES_VAR(GL_OES_packed_depth_stencil)


// ****************************************************************
/* other remapped identifiers */
inline decltype(glDrawBuffers)*       const __glDrawBuffers       = &glDrawBuffers;
inline decltype(glDrawRangeElements)* const __glDrawRangeElements = &glDrawRangeElements;

#define glClearDepth                     glClearDepthf
#define glDepthRange                     glDepthRangef
#define glDrawBuffer(a)                  CALL(const GLenum A = (a); glDrawBuffers(1, &A);)
#define glDrawBuffers(a,b)               CALL(if(__CORE_GLES_VAR(bES30)) __glDrawBuffers(a, b);)
#define glDrawRangeElements(a,b,c,d,e,f) CALL(if(__CORE_GLES_VAR(bES30)) __glDrawRangeElements(a, b, c, d, e, f); else glDrawElements(a, d, e, f);)


// ****************************************************************
/* unused definitions and functions */
#define GL_CLIPPING_OUTPUT_PRIMITIVES                   0x82F7
#define GL_FRAGMENT_SHADER_INVOCATIONS                  0x82F4
#define GL_FRAMEBUFFER_SRGB                             0x8DB9
#define GL_GENERATE_MIPMAP                              0x8191
#define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX 0x9049
#define GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX         0x9047
#define GL_LUMINANCE8                                   0x8040
#define GL_LUMINANCE16                                  0x8042
#define GL_MAP_PERSISTENT_BIT                           0x00000040
#define GL_MULTISAMPLE                                  0x809D
#define GL_MULTISAMPLE_FILTER_HINT_NV                   0x8534
#define GL_PERSPECTIVE_CORRECTION_HINT                  0x0C50
#define GL_PRIMITIVES_SUBMITTED                         0x82EF
#define GL_TEXTURE_COMPRESSION_HINT                     0x84EF
#define GL_TIMESTAMP                                    0x8E28
#define GL_VERTEX_SHADER_INVOCATIONS                    0x82F0

#define glBindFragDataLocation(...)
#define glBindMultiTextureEXT(...)
#define glBindTextures(...) __CORE_GLES_UNUSED_ARGS(__VA_ARGS__)
#define glBindTextureUnit(...)
#define glBlitNamedFramebuffer(...)
#define glBufferStorage(...)
#define glClearBufferData(...)
#define glClearNamedBufferData(...)
#define glClearNamedBufferDataEXT(...)
#define glClearTexImage(...)
#define glCopyNamedBufferSubData(...)
#define glCopyTextureSubImage2D(...)
#define glCopyTextureSubImage2DEXT(...)
#define glFlushMappedNamedBufferRange(...)
#define glFlushMappedNamedBufferRangeEXT(...)
#define glGenerateTextureMipmap(...)
#define glGenerateTextureMipmapEXT(...)
#define glGetQueryObjectui64v(...)
#define glGetTexImage(...)
#define glGetTextureImage(...)
#define glGetTextureImageEXT(...)
#define glInvalidateNamedFramebufferData(...)
#define glMakeBufferResidentNV(...)
#define glMapNamedBufferRange(...) (I_TO_P(-1))
#define glMapNamedBufferRangeEXT(...) (I_TO_P(-1))
#define glNamedCopyBufferSubDataEXT(...)
#define glQueryCounter(...)
#define glRenderbufferStorageMultisampleAdvancedAMD(...)
#define glRenderbufferStorageMultisampleCoverageNV(...)
#define glTextureSubImage2D(...)
#define glTextureSubImage2DEXT(...)
#define glUnmapNamedBuffer(...) (false)
#define glUnmapNamedBufferEXT(...) (false)


// ****************************************************************
/* context structure */
struct coreContext final
{
    coreFloat __fVersion;
    coreBool  __bES30;
    coreBool  __bES31;
    coreBool  __bES32;

    coreBool __GL_EXT_discard_framebuffer;
    coreBool __GL_EXT_texture_storage;
    coreBool __GL_EXT_texture_filter_anisotropic;
    coreBool __GL_EXT_texture_compression_rgtc;
    coreBool __GL_EXT_texture_compression_s3tc;
    coreBool __GL_EXT_texture_norm16;
    coreBool __GL_EXT_texture_type_2_10_10_10_rev;
    coreBool __GL_KHR_parallel_shader_compile;
    coreBool __GL_NV_pixel_buffer_object;
    coreBool __GL_NV_framebuffer_blit;
    coreBool __GL_NV_framebuffer_multisample;
    coreBool __GL_OES_vertex_array_object;
    coreBool __GL_OES_vertex_type_2_10_10_10_rev;
    coreBool __GL_OES_depth_texture;
    coreBool __GL_OES_texture_stencil8;
    coreBool __GL_OES_packed_depth_stencil;

    PFNGLDISCARDFRAMEBUFFEREXTPROC            __glDiscardFramebufferEXT;
    PFNGLTEXSTORAGE2DEXTPROC                  __glTexStorage2DEXT;
    PFNGLMAXSHADERCOMPILERTHREADSARBPROC      __glMaxShaderCompilerThreadsKHR;
    PFNGLBLITFRAMEBUFFERNVPROC                __glBlitFramebufferNV;
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLENVPROC __glRenderbufferStorageMultisampleNV;
    PFNGLBINDVERTEXARRAYOESPROC               __glBindVertexArrayOES;
    PFNGLDELETEVERTEXARRAYSOESPROC            __glDeleteVertexArraysOES;
    PFNGLGENVERTEXARRAYSOESPROC               __glGenVertexArraysOES;
};

extern coreString  g_sExtensions;   // full extension string
extern coreContext g_CoreContext;   // context object

#define __CORE_GLES_CHECK(x,b)        (g_CoreContext.__ ## x = ((g_sExtensions.find(#x " ") != coreString::npos) || b))
#define __CORE_GLES_FUNC(f)           (g_CoreContext.__ ## f)
#define __CORE_GLES_FUNC_FETCH(f,a,b) {g_CoreContext.__ ## f ## a = r_cast<decltype(g_CoreContext.__ ## f ## a)>(eglGetProcAddress(b ? #f : #f #a));}
#define __CORE_GLES_VAR(v)            (g_CoreContext.__ ## v)

template <typename ...A> void __UNUSED_ARGS(A...) {}
#define __CORE_GLES_UNUSED_ARGS(...) {if(false) __UNUSED_ARGS(__VA_ARGS__);}


// ****************************************************************
/* support deprecated features */
#define CORE_GL_V2_compatibility false
extern coreBool CORE_GL_ES2_restriction;


// ****************************************************************
/* generate and delete resource names */
#define coreGenTextures2D   glGenTextures
#define coreGenBuffers      glGenBuffers
#define coreGenVertexArrays glGenVertexArrays
#define coreDelTextures2D   glDeleteTextures
#define coreDelBuffers      glDeleteBuffers
#define coreDelVertexArrays glDeleteVertexArrays


// ****************************************************************
/* init and exit OpenGL ES */
extern void __coreInitOpenGLES();
#define coreInitOpenGL __coreInitOpenGLES
#define coreExitOpenGL [](){}


// ****************************************************************
/* check for extensions */
extern void coreExtensions        (coreString* OUTPUT psOutput);
extern void corePlatformExtensions(coreString* OUTPUT psOutput);
#define CORE_GL_SUPPORT(e) (CORE_GL_ ## e)


#endif /* _CORE_GUARD_GLES_H_ */