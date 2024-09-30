///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////


// #version                         (#)   // shader version
// #define _CORE_*_SHADER_          (1)   // shader type (vertex, fragment, ...)
// #define _CORE_OPTION_*_          (1)   // multiple preprocessor options
// #define _CORE_QUALITY_           (#)   // quality level
// #define _CORE_WEBGL_             (1)   // WebGL environment
// #define CORE_NUM_TEXTURES_2D     (#)   // number of 2d texture units
// #define CORE_NUM_TEXTURES_SHADOW (#)   // number of shadow texture units
// #define CORE_NUM_LIGHTS          (#)   // number of light sources
// #define CORE_NUM_OUTPUTS         (#)   // number of output colors

// compiler configuration
#if defined(GL_ES)
    #extension GL_EXT_conservative_depth               : enable
    #extension GL_EXT_demote_to_helper_invocation      : enable
    #extension GL_EXT_draw_buffers                     : enable
    #extension GL_EXT_gpu_shader5                      : enable
    #extension GL_EXT_shader_group_vote                : enable
    #extension GL_EXT_shader_io_blocks                 : enable
    #extension GL_EXT_shader_texture_lod               : enable
    #extension GL_EXT_shadow_samplers                  : enable
    #extension GL_EXT_texture_query_lod                : enable
    #extension GL_NV_draw_buffers                      : enable
    #extension GL_OES_sample_variables                 : enable
    #extension GL_OES_shader_io_blocks                 : enable
    #extension GL_OES_shader_multisample_interpolation : enable
    #extension GL_OES_standard_derivatives             : enable
    #extension GL_OES_texture_3D                       : enable
#else
    #extension GL_AMD_conservative_depth               : enable
    #extension GL_AMD_gpu_shader_half_float            : enable
    #extension GL_AMD_shader_trinary_minmax            : enable
    #extension GL_ARB_conservative_depth               : enable
    #extension GL_ARB_enhanced_layouts                 : enable
    #extension GL_ARB_gpu_shader5                      : enable
    #extension GL_ARB_sample_shading                   : enable
    #extension GL_ARB_shader_group_vote                : enable
    #extension GL_ARB_shader_image_load_store          : enable
    #extension GL_ARB_shader_texture_lod               : enable
    #extension GL_ARB_shading_language_packing         : enable
    #extension GL_ARB_texture_query_lod                : enable
    #extension GL_ARB_uniform_buffer_object            : enable
    #extension GL_EXT_demote_to_helper_invocation      : enable
    #extension GL_EXT_gpu_shader4                      : enable
    #extension GL_EXT_shader_image_load_store          : enable
    #extension GL_NV_gpu_shader5                       : enable
#endif
#pragma optimize(on)
#pragma debug(off)

// feature helpers
#if defined(GL_ES)
    #define CORE_GL_VERSION    (0)
    #define CORE_GL_ES_VERSION (__VERSION__)
#else
    #define CORE_GL_VERSION    (__VERSION__)
    #define CORE_GL_ES_VERSION (0)
#endif
#if (CORE_GL_VERSION >= 130) || (CORE_GL_ES_VERSION >= 300)
    #define CORE_GL_MODERN_API
#endif
#if (defined(GL_AMD_conservative_depth) || defined(GL_ARB_conservative_depth) || defined(GL_EXT_conservative_depth) || (CORE_GL_VERSION >= 420)) && !defined(_CORE_WEBGL_)
    #define CORE_GL_conservative_depth
#endif
#if (defined(GL_EXT_draw_buffers) || defined(GL_NV_draw_buffers) || (CORE_GL_VERSION >= 110) || (CORE_GL_ES_VERSION >= 300))
    #define CORE_GL_draw_buffers
#endif
#if (defined(GL_EXT_gpu_shader4) || (CORE_GL_VERSION >= 130) || (CORE_GL_ES_VERSION >= 300))
    #define CORE_GL_gpu_shader4
#endif
#if (defined(GL_ARB_gpu_shader5) || defined(GL_EXT_gpu_shader5) || (CORE_GL_VERSION >= 400) || (CORE_GL_ES_VERSION >= 320))
    #define CORE_GL_gpu_shader5
