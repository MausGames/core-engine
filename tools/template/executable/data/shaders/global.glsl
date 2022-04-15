///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////


// ****************************************************************
// #version                         (#)   // shader version
// #define _CORE_*_SHADER_          (1)   // shader type (vertex, fragment, ...)
// #define _CORE_OPTION_*_          (1)   // multiple preprocessor options
// #define _CORE_TARGET_*_          (1)   // multiple target adjustments
// #define _CORE_QUALITY_           (#)   // quality level
// #define CORE_NUM_TEXTURES_2D     (#)   // number of 2d texture units
// #define CORE_NUM_TEXTURES_SHADOW (#)   // number of shadow texture units
// #define CORE_NUM_LIGHTS          (#)   // number of light sources
// #define CORE_NUM_OUTPUTS         (#)   // number of output colors

// compiler configuration
#if defined(GL_ES)
    #extension GL_EXT_conservative_depth          : enable
    #extension GL_EXT_shadow_samplers             : enable
    #extension GL_OES_sample_variables            : enable
    #extension GL_OES_standard_derivatives        : enable
#else
    #extension GL_AMD_conservative_depth          : enable
    #extension GL_AMD_gpu_shader_half_float       : enable
    #extension GL_AMD_shader_trinary_minmax       : enable
    #extension GL_ARB_conservative_depth          : enable
    #extension GL_ARB_enhanced_layouts            : enable
    #extension GL_ARB_gpu_shader5                 : enable
    #extension GL_ARB_sample_shading              : enable
    #extension GL_ARB_shader_group_vote           : enable
    #extension GL_ARB_shader_image_load_store     : enable
    #extension GL_ARB_shading_language_packing    : enable
    #extension GL_ARB_uniform_buffer_object       : enable
    #extension GL_EXT_demote_to_helper_invocation : enable
    #extension GL_EXT_gpu_shader4                 : enable
    #extension GL_EXT_shader_image_load_store     : enable
    #extension GL_NV_gpu_shader5                  : enable
#endif
#pragma optimize(on)
#pragma debug(off)

// feature helper
#if defined(GL_ES)
    #define CORE_GL_VERSION    (0)
    #define CORE_GL_ES_VERSION (__VERSION__)
#else
    #define CORE_GL_VERSION    (__VERSION__)
    #define CORE_GL_ES_VERSION (0)
#endif
#if defined(GL_AMD_conservative_depth) || defined(GL_ARB_conservative_depth) || defined(GL_EXT_conservative_depth) || (CORE_GL_VERSION >= 420)
    #define CORE_GL_conservative_depth
#endif
#if defined(GL_EXT_gpu_shader4) || (CORE_GL_VERSION >= 130) || (CORE_GL_ES_VERSION >= 300)
    #define CORE_GL_gpu_shader4
#endif
#if defined(GL_ARB_sample_shading) || defined(GL_OES_sample_variables) || (CORE_GL_VERSION >= 400) || (CORE_GL_ES_VERSION >= 320)
    #define CORE_GL_sample_shading
#endif
#if defined(GL_ARB_shader_image_load_store) || defined(GL_EXT_shader_image_load_store) || (CORE_GL_VERSION >= 420) || (CORE_GL_ES_VERSION >= 310)
    #define CORE_GL_shader_image_load_store
#endif
#if defined(GL_OES_standard_derivatives) || (CORE_GL_VERSION >= 110) || (CORE_GL_ES_VERSION >= 300)
    #define CORE_GL_standard_derivatives
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
#if defined(_CORE_FRAGMENT_SHADER_) && !defined(_CORE_OPTION_NO_EARLY_DEPTH_) && ((CORE_GL_VERSION >= 130) || (CORE_GL_ES_VERSION >= 300))
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
#if (CORE_GL_VERSION >= 130) || (CORE_GL_ES_VERSION >= 300)
    #if defined(_CORE_VERTEX_SHADER_)
        #define attribute in
        #define varying   out
    #endif
    #if defined(_CORE_FRAGMENT_SHADER_)
        #define varying in
    #endif
#else
    #undef _CORE_OPTION_INSTANCING_
#endif
#if !defined(CORE_GL_gpu_shader4)
    #define flat
    #define noperspective
    #define smooth
    #define centroid
#endif
#if !defined(GL_ARB_gpu_shader5)
    #define sample
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
#if !defined(CORE_GL_gpu_shader4)
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


