///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_GLES_H_
#define _CORE_GUARD_GLES_H_

// TODO 2: __CORE_GLES_CHECK does not handle sub-strings
// TODO 3: add name-pooling to OpenGL ES
// TODO 5: GL_EXT_buffer_storage, GL_EXT_clear_texture, GL_EXT_copy_image, GL_EXT_draw_buffers, GL_EXT_geometry_shader, GL_EXT_gpu_shader5(?), GL_EXT_shader_group_vote, GL_EXT_shadow_samplers, GL_EXT_texture_rg, GL_EXT_texture_type_2_10_10_10_rev, GL_NV_texture_compression_s3tc
// TODO 5: GL_OES_copy_image GL_EXT_tessellation_shader GL_OES_tessellation_shader GL_OES_framebuffer_object GL_NV_packed_float GL_EXT_map_buffer_range GL_EXT_sRGB_write_control GL_NV_pixel_buffer_object GL_KHR_parallel_shader_compile
// TODO 5: GL_NV_framebuffer_multisample GL_NV_framebuffer_blit GL_OES_vertex_array_object GL_OES_texture_stencil8 GL_NV_copy_buffer GL_OES_sample_shading GL_OES_packed_depth_stencil GL_EXT_instanced_arrays GL_NV_instanced_arrays
// TODO 5: ANDROID_extension_pack_es31a
// TODO 3: ES 3.0, 3.1, 3.2
// TODO 3: WebGL 1 & 2 (+ extensions, especially texture compression)


// ****************************************************************
/* default extensions */
#define CORE_GL_V2_compatibility                     false
#define CORE_GL_AMD_framebuffer_multisample_advanced false
#define CORE_GL_ARB_buffer_storage                   false
#define CORE_GL_ARB_clear_buffer_object              false
#define CORE_GL_ARB_clear_texture                    false
#define CORE_GL_ARB_compute_shader                   false
#define CORE_GL_ARB_copy_buffer                      false
#define CORE_GL_ARB_copy_image                       false
#define CORE_GL_ARB_depth_buffer_float               false
#define CORE_GL_ARB_direct_state_access              false
#define CORE_GL_ARB_geometry_shader4                 false
#define CORE_GL_ARB_half_float_vertex                false   // different token value in OES_vertex_half_float
#define CORE_GL_ARB_multi_bind                       false
#define CORE_GL_ARB_parallel_shader_compile          false
#define CORE_GL_ARB_pipeline_statistics_query        false
#define CORE_GL_ARB_program_interface_query          false
#define CORE_GL_ARB_sample_shading                   false
#define CORE_GL_ARB_shader_image_load_store          false
#define CORE_GL_ARB_sync                             false
#define CORE_GL_ARB_tessellation_shader              false
#define CORE_GL_ARB_texture_compression_rgtc         false
#define CORE_GL_ARB_texture_float                    false
#define CORE_GL_ARB_texture_rg                       false
#define CORE_GL_ARB_timer_query                      false
#define CORE_GL_ARB_uniform_buffer_object            false   // controls shader-handling
#define CORE_GL_ARB_vertex_attrib_binding            false
#define CORE_GL_EXT_direct_state_access              false
#define CORE_GL_EXT_framebuffer_object               true
#define CORE_GL_EXT_gpu_shader4                      false
#define CORE_GL_EXT_packed_depth_stencil             false
#define CORE_GL_EXT_packed_float                     false
#define CORE_GL_KHR_debug                            false
#define CORE_GL_NV_framebuffer_multisample_coverage  false
#define CORE_GL_NV_multisample_filter_hint           false
#define CORE_GL_NV_shader_buffer_load                false


// ****************************************************************
/* GL_EXT_discard_framebuffer (mapped on GL_ARB_invalidate_subdata) */
#define CORE_GL_ARB_invalidate_subdata __CORE_GLES_VAR(GL_EXT_discard_framebuffer)

#define GL_COLOR   0x1800
#define GL_DEPTH   0x1801
#define GL_STENCIL 0x1802

