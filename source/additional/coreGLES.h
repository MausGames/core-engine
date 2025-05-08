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

// TODO 3: add name-pooling to OpenGL ES (but not for WebGL, as there are no multi-gen functions)
// TODO 2: extensions should only be enabled, when all related functions are successfully retrieved
// TODO 5: GL_EXT_map_buffer_range
// TODO 5: GL_EXT_texture_storage_compression, GL_IMG_texture_filter_cubic
// TODO 3: static functions for WebGL (removing GetProcAddress, extension-checks could be enough, how to merge extensions-functions?)
// TODO 4: add all missing "shader extensions"
// TODO 3: CORE_CONFIG_GRAPHICS_DISABLEEXTENSIONS does not work for static version-checked extensions
// TODO 3: EGL_ANGLE_memory_usage_report


// ****************************************************************
/* default extensions */
#define CORE_GL_ARB_clear_buffer_object             (false)
#define CORE_GL_ARB_compute_shader                  (__CORE_GLES_VAR(bES31) && !DEFINED(_CORE_EMSCRIPTEN_))
#define CORE_GL_ARB_depth_buffer_float              (__CORE_GLES_VAR(bES30))
#define CORE_GL_ARB_direct_state_access             (false)
#define CORE_GL_ARB_map_buffer_range                (__CORE_GLES_VAR(bES30) && !DEFINED(_CORE_EMSCRIPTEN_) && !DEFINED(_CORE_ANGLE_))
#define CORE_GL_ARB_multi_bind                      (false)
#define CORE_GL_ARB_multisample                     (false)
#define CORE_GL_ARB_pipeline_statistics_query       (false)
#define CORE_GL_ARB_program_interface_query         (__CORE_GLES_VAR(bES31) && !DEFINED(_CORE_EMSCRIPTEN_))
#define CORE_GL_ARB_seamless_cube_map               (false)
#define CORE_GL_ARB_shader_image_load_store         (__CORE_GLES_VAR(bES31) && !DEFINED(_CORE_EMSCRIPTEN_))
#define CORE_GL_ARB_sync                            (__CORE_GLES_VAR(bES30))
#define CORE_GL_ARB_uniform_buffer_object           (__CORE_GLES_VAR(bES30))
#define CORE_GL_ARB_vertex_attrib_binding           (__CORE_GLES_VAR(bES31) && !DEFINED(_CORE_EMSCRIPTEN_))
#define CORE_GL_EXT_direct_state_access             (false)
#define CORE_GL_EXT_framebuffer_object              (true)   // always available
#define CORE_GL_EXT_gpu_shader4                     (__CORE_GLES_VAR(bES30))
#define CORE_GL_NVX_gpu_memory_info                 (false)
#define CORE_GL_NV_framebuffer_multisample_coverage (false)
#define CORE_GL_NV_multisample_filter_hint          (false)


// ****************************************************************
/* GL_ANGLE_texture_usage */
#define CORE_GL_AMD_framebuffer_multisample_advanced (__CORE_GLES_VAR(GL_AMD_framebuffer_multisample_advanced) && !DEFINED(_CORE_EMSCRIPTEN_))

#define GL_MAX_COLOR_FRAMEBUFFER_SAMPLES_AMD         0x91B3
#define GL_MAX_COLOR_FRAMEBUFFER_STORAGE_SAMPLES_AMD 0x91B4
#define GL_MAX_DEPTH_STENCIL_FRAMEBUFFER_SAMPLES_AMD 0x91B5

using PFNGLRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC = void (GL_APIENTRY *) (GLenum target, GLsizei samples, GLsizei storageSamples, GLenum internalformat, GLsizei width, GLsizei height);
#define glRenderbufferStorageMultisampleAdvancedAMD __CORE_GLES_FUNC(glRenderbufferStorageMultisampleAdvanced)


// ****************************************************************
/* GL_ANGLE_texture_usage */
#define CORE_GL_ANGLE_texture_usage (__CORE_GLES_VAR(GL_ANGLE_texture_usage) && !DEFINED(_CORE_EMSCRIPTEN_))

#define GL_FRAMEBUFFER_ATTACHMENT_ANGLE 0x93A3
#define GL_TEXTURE_USAGE_ANGLE          0x93A2


// ****************************************************************
/* GL_CORE_texture_float (mapped on GL_ARB_texture_float) */
#define CORE_GL_ARB_texture_float (__CORE_GLES_VAR(GL_CORE_texture_float))


// ****************************************************************
/* GL_CORE_vertex_type_2_10_10_10_rev (mapped on GL_ARB_vertex_type_2_10_10_10_rev) */
#define CORE_GL_ARB_vertex_type_2_10_10_10_rev (__CORE_GLES_VAR(GL_CORE_vertex_type_2_10_10_10_rev))


