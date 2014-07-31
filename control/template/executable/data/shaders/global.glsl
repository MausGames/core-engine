//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////


// ****************************************************************
// #version                  (#)   // shader version
// #define _CORE_*_SHADER_   (1)   // shader type (vertex, fragment, ...)
// #define _CORE_OPTION_*_   (1)   // multiple preprocessor options
// #define _CORE_QUALITY_    (#)   // quality level
// #define CORE_NUM_TEXTURES (#)   // number of texture units
// #define CORE_NUM_LIGHTS   (#)   // number of light sources
// #define CORE_NUM_OUTPUTS  (#)   // number of output colors

// extensions
#extension AMD_shader_trinary_minmax : enable

// precision qualifier
#ifdef GL_ES
    #ifdef _CORE_FRAGMENT_SHADER_
        precision mediump float;
    #endif
#endif

// light structure
struct coreLight
{
    vec4 v4Position;
    vec4 v4Direction;
    vec4 v4Value;
};


// ****************************************************************
#if (__VERSION__) >= 140 // >= OpenGL 3.1

    layout(std140) uniform b_Global
    {
        // transformation uniforms
        mat4 u_m4ViewProj;
        mat4 u_m4Camera;
        mat4 u_m4Perspective;
        mat4 u_m4Ortho;
        vec4 u_v4Resolution;

        // ambient uniforms
        coreLight u_asLight[CORE_NUM_LIGHTS];
    };

#else

    // transformation uniforms
    uniform mat4 u_m4ViewProj;
    uniform mat4 u_m4Camera;
    uniform mat4 u_m4Perspective;
    uniform mat4 u_m4Ortho;
    uniform vec4 u_v4Resolution;

    // ambient uniforms
    uniform coreLight u_asLight[CORE_NUM_LIGHTS];

#endif

// 3d-object uniforms
uniform mat4 u_m4ModelView;
uniform mat4 u_m4ModelViewProj;
uniform mat3 u_m3Normal;

// 2d-object uniforms
uniform mat3 u_m3ScreenView;

// default object uniforms
uniform vec4 u_v4Color;
uniform vec4 u_v4TexParam;

// texture uniforms
uniform sampler2D u_as2Texture[CORE_NUM_TEXTURES];

// remapped variables
vec2 u_v2TexSize   = u_v4TexParam.xy;
vec2 u_v2TexOffset = u_v4TexParam.zw;


// ****************************************************************
#ifdef _CORE_VERTEX_SHADER_

    #if (__VERSION__) >= 140 // >= OpenGL 3.1

        // vertex attributes
        in vec3 a_v3Position;
        in vec2 a_v2Texture;
        in vec3 a_v3Normal;
        in vec4 a_v4Tangent;
        
        // instancing attributes
        in mat4 a_m4DivModelView;
        in vec3 a_v3DivPosition;
        in vec3 a_v3DivData;
        in uint a_iDivColor;
        in vec4 a_v4DivTexParam;

        // shader output
        out b_Varying
        {
            vec4 v_v4VarColor;
            vec2 v_av2TexCoord[CORE_NUM_TEXTURES];
            vec4 v_av4LightDir[CORE_NUM_LIGHTS];
            vec3 v_v3ViewDir;
        };

    #else

        // vertex attributes
        attribute vec3 a_v3Position;
        attribute vec2 a_v2Texture;
        attribute vec3 a_v3Normal;
        attribute vec4 a_v4Tangent;
        
        // instancing uniforms (used like attributes)
        uniform mat4 a_m4DivModelView;
        uniform vec3 a_v3DivPosition;
        uniform vec3 a_v3DivData;
        uniform vec4 a_iDivColor;
        uniform vec4 a_v4DivTexParam;

        // shader output
        varying vec4 v_v4VarColor;
        varying vec2 v_av2TexCoord[CORE_NUM_TEXTURES];
        varying vec4 v_av4LightDir[CORE_NUM_LIGHTS];
        varying vec3 v_v3ViewDir;

    #endif
    
    // remapped variables
    float a_fDivScale      = a_v3DivData.x;
    float a_fDivAngle      = a_v3DivData.y;
    float a_fDivValue      = a_v3DivData.z;
    vec2  a_v2DivTexSize   = a_v4DivTexParam.xy;
    vec2  a_v2DivTexOffset = a_v4DivTexParam.zw;

#endif // _CORE_VERTEX_SHADER_


// ****************************************************************
#ifdef _CORE_TESS_CONTROL_SHADER_