typedef void (GL_APIENTRY *PFNGLDISCARDFRAMEBUFFEREXTPROC) (GLenum target, GLsizei numAttachments, const GLenum* attachments);
#define glInvalidateBufferData(...)
#define glInvalidateBufferSubData(...) __CORE_GLES_UNUSED_ARGS(__VA_ARGS__)
#define glInvalidateFramebuffer        __CORE_GLES_FUNC(glDiscardFramebufferEXT)
#define glInvalidateSubFramebuffer(...)
#define glInvalidateTexImage(...)
#define glInvalidateTexSubImage(...)


// ****************************************************************
/* GL_EXT_texture_storage (mapped on GL_ARB_texture_storage)  */
#define CORE_GL_ARB_texture_storage __CORE_GLES_VAR(GL_EXT_texture_storage)

typedef void (GL_APIENTRY *PFNGLTEXSTORAGE2DEXTPROC) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
#define glTexStorage2D __CORE_GLES_FUNC(glTexStorage2DEXT)


// ****************************************************************
/* GL_EXT_map_buffer_range (mapped on GL_ARB_map_buffer_range) */
#define CORE_GL_ARB_map_buffer_range __CORE_GLES_VAR(GL_EXT_map_buffer_range)

#define GL_MAP_WRITE_BIT             0x0002
#define GL_MAP_INVALIDATE_RANGE_BIT  0x0004
#define GL_MAP_INVALIDATE_BUFFER_BIT 0x0008
#define GL_MAP_UNSYNCHRONIZED_BIT    0x0020

typedef GLvoid*   (GL_APIENTRY *PFNGLMAPBUFFERRANGEEXTPROC) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef GLboolean (GL_APIENTRY *PFNGLUNMAPBUFFEROESPROC)    (GLenum target);
#define glFlushMappedBufferRange(...)
#define glMapBufferRange __CORE_GLES_FUNC(glMapBufferRangeEXT)
#define glUnmapBuffer    __CORE_GLES_FUNC(glUnmapBufferOES)


// ****************************************************************
/* GL_EXT_instanced_arrays (mapped on GL_ARB_instanced_arrays) */
#define CORE_GL_ARB_instanced_arrays __CORE_GLES_VAR(GL_EXT_instanced_arrays)