// ****************************************************************
#define PI    (3.1415926535897932384626433832795)
#define EU    (2.7182818284590452353602874713527)
#define GR    (1.6180339887498948482045868343656)
#define GA    (2.3999632297286533222315555066336)
#define SQRT2 (1.4142135623730950488016887242097)
#define SQRT3 (1.7320508075688772935274463415059)

// evaluate shader per sample
#if defined(CORE_GL_sample_shading)
    #define CORE_SAMPLE_SHADING {int A = gl_SampleID;}
#else
    #define CORE_SAMPLE_SHADING
#endif

// trinary min and max
#if defined(GL_AMD_shader_trinary_minmax)
    #define coreMin3(a,b,c) (min3(a, b, c))
    #define coreMax3(a,b,c) (max3(a, b, c))
    #define coreMed3(a,b,c) (mid3(a, b, c))
#else
    #define coreMin3(a,b,c) (min(a, min(b, c)))
    #define coreMax3(a,b,c) (max(a, max(b, c)))
    float coreMed3(const in float a, const in float b, const in float c) {return max(min(max(a, b), c), min(a, b));}
    vec2  coreMed3(const in vec2  a, const in vec2  b, const in vec2  c) {return max(min(max(a, b), c), min(a, b));}
    vec3  coreMed3(const in vec3  a, const in vec3  b, const in vec3  c) {return max(min(max(a, b), c), min(a, b));}
    vec4  coreMed3(const in vec4  a, const in vec4  b, const in vec4  c) {return max(min(max(a, b), c), min(a, b));}
#endif

// condition across group of shader invocations
#if defined(GL_ARB_shader_group_vote)
    #define coreAnyInvocation(x)  (anyInvocationARB (x))
    #define coreAllInvocations(x) (allInvocationsARB(x))
#elif defined(GL_NV_gpu_shader5)
    #define coreAnyInvocation(x)  (anyThreadNV (x))
    #define coreAllInvocations(x) (allThreadsNV(x))
#else
    #define coreAnyInvocation(x)  (x)
    #define coreAllInvocations(x) (x)
#endif

// clamp values between 0.0 and 1.0
#define coreSaturate(x) (clamp(x, 0.0, 1.0))

// linear interpolation between 0.0 and 1.0
float coreLinearStep(const in float e0, const in float e1, const in float v) {return coreSaturate((v - e0) / (e1 - e0));}
vec2  coreLinearStep(const in vec2  e0, const in vec2  e1, const in vec2  v) {return coreSaturate((v - e0) / (e1 - e0));}
vec3  coreLinearStep(const in vec3  e0, const in vec3  e1, const in vec3  v) {return coreSaturate((v - e0) / (e1 - e0));}
vec4  coreLinearStep(const in vec4  e0, const in vec4  e1, const in vec4  v) {return coreSaturate((v - e0) / (e1 - e0));}

// extract the sign without returning 0.0
float coreSign(const in float v) {return (v >= 0.0) ? 1.0 : -1.0;}
#if (CORE_GL_VERSION >= 130) || (CORE_GL_ES_VERSION >= 300)
    vec2 coreSign(const in vec2 v) {return mix(vec2(-1.0), vec2(1.0), greaterThanEqual(v, vec2(0.0)));}
    vec3 coreSign(const in vec3 v) {return mix(vec3(-1.0), vec3(1.0), greaterThanEqual(v, vec3(0.0)));}
    vec4 coreSign(const in vec4 v) {return mix(vec4(-1.0), vec4(1.0), greaterThanEqual(v, vec4(0.0)));}
#else
    vec2 coreSign(const in vec2 v) {return vec2(coreSign(v.x), coreSign(v.y));}
    vec3 coreSign(const in vec3 v) {return vec3(coreSign(v.x), coreSign(v.y), coreSign(v.z));}
    vec4 coreSign(const in vec4 v) {return vec4(coreSign(v.x), coreSign(v.y), coreSign(v.z), coreSign(v.w));}
#endif

// test if the parameter is not a number
#if (CORE_GL_VERSION >= 130) || (CORE_GL_ES_VERSION >= 300)
    #define coreIsNan(x) (isnan(x))
#else
    bool  coreIsNan(const in float v) {return !((v == 0.0) || (v < 0.0) || (v > 0.0));}
    bvec2 coreIsNan(const in vec2  v) {return bvec2(coreIsNan(v.x), coreIsNan(v.y));}
    bvec3 coreIsNan(const in vec3  v) {return bvec3(coreIsNan(v.x), coreIsNan(v.y), coreIsNan(v.z));}
    bvec4 coreIsNan(const in vec4  v) {return bvec4(coreIsNan(v.x), coreIsNan(v.y), coreIsNan(v.z), coreIsNan(v.w));}
