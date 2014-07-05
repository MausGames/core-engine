//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////


// ****************************************************************
// #version                          #
// #define _CORE_*_SHADER_           1
// #define _CORE_QUALITY_            #
// #define CORE_TEXTURE_UNITS        #
// #define CORE_GRAPHICS_LIGHTS      #
// #define CORE_SHADER_OUTPUT_COLORS #


// global definitions
#define PI (3.1415926535897932384626433832795)
#define EU (2.7182818284590452353602874713527)


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
        coreLight u_asLight[CORE_GRAPHICS_LIGHTS];
    };

#else

    // transformation uniforms
    uniform mat4 u_m4ViewProj;
    uniform mat4 u_m4Camera;
    uniform mat4 u_m4Perspective;
    uniform mat4 u_m4Ortho;
    uniform vec4 u_v4Resolution;

    // ambient uniforms
    uniform coreLight u_asLight[CORE_GRAPHICS_LIGHTS];

#endif

// 3d-object uniforms
uniform mat4 u_m4ModelView;
uniform mat4 u_m4ModelViewProj;
uniform mat3 u_m3Normal;

// 2d-object uniforms
uniform mat4 u_m4ScreenView;

// default object uniforms
uniform vec4 u_v4Color;
uniform vec2 u_v2TexSize;
uniform vec2 u_v2TexOffset;

// texture uniforms
uniform sampler2D u_as2Texture[CORE_TEXTURE_UNITS];


// square length function
float coreLengthSq(in vec2 v) {return dot(v, v);}
float coreLengthSq(in vec3 v) {return dot(v, v);}

// color convert function
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

#if (__VERSION__) >= 400 // >= OpenGL 4.0

    // unpacking function
    vec4 coreUnpackUnorm4x8(in uint x) {return unpackUnorm4x8(x);}
                                   
#elif (__VERSION__) >= 130 // >= OpenGL 3.0

    // unpacking function
    vec4 coreUnpackUnorm4x8(in uint x)
    {
        return vec4(float( x        & 0xFF),
                    float((x >>  8) & 0xFF),
                    float((x >> 16) & 0xFF),
                    float((x >> 24) & 0xFF)) * 0.003921569);
    }
                                    
#endif


// ****************************************************************
#ifdef _CORE_VERTEX_SHADER_

    #if (__VERSION__) >= 140 // >= OpenGL 3.1

        // vertex attributes
        in vec3 a_v3Position;
        in vec2 a_v2Texture;
        in vec3 a_v3Normal;
        in vec4 a_v4Tangent;

        // shader output
        out b_Varying
        {
            vec2 v_av2TexCoord[CORE_TEXTURE_UNITS];
            vec4 v_av4LightDir[CORE_GRAPHICS_LIGHTS];
            vec3 v_v3ViewDir;
        };

    #else

        // vertex attributes
        attribute vec3 a_v3Position;
        attribute vec2 a_v2Texture;
        attribute vec3 a_v3Normal;
        attribute vec4 a_v4Tangent;

        // shader output
        varying vec2 v_av2TexCoord[CORE_TEXTURE_UNITS];
        varying vec4 v_av4LightDir[CORE_GRAPHICS_LIGHTS];
        varying vec3 v_v3ViewDir;

    #endif

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
        vec2 v_av2TexCoord[CORE_TEXTURE_UNITS];
        vec4 v_av4LightDir[CORE_GRAPHICS_LIGHTS];
        vec3 v_v3ViewDir;
    } In[];

    // shader output
    out b_Varying
    {
        vec2 v_av2TexCoord[CORE_TEXTURE_UNITS];
        vec4 v_av4LightDir[CORE_GRAPHICS_LIGHTS];
        vec3 v_v3ViewDir;
    } Out;

#endif // _CORE_GEOMETRY_SHADER_


// ****************************************************************
#ifdef _CORE_FRAGMENT_SHADER_

    #if (__VERSION__) >= 140 // >= OpenGL 3.1

        // shader input
        in b_Varying
        {
            vec2 v_av2TexCoord[CORE_TEXTURE_UNITS];
            vec4 v_av4LightDir[CORE_GRAPHICS_LIGHTS];
            vec3 v_v3ViewDir;
        };

        // shader output
        out vec4 o_av4Color[CORE_SHADER_OUTPUT_COLORS];

    #else

        // shader input
        varying vec2 v_av2TexCoord[CORE_TEXTURE_UNITS];
        varying vec4 v_av4LightDir[CORE_GRAPHICS_LIGHTS];
        varying vec3 v_v3ViewDir;

    #endif

#endif // _CORE_FRAGMENT_SHADER_


#line 1