typedef void (GL_APIENTRY *PFNGLDRAWARRAYSINSTANCEDEXTPROC)   (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
typedef void (GL_APIENTRY *PFNGLDRAWELEMENTSINSTANCEDEXTPROC) (GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount);
typedef void (GL_APIENTRY *PFNGLVERTEXATTRIBDIVISOREXTPROC)   (GLuint index, GLuint divisor);
#define glDrawArraysInstanced   __CORE_GLES_FUNC(glDrawArraysInstancedEXT)
#define glDrawElementsInstanced __CORE_GLES_FUNC(glDrawElementsInstancedEXT)
#define glVertexAttribDivisor   __CORE_GLES_FUNC(glVertexAttribDivisorEXT)


// ****************************************************************
/* GL_EXT_multisample_compatibility (mapped on GL_ARB_multisample) */
#define CORE_GL_ARB_multisample __CORE_GLES_VAR(GL_EXT_multisample_compatibility)

#define GL_MULTISAMPLE 0x809D


// ****************************************************************
/* GL_EXT_sRGB_write_control (mapped on GL_ARB_framebuffer_sRGB) */
#define CORE_GL_ARB_framebuffer_sRGB __CORE_GLES_VAR(GL_EXT_sRGB_write_control)

#define GL_FRAMEBUFFER_SRGB 0x8DB9


// ****************************************************************
/* GL_EXT_texture_filter_anisotropic (mapped on GL_ARB_texture_filter_anisotropic) */
#define CORE_GL_ARB_texture_filter_anisotropic __CORE_GLES_VAR(GL_EXT_texture_filter_anisotropic)

#define GL_TEXTURE_MAX_ANISOTROPY     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY 0x84FF


// ****************************************************************
/* GL_EXT_texture_compression_s3tc */
#define CORE_GL_EXT_texture_compression_s3tc __CORE_GLES_VAR(GL_EXT_texture_compression_s3tc)

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT  0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3


// ****************************************************************
/* GL_NV_pixel_buffer_object (mapped on GL_ARB_pixel_buffer_object) */
#define CORE_GL_ARB_pixel_buffer_object __CORE_GLES_VAR(GL_NV_pixel_buffer_object)

#define GL_PIXEL_PACK_BUFFER   0x88EB
#define GL_PIXEL_UNPACK_BUFFER 0x88EC


// ****************************************************************
/* GL_NV_framebuffer_blit (mapped on GL_EXT_framebuffer_blit) */
#define CORE_GL_EXT_framebuffer_blit __CORE_GLES_VAR(GL_NV_framebuffer_blit)

#define GL_READ_FRAMEBUFFER __CORE_GLES_VAR(GL_READ_FRAMEBUFFER)
#define GL_DRAW_FRAMEBUFFER __CORE_GLES_VAR(GL_DRAW_FRAMEBUFFER)

typedef void (GL_APIENTRY *PFNGLBLITFRAMEBUFFERNVPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
#define glBlitFramebuffer __CORE_GLES_FUNC(glBlitFramebufferNV)


// ****************************************************************
/* GL_NV_framebuffer_multisample (mapped on GL_EXT_framebuffer_multisample) */
#define CORE_GL_EXT_framebuffer_multisample __CORE_GLES_VAR(GL_NV_framebuffer_multisample)

#define GL_MAX_SAMPLES 0x8D57

typedef void (GL_APIENTRY *PFNGLRENDERBUFFERSTORAGEMULTISAMPLENVPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
#define glRenderbufferStorageMultisample __CORE_GLES_FUNC(glRenderbufferStorageMultisampleNV)


// ****************************************************************
/* GL_OES_vertex_array_object (mapped on GL_ARB_vertex_array_object) */
#define CORE_GL_ARB_vertex_array_object __CORE_GLES_VAR(GL_OES_vertex_array_object)

typedef void (GL_APIENTRY *PFNGLBINDVERTEXARRAYOESPROC)    (GLuint array);
typedef void (GL_APIENTRY *PFNGLDELETEVERTEXARRAYSOESPROC) (GLsizei n, const GLuint* arrays);
typedef void (GL_APIENTRY *PFNGLGENVERTEXARRAYSOESPROC)    (GLsizei n, GLuint* arrays);
#define glBindVertexArray    __CORE_GLES_FUNC(glBindVertexArrayOES)
#define glDeleteVertexArrays __CORE_GLES_FUNC(glDeleteVertexArraysOES)
#define glGenVertexArrays    __CORE_GLES_FUNC(glGenVertexArraysOES)


// ****************************************************************
/* GL_OES_vertex_type_2_10_10_10_rev (mapped on GL_ARB_vertex_type_2_10_10_10_rev) */
#define CORE_GL_ARB_vertex_type_2_10_10_10_rev __CORE_GLES_VAR(GL_OES_vertex_type_2_10_10_10_rev)

#define GL_UNSIGNED_INT_2_10_10_10_REV 0x8368
#define GL_INT_2_10_10_10_REV          0x8D9F


// ****************************************************************
/* GL_OES_depth_texture (mapped on GL_ARB_depth_texture) */
#define CORE_GL_ARB_depth_texture __CORE_GLES_VAR(GL_OES_depth_texture)


// ****************************************************************
/* GL_OES_texture_stencil8 (mapped on GL_ARB_texture_stencil8) */
#define CORE_GL_ARB_texture_stencil8 __CORE_GLES_VAR(GL_OES_texture_stencil8)


// ****************************************************************
/* other remapped identifiers */
#define glClearDepth                     glClearDepthf
#define glDepthRange                     glDepthRangef
#define glDrawRangeElements(a,b,c,d,e,f) glDrawElements(a, d, e, f)


// ****************************************************************
/* unused definitions and functions */
#define GL_CLIPPING_OUTPUT_PRIMITIVES  0x82F7
#define GL_COMPRESSED_RED_RGTC1        0x8DBB
#define GL_COMPRESSED_RG_RGTC2         0x8DBD
#define GL_FRAGMENT_SHADER_INVOCATIONS 0x82F4
#define GL_GENERATE_MIPMAP             0x8191
#define GL_LUMINANCE8                  0x8040
#define GL_LUMINANCE16                 0x8042
#define GL_MAP_PERSISTENT_BIT          0x00000040
#define GL_MULTISAMPLE_FILTER_HINT_NV  0x8534
#define GL_PERSPECTIVE_CORRECTION_HINT 0x0C50
#define GL_PRIMITIVES_SUBMITTED        0x82EF
#define GL_R16                         0x822A
#define GL_RG16                        0x822C
#define GL_RGB16                       0x8054
#define GL_RGBA16                      0x805B
#define GL_TEXTURE_COMPRESSION_HINT    0x84EF
#define GL_TIMESTAMP                   0x8E28
#define GL_VERTEX_SHADER_INVOCATIONS   0x82F0

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
#define glDrawBuffer(...)
#define glDrawBuffers(...)   // TODO 1: ES3
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
#define glMaxShaderCompilerThreadsARB(...)
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
    coreBool  __bES3;

    GLboolean __GL_EXT_discard_framebuffer;
    GLboolean __GL_EXT_texture_storage;
    GLboolean __GL_EXT_map_buffer_range;
    GLboolean __GL_EXT_instanced_arrays;
    GLboolean __GL_EXT_multisample_compatibility;
    GLboolean __GL_EXT_sRGB_write_control;
    GLboolean __GL_EXT_texture_filter_anisotropic;
    GLboolean __GL_EXT_texture_compression_s3tc;
    GLboolean __GL_NV_pixel_buffer_object;
    GLboolean __GL_NV_framebuffer_blit;
    GLboolean __GL_NV_framebuffer_multisample;
    GLboolean __GL_OES_vertex_array_object;
    GLboolean __GL_OES_vertex_type_2_10_10_10_rev;
    GLboolean __GL_OES_depth_texture;
    GLboolean __GL_OES_texture_stencil8;
    GLboolean __GL_OES_mapbuffer;    // silent

    GLenum __GL_READ_FRAMEBUFFER;
    GLenum __GL_DRAW_FRAMEBUFFER;

    PFNGLDISCARDFRAMEBUFFEREXTPROC            __glDiscardFramebufferEXT;
    PFNGLTEXSTORAGE2DEXTPROC                  __glTexStorage2DEXT;
    PFNGLMAPBUFFERRANGEEXTPROC                __glMapBufferRangeEXT;
    PFNGLUNMAPBUFFEROESPROC                   __glUnmapBufferOES;
    PFNGLDRAWARRAYSINSTANCEDEXTPROC           __glDrawArraysInstancedEXT;
    PFNGLDRAWELEMENTSINSTANCEDEXTPROC         __glDrawElementsInstancedEXT;
    PFNGLVERTEXATTRIBDIVISOREXTPROC           __glVertexAttribDivisorEXT;
    PFNGLBLITFRAMEBUFFERNVPROC                __glBlitFramebufferNV;
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLENVPROC __glRenderbufferStorageMultisampleNV;
    PFNGLBINDVERTEXARRAYOESPROC               __glBindVertexArrayOES;
    PFNGLDELETEVERTEXARRAYSOESPROC            __glDeleteVertexArraysOES;
    PFNGLGENVERTEXARRAYSOESPROC               __glGenVertexArraysOES;
};

extern coreString  g_sExtensions;   // full extension string
extern coreContext g_CoreContext;   // context object

#define __CORE_GLES_CHECK(x,b)        (g_CoreContext.__ ## x = ((g_sExtensions.find(#x) != coreString::npos) || b))
#define __CORE_GLES_FUNC(f)           (g_CoreContext.__ ## f)
#define __CORE_GLES_FUNC_FETCH(f,a,b) {g_CoreContext.__ ## f ## a = r_cast<decltype(g_CoreContext.__ ## f ## a)>(eglGetProcAddress(b ? #f : #f #a));}
#define __CORE_GLES_VAR(v)            (g_CoreContext.__ ## v)
#define __CORE_GLES_VAR_SET(v,a)      {g_CoreContext.__ ## v = (a);}

template <typename ...A> void __UNUSED_ARGS(A...) {}
#define __CORE_GLES_UNUSED_ARGS(...) {if(false) __UNUSED_ARGS(__VA_ARGS__);}


// ****************************************************************
/* generate resource names */
#define coreGenTextures2D   glGenTextures
#define coreGenBuffers      glGenBuffers
#define coreGenVertexArrays glGenVertexArrays


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