#endif

// color convert
vec3 coreRgbToHsv(const in vec3 v3Rgb)
{
    float R = v3Rgb.r;
    float G = v3Rgb.g;
    float B = v3Rgb.b;

    float v = coreMax3(R, G, B);
    float d = v - coreMin3(R, G, B);

    if(d == 0.0) return vec3(0.0, 0.0, v);

    float s = d / v;

    if(R == v) return vec3((0.0 + (G - B) / d) / 6.0, s, v);
    if(G == v) return vec3((2.0 + (B - R) / d) / 6.0, s, v);
               return vec3((4.0 + (R - G) / d) / 6.0, s, v);
}
vec3 coreHsvToRgb(const in vec3 v3Hsv)
{
    float H = v3Hsv.x * 6.0;
    float S = v3Hsv.y;
    float V = v3Hsv.z;

    float h = floor(H);

    float s = V * S;
    float t = s * (H - h);
    float p = V - s;

    if(h == 1.0) return vec3(V - t, V,     p);
    if(h == 2.0) return vec3(p,     V,     p + t);
    if(h == 3.0) return vec3(p,     V - t, V);
    if(h == 4.0) return vec3(p + t, p,     V);
    if(h == 5.0) return vec3(V,     p,     V - t);
                 return vec3(V,     p + t, p);
}
vec3 coreRgbToYiq(const in vec3 v3Rgb)
{
    return mat3(0.299,  0.587,  0.114,
                0.596, -0.275, -0.321,
                0.212, -0.523,  0.311) * v3Rgb;
}
vec3 coreYiqToRgb(const in vec3 v3Yiq)
{
    return mat3(1.000,  0.956,  0.620,
                1.000, -0.272, -0.647,
                1.000, -1.108,  1.705) * v3Yiq;
}
vec3 coreRgbToYuv(const in vec3 v3Rgb)
{
    return mat3( 0.21260,  0.71520,  0.07220,
                -0.09991, -0.33609,  0.43600,
                 0.61500, -0.55861, -0.05639) * v3Rgb;
}
vec3 coreYuvToRgb(const in vec3 v3Yuv)
{
    return mat3(1.00000,  0.00000,  1.28033,
                1.00000, -0.21482, -0.38059,
                1.00000,  2.12798,  0.00000) * v3Yuv;
}
vec3 coreRgbToYcbcr(const in vec3 v3Rgb)
{
    return mat3( 0.299000,  0.587000,  0.114000,
                -0.168736, -0.331264,  0.500000,
                 0.500000, -0.418688, -0.081312) * v3Rgb + vec3(0.0, 0.5, 0.5);
}
vec3 coreYcbcrToRgb(const in vec3 v3Ycbcr)
{
    return mat3(1.00000,  0.00000,  1.40200,
                1.00000, -0.34414, -0.71414,
                1.00000,  1.77200,  0.00000) * (v3Ycbcr - vec3(0.0, 0.5, 0.5));
}
float coreLuminance(const in vec3 v3Rgb)
{
    return dot(v3Rgb, vec3(0.2126, 0.7152, 0.0722));
}

// vector square length
float coreLengthSq(const in vec2 v) {return dot(v, v);}
float coreLengthSq(const in vec3 v) {return dot(v, v);}

// vector normal pack and unpack
vec2 corePackNormalSphere(const in vec3 v)
{
    float A = inversesqrt(v.z * 8.0 + 8.0);
    return v.xy * A + 0.5;
}
vec3 coreUnpackNormalSphere(const in vec2 v)
{
    vec2  A = v * 4.0 - 2.0;
    float B = coreLengthSq(A);
    float C = sqrt(1.0 - B / 4.0);
    return vec3(A * C, 1.0 - B / 2.0);
}
vec3 coreUnpackNormalMap(const in vec2 v)
{
    vec2 A = v * 2.0 - 1.0;
    return normalize(vec3(A, sqrt(1.0 - coreLengthSq(A))));
}
vec3 coreUnpackNormalMapDeriv(const in vec2 v)
{
    vec2 A = v * 2.0 - 1.0;
    return normalize(vec3(A, 1.0));
}
vec2 corePackNormalOcta(const in vec3 v)
{
    vec2 A = v.xy / (abs(v.x) + abs(v.y) + abs(v.z));
    A = (v.z >= 0.0) ? A : ((vec2(1.0) - abs(A.yx)) * coreSign(A));
    return A;
}
vec3 coreUnpackNormalOcta(const in vec2 v)
{
    vec3 A = vec3(v, 1.0 - abs(v.x) - abs(v.y));
    A.xy += coreSign(A.xy) * -coreSaturate(-A.z);
    return normalize(A);
}

