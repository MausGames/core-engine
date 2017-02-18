//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////


// ****************************************************************
// #version                         (#)   // shader version
// #define _CORE_*_SHADER_          (1)   // shader type (vertex, fragment, ...)
// #define _CORE_OPTION_*_          (1)   // multiple preprocessor options
// #define _CORE_QUALITY_           (#)   // quality level
// #define CORE_NUM_TEXTURES_2D     (#)   // number of 2d texture units
// #define CORE_NUM_TEXTURES_SHADOW (#)   // number of shadow texture units
// #define CORE_NUM_LIGHTS          (#)   // number of light sources
// #define CORE_NUM_OUTPUTS         (#)   // number of output colors

// compiler configuration
#if defined(GL_ES)
    #extension GL_EXT_shadow_samplers          : enable
#else
    #extension GL_AMD_shader_trinary_minmax    : enable
    #extension GL_ARB_conservative_depth       : enable
    #extension GL_ARB_enhanced_layouts         : enable
    #extension GL_ARB_sample_shading           : enable
    #extension GL_ARB_shader_group_vote        : enable
    #extension GL_ARB_shader_image_load_store  : enable
    #extension GL_ARB_shading_language_packing : enable
    #extension GL_ARB_uniform_buffer_object    : enable
    #extension GL_EXT_gpu_shader4              : enable
#endif
#if (__VERSION__) < 130
    #undef GL_ARB_conservative_depth
    #undef GL_ARB_shader_image_load_store
#endif
#pragma optimize(on)
#pragma debug(off)

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
    #if defined(GL_ARB_conservative_depth)
        layout(depth_unchanged) out float gl_FragDepth;
    #endif
    #if defined(GL_ARB_shader_image_load_store) || (defined(GL_ES) && (__VERSION__) >= 310)
        layout(early_fragment_tests) in;
    #endif
#endif
#if defined(GL_ARB_enhanced_layouts)
    #define align(x) layout(align = x)
#else
    #define align(x)
#endif

// compatibility adjustments
#if defined(GL_ES) && (__VERSION__) < 300
    #if defined(GL_EXT_shadow_samplers)
        #define shadow2DProj(t,v) (shadow2DProjEXT(t, v))
    #else
        #define sampler2DShadow   sampler2D
        #define shadow2DProj(t,v) ((texture2DProj(t, v).r < (v.z / v.w)) ? 1.0 : 0.0)
    #endif
#endif
#if (__VERSION__) >= 130
    #if defined(_CORE_VERTEX_SHADER_)
        #define attribute in
        #define varying   out
    #endif
    #if defined(_CORE_FRAGMENT_SHADER_)
        #define varying in
    #endif
#else
    #undef  _CORE_OPTION_INSTANCING_
    #define flat
    #define noperspective
    #define smooth
#endif
#if !defined(GL_EXT_gpu_shader4)
    #define uint  int
    #define uvec2 ivec2
    #define uvec3 ivec3
    #define uvec4 ivec4
#endif


// ****************************************************************
#define PI    (3.1415926535897932384626433832795)
#define EU    (2.7182818284590452353602874713527)
#define SQRT2 (1.4142135623730950488016887242097)

// light structure
struct coreLight
{
    vec4 v4Position;
    vec4 v4Direction;
    vec4 v4Value;
};

// evaluate shader per sample
#if defined(GL_ARB_sample_shading)
    #define CORE_SAMPLE_SHADING {gl_SampleID;}
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
#else
    #define coreAnyInvocation(x)  (x)
    #define coreAllInvocations(x) (x)
#endif