#endif
#if (defined(GL_ARB_gpu_shader5) || defined(GL_OES_shader_multisample_interpolation) || (CORE_GL_VERSION >= 400) || (CORE_GL_ES_VERSION >= 320))
    #define CORE_GL_sample_interpolation
#endif
#if (defined(GL_ARB_sample_shading) || defined(GL_OES_sample_variables) || (CORE_GL_VERSION >= 400) || (CORE_GL_ES_VERSION >= 320))
    #define CORE_GL_sample_shading
#endif
#if (defined(GL_ARB_shader_image_load_store) || defined(GL_EXT_shader_image_load_store) || (CORE_GL_VERSION >= 420) || (CORE_GL_ES_VERSION >= 310))
    #define CORE_GL_shader_image_load_store
#endif
#if (defined(GL_EXT_shader_io_blocks) || defined(GL_OES_shader_io_blocks) || (CORE_GL_VERSION >= 140) || (CORE_GL_ES_VERSION >= 320))
    #define CORE_GL_shader_io_blocks
#endif
#if (defined(GL_OES_standard_derivatives) || (CORE_GL_VERSION >= 110) || (CORE_GL_ES_VERSION >= 300))
    #define CORE_GL_standard_derivatives
#endif
#if (defined(GL_OES_texture_3D) || (CORE_GL_VERSION >= 110) || (CORE_GL_ES_VERSION >= 300))
    #define CORE_GL_texture_3D
#endif
#if (defined(GL_ARB_texture_query_lod) || defined(GL_EXT_texture_query_lod) || (CORE_GL_VERSION >= 400)) && defined(CORE_GL_MODERN_API)
    #define CORE_GL_texture_query_lod
#endif
#if (defined(GL_ARB_uniform_buffer_object) || (CORE_GL_VERSION >= 140) || (CORE_GL_ES_VERSION >= 300))
    #define CORE_GL_uniform_buffer_object
#endif

// precision qualifiers
#if defined(GL_ES)
    #if defined(_CORE_FRAGMENT_SHADER_)
        precision mediump float;
    #endif
    #if !defined(GL_FRAGMENT_PRECISION_HIGH)
        #define highp mediump   // in all shaders
    #endif
#else
    #define highp
    #define mediump
    #define lowp
#endif

// layout qualifiers
#if defined(_CORE_FRAGMENT_SHADER_) && !defined(_CORE_OPTION_NO_EARLY_DEPTH_)
    #if defined(CORE_GL_conservative_depth)
        layout(depth_unchanged) out float gl_FragDepth;
    #endif
    #if defined(CORE_GL_shader_image_load_store)
        layout(early_fragment_tests) in;
    #endif
#endif
#if defined(GL_ARB_enhanced_layouts)
    #define layoutEx(x) layout(x)
#else
    #define layoutEx(x)
#endif

// compatibility adjustments
#if defined(GL_ES) && (CORE_GL_ES_VERSION < 300)
    #if defined(GL_EXT_shadow_samplers)
        #define shadow2DProj(t,v) (shadow2DProjEXT(t, v))
    #else
        #define sampler2DShadow   sampler2D
        #define shadow2DProj(t,v) (coreShadow2DProj(t, v))
        vec4 coreShadow2DProj(const in sampler2DShadow t, const in vec4 v) {return (texture2DProj(t, v).r < (v.z / v.w)) ? vec4(1.0) : vec4(0.0);}
    #endif
#endif
#if defined(GL_EXT_shader_texture_lod)
    #define texture2DLod(t,v,l)     (texture2DLodEXT    (t, v, l))
    #define texture2DProjLod(t,v,l) (texture2DProjLodEXT(t, v, l))
    #define shadow2DProjLod(t,v,l)  (shadow2DProj       (t, v))
#elif !defined(GL_ARB_shader_texture_lod)
    #define texture2DLod(t,v,l)     (texture2D          (t, v))
    #define texture2DProjLod(t,v,l) (texture2DProj      (t, v))
    #define shadow2DProjLod(t,v,l)  (shadow2DProj       (t, v))