// quaternion transformation
vec3 coreQuatApply(const in vec4 q, const in vec3 v)
{
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

// matrix transpose
mat2 coreTranspose(const in mat2 m)
{
#if (CORE_GL_VERSION >= 120) || (CORE_GL_ES_VERSION >= 300)
    return transpose(m);
#else
    return mat2(m[0][0], m[1][0],
                m[0][1], m[1][1]);
#endif
}
mat3 coreTranspose(const in mat3 m)
{
#if (CORE_GL_VERSION >= 120) || (CORE_GL_ES_VERSION >= 300)
    return transpose(m);
#else
    return mat3(m[0][0], m[1][0], m[2][0],
                m[0][1], m[1][1], m[2][1],
                m[0][2], m[1][2], m[2][2]);
#endif
}
mat4 coreTranspose(const in mat4 m)
{
#if (CORE_GL_VERSION >= 120) || (CORE_GL_ES_VERSION >= 300)
    return transpose(m);
#else
    return mat4(m[0][0], m[1][0], m[2][0], m[3][0],
                m[0][1], m[1][1], m[2][1], m[3][1],
                m[0][2], m[1][2], m[2][2], m[3][2],
                m[0][3], m[1][3], m[2][3], m[3][3]);
#endif
}

// matrix invert
mat3 coreInvert(const in mat3 m)
{
#if (CORE_GL_VERSION >= 150) || (CORE_GL_ES_VERSION >= 300)
    return inverse(m);
#else
    float A = m[1][1]*m[2][2] - m[1][2]*m[2][1];
    float B = m[1][2]*m[2][0] - m[1][0]*m[2][2];
    float C = m[1][0]*m[2][1] - m[1][1]*m[2][0];

    return mat3(A, m[0][2]*m[2][1] - m[0][1]*m[2][2], m[0][1]*m[1][2] - m[0][2]*m[1][1],
                B, m[0][0]*m[2][2] - m[0][2]*m[2][0], m[0][2]*m[1][0] - m[0][0]*m[1][2],
                C, m[0][1]*m[2][0] - m[0][0]*m[2][1], m[0][0]*m[1][1] - m[0][1]*m[1][0])
                * (1.0 / (m[0][0]*A + m[0][1]*B + m[0][2]*C));
#endif
}
mat4 coreInvert(const in mat4 m)
{
#if (CORE_GL_VERSION >= 150) || (CORE_GL_ES_VERSION >= 300)
    return inverse(m);
#else
    float A = m[0][0]*m[1][1] - m[0][1]*m[1][0];
    float B = m[0][0]*m[1][2] - m[0][2]*m[1][0];
    float C = m[0][0]*m[1][3] - m[0][3]*m[1][0];
    float D = m[0][1]*m[1][2] - m[0][2]*m[1][1];
    float E = m[0][1]*m[1][3] - m[0][3]*m[1][1];
    float F = m[0][2]*m[1][3] - m[0][3]*m[1][2];
    float G = m[2][0]*m[3][1] - m[2][1]*m[3][0];
    float H = m[2][0]*m[3][2] - m[2][2]*m[3][0];
    float I = m[2][0]*m[3][3] - m[2][3]*m[3][0];
    float J = m[2][1]*m[3][2] - m[2][2]*m[3][1];
    float K = m[2][1]*m[3][3] - m[2][3]*m[3][1];
    float L = m[2][2]*m[3][3] - m[2][3]*m[3][2];

    return mat4(m[1][1]*L - m[1][2]*K + m[1][3]*J, m[0][2]*K - m[0][1]*L - m[0][3]*J, m[3][1]*F - m[3][2]*E + m[3][3]*D, m[2][2]*E - m[2][1]*F - m[2][3]*D,
                m[1][2]*I - m[1][0]*L - m[1][3]*H, m[0][0]*L - m[0][2]*I + m[0][3]*H, m[3][2]*C - m[3][0]*F - m[3][3]*B, m[2][0]*F - m[2][2]*C + m[2][3]*B,
                m[1][0]*K - m[1][1]*I + m[1][3]*G, m[0][1]*I - m[0][0]*K - m[0][3]*G, m[3][0]*E - m[3][1]*C + m[3][3]*A, m[2][1]*C - m[2][0]*E - m[2][3]*A,
                m[1][1]*H - m[1][0]*J - m[1][2]*G, m[0][0]*J - m[0][1]*H + m[0][2]*G, m[3][1]*B - m[3][0]*D - m[3][2]*A, m[2][0]*D - m[2][1]*B + m[2][2]*A)
                * (1.0 / (A*L - B*K + C*J + D*I - E*H + F*G));
#endif
}

// matrix convert
#if (CORE_GL_VERSION >= 120) || (CORE_GL_ES_VERSION >= 300)
    #define coreMat4to3(m) mat3(m)
    #define coreMat3to2(m) mat2(m)
#else
    #define coreMat4to3(m) mat3(m[0].xyz, m[1].xyz, m[2].xyz)
    #define coreMat3to2(m) mat2(m[0].xy,  m[1].xy)
#endif
#define coreMat4to2(m) coreMat3to2(m)

// value pack and unpack
#if defined(CORE_GL_gpu_shader4)

    uint corePackUnorm4x8(const in vec4 x)
    {
    #if defined(GL_ARB_shading_language_packing)
        return packUnorm4x8(x);
    #else
        return (uint(x.a * 255.0) << 24) +
               (uint(x.b * 255.0) << 16) +
               (uint(x.g * 255.0) <<  8) +
               (uint(x.r * 255.0));
    #endif
    }

    vec4 coreUnpackUnorm4x8(const in uint x)
    {
    #if defined(GL_ARB_shading_language_packing)
        return unpackUnorm4x8(x);
    #else
        return vec4(float( x        & 0xFFu),
                    float((x >>  8) & 0xFFu),
                    float((x >> 16) & 0xFFu),
                    float((x >> 24) & 0xFFu)) * 0.003921569;
    #endif
    }

#endif


// ****************************************************************
#if (CORE_GL_VERSION >= 130) || (CORE_GL_ES_VERSION >= 300)

    // transformation uniforms
    layout(std140) uniform b_Transform
    {
        layoutEx(align = 16) highp   mat4 u_m4ViewProj;
        layoutEx(align = 16) highp   mat4 u_m4Camera;
        layoutEx(align = 16) highp   mat4 u_m4Perspective;
        layoutEx(align = 16) highp   mat4 u_m4Ortho;
        layoutEx(align = 16) highp   vec4 u_v4Resolution;
        layoutEx(align = 16) highp   vec3 u_v3CamPosition;
    };

    // ambient uniforms
    layout(std140) uniform b_Ambient
    {
        layoutEx(align = 16) highp   vec4 u_av4LightPos  [CORE_NUM_LIGHTS];
        layoutEx(align = 16) mediump vec4 u_av4LightDir  [CORE_NUM_LIGHTS];
        layoutEx(align = 16) mediump vec4 u_av4LightValue[CORE_NUM_LIGHTS];
    };

#else

    // transformation uniforms
    uniform highp   mat4 u_m4ViewProj;
    uniform highp   mat4 u_m4Camera;
    uniform highp   mat4 u_m4Perspective;
    uniform highp   mat4 u_m4Ortho;
    uniform highp   vec4 u_v4Resolution;
    uniform highp   vec3 u_v3CamPosition;

    // ambient uniforms
    uniform highp   vec4 u_av4LightPos  [CORE_NUM_LIGHTS];
    uniform mediump vec4 u_av4LightDir  [CORE_NUM_LIGHTS];
    uniform mediump vec4 u_av4LightValue[CORE_NUM_LIGHTS];

#endif

// 3d-object uniforms
uniform highp   vec3 u_v3Position;
uniform mediump vec3 u_v3Size;
uniform mediump vec4 u_v4Rotation;

// 2d-object uniforms
uniform highp   mat3 u_m3ScreenView;

// default object uniforms
uniform lowp    vec4 u_v4Color;
uniform mediump vec4 u_v4TexParam;

// texture uniforms
uniform lowp    sampler2D       u_as2Texture2D    [CORE_NUM_TEXTURES_2D];
uniform mediump sampler2DShadow u_as2TextureShadow[CORE_NUM_TEXTURES_SHADOW];


// ****************************************************************
#if defined(_CORE_VERTEX_SHADER_)

    #if (CORE_GL_VERSION >= 130) || (CORE_GL_ES_VERSION >= 300)

        // vertex attributes
        in vec3 a_v3RawPosition;
        in vec2 a_v2RawTexCoord;
        in vec3 a_v3RawNormal;
        in vec4 a_v4RawTangent;

        // instancing attributes
        in vec3 a_v3DivPosition;
        in vec3 a_v3DivSize;
        in vec4 a_v4DivRotation;
        in vec3 a_v3DivData;
        in vec4 a_v4DivColor;
        in vec4 a_v4DivTexParam;

    #else

        // vertex attributes
        attribute vec3 a_v3RawPosition;
        attribute vec2 a_v2RawTexCoord;
        attribute vec3 a_v3RawNormal;
        attribute vec4 a_v4RawTangent;

        // instancing uniforms
        uniform highp   vec3 a_v3DivPosition;
        uniform mediump vec3 a_v3DivData;

    #endif

    #if (CORE_GL_VERSION >= 140) || (CORE_GL_ES_VERSION >= 310)

        // shader output
        out b_Varying
        {
            flat vec4 v_v4VarColor;
            vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D];
            vec4 v_av4LightPos[CORE_NUM_LIGHTS];
            vec4 v_av4LightDir[CORE_NUM_LIGHTS];
            vec3 v_v3TangentPos;
            vec3 v_v3TangentCam;
        };

    #else

        // shader output
        flat varying vec4 v_v4VarColor;
        varying vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D];
        varying vec4 v_av4LightPos[CORE_NUM_LIGHTS];
        varying vec4 v_av4LightDir[CORE_NUM_LIGHTS];
        varying vec3 v_v3TangentPos;
        varying vec3 v_v3TangentCam;

    #endif

    // low-memory attributes
    vec2 a_v2LowPosition;
    vec2 a_v2LowTexCoord;

    // remapped variables
    #if defined(_CORE_OPTION_INSTANCING_)
        #define u_v3Position  (a_v3DivPosition)
        #define u_v3Size      (a_v3DivSize)
        #define u_v4Rotation  (a_v4DivRotation)
        #define u_v4Color     (a_v4DivColor)
        #define u_v2TexSize   (a_v4DivTexParam.xy)
        #define u_v2TexOffset (a_v4DivTexParam.zw)
    #else
        #define u_v2TexSize   (u_v4TexParam.xy)
        #define u_v2TexOffset (u_v4TexParam.zw)
    #endif
    #define a_v1DivScale (a_v3DivData.x)
    #define a_v1DivAngle (a_v3DivData.y)
    #define a_v1DivValue (a_v3DivData.z)

    // main function
    void VertexMain();
    void ShaderMain()
    {
    #if defined(_CORE_TARGET_MACOS_)
        v_v4VarColor   = vec4(0.0);
        for(int i = 0; i < CORE_NUM_TEXTURES_2D; ++i) v_av2TexCoord[i] = vec2(0.0);
        for(int i = 0; i < CORE_NUM_LIGHTS;      ++i) v_av4LightPos[i] = vec4(0.0);
        for(int i = 0; i < CORE_NUM_LIGHTS;      ++i) v_av4LightDir[i] = vec4(0.0);
        v_v3TangentPos = vec3(0.0);
        v_v3TangentCam = vec3(0.0);
    #endif

    #if defined(_CORE_OPTION_INSTANCING_)
        v_v4VarColor = a_v4DivColor;
    #endif

        a_v2LowPosition = a_v3RawPosition.xy;
        a_v2LowTexCoord = vec2(0.5 + a_v3RawPosition.x, 0.5 - a_v3RawPosition.y);
        VertexMain();
    }

