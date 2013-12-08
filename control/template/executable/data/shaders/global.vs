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
#define PI (3.1415926535897932384626433832795)   // Archimedes' constant
#define EU (2.7182818284590452353602874713527)   // Euler's number
#define P3 (0.3333333333333333333333333333333)   // periodic value of 1.0/3.0
#define P6 (0.6666666666666666666666666666667)   // periodic value of 2.0/3.0


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

    // vertex attributes
    in vec3 a_v3Position;
    in vec2 a_v2Texture;
    in vec3 a_v3Normal;
    in vec4 a_v4Tangent;

    // shader output
    out vec2 v_av2TexCoord[CORE_TEXTURE_UNITS];
    out vec4 v_av4LightDir[CORE_GRAPHICS_LIGHTS];
    out vec3 v_v3ViewDir;

#else

    // ambient uniforms
    uniform coreLight u_asLight[CORE_GRAPHICS_LIGHTS];

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


// 3d-object uniforms
uniform mat4 u_m4ModelView;       // camera * model
uniform mat4 u_m4ModelViewProj;   // projection * camera * model
uniform mat3 u_m3Normal;

// 2d-object uniforms
uniform mat3 u_m3ScreenView;

// default object uniforms
uniform vec4 u_v4Color;
uniform vec2 u_v2TexSize;
uniform vec2 u_v2TexOffset;