// ****************************************************************
/* GL_EXT_buffer_storage (mapped on GL_ARB_buffer_storage) */
#define CORE_GL_ARB_buffer_storage (__CORE_GLES_VAR(GL_EXT_buffer_storage) && !DEFINED(_CORE_EMSCRIPTEN_))

#define GL_CLIENT_STORAGE_BIT  0x0200
#define GL_DYNAMIC_STORAGE_BIT 0x0100
#define GL_MAP_PERSISTENT_BIT  0x0040

using PFNGLBUFFERSTORAGEPROC = void (GL_APIENTRY *) (GLenum target, GLsizeiptr size, const void* data, GLbitfield flags);
#define glBufferStorage __CORE_GLES_FUNC(glBufferStorage)


// ****************************************************************
/* GL_EXT_clear_texture (mapped on GL_ARB_clear_texture) */
#define CORE_GL_ARB_clear_texture (__CORE_GLES_VAR(GL_EXT_clear_texture) && !DEFINED(_CORE_EMSCRIPTEN_))

using PFNGLCLEARTEXIMAGEPROC = void (GL_APIENTRY *) (GLuint texture, GLint level, GLenum format, GLenum type, const void* data);
#define glClearTexImage __CORE_GLES_FUNC(glClearTexImage)


// ****************************************************************
/* GL_EXT_depth_clamp (mapped on GL_ARB_depth_clamp) */
#define CORE_GL_ARB_depth_clamp (__CORE_GLES_VAR(GL_EXT_depth_clamp))

#define GL_DEPTH_CLAMP 0x864F


// ****************************************************************
/* GL_EXT_discard_framebuffer (mapped on GL_ARB_invalidate_subdata) */
#define CORE_GL_ARB_invalidate_subdata (__CORE_GLES_VAR(GL_EXT_discard_framebuffer))

using PFNGLDISCARDFRAMEBUFFERPROC = void (GL_APIENTRY *) (GLenum target, GLsizei numAttachments, const GLenum* attachments);
#define glInvalidateBufferData(...)
#define glInvalidateBufferSubData(...) __CORE_GLES_UNUSED_ARGS(__VA_ARGS__)
#define glInvalidateFramebuffer(...)   (__CORE_GLES_VAR(bES30) ? glInvalidateFramebuffer(__VA_ARGS__) : __CORE_GLES_FUNC(glDiscardFramebuffer)(__VA_ARGS__))
#define glInvalidateSubFramebuffer(...)
#define glInvalidateTexImage(...)
#define glInvalidateTexSubImage(...)


// ****************************************************************
/* GL_EXT_disjoint_timer_query (mapped on GL_ARB_timer_query) */
#define CORE_GL_ARB_timer_query (__CORE_GLES_VAR(GL_EXT_disjoint_timer_query))

#define GL_TIMESTAMP 0x8E28

using PFNGLDELETEQUERIESPROC       = void (GL_APIENTRY *) (GLsizei n, const GLuint* ids);
using PFNGLGENQUERIESPROC          = void (GL_APIENTRY *) (GLsizei n, GLuint* ids);
using PFNGLGETQUERYOBJECTUI64VPROC = void (GL_APIENTRY *) (GLuint id, GLenum pname, GLuint64* params);
using PFNGLQUERYCOUNTERPROC        = void (GL_APIENTRY *) (GLuint id, GLenum target);
#define glDeleteQueries       __CORE_GLES_FUNC(glDeleteQueries)
#define glGenQueries          __CORE_GLES_FUNC(glGenQueries)
#define glGetQueryObjectui64v __CORE_GLES_FUNC(glGetQueryObjectui64v)
#define glQueryCounter        __CORE_GLES_FUNC(glQueryCounter)


// ****************************************************************
/* GL_EXT_draw_buffers */
#define CORE_GL_EXT_draw_buffers (__CORE_GLES_VAR(GL_EXT_draw_buffers))

using PFNGLDRAWBUFFERSPROC = void (GL_APIENTRY *) (GLsizei n, const GLenum* bufs);
#define glDrawBuffers(...) __CORE_GLES_FUNC_STATIC(bES30, glDrawBuffers, __VA_ARGS__)


// ****************************************************************
/* GL_EXT_instanced_arrays (mapped on GL_ARB_instanced_arrays) */
#define CORE_GL_ARB_instanced_arrays (__CORE_GLES_VAR(GL_EXT_instanced_arrays))