#endif // _CORE_VERTEX_SHADER_


// ****************************************************************
#if defined(_CORE_TESS_CONTROL_SHADER_)

    // main function
    void TessControlMain();
    void ShaderMain()
    {
        TessControlMain();
    }

#endif // _CORE_TESS_CONTROL_SHADER_


// ****************************************************************
#if defined(_CORE_TESS_EVALUATION_SHADER_)

    // main function
    void TessEvaluationMain();
    void ShaderMain()
    {
        TessEvaluationMain();
    }

#endif // _CORE_TESS_EVALUATION_SHADER_


// ****************************************************************
#if defined(_CORE_GEOMETRY_SHADER_)

    // shader input
    in b_Varying
    {
        flat vec4 v_v4VarColor;
        vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D];
        vec4 v_av4LightPos[CORE_NUM_LIGHTS];
        vec4 v_av4LightDir[CORE_NUM_LIGHTS];
        vec3 v_v3TangentPos;
        vec3 v_v3TangentCam;
    }
    In[];

    // shader output
    out b_Varying
    {
        flat vec4 v_v4VarColor;
        vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D];
        vec4 v_av4LightPos[CORE_NUM_LIGHTS];
        vec4 v_av4LightDir[CORE_NUM_LIGHTS];
        vec3 v_v3TangentPos;
        vec3 v_v3TangentCam;
    }
    Out;

    // main function
    void GeometryMain();
    void ShaderMain()
    {
        GeometryMain();
    }