// color convert
vec3 coreRgbToHsv(const in vec3 v3RGB)
{
    float R = v3RGB.r;
    float G = v3RGB.g;
    float B = v3RGB.b;

    float v = coreMax3(R, G, B);
    float d = v - coreMin3(R, G, B);

    if(d == 0.0) return vec3(0.0, 0.0, v);

    float s = d / v;

    if(R == v) return vec3((0.0 + (G - B) / d) / 6.0, s, v);
    if(G == v) return vec3((2.0 + (B - R) / d) / 6.0, s, v);
               return vec3((4.0 + (R - G) / d) / 6.0, s, v);
}
vec3 coreHsvToRgb(const in vec3 v3HSV)
{
    float H = v3HSV.x * 6.0;
    float S = v3HSV.y;
    float V = v3HSV.z;

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
vec3 coreRgbToYiq(const in vec3 v3RGB)
{
    return mat3(0.299,  0.587,  0.114,
                0.596, -0.275, -0.321,
                0.212, -0.523,  0.311) * v3RGB;
}
vec3 coreYiqToRgb(const in vec3 v3YIQ)
{
    return mat3(1.000,  0.956,  0.620,
                1.000, -0.272, -0.647,
                1.000, -1.108,  1.705) * v3YIQ;
}
vec3 coreRgbToYuv(const in vec3 v3RGB)
{
    return mat3( 0.21260,  0.71520,  0.07220,
                -0.09991, -0.33609,  0.43600,
                 0.61500, -0.55861, -0.05639) * v3RGB;
}
vec3 coreYuvToRgb(const in vec3 v3YUV)
{
    return mat3(1.00000,  0.00000,  1.28033,
                1.00000, -0.21482, -0.38059,
                1.00000,  2.12798,  0.00000) * v3YUV;
}
vec3 coreRgbToYcbcr(const in vec3 v3RGB)
{
    return mat3( 0.299000,  0.587000,  0.114000,
                -0.168736, -0.331264,  0.500000,
                 0.500000, -0.418688, -0.081312) * v3RGB + vec3(0.0, 0.5, 0.5);
}
vec3 coreYcbcrToRgb(const in vec3 v3YCbCr)
{
    return mat3(1.00000,  0.00000,  1.40200,
                1.00000, -0.34414, -0.71414,
                1.00000,  1.77200,  0.00000) * (v3YCbCr - vec3(0.0, 0.5, 0.5));
}
float coreLuminance(const in vec3 v3RGB)
{
    return dot(v3RGB, vec3(0.212671, 0.715160, 0.072169));
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

// quaternion transformation
vec3 coreQuatApply(const in vec4 q, const in vec3 v)
{
    return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

// matrix transpose
mat3 coreTranspose(const in mat3 m)
{
#if (__VERSION__) >= 120
    return transpose(m);
#else
    return mat3(m[0][0], m[1][0], m[2][0],
                m[0][1], m[1][1], m[2][1],
                m[0][2], m[1][2], m[2][2]);
#endif
}
mat4 coreTranspose(const in mat4 m)
{
#if (__VERSION__) >= 120
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
#if (__VERSION__) >= 150
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
#if (__VERSION__) >= 150
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
#if (__VERSION__) >= 120
    #define coreMat4to3(m) mat3(m)
    #define coreMat3to2(m) mat2(m)
#else
    #define coreMat4to3(m) mat3(m[0].xyz, m[1].xyz, m[2].xyz)
    #define coreMat3to2(m) mat2(m[0].xy,  m[1].xy)
#endif
#define coreMat4to2(m) coreMat3to2(m)

// value pack and unpack
#if defined(GL_EXT_gpu_shader4)

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

#else
    #define corePackUnorm4x8(x)   (x)
    #define coreUnpackUnorm4x8(x) (x)
#endif


// ****************************************************************
#if (__VERSION__) >= 130

    // transformation uniforms
    layout(std140) uniform b_Transform
    {
        align(16) mat4 u_m4ViewProj;
        align(16) mat4 u_m4Camera;
        align(16) mat4 u_m4Perspective;
        align(16) mat4 u_m4Ortho;
        align(16) vec4 u_v4Resolution;
        align(16) vec3 u_v3CamPosition;
    };

    // ambient uniforms
    layout(std140) uniform b_Ambient
    {
        align(16) coreLight u_aLight[CORE_NUM_LIGHTS];
    };

#else

    // transformation uniforms
    uniform mat4 u_m4ViewProj;
    uniform mat4 u_m4Camera;
    uniform mat4 u_m4Perspective;
    uniform mat4 u_m4Ortho;
    uniform vec4 u_v4Resolution;
    uniform vec3 u_v3CamPosition;

    // ambient uniforms
    uniform coreLight u_aLight[CORE_NUM_LIGHTS];

#endif

// 3d-object uniforms
uniform vec3 u_v3Position;
uniform vec3 u_v3Size;
uniform vec4 u_v4Rotation;

// 2d-object uniforms
uniform mat3 u_m3ScreenView;

// default object uniforms
uniform lowp    vec4 u_v4Color;
uniform mediump vec4 u_v4TexParam;

// texture uniforms
uniform sampler2D       u_as2Texture2D    [CORE_NUM_TEXTURES_2D];
uniform sampler2DShadow u_as2TextureShadow[CORE_NUM_TEXTURES_SHADOW];


// ****************************************************************
#if defined(_CORE_VERTEX_SHADER_)

    #if (__VERSION__) >= 130

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

        // vertex attributes
        attribute vec3 a_v3RawPosition;
        attribute vec2 a_v2RawTexCoord;
        attribute vec3 a_v3RawNormal;
        attribute vec4 a_v4RawTangent;

        // instancing uniforms
        uniform vec3 a_v3DivPosition;
        uniform vec3 a_v3DivData;

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
    void main()
    {
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

#endif // _CORE_TESS_CONTROL_SHADER_


// ****************************************************************
#if defined(_CORE_TESS_EVALUATION_SHADER_)

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
    } In[];

    // shader output
    out b_Varying
    {
        flat vec4 v_v4VarColor;
        vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D];
        vec4 v_av4LightPos[CORE_NUM_LIGHTS];
        vec4 v_av4LightDir[CORE_NUM_LIGHTS];
        vec3 v_v3TangentPos;
        vec3 v_v3TangentCam;
    } Out;

    // main function
    void GeometryMain();
    void main()
    {
        GeometryMain();
    }

#endif // _CORE_GEOMETRY_SHADER_


// ****************************************************************
#if defined(_CORE_FRAGMENT_SHADER_)

    #if (__VERSION__) >= 130

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

        // shader output
        out vec4 o_av4OutColor[CORE_NUM_OUTPUTS];
        #define gl_FragColor o_av4OutColor[0]

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
    void main()
    {
        v_v3ViewDir = v_v3TangentCam - v_v3TangentPos;
        FragmentMain();
    }

#endif // _CORE_FRAGMENT_SHADER_


// ****************************************************************
#if defined(_CORE_COMPUTE_SHADER_)

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
            v_av4LightPos[i] = vec4(TBN *  u_aLight[i].v4Position .xyz, u_aLight[i].v4Position .w);
            v_av4LightDir[i] = vec4(TBN * -u_aLight[i].v4Direction.xyz, u_aLight[i].v4Direction.w);
        }
        v_v3TangentPos = TBN * v3Position;
        v_v3TangentCam = TBN * u_v3CamPosition;
    }

#endif // _CORE_VERTEX_SHADER_

// recommended texture lookup
#if (__VERSION__) >= 130
    #define coreTexture2D(u,c)     (texture    (u_as2Texture2D    [u], c))
    #define coreTextureShadow(u,c) (textureProj(u_as2TextureShadow[u], c))
#else
    #define coreTexture2D(u,c)     (texture2D   (u_as2Texture2D    [u], c))
    #define coreTextureShadow(u,c) (shadow2DProj(u_as2TextureShadow[u], c).r)
#endif


#line 1