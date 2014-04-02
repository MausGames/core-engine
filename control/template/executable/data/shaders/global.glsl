//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
// #version
// #define CORE_TEXTURE_UNITS
// #define CORE_GRAPHICS_LIGHTS
// #define CORE_SHADER_OUTPUT_COLORS


// global definitions
#define PI (3.1415926535897932384626433832795)
#define EU (2.7182818284590452353602874713527)
#define P3 (0.3333333333333333333333333333333)
#define P6 (0.6666666666666666666666666666667)


// light structure
struct coreLight
{
    vec4 v4Position;
    vec4 v4Direction;
    vec4 v4Value;
};

// vertex structure
struct coreVertex
{
    vec2 v_av2TexCoord[CORE_TEXTURE_UNITS];
    vec4 v_av4LightDir[CORE_GRAPHICS_LIGHTS];
    vec3 v_v3ViewDir;
};


#if (__VERSION__) >= 140   // >= OpenGL 3.1

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


#if (__VERSION__) >= 400   // >= OpenGL 4.0

    // unpacking function
    #define coreUnpackUnorm4x8(x) unpackUnorm4x8(x)
                                   
#else

    // unpacking function (without bit operators, red color with double-mod)
    #define coreUnpackUnorm4x8(x) (vec4(float(mod((x) - 16777216*((x) / 16777216), 256)), \
                                        float(mod((x) / 256,                       256)), \
                                        float(mod((x) / 65536,                     256)), \
                                        float(mod((x) / 16777216,                  256)))*0.003921569);
                                    
#endif


#line 1