#endif // _CORE_GEOMETRY_SHADER_


// ****************************************************************
#if defined(_CORE_FRAGMENT_SHADER_)

    #if (CORE_GL_VERSION >= 130) || (CORE_GL_ES_VERSION >= 300)

        // shader output
        out vec4 o_av4OutColor[CORE_NUM_OUTPUTS];
        #define gl_FragColor (o_av4OutColor[0])

    #endif

    #if (CORE_GL_VERSION >= 140) || (CORE_GL_ES_VERSION >= 310)

        // shader input
        in b_Varying
        {
            flat vec4 v_v4VarColor;
            vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D];
            vec4 v_av4LightPos[CORE_NUM_LIGHTS];
            vec4 v_av4LightDir[CORE_NUM_LIGHTS];
            vec3 v_v3TangentPos;
            vec3 v_v3TangentCam;
        };

    #else

        // shader input
        flat varying vec4 v_v4VarColor;
        varying vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D];
        varying vec4 v_av4LightPos[CORE_NUM_LIGHTS];
        varying vec4 v_av4LightDir[CORE_NUM_LIGHTS];
        varying vec3 v_v3TangentPos;
        varying vec3 v_v3TangentCam;

    #endif

    // pre-calculated view direction
    vec3 v_v3ViewDir;

    // remapped variables
    #if defined(_CORE_OPTION_INSTANCING_)
        #define u_v4Color (v_v4VarColor)
    #endif
    #define u_v2TexSize   (u_v4TexParam.xy)
    #define u_v2TexOffset (u_v4TexParam.zw)

    // main function
    void FragmentMain();
    void ShaderMain()
    {
        v_v3ViewDir = v_v3TangentCam - v_v3TangentPos;
        FragmentMain();
    }