using PFNGLDRAWARRAYSINSTANCEDARBPROC   = void (GL_APIENTRY *) (GLenum mode, GLint first, GLsizei count, GLsizei primcount);
using PFNGLDRAWELEMENTSINSTANCEDARBPROC = void (GL_APIENTRY *) (GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount);
using PFNGLVERTEXATTRIBDIVISORARBPROC   = void (GL_APIENTRY *) (GLuint index, GLuint divisor);
#define glDrawArraysInstanced(...)   __CORE_GLES_FUNC_STATIC(bES30, glDrawArraysInstanced,   __VA_ARGS__)
#define glDrawElementsInstanced(...) __CORE_GLES_FUNC_STATIC(bES30, glDrawElementsInstanced, __VA_ARGS__)
#define glVertexAttribDivisor(...)   __CORE_GLES_FUNC_STATIC(bES30, glVertexAttribDivisor,   __VA_ARGS__)


// ****************************************************************
/* GL_EXT_robustness (mapped on GL_ARB_robustness) */
#define CORE_GL_ARB_robustness (__CORE_GLES_VAR(GL_EXT_robustness) && !DEFINED(_CORE_EMSCRIPTEN_))

#define GL_GUILTY_CONTEXT_RESET   0x8253
#define GL_INNOCENT_CONTEXT_RESET 0x8254
#define GL_UNKNOWN_CONTEXT_RESET  0x8255

using PFNGLGETGRAPHICSRESETSTATUSPROC = GLenum (GL_APIENTRY *) ();
using PFNGLGETNUNIFORMFVPROC          = void   (GL_APIENTRY *) (GLuint program, GLint location, GLsizei bufSize, GLfloat* params);
using PFNGLGETNUNIFORMIVPROC          = void   (GL_APIENTRY *) (GLuint program, GLint location, GLsizei bufSize, GLint* params);
using PFNGLREADNPIXELSPROC            = void   (GL_APIENTRY *) (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void* data);
#define glGetGraphicsResetStatus __CORE_GLES_FUNC(glGetGraphicsResetStatus)
#define glGetnUniformfv          __CORE_GLES_FUNC(glGetnUniformfv)
#define glGetnUniformiv          __CORE_GLES_FUNC(glGetnUniformiv)
#define glReadnPixels            __CORE_GLES_FUNC(glReadnPixels)


// ****************************************************************
/* GL_EXT_sRGB_write_control (mapped on GL_ARB_framebuffer_sRGB) */
#define CORE_GL_ARB_framebuffer_sRGB (__CORE_GLES_VAR(GL_EXT_sRGB_write_control) && !DEFINED(_CORE_EMSCRIPTEN_))

#define GL_FRAMEBUFFER_SRGB 0x8DB9


// ****************************************************************
/* GL_EXT_texture_compression_bptc (mapped on GL_ARB_texture_compression_bptc) */
#define CORE_GL_ARB_texture_compression_bptc (__CORE_GLES_VAR(GL_EXT_texture_compression_bptc))

#define GL_COMPRESSED_RGBA_BPTC_UNORM_ARB 0x8E8C


// ****************************************************************
/* GL_EXT_texture_compression_rgtc (mapped on GL_ARB_texture_compression_rgtc) */
#define CORE_GL_ARB_texture_compression_rgtc (__CORE_GLES_VAR(GL_EXT_texture_compression_rgtc))

#define GL_COMPRESSED_RED_RGTC1 0x8DBB
#define GL_COMPRESSED_RG_RGTC2  0x8DBD


// ****************************************************************
/* GL_EXT_texture_compression_s3tc */
#define CORE_GL_EXT_texture_compression_s3tc (__CORE_GLES_VAR(GL_EXT_texture_compression_s3tc))

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT  0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT 0x83F3


// ****************************************************************
/* GL_EXT_texture_filter_anisotropic (mapped on GL_ARB_texture_filter_anisotropic) */
#define CORE_GL_ARB_texture_filter_anisotropic (__CORE_GLES_VAR(GL_EXT_texture_filter_anisotropic))

#define GL_MAX_TEXTURE_MAX_ANISOTROPY 0x84FF
#define GL_TEXTURE_MAX_ANISOTROPY     0x84FE


// ****************************************************************
/* GL_EXT_texture_norm16 */
#define CORE_GL_EXT_texture_norm16 (__CORE_GLES_VAR(GL_EXT_texture_norm16))

#define GL_R16    0x822A
#define GL_RG16   0x822C
#define GL_RGB16  0x8054
#define GL_RGBA16 0x805B


// ****************************************************************
/* GL_EXT_texture_rg (mapped on GL_ARB_texture_rg) */
#define CORE_GL_ARB_texture_rg (__CORE_GLES_VAR(GL_EXT_texture_rg))


// ****************************************************************
/* GL_EXT_texture_storage (mapped on GL_ARB_texture_storage)  */
#define CORE_GL_ARB_texture_storage (__CORE_GLES_VAR(GL_EXT_texture_storage))

