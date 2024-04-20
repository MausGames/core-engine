///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////


// ****************************************************************
// general definitions
#define PI    (3.1415926535897932384626433832795)
#define EU    (2.7182818284590452353602874713527)
#define GR    (1.6180339887498948482045868343656)
#define GA    (2.3999632297286533222315555066336)
#define SQRT2 (1.4142135623730950488016887242097)
#define SQRT3 (1.7320508075688772935274463415059)


// ****************************************************************
// evaluate shader per sample
#if defined(CORE_GL_sample_shading)
    #define CORE_SAMPLE_SHADING {int A = gl_SampleID;}
#else
    #define CORE_SAMPLE_SHADING
#endif


// ****************************************************************
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


// ****************************************************************
// condition across group of shader invocations
#if defined(GL_ARB_shader_group_vote)
    #define coreAnyInvocation(x)  (anyInvocationARB (x))
    #define coreAllInvocations(x) (allInvocationsARB(x))
#elif defined(GL_EXT_shader_group_vote)
    #define coreAnyInvocation(x)  (anyInvocationEXT (x))
    #define coreAllInvocations(x) (allInvocationsEXT(x))
#elif defined(GL_NV_gpu_shader5)
    #define coreAnyInvocation(x)  (anyThreadNV (x))
    #define coreAllInvocations(x) (allThreadsNV(x))
#else
    #define coreAnyInvocation(x)  (x)
    #define coreAllInvocations(x) (x)
#endif


// ****************************************************************
// clamp values between 0.0 and 1.0
#define coreSaturate(x) (clamp(x, 0.0, 1.0))


// ****************************************************************
// linear interpolation between 0.0 and 1.0
float coreLinearStep(const in float e0, const in float e1, const in float v) {return coreSaturate((v - e0) / (e1 - e0));}
vec2  coreLinearStep(const in vec2  e0, const in vec2  e1, const in vec2  v) {return coreSaturate((v - e0) / (e1 - e0));}
vec3  coreLinearStep(const in vec3  e0, const in vec3  e1, const in vec3  v) {return coreSaturate((v - e0) / (e1 - e0));}
vec4  coreLinearStep(const in vec4  e0, const in vec4  e1, const in vec4  v) {return coreSaturate((v - e0) / (e1 - e0));}


// ****************************************************************
// extract the sign without returning 0.0
float coreSign(const in float v) {return (v >= 0.0) ? 1.0 : -1.0;}
#if (CORE_GL_VERSION >= 450) || (CORE_GL_ES_VERSION >= 300)
    vec2 coreSign(const in vec2 v) {return mix(vec2(-1.0), vec2(1.0), greaterThanEqual(v, vec2(0.0)));}
    vec3 coreSign(const in vec3 v) {return mix(vec3(-1.0), vec3(1.0), greaterThanEqual(v, vec3(0.0)));}
    vec4 coreSign(const in vec4 v) {return mix(vec4(-1.0), vec4(1.0), greaterThanEqual(v, vec4(0.0)));}
#else
    vec2 coreSign(const in vec2 v) {return vec2(coreSign(v.x), coreSign(v.y));}
    vec3 coreSign(const in vec3 v) {return vec3(coreSign(v.x), coreSign(v.y), coreSign(v.z));}
    vec4 coreSign(const in vec4 v) {return vec4(coreSign(v.x), coreSign(v.y), coreSign(v.z), coreSign(v.w));}
#endif


// ****************************************************************
// test if the parameter is not a number
#if (CORE_GL_VERSION >= 130) || (CORE_GL_ES_VERSION >= 300)
    #define coreIsNan(x) (isnan(x))
#else
    bool  coreIsNan(const in float v) {return !((v == 0.0) || (v < 0.0) || (v > 0.0));}
    bvec2 coreIsNan(const in vec2  v) {return bvec2(coreIsNan(v.x), coreIsNan(v.y));}
    bvec3 coreIsNan(const in vec3  v) {return bvec3(coreIsNan(v.x), coreIsNan(v.y), coreIsNan(v.z));}
    bvec4 coreIsNan(const in vec4  v) {return bvec4(coreIsNan(v.x), coreIsNan(v.y), coreIsNan(v.z), coreIsNan(v.w));}
#endif


// ****************************************************************
// integer modulo operation
#if defined(CORE_GL_gpu_shader4)
    #define coreIntMod(a,b) ((a) % (b))
#else
    int   coreIntMod(const in int   a, const in int b) {return (a - (b * (a / b)));}
    ivec2 coreIntMod(const in ivec2 a, const in int b) {return (a - (b * (a / b)));}
    ivec3 coreIntMod(const in ivec3 a, const in int b) {return (a - (b * (a / b)));}
    ivec4 coreIntMod(const in ivec4 a, const in int b) {return (a - (b * (a / b)));}
#endif


// ****************************************************************
// vector square length
float coreLengthSq(const in vec2 v) {return dot(v, v);}
float coreLengthSq(const in vec3 v) {return dot(v, v);}


// ****************************************************************
// quaternion transformation
vec3 coreQuatApply(const in vec4 q, const in vec3 v)
{
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}