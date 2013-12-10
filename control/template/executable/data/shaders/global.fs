//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
// #version
// #define CORE_TEXTURE_UNITS
// #define CORE_GRAPHICS_LIGHTS


// global definitions
#define PI (3.1415926535897932384626433832795)
#define EU (2.7182818284590452353602874713527)
#define P3 (0.3333333333333333333333333333333)
#define P6 (0.6666666666666666666666666666667)


// precision qualifier
#ifdef GL_ES
precision mediump float;
#endif


// light structure
struct coreLight
{
    vec4 v4Position;
    vec4 v4Direction;

    vec4 v4Value;
};


#if (__VERSION__) >= 140   // >= OpenGL 3.1

    layout(std140) uniform b_Global
    {
        // ambient uniforms
        coreLight u_asLight[CORE_GRAPHICS_LIGHTS];
    };

    // shadow uniforms
    uniform sampler2DShadow u_s2Shadow;

    // shader input
    in vec2 v_av2TexCoord[CORE_TEXTURE_UNITS];
    in vec4 v_av4LightDir[CORE_GRAPHICS_LIGHTS];
    in vec3 v_v3ViewDir;

    // shader output
    out vec4 o_v4Color0;
    out vec4 o_v4Color1;
    out vec4 o_v4Color2;
    out vec4 o_v4Color3;

#else

    // ambient uniforms
    uniform coreLight u_asLight[CORE_GRAPHICS_LIGHTS];

    // shader input
    varying vec2 v_av2TexCoord[CORE_TEXTURE_UNITS];
    varying vec4 v_av4LightDir[CORE_GRAPHICS_LIGHTS];
    varying vec3 v_v3ViewDir;

#endif


// 3d-object uniforms
uniform mat4 u_m4ModelView;
uniform mat4 u_m4ModelViewProj;
uniform mat3 u_m3Normal;

// 2d-object uniforms
uniform mat3 u_m3ScreenView;

// default object uniforms
uniform vec4 u_v4Color;
uniform vec2 u_v2TexSize;
uniform vec2 u_v2TexOffset;

// texture uniforms
uniform sampler2D u_as2Texture[CORE_TEXTURE_UNITS];