using PFNGLTEXSTORAGE2DPROC = void (GL_APIENTRY *) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
#define glTexStorage2D(...) __CORE_GLES_FUNC_STATIC(bES30, glTexStorage2D, __VA_ARGS__)


// ****************************************************************
/* GL_EXT_texture_type_2_10_10_10_rev */
#define CORE_GL_EXT_texture_type_2_10_10_10_rev (__CORE_GLES_VAR(GL_EXT_texture_type_2_10_10_10_REV))


// ****************************************************************
/* GL_INTEL_conservative_rasterization */
#define CORE_GL_INTEL_conservative_rasterization (__CORE_GLES_VAR(GL_INTEL_conservative_rasterization) && !DEFINED(_CORE_EMSCRIPTEN_))

#define GL_CONSERVATIVE_RASTERIZATION_INTEL 0x83FE


// ****************************************************************
/* GL_INTEL_framebuffer_CMAA */
#define CORE_GL_INTEL_framebuffer_CMAA (__CORE_GLES_VAR(GL_INTEL_framebuffer_CMAA) && !DEFINED(_CORE_EMSCRIPTEN_))

using PFNGLAPPLYFRAMEBUFFERATTACHMENTCMAAPROC = void (GL_APIENTRY *) ();
#define glApplyFramebufferAttachmentCMAAINTEL __CORE_GLES_FUNC(glApplyFramebufferAttachmentCMAA)


// ****************************************************************
/* GL_KHR_debug */
#define CORE_GL_KHR_debug (__CORE_GLES_VAR(GL_KHR_debug) && !DEFINED(_CORE_EMSCRIPTEN_))