#endif // _CORE_TESS_CONTROL_SHADER_


// ****************************************************************
#ifdef _CORE_TESS_EVALUATION_SHADER_

#endif // _CORE_TESS_EVALUATION_SHADER_


// ****************************************************************
#ifdef _CORE_GEOMETRY_SHADER_

    // shader input
    in b_Varying
    {
        vec4 v_v4VarColor;
        vec2 v_av2TexCoord[CORE_NUM_TEXTURES];
        vec4 v_av4LightDir[CORE_NUM_LIGHTS];
        vec3 v_v3ViewDir;
    } In[];

    // shader output
    out b_Varying
    {
        vec4 v_v4VarColor;
        vec2 v_av2TexCoord[CORE_NUM_TEXTURES];
        vec4 v_av4LightDir[CORE_NUM_LIGHTS];
        vec3 v_v3ViewDir;
    } Out;

#endif // _CORE_GEOMETRY_SHADER_


// ****************************************************************
#ifdef _CORE_FRAGMENT_SHADER_

    #if (__VERSION__) >= 140 // >= OpenGL 3.1

        // shader input
        in b_Varying
        {
            vec4 v_v4VarColor;
            vec2 v_av2TexCoord[CORE_NUM_TEXTURES];
            vec4 v_av4LightDir[CORE_NUM_LIGHTS];
            vec3 v_v3ViewDir;
        };

        // shader output
        out vec4 o_av4OutColor[CORE_NUM_OUTPUTS];

    #else

        // shader input
        varying vec4 v_v4VarColor;
        varying vec2 v_av2TexCoord[CORE_NUM_TEXTURES];
        varying vec4 v_av4LightDir[CORE_NUM_LIGHTS];
        varying vec3 v_v3ViewDir;

    #endif

#endif // _CORE_FRAGMENT_SHADER_


// ****************************************************************
#define PI (3.1415926535897932384626433832795)
#define EU (2.7182818284590452353602874713527)

// trinary min and max functions
#ifdef GL_AMD_shader_trinary_minmax
    #define coreMin3(a,b,c) (min3(a, b, c))
    #define coreMax3(a,b,c) (max3(a, b, c))
#else
    #define coreMin3(a,b,c) (min(a, min(b, c)))
    #define coreMax3(a,b,c) (max(a, max(b, c)))
#endif

// square length functions
float coreLengthSq(in vec2 v) {return dot(v, v);}
float coreLengthSq(in vec3 v) {return dot(v, v);}

// color convert functions
vec3 coreHSVtoRGB(in vec3 v3HSV)
{
    float H = v3HSV.x * 6.0;
    float S = v3HSV.y;
    float V = v3HSV.z;

    float h = floor(H);
    float f = H - h;

    float VS = V  * S;
    float VR = VS * f;

    float p = V - VS;
    float q = V - VR;
    float t = p + VR;

    if(h == 1.0) return vec3(q, V, p);
    if(h == 2.0) return vec3(p, V, t);
    if(h == 3.0) return vec3(p, q, V);
    if(h == 4.0) return vec3(t, p, V);
    if(h == 5.0) return vec3(V, p, q);
                 return vec3(V, t, p);
}
vec3 coreRGBtoHSV(in vec3 v3RGB)
{
    float R = v3RGB.r;
    float G = v3RGB.g;
    float B = v3RGB.b;

    float v = coreMax3(R, G, B);
    float d = v - coreMin3(R, G, B);

    if(d == 0.0) return vec3(0.0, 0.0, v);

         if(R == v) return vec3((0.0 + (G - B) / d) / 6.0, d / v, v);
    else if(G == v) return vec3((2.0 + (B - R) / d) / 6.0, d / v, v);
               else return vec3((4.0 + (R - G) / d) / 6.0, d / v, v);
}

// pack and unpacking functions
#if (__VERSION__) >= 400 // >= OpenGL 4.0

    vec4 coreUnpackUnorm4x8(in uint x) {return unpackUnorm4x8(x);}
                                   
#elif (__VERSION__) >= 130 // >= OpenGL 3.0

    vec4 coreUnpackUnorm4x8(in uint x)
    {
        return vec4(float( x        & 0xFFu),
                    float((x >>  8) & 0xFFu),
                    float((x >> 16) & 0xFFu),
                    float((x >> 24) & 0xFFu)) * 0.003921569;
    }
    
#else
    #define coreUnpackUnorm4x8(x) (x)
#endif


#line 1