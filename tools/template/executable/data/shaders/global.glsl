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
    #extension GL_EXT_shadow_samplers : enable
#else
    #extension GL_AMD_shader_trinary_minmax : enable
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
#if defined(_CORE_FRAGMENT_SHADER_)
    #if (__VERSION__) >= 420 || (defined(GL_ES) && (__VERSION__) >= 310)
        layout(early_fragment_tests) in;
    #endif
#endif
#if (__VERSION__) >= 440
    #define std140 std140, align = 16
#endif

// compatibility adjustments
#if defined(GL_ES) && (__VERSION__) < 300
    #if defined(GL_EXT_shadow_samplers)
        #define shadow2DProj(t,v) (shadow2DProjEXT(t, v))
    #else
        #define sampler2DShadow   sampler2D
        #define shadow2DProj(t,v) ((texture2DProj(t, v).r < (v.z/v.w)) ? 1.0 : 0.0)
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


// ****************************************************************
#define PI (3.1415926535897932384626433832795)
#define EU (2.7182818284590452353602874713527)

// light structure
struct coreLight
{
    vec4 v4Position;
    vec4 v4Direction;
    vec4 v4Value;
};

// trinary min and max
#if defined(GL_AMD_shader_trinary_minmax)
    #define coreMin3(a,b,c) (min3(a, b, c))
    #define coreMax3(a,b,c) (max3(a, b, c))
#else
    #define coreMin3(a,b,c) (min(a, min(b, c)))
    #define coreMax3(a,b,c) (max(a, max(b, c)))
#endif

// color convert
vec3 coreHSVtoRGB(const in vec3 v3HSV)
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
vec3 coreRGBtoHSV(const in vec3 v3RGB)
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

// vector square length
float coreLengthSq(const in vec2 v) {return dot(v, v);}
float coreLengthSq(const in vec3 v) {return dot(v, v);}

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
#if (__VERSION__) >= 140
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
#if (__VERSION__) >= 140
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
    #define coreToMat3(m) mat3(m)
#else
    #define coreToMat3(m) mat3(m[0].xyz, m[1].xyz, m[2].xyz)
#endif

// value pack and unpack
#if (__VERSION__) >= 130

    uint corePackUnorm4x8(const in vec4 x)
    {
    #if (__VERSION__) >= 400
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
    #if (__VERSION__) >= 400
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
        mat4 u_m4ViewProj;
        mat4 u_m4Camera;
        mat4 u_m4Perspective;
        mat4 u_m4Ortho;
        vec4 u_v4Resolution;
    };

    // ambient uniforms
    layout(std140) uniform b_Ambient
    {
        coreLight u_aLight[CORE_NUM_LIGHTS];
    };

#else

    // transformation uniforms
    uniform mat4 u_m4ViewProj;
    uniform mat4 u_m4Camera;
    uniform mat4 u_m4Perspective;
    uniform mat4 u_m4Ortho;
    uniform vec4 u_v4Resolution;

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
            vec4 v_v4VarColor;
            vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D];
            vec4 v_av4LightDir[CORE_NUM_LIGHTS];
            vec3 v_v3ViewDir;
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
        varying vec4 v_v4VarColor;
        varying vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D];
        varying vec4 v_av4LightDir[CORE_NUM_LIGHTS];
        varying vec3 v_v3ViewDir;

    #endif
    
    // low-memory attributes
    vec2 a_v2LowPosition = a_v3RawPosition.xy;
    vec2 a_v2LowTexCoord = vec2(0.5+a_v3RawPosition.x, 0.5-a_v3RawPosition.y);
    
    // remapped variables
    #if defined(_CORE_OPTION_INSTANCING_)
        #define u_v3Position  (a_v3DivPosition)
        #define u_v3Size      (a_v3DivSize)
        #define u_v4Rotation  (a_v4DivRotation)
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
        vec4 v_v4VarColor;
        vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D];
        vec4 v_av4LightDir[CORE_NUM_LIGHTS];
        vec3 v_v3ViewDir;
    } In[];

    // shader output
    out b_Varying
    {
        vec4 v_v4VarColor;
        vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D];
        vec4 v_av4LightDir[CORE_NUM_LIGHTS];
        vec3 v_v3ViewDir;
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
            vec4 v_v4VarColor;
            vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D];
            vec4 v_av4LightDir[CORE_NUM_LIGHTS];
            vec3 v_v3ViewDir;
        };

        // shader output
        out vec4 o_av4OutColor[CORE_NUM_OUTPUTS];

    #else

        // shader input
        varying vec4 v_v4VarColor;
        varying vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D];
        varying vec4 v_av4LightDir[CORE_NUM_LIGHTS];
        varying vec3 v_v3ViewDir;

    #endif
    
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
        FragmentMain();
    }

#endif // _CORE_FRAGMENT_SHADER_


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
        return coreTranspose(coreToMat3(u_m4Camera)) * vec3(a_v2LowPosition * a_v1DivScale, 0.0);
    #else
        float v1Sin = sin(a_v1DivAngle);
        float v1Cos = cos(a_v1DivAngle);
        return coreTranspose(coreToMat3(u_m4Camera)) * vec3(mat2(v1Cos, v1Sin, -v1Sin, v1Cos) * (a_v2LowPosition * a_v1DivScale), 0.0);
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

    // recommended color value access
    vec4 coreObject3DColor()
    {
    #if defined(_CORE_OPTION_INSTANCING_)
        return v_v4VarColor;
    #else
        return u_v4Color;
    #endif
    }
    #define coreObject2DColor() (u_v4Color)
    #define coreParticleColor() (coreObject3DColor())

    // dot-3 bump mapping initialization and transformation
    vec3 g_n, g_t, g_b;
    void coreDot3VertexInit(const in vec4 v4Rotation, const in vec3 v3Normal, const in vec4 v4Tangent)
    { 
    #if defined(_CORE_OPTION_NO_ROTATION_)
        g_n = v3Normal;
        g_t = v4Tangent.xyz;
    #else
        g_n = coreQuatApply(v4Rotation, v3Normal);
        g_t = coreQuatApply(v4Rotation, v4Tangent.xyz);
    #endif
        g_b = cross(g_n, g_t) * v4Tangent.w;
    }
    void coreDot3VertexInit()                            {coreDot3VertexInit(u_v4Rotation, a_v3RawNormal, a_v4RawTangent);}
    vec3 coreDot3VertexTransform(const in vec3 v3Vector) {return normalize(vec3(dot(v3Vector, g_t), dot(v3Vector, g_b), dot(v3Vector, g_n)));}
    
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