using PFNGLDEBUGMESSAGECALLBACKPROC = void (GL_APIENTRY *) (GLDEBUGPROC callback, const void* userParam);
using PFNGLDEBUGMESSAGECONTROLPROC  = void (GL_APIENTRY *) (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
using PFNGLOBJECTLABELPROC          = void (GL_APIENTRY *) (GLenum identifier, GLuint name, GLsizei length, const GLchar* label);
#define glDebugMessageCallback __CORE_GLES_FUNC(glDebugMessageCallback)
#define glDebugMessageControl  __CORE_GLES_FUNC(glDebugMessageControl)
#define glObjectLabel          __CORE_GLES_FUNC(glObjectLabel)


// ****************************************************************
/* GL_KHR_parallel_shader_compile (mapped on GL_ARB_parallel_shader_compile) */
#define CORE_GL_ARB_parallel_shader_compile (__CORE_GLES_VAR(GL_KHR_parallel_shader_compile))

#define GL_COMPLETION_STATUS_ARB 0x91B1

using PFNGLMAXSHADERCOMPILERTHREADSPROC = void (GL_APIENTRY *) (GLuint count);
#define glMaxShaderCompilerThreadsARB __CORE_GLES_FUNC(glMaxShaderCompilerThreads)


// ****************************************************************
/* GL_NV_conservative_raster */
#define CORE_GL_NV_conservative_raster (__CORE_GLES_VAR(GL_NV_conservative_raster) && !DEFINED(_CORE_EMSCRIPTEN_))

#define GL_CONSERVATIVE_RASTERIZATION_NV 0x9346


// ****************************************************************
/* GL_NV_copy_buffer (mapped on GL_ARB_copy_buffer) */
#define CORE_GL_ARB_copy_buffer (__CORE_GLES_VAR(GL_NV_copy_buffer))

using PFNGLCOPYBUFFERSUBDATAPROC = void (GL_APIENTRY *) (GLenum readtarget, GLenum writetarget, GLintptr readoffset, GLintptr writeoffset, GLsizeiptr size);
#define glCopyBufferSubData(...) __CORE_GLES_FUNC_STATIC(bES30, glCopyBufferSubData, __VA_ARGS__)


// ****************************************************************
/* GL_NV_framebuffer_blit (mapped on GL_EXT_framebuffer_blit) */
#define CORE_GL_EXT_framebuffer_blit (__CORE_GLES_VAR(GL_NV_framebuffer_blit))

using PFNGLBLITFRAMEBUFFERPROC = void (GL_APIENTRY *) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
#define glBlitFramebuffer(...) __CORE_GLES_FUNC_STATIC(bES30, glBlitFramebuffer, __VA_ARGS__)


// ****************************************************************
/* GL_NV_framebuffer_multisample (mapped on GL_EXT_framebuffer_multisample) */
#define CORE_GL_EXT_framebuffer_multisample (__CORE_GLES_VAR(GL_NV_framebuffer_multisample))

using PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC = void (GL_APIENTRY *) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
#define glRenderbufferStorageMultisample(...) __CORE_GLES_FUNC_STATIC(bES30, glRenderbufferStorageMultisample, __VA_ARGS__)


// ****************************************************************
/* GL_NV_packed_float (mapped on GL_EXT_packed_float) */
#define CORE_GL_EXT_packed_float (__CORE_GLES_VAR(GL_NV_packed_float))


// ****************************************************************
/* GL_NV_pixel_buffer_object (mapped on GL_ARB_pixel_buffer_object) */
#define CORE_GL_ARB_pixel_buffer_object (__CORE_GLES_VAR(GL_NV_pixel_buffer_object))


// ****************************************************************
/* GL_OES_copy_image (mapped on GL_ARB_copy_image) */
#define CORE_GL_ARB_copy_image (__CORE_GLES_VAR(GL_OES_copy_image) && !DEFINED(_CORE_EMSCRIPTEN_))

using PFNGLCOPYIMAGESUBDATAPROC = void (GL_APIENTRY *) (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth);
#define glCopyImageSubData __CORE_GLES_FUNC(glCopyImageSubData)


// ****************************************************************
/* GL_OES_depth_texture (mapped on GL_ARB_depth_texture) */
#define CORE_GL_ARB_depth_texture (__CORE_GLES_VAR(GL_OES_depth_texture))


// ****************************************************************
/* GL_OES_geometry_shader (mapped on GL_ARB_geometry_shader4) */
#define CORE_GL_ARB_geometry_shader4 (__CORE_GLES_VAR(GL_OES_geometry_shader) && !DEFINED(_CORE_EMSCRIPTEN_))


// ****************************************************************
/* GL_OES_get_program_binary (mapped on GL_ARB_get_program_binary) */
#define CORE_GL_ARB_get_program_binary (__CORE_GLES_VAR(GL_OES_get_program_binary) && !DEFINED(_CORE_EMSCRIPTEN_))

#define GL_PROGRAM_BINARY_LENGTH           0x8741
#define GL_PROGRAM_BINARY_RETRIEVABLE_HINT 0x8257

using PFNGLGETPROGRAMBINARYPROC = void (GL_APIENTRY *) (GLuint program, GLsizei bufSize, GLsizei* length, GLenum *binaryFormat, void* binary);
using PFNGLPROGRAMBINARYPROC    = void (GL_APIENTRY *) (GLuint program, GLenum binaryFormat, const void* binary, GLsizei length);
#define glGetProgramBinary(...) __CORE_GLES_FUNC_STATIC(bES30, glGetProgramBinary, __VA_ARGS__)
#define glProgramBinary(...)    __CORE_GLES_FUNC_STATIC(bES30, glProgramBinary,    __VA_ARGS__)
#define glProgramParameteri(...)


// ****************************************************************
/* GL_OES_packed_depth_stencil (mapped on GL_EXT_packed_depth_stencil) */
#define CORE_GL_EXT_packed_depth_stencil (__CORE_GLES_VAR(GL_OES_packed_depth_stencil))


// ****************************************************************
/* GL_OES_sample_shading (mapped on GL_ARB_sample_shading) */
#define CORE_GL_ARB_sample_shading (__CORE_GLES_VAR(GL_OES_sample_shading) && !DEFINED(_CORE_EMSCRIPTEN_))

using PFNGLMINSAMPLESHADINGPROC = void (GL_APIENTRY *) (GLclampf value);
#define glMinSampleShading __CORE_GLES_FUNC(glMinSampleShading)


// ****************************************************************
/* GL_OES_tessellation_shader (mapped on GL_ARB_tessellation_shader) */
#define CORE_GL_ARB_tessellation_shader (__CORE_GLES_VAR(GL_OES_tessellation_shader) && !DEFINED(_CORE_EMSCRIPTEN_))


// ****************************************************************
/* GL_OES_texture_3D (mapped on GL_EXT_texture3D) */
#define CORE_GL_EXT_texture3D (__CORE_GLES_VAR(GL_OES_texture_3D))

using PFNGLTEXIMAGE3DPROC    = void (GL_APIENTRY *) (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void* pixels);
using PFNGLTEXSUBIMAGE3DPROC = void (GL_APIENTRY *) (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void* pixels);
#define glTexImage3D(...)    __CORE_GLES_FUNC_STATIC(bES30, glTexImage3D,    __VA_ARGS__)
#define glTexSubImage3D(...) __CORE_GLES_FUNC_STATIC(bES30, glTexSubImage3D, __VA_ARGS__)


// ****************************************************************
/* GL_OES_texture_stencil8 (mapped on GL_ARB_texture_stencil8) */
#define CORE_GL_ARB_texture_stencil8 (__CORE_GLES_VAR(GL_OES_texture_stencil8) && !DEFINED(_CORE_EMSCRIPTEN_))


// ****************************************************************
/* GL_OES_vertex_array_object (mapped on GL_ARB_vertex_array_object) */
#define CORE_GL_ARB_vertex_array_object (__CORE_GLES_VAR(GL_OES_vertex_array_object))

using PFNGLBINDVERTEXARRAYPROC    = void (GL_APIENTRY *) (GLuint array);
using PFNGLDELETEVERTEXARRAYSPROC = void (GL_APIENTRY *) (GLsizei n, const GLuint* arrays);
using PFNGLGENVERTEXARRAYSPROC    = void (GL_APIENTRY *) (GLsizei n, GLuint* arrays);
#define glBindVertexArray(...)    __CORE_GLES_FUNC_STATIC(bES30, glBindVertexArray,    __VA_ARGS__)
#define glDeleteVertexArrays(...) __CORE_GLES_FUNC_STATIC(bES30, glDeleteVertexArrays, __VA_ARGS__)
#define glGenVertexArrays(...)    __CORE_GLES_FUNC_STATIC(bES30, glGenVertexArrays,    __VA_ARGS__)


// ****************************************************************
/* GL_OES_vertex_half_float (mapped on GL_ARB_half_float_vertex) */
#define CORE_GL_ARB_half_float_vertex (__CORE_GLES_VAR(GL_OES_vertex_half_float))

#undef  GL_HALF_FLOAT
#define GL_HALF_FLOAT (__CORE_GLES_VAR(bES30) ? 0x140B : 0x8D61)


// ****************************************************************
/* other remapped identifiers */
#define glClearDepth                     glClearDepthf
#define glDepthRange                     glDepthRangef
#define glDrawBuffer(a)                  ([](const GLenum A) {glDrawBuffers(1, &A);}(a))
#define glDrawRangeElements(a,b,c,d,e,f) (__CORE_GLES_VAR(bES30) ? glDrawRangeElements(a, b, c, d, e, f) : glDrawElements(a, d, e, f))

using PFNGLGETBUFFERSUBDATAPROC = void (GL_APIENTRY *) (GLenum target, GLintptr offset, GLsizeiptr size, void* data);
#define glGetBufferSubData __CORE_GLES_FUNC(glGetBufferSubData)


// ****************************************************************
/* unused definitions and functions */
#define GL_CLIPPING_OUTPUT_PRIMITIVES                   0x82F7
#define GL_CONTEXT_PROFILE_MASK                         0x9126
#define GL_FRAGMENT_SHADER_INVOCATIONS                  0x82F4
#define GL_GENERATE_MIPMAP                              0x8191
#define GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX 0x9049
#define GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX         0x9047
#define GL_LUMINANCE8                                   0x8040
#define GL_LUMINANCE16                                  0x8042
#define GL_MAX_MULTISAMPLE_COVERAGE_MODES_NV            0x8E11
#define GL_MULTISAMPLE                                  0x809D
#define GL_MULTISAMPLE_COVERAGE_MODES_NV                0x8E12
#define GL_MULTISAMPLE_FILTER_HINT_NV                   0x8534
#define GL_PERSPECTIVE_CORRECTION_HINT                  0x0C50
#define GL_PRIMITIVES_SUBMITTED                         0x82EF
#define GL_TEXTURE_COMPRESSION_HINT                     0x84EF
#define GL_TEXTURE_CUBE_MAP_SEAMLESS                    0x884F
#define GL_TEXTURE_LOD_BIAS                             0x8501
#define GL_VERTEX_SHADER_INVOCATIONS                    0x82F0

#define glBindFragDataLocation(...)
#define glBindMultiTextureEXT(...)
#define glBindTextures(...) __CORE_GLES_UNUSED_ARGS(__VA_ARGS__)
#define glBindTextureUnit(...)
#define glBlitNamedFramebuffer(...)
#define glClearBufferData(...)
#define glClearNamedBufferData(...)
#define glClearNamedBufferDataEXT(...)
#define glCopyNamedBufferSubData(...)
#define glCopyTextureSubImage2D(...)
#define glCopyTextureSubImage2DEXT(...)
#define glFlushMappedNamedBufferRange(...)
#define glFlushMappedNamedBufferRangeEXT(...)
#define glGenerateTextureMipmap(...) __CORE_GLES_UNUSED_ARGS(__VA_ARGS__)
#define glGenerateTextureMipmapEXT(...) __CORE_GLES_UNUSED_ARGS(__VA_ARGS__)
#define glGetTexImage(...)
#define glGetTextureImage(...)
#define glGetTextureImageEXT(...)
#define glInvalidateNamedFramebufferData(...)
#define glMapNamedBufferRange(...) (I_TO_P(-1))
#define glMapNamedBufferRangeEXT(...) (I_TO_P(-1))
#define glNamedCopyBufferSubDataEXT(...)
#define glRenderbufferStorageMultisampleCoverageNV(...)
#define glTextureSubImage2D(...)
#define glTextureSubImage2DEXT(...)
#define glTextureSubImage3D(...)
#define glTextureSubImage3DEXT(...)
#define glUnmapNamedBuffer(...) (false)
#define glUnmapNamedBufferEXT(...) (false)

#if defined(_CORE_EMSCRIPTEN_)

    #define glBindVertexBuffer(...)
    #define glFlushMappedBufferRange(...)
    #define glGetProgramInterfaceiv(...)
    #define glGetProgramResourceiv(...) __CORE_GLES_UNUSED_ARGS(__VA_ARGS__)
    #define glGetProgramResourceName(...) __CORE_GLES_UNUSED_ARGS(__VA_ARGS__)
    #define glMapBufferRange(...) (I_TO_P(-1))
    #define glUnmapBuffer(...) (false)
    #define glVertexAttribBinding(...)
    #define glVertexAttribFormat(...) __CORE_GLES_UNUSED_ARGS(__VA_ARGS__)
    #define glVertexAttribIFormat(...)
    #define glVertexBindingDivisor(...)

#endif


// ****************************************************************
/* context structure */
struct coreContext final
{
    corePoint2U8 __Version;
    coreBool     __bES30;
    coreBool     __bES31;
    coreBool     __bES32;

    coreBool __GL_AMD_framebuffer_multisample_advanced;
    coreBool __GL_ANDROID_extension_pack_es31a;
    coreBool __GL_ANGLE_texture_usage;
    coreBool __GL_CORE_texture_float;
    coreBool __GL_CORE_vertex_type_2_10_10_10_rev;
    coreBool __GL_EXT_buffer_storage;
    coreBool __GL_EXT_clear_texture;
    coreBool __GL_EXT_color_buffer_float;
    coreBool __GL_EXT_color_buffer_half_float;
    coreBool __GL_EXT_depth_clamp;
    coreBool __GL_EXT_discard_framebuffer;
    coreBool __GL_EXT_disjoint_timer_query;
    coreBool __GL_EXT_draw_buffers;
    coreBool __GL_EXT_instanced_arrays;
    coreBool __GL_EXT_robustness;
    coreBool __GL_EXT_sRGB_write_control;
    coreBool __GL_EXT_texture_compression_bptc;
    coreBool __GL_EXT_texture_compression_rgtc;
    coreBool __GL_EXT_texture_compression_s3tc;
    coreBool __GL_EXT_texture_filter_anisotropic;
    coreBool __GL_EXT_texture_norm16;
    coreBool __GL_EXT_texture_rg;
    coreBool __GL_EXT_texture_storage;
    coreBool __GL_EXT_texture_type_2_10_10_10_REV;   // big REV
    coreBool __GL_INTEL_conservative_rasterization;
    coreBool __GL_INTEL_framebuffer_CMAA;
    coreBool __GL_KHR_debug;
    coreBool __GL_KHR_parallel_shader_compile;
    coreBool __GL_NV_conservative_raster;
    coreBool __GL_NV_copy_buffer;
    coreBool __GL_NV_framebuffer_blit;
    coreBool __GL_NV_framebuffer_multisample;
    coreBool __GL_NV_packed_float;
    coreBool __GL_NV_pixel_buffer_object;
    coreBool __GL_OES_copy_image;
    coreBool __GL_OES_depth_texture;
    coreBool __GL_OES_geometry_shader;
    coreBool __GL_OES_get_program_binary;
    coreBool __GL_OES_packed_depth_stencil;
    coreBool __GL_OES_sample_shading;
    coreBool __GL_OES_tessellation_shader;
    coreBool __GL_OES_texture_3D;
    coreBool __GL_OES_texture_float;
    coreBool __GL_OES_texture_float_linear;
    coreBool __GL_OES_texture_half_float;
    coreBool __GL_OES_texture_half_float_linear;
    coreBool __GL_OES_texture_stencil8;
    coreBool __GL_OES_vertex_array_object;
    coreBool __GL_OES_vertex_half_float;
    coreBool __GL_WEBGL_color_buffer_float;

    PFNGLRENDERBUFFERSTORAGEMULTISAMPLEADVANCEDAMDPROC __glRenderbufferStorageMultisampleAdvanced;
    PFNGLBUFFERSTORAGEPROC                             __glBufferStorage;
    PFNGLCLEARTEXIMAGEPROC                             __glClearTexImage;
    PFNGLDISCARDFRAMEBUFFERPROC                        __glDiscardFramebuffer;
    PFNGLDELETEQUERIESPROC                             __glDeleteQueries;
    PFNGLGENQUERIESPROC                                __glGenQueries;
    PFNGLGETQUERYOBJECTUI64VPROC                       __glGetQueryObjectui64v;
    PFNGLQUERYCOUNTERPROC                              __glQueryCounter;
    PFNGLDRAWBUFFERSPROC                               __glDrawBuffers;
    PFNGLDRAWARRAYSINSTANCEDARBPROC                    __glDrawArraysInstanced;
    PFNGLDRAWELEMENTSINSTANCEDARBPROC                  __glDrawElementsInstanced;
    PFNGLVERTEXATTRIBDIVISORARBPROC                    __glVertexAttribDivisor;
    PFNGLGETGRAPHICSRESETSTATUSPROC                    __glGetGraphicsResetStatus;
    PFNGLGETNUNIFORMFVPROC                             __glGetnUniformfv;
    PFNGLGETNUNIFORMIVPROC                             __glGetnUniformiv;
    PFNGLREADNPIXELSPROC                               __glReadnPixels;
    PFNGLTEXSTORAGE2DPROC                              __glTexStorage2D;
    PFNGLAPPLYFRAMEBUFFERATTACHMENTCMAAPROC            __glApplyFramebufferAttachmentCMAA;
    PFNGLDEBUGMESSAGECALLBACKPROC                      __glDebugMessageCallback;
    PFNGLDEBUGMESSAGECONTROLPROC                       __glDebugMessageControl;
    PFNGLOBJECTLABELPROC                               __glObjectLabel;
    PFNGLMAXSHADERCOMPILERTHREADSPROC                  __glMaxShaderCompilerThreads;
    PFNGLCOPYBUFFERSUBDATAPROC                         __glCopyBufferSubData;
    PFNGLBLITFRAMEBUFFERPROC                           __glBlitFramebuffer;
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC            __glRenderbufferStorageMultisample;
    PFNGLCOPYIMAGESUBDATAPROC                          __glCopyImageSubData;
    PFNGLGETPROGRAMBINARYPROC                          __glGetProgramBinary;
    PFNGLPROGRAMBINARYPROC                             __glProgramBinary;
    PFNGLMINSAMPLESHADINGPROC                          __glMinSampleShading;
    PFNGLTEXIMAGE3DPROC                                __glTexImage3D;
    PFNGLTEXSUBIMAGE3DPROC                             __glTexSubImage3D;
    PFNGLBINDVERTEXARRAYPROC                           __glBindVertexArray;
    PFNGLDELETEVERTEXARRAYSPROC                        __glDeleteVertexArrays;
    PFNGLGENVERTEXARRAYSPROC                           __glGenVertexArrays;
    PFNGLGETBUFFERSUBDATAPROC                          __glGetBufferSubData;
};

extern coreString  g_sExtensions;   // full extension string
extern coreContext g_CoreContext;   // context object

#define __CORE_GLES_CHECK(x,b)           (g_CoreContext.__ ## x = ((b) || g_sExtensions.contains(#x " ")))
#define __CORE_GLES_FUNC(f)              (g_CoreContext.__ ## f)
#define __CORE_GLES_FUNC_STATIC(v,f,...) (__CORE_GLES_VAR(v) ? f(__VA_ARGS__) : __CORE_GLES_FUNC(f)(__VA_ARGS__))
#define __CORE_GLES_FUNC_FETCH(f,a,b)    {g_CoreContext.__ ## f = r_cast<decltype(g_CoreContext.__ ## f)>(eglGetProcAddress((b) ? #f : #f #a));}
#define __CORE_GLES_VAR(v)               (g_CoreContext.__ ## v)

template <typename... A> void __UNUSED_ARGS(A...) {}
#define __CORE_GLES_UNUSED_ARGS(...) {if(false) __UNUSED_ARGS(__VA_ARGS__);}


// ****************************************************************
/* handle custom extensions */
#define CORE_GL_CORE_shared_context    true
#define CORE_GL_CORE_gl2_compatibility false
extern coreBool CORE_GL_CORE_es2_restriction;


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
extern void __coreExitOpenGLES();
#define coreInitOpenGL __coreInitOpenGLES
#define coreExitOpenGL __coreExitOpenGLES


// ****************************************************************
/* check for extensions */
extern void coreExtensions        (coreString* OUTPUT psOutput);
extern void corePlatformExtensions(coreString* OUTPUT psOutput);
#define CORE_GL_SUPPORT(e) (CORE_GL_ ## e)


#endif /* _CORE_GUARD_GLES_H_ */