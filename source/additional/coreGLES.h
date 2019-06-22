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

// TODO: __CORE_GLES_CHECK does not handle sub-strings
// TODO: add name-pooling to OpenGL ES
// TODO: GL_EXT_buffer_storage, GL_EXT_clear_texture, GL_EXT_copy_image, GL_EXT_draw_buffers, GL_EXT_geometry_shader, GL_EXT_gpu_shader5(?), GL_EXT_shader_group_vote, GL_EXT_shadow_smaplers, GL_EXT_texture_rg, GL_EXT_Texture_Type_2_10_10_10_REV, GL_NV_texture_compression_s3tc
// TODO: ANDROID_extension_pack_es31a, ES 3.X
// TODO: check more extensions to improve performance (if necessary after recent port)


// ****************************************************************
/* default extensions */
#define CORE_GL_V2_compatibility                    false
#define CORE_GL_ARB_buffer_storage                  false
#define CORE_GL_ARB_clear_buffer_object             false
#define CORE_GL_ARB_clear_texture                   false
#define CORE_GL_ARB_compute_shader                  false
#define CORE_GL_ARB_copy_buffer                     false
#define CORE_GL_ARB_copy_image                      false
#define CORE_GL_ARB_depth_buffer_float              false
#define CORE_GL_ARB_direct_state_access             false
#define CORE_GL_ARB_geometry_shader4                false
#define CORE_GL_ARB_half_float_vertex               false   // different token value in OES_vertex_half_float
#define CORE_GL_ARB_multi_bind                      false
#define CORE_GL_ARB_parallel_shader_compile         false
#define CORE_GL_ARB_program_interface_query         false
#define CORE_GL_ARB_sample_shading                  false
#define CORE_GL_ARB_sync                            false
#define CORE_GL_ARB_tessellation_shader             false
#define CORE_GL_ARB_texture_float                   false
#define CORE_GL_ARB_texture_rg                      false
#define CORE_GL_ARB_timer_query                     false
#define CORE_GL_ARB_uniform_buffer_object           false   // controls shader-handling
#define CORE_GL_ARB_vertex_attrib_binding           false
#define CORE_GL_EXT_direct_state_access             false
#define CORE_GL_EXT_framebuffer_object              true
#define CORE_GL_EXT_gpu_shader4                     false
#define CORE_GL_EXT_packed_float                    false
#define CORE_GL_EXT_shader_image_load_store         false
#define CORE_GL_EXT_texture_compression_rgtc        false
#define CORE_GL_KHR_debug                           false
#define CORE_GL_NV_framebuffer_multisample_coverage false
#define CORE_GL_NV_multisample_filter_hint          false
#define CORE_GL_NV_shader_buffer_load               false


// ****************************************************************
/* GL_EXT_discard_framebuffer (mapped on GL_ARB_invalidate_subdata) */
#define CORE_GL_ARB_invalidate_subdata __CORE_GLES_VAR(GL_EXT_discard_framebuffer)

#define GL_COLOR   0x1800
#define GL_DEPTH   0x1801
#define GL_STENCIL 0x1802

typedef void (GL_APIENTRY *PFNGLDISCARDFRAMEBUFFEREXTPROC) (GLenum target, GLsizei numAttachments, const GLenum* attachments);
#define glInvalidateBufferData(x)
#define glInvalidateFramebuffer __CORE_GLES_FUNC(glDiscardFramebufferEXT)
#define glInvalidateTexImage(x,y)


// ****************************************************************
/* GL_EXT_texture_storage */
#define CORE_GL_EXT_texture_storage __CORE_GLES_VAR(GL_EXT_texture_storage)

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
/* GL_EXT_texture_filter_anisotropic */
#define CORE_GL_EXT_texture_filter_anisotropic __CORE_GLES_VAR(GL_EXT_texture_filter_anisotropic)

#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF


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
typedef int32_t* GLsync;
typedef uint64_t GLuint64;