#endif // _CORE_FRAGMENT_SHADER_


// ****************************************************************
#if defined(_CORE_COMPUTE_SHADER_)

    // main function
    void ComputeMain();
    void ShaderMain()
    {
        ComputeMain();
    }

#endif // _CORE_COMPUTE_SHADER_


// ****************************************************************
#if defined(_CORE_VERTEX_SHADER_)

    // default model transformation
    vec3 coreObject3DTransform(const in vec3 v3Vector)
    {
    #if defined(_CORE_OPTION_NO_ROTATION_)
        return v3Vector * u_v3Size + u_v3Position;
    #else
        return coreQuatApply(u_v4Rotation, v3Vector * u_v3Size) + u_v3Position;
    #endif
    }
    vec3 coreObject3DTransformRaw() {return coreObject3DTransform(a_v3RawPosition);}
    vec3 coreObject3DTransformLow() {return coreObject3DTransform(vec3(a_v2LowPosition, 0.0));}

    // default particle range calculation
    vec3 coreParticleRange()
    {
    #if defined(_CORE_OPTION_NO_ROTATION_)
        return coreTranspose(coreMat4to3(u_m4Camera)) * vec3(a_v2LowPosition * a_v1DivScale, 0.0);
    #else
        float v1Sin = sin(a_v1DivAngle);
        float v1Cos = cos(a_v1DivAngle);
        return coreTranspose(coreMat4to3(u_m4Camera)) * vec3(mat2(v1Cos, v1Sin, -v1Sin, v1Cos) * (a_v2LowPosition * a_v1DivScale), 0.0);
    #endif
    }

    // default position transformation
    vec4 coreObject3DPositionRaw() {return u_m4ViewProj * vec4(coreObject3DTransformRaw(), 1.0);}
    vec4 coreObject3DPositionLow() {return u_m4ViewProj * vec4(coreObject3DTransformLow(), 1.0);}
    vec4 coreObject2DPosition()    {return vec4((u_m3ScreenView * vec3(a_v2LowPosition, 1.0)).xy, 1.0, 1.0);}
    vec4 coreParticlePosition()    {return u_m4ViewProj * vec4(coreParticleRange() + a_v3DivPosition, 1.0);}

    // default texture coordinate transformation
    vec2 coreObject3DTexCoordRaw() {return a_v2RawTexCoord * u_v2TexSize + u_v2TexOffset;}
    vec2 coreObject3DTexCoordLow() {return a_v2LowTexCoord * u_v2TexSize + u_v2TexOffset;}
    vec2 coreObject2DTexCoord()    {return a_v2LowTexCoord * u_v2TexSize + u_v2TexOffset;}
    vec2 coreParticleTexCoord()    {return a_v2LowTexCoord;}

    // calculate tangent-space matrix
    mat3 coreTangentSpaceMatrix(const in vec4 v4Rotation, const in vec3 v3Normal, const in vec4 v4Tangent)
    {
    #if defined(_CORE_OPTION_NO_ROTATION_)
        vec3 N = v3Normal;
        vec3 T = v4Tangent.xyz;
    #else
        vec3 N = coreQuatApply(v4Rotation, v3Normal);
        vec3 T = coreQuatApply(v4Rotation, v4Tangent.xyz);
    #endif
        vec3 B = cross(N, T) * v4Tangent.w;
        return coreTranspose(mat3(T, B, N));
    }
    mat3 coreTangentSpaceMatrix() {return coreTangentSpaceMatrix(u_v4Rotation, a_v3RawNormal, a_v4RawTangent);}

    // transform lighting properties into tangent-space
    void coreLightingTransform(const in vec3 v3Position)
    {
        mat3 TBN = coreTangentSpaceMatrix();
        for(int i = 0; i < CORE_NUM_LIGHTS; ++i)
        {
            v_av4LightPos[i] = vec4(TBN *  u_av4LightPos[i].xyz, u_av4LightPos[i].w);
            v_av4LightDir[i] = vec4(TBN * -u_av4LightDir[i].xyz, u_av4LightDir[i].w);
        }
        v_v3TangentPos = TBN * v3Position;
        v_v3TangentCam = TBN * u_v3CamPosition;
    }
    void coreLightingTransformRaw() {coreLightingTransform(coreObject3DTransformRaw());}
    void coreLightingTransformLow() {coreLightingTransform(coreObject3DTransformLow());}