#endif
#if defined(CORE_GL_MODERN_API)
    #if defined(_CORE_VERTEX_SHADER_)
        #define attribute in
        #define varying   out
    #endif
    #if defined(_CORE_FRAGMENT_SHADER_)
        #define varying in
    #endif
#endif
#if !defined(CORE_GL_gpu_shader4)
    #define flat
    #define noperspective
    #define smooth
    #define centroid
#endif

#if !defined(CORE_GL_sample_interpolation)
    #define sample
#endif
#if !defined(CORE_GL_gpu_shader5)
    #define precise
    #define fma(a,b,c) ((a) * (b) + (c))
#endif
#if !defined(GL_ES) && (CORE_GL_VERSION < 120)
    #define invariant
#endif
#if !defined(CORE_GL_standard_derivatives)
    #define dFdx(x)   ((x) * 0.0)
    #define dFdy(x)   ((x) * 0.0)
    #define fwidth(x) ((x) * 0.0)
#endif
#if defined(GL_EXT_demote_to_helper_invocation)
    #define discard demote
#endif

// type definitions
#if defined(CORE_GL_gpu_shader4)
    #if !defined(GL_ES) && (CORE_GL_VERSION < 130)
        #define uint unsigned int
    #endif
#else
    #define uint  int
    #define uvec2 ivec2
    #define uvec3 ivec3
    #define uvec4 ivec4
#endif
#if defined(GL_AMD_gpu_shader_half_float) || defined(GL_NV_gpu_shader5)
    #define half  float16_t
    #define hvec2 f16vec2
    #define hvec3 f16vec3
    #define hvec4 f16vec4
#else
    #define half  float
    #define hvec2 vec2
    #define hvec3 vec3
    #define hvec4 vec4
#endif

// 3d-object uniforms
uniform highp   vec3 u_v3Position;
uniform mediump vec3 u_v3Size;
uniform mediump vec4 u_v4Rotation;

// 2d-object uniforms
uniform highp   vec3 u_v3TwoPosition;
uniform highp   vec2 u_v2TwoSize;
uniform mediump vec2 u_v2TwoRotation;

// default object uniforms
uniform lowp    vec4 u_v4Color;
uniform mediump vec4 u_v4TexParam;

// texture uniforms
uniform lowp    sampler2D       u_as2Texture2D    [CORE_NUM_TEXTURES_2D];
uniform mediump sampler2DShadow u_as2TextureShadow[CORE_NUM_TEXTURES_SHADOW];

// recommended texture lookup
#if defined(CORE_GL_MODERN_API)
    #define coreTexture2D(u,c)         (texture         (u_as2Texture2D    [u], c))
    #define coreTextureProj(u,c)       (textureProj     (u_as2Texture2D    [u], c))
    #define coreTextureShadow(u,c)     (textureProj     (u_as2TextureShadow[u], c))
    #define coreTextureBase2D(u,c)     (textureLod      (u_as2Texture2D    [u], c, 0.0))
    #define coreTextureBaseProj(u,c)   (textureProjLod  (u_as2Texture2D    [u], c, 0.0))
    #define coreTextureBaseShadow(u,c) (textureProjLod  (u_as2TextureShadow[u], c, 0.0))
#else
    #define coreTexture2D(u,c)         (texture2D       (u_as2Texture2D    [u], c))
    #define coreTextureProj(u,c)       (texture2DProj   (u_as2Texture2D    [u], c))
    #define coreTextureShadow(u,c)     (shadow2DProj    (u_as2TextureShadow[u], c).r)
    #define coreTextureBase2D(u,c)     (texture2DLod    (u_as2Texture2D    [u], c, 0.0))
    #define coreTextureBaseProj(u,c)   (texture2DProjLod(u_as2Texture2D    [u], c, 0.0))
    #define coreTextureBaseShadow(u,c) (shadow2DProjLod (u_as2TextureShadow[u], c, 0.0).r)
#endif
#if defined(CORE_GL_texture_query_lod)
    #define coreTextureSharp2D(u,c,s)  (textureLod(u_as2Texture2D[u], c, textureQueryLod(u_as2Texture2D[u], c).y - (s)))
#else
    #define coreTextureSharp2D(u,c,s)  (coreTexture2D(u, c))
#endif