///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "util_base.glsl"


#if defined(_CORE_FRAGMENT_SHADER_)

    #if defined(CORE_GL_shader_io_blocks)

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

    #if defined(CORE_GL_gpu_shader4)

        // shader output
        out vec4 o_av4OutColor[CORE_NUM_OUTPUTS];
        #define gl_FragColor (o_av4OutColor[0])

    #elif defined(CORE_GL_draw_buffers)

        // shader output
        #define o_av4OutColor (gl_FragData)

    #endif

    // pre-calculated view direction
    #if defined(GL_ES)
        #define v_v3ViewDir (v_v3TangentPos)
    #else
        #define v_v3ViewDir (v_v3TangentCam - v_v3TangentPos)
    #endif

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
        FragmentMain();
    }

    // GGX specular function
    float coreGGX(const in float v1Dot, const in float v1Rough)
    {
        float v1DotSq   = v1Dot   * v1Dot;
        float v1RoughSq = v1Rough * v1Rough;
        float v1Value   = 1.0 + v1DotSq * (v1RoughSq - 1.0);

        return v1RoughSq / (PI * v1Value * v1Value);
    }

    // ordered dithering function (modified)
    float coreDither(const in ivec2 i2PixelCoord)
    {
        mat4 c_m4Matrix = mat4( 0.0,  8.0,  2.0, 10.0,   // # Intel hotfix: do not declare as const
                               12.0,  4.0, 14.0,  6.0,
                                3.0, 11.0,  1.0,  9.0,
                               15.0,  7.0, 13.0,  5.0) / 15.0 - 0.5;

        ivec2 i2Index = coreIntMod(i2PixelCoord, 4);
        #if defined(GL_ES)
            for(int i = 0; i < 4; ++i) for(int j = 0; j < 4; ++j) if((i == i2Index.y) && (j == i2Index.x)) return c_m4Matrix[i][j];
        #else
            return c_m4Matrix[i2Index.y][i2Index.x];
        #endif
    }
    float coreDither() {return coreDither(ivec2(gl_FragCoord.xy));}

#endif