#endif // _CORE_VERTEX_SHADER_

#if defined(_CORE_FRAGMENT_SHADER_)

    // GGX specular function
    float coreGGX(const in float v1Dot, const in float v1Rough)
    {
        float v1DotSq   = v1Dot   * v1Dot;
        float v1RoughSq = v1Rough * v1Rough;
        float v1Value   = 1.0 + v1DotSq * (v1RoughSq - 1.0);
        return v1RoughSq / (PI * v1Value * v1Value);
    }

#endif // _CORE_FRAGMENT_SHADER_

// recommended texture lookup
#if (CORE_GL_VERSION >= 130) || (CORE_GL_ES_VERSION >= 300)
    #define coreTexture2D(u,c)         (texture       (u_as2Texture2D    [u], c))
    #define coreTextureProj(u,c)       (textureProj   (u_as2Texture2D    [u], c))
    #define coreTextureShadow(u,c)     (textureProj   (u_as2TextureShadow[u], c))
    #define coreTextureBase2D(u,c)     (textureLod    (u_as2Texture2D    [u], c, 0.0))
    #define coreTextureBaseProj(u,c)   (textureProjLod(u_as2Texture2D    [u], c, 0.0))
    #define coreTextureBaseShadow(u,c) (textureProjLod(u_as2TextureShadow[u], c, 0.0))
#else
    #define coreTexture2D(u,c)         (texture2D     (u_as2Texture2D    [u], c))
    #define coreTextureProj(u,c)       (texture2DProj (u_as2Texture2D    [u], c))
    #define coreTextureShadow(u,c)     (shadow2DProj  (u_as2TextureShadow[u], c).r)
    #define coreTextureBase2D(u,c)     (coreTexture2D    (u, c))
    #define coreTextureBaseProj(u,c)   (coreTextureProj  (u, c))
    #define coreTextureBaseShadow(u,c) (coreTextureShadow(u, c))
#endif


#line 1