#define GL_ACTIVE_RESOURCES                0x92F5
#define GL_COMPARE_REF_TO_TEXTURE          0x884E
#define GL_COMPRESSED_RED_GREEN_RGTC2_EXT  0x8DBD
#define GL_COMPRESSED_RED_RGTC1_EXT        0x8DBB
#define GL_COMPUTE_SHADER                  0x91B9
#define GL_DEBUG_OUTPUT                    0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS        0x8242
#define GL_DEPTH_STENCIL                   0x84F9
#define GL_FRAGMENT_SHADER_DERIVATIVE_HINT 0x8B8B
#define GL_GENERATE_MIPMAP                 0x8191
#define GL_GEOMETRY_SHADER                 0x8DD9
#define GL_HALF_FLOAT                      0x140B   // 0x8D61 in OES_vertex_half_float
#define GL_INVALID_INDEX                   0xFFFFFFFF
#define GL_LOCATION                        0x930E
#define GL_MAP_COHERENT_BIT                0x00000080
#define GL_MAP_FLUSH_EXPLICIT_BIT          0x0010
#define GL_MAP_PERSISTENT_BIT              0x00000040
#define GL_MULTISAMPLE_FILTER_HINT_NV      0x8534
#define GL_OFFSET                          0x92FC
#define GL_PERSPECTIVE_CORRECTION_HINT     0x0C50
#define GL_PROGRAM_INPUT                   0x92E3
#define GL_QUERY_RESULT                    0x8866
#define GL_R8                              0x8229
#define GL_RED                             0x1903
#define GL_RG                              0x8227
#define GL_RG8                             0x822B
#define GL_RGB8                            0x8051
#define GL_RGBA8                           0x8058
#define GL_SYNC_FLUSH_COMMANDS_BIT         0x00000001
#define GL_SYNC_GPU_COMMANDS_COMPLETE      0x9117
#define GL_TESS_CONTROL_SHADER             0x8E88
#define GL_TESS_EVALUATION_SHADER          0x8E87
#define GL_TEXTURE_COMPARE_FUNC            0x884D
#define GL_TEXTURE_COMPARE_MODE            0x884C
#define GL_TEXTURE_COMPRESSION_HINT        0x84EF
#define GL_TIMEOUT_EXPIRED                 0x911B
#define GL_TIMEOUT_IGNORED                 0xFFFFFFFFFFFFFFFF
#define GL_TIMESTAMP                       0x8E28
#define GL_UNIFORM                         0x92E1
#define GL_UNIFORM_BUFFER                  0x8A11

#define glBindBufferBase(...)
#define glBindBufferRange(...)
#define glBindFragDataLocation(...)
#define glBindImageTexture(...)
#define glBindMultiTextureEXT(...)
#define glBindTextures(...)
#define glBindTextureUnit(...)
#define glBindVertexBuffer(...)
#define glBlitNamedFramebuffer(...)
#define glBufferStorage(...)
#define glClearBufferData(...)
#define glClearNamedBufferData(...)
#define glClearNamedBufferDataEXT(...)
#define glClearTexImage(...)
#define glClientWaitSync(...) (0)
#define glCopyTextureSubImage2D(...)
#define glCopyTextureSubImage2DEXT(...)
#define glDebugMessageCallback(...)
#define glDebugMessageControl(...)
#define glDeleteSync(...)
#define glDispatchCompute(...)
#define glDrawBuffer(...)
#define glDrawBuffers(...)
#define glFenceSync(...) (NULL)
#define glFlushMappedBufferRange(...)
#define glFlushMappedNamedBufferRange(...)
#define glFlushMappedNamedBufferRangeEXT(...)
#define glGenerateTextureMipmap(...)
#define glGenerateTextureMipmapEXT(...)
#define glGenQueries(...)
#define glGetProgramInterfaceiv(...)
#define glGetProgramResourceiv(...)
#define glGetProgramResourceName(...)
#define glGetQueryObjectui64v(...)
#define glGetUniformBlockIndex(...) (0)
#define glInvalidateNamedFramebufferData(...)
#define glMapNamedBufferRange(...) (NULL)
#define glMapNamedBufferRangeEXT(...) (NULL)
#define glMaxShaderCompilerThreadsARB(...)
#define glMinSampleShading(...)
#define glQueryCounter(...)
#define glRenderbufferStorageMultisampleCoverageNV(...)
#define glTextureSubImage2D(...)
#define glTextureSubImage2DEXT(...)
#define glUniformBlockBinding(...)
#define glUnmapNamedBuffer(...) (false)
#define glUnmapNamedBufferEXT(...) (false)
#define glVertexAttribBinding(...)
#define glVertexAttribFormat(...)
#define glVertexAttribIFormat(...)
#define glVertexAttribIPointer(...)
#define glVertexBindingDivisor(...)


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

extern std::string g_sExtensions;   //!< full extension string
extern coreContext g_CoreContext;   //!< context object

#define __CORE_GLES_CHECK(x,b)        (g_CoreContext.__ ## x = ((g_sExtensions.find(#x) != std::string::npos) || b))
#define __CORE_GLES_FUNC(f)           (g_CoreContext.__ ## f)
#define __CORE_GLES_FUNC_FETCH(f,a,b) {g_CoreContext.__ ## f ## a = r_cast<decltype(g_CoreContext.__ ## f ## a)>(eglGetProcAddress(b ? #f : #f #a));}
#define __CORE_GLES_VAR(v)            (g_CoreContext.__ ## v)
#define __CORE_GLES_VAR_SET(v,a)      {g_CoreContext.__ ## v = (a);}


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
extern void coreExtensions        (std::string* OUTPUT psOutput);
extern void corePlatformExtensions(std::string* OUTPUT psOutput);
#define CORE_GL_SUPPORT(e) (CORE_GL_ ## e)


#endif /* _CORE_GUARD_GLES_H_ */