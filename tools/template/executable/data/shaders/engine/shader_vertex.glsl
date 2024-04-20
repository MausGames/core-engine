///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////


#if defined(_CORE_VERTEX_SHADER_)

    // vertex attributes
    attribute vec3 a_v3RawPosition;
    attribute vec2 a_v2RawTexCoord;
    attribute vec3 a_v3RawNormal;
    attribute vec4 a_v4RawTangent;

    // instancing attributes
    attribute vec3 a_v3DivPosition;
    attribute vec3 a_v3DivSize;
    attribute vec4 a_v4DivRotation;
    attribute vec3 a_v3DivData;
    attribute vec4 a_v4DivColor;
    attribute vec4 a_v4DivTexParam;

    // instancing uniforms
    uniform highp   vec3 u_v3DivPosition;
    uniform mediump vec3 u_v3DivData;

    #if defined(CORE_GL_shader_io_blocks)

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
        #define u_v3Position    (a_v3DivPosition)
        #define u_v3Size        (a_v3DivSize)
        #define u_v4Rotation    (a_v4DivRotation)
        #define u_v4Color       (a_v4DivColor)
        #define u_v2TexSize     (a_v4DivTexParam.xy)
        #define u_v2TexOffset   (a_v4DivTexParam.zw)
    #else
        #define a_v3DivPosition (u_v3DivPosition)
        #define a_v3DivData     (u_v3DivData)
        #define u_v2TexSize     (u_v4TexParam.xy)
        #define u_v2TexOffset   (u_v4TexParam.zw)
    #endif
    #define a_v1DivScale (a_v3DivData.x)
    #define a_v1DivAngle (a_v3DivData.y)
    #define a_v1DivValue (a_v3DivData.z)

    // main function
    void VertexMain();
    void ShaderMain()
    {
    #if defined(GL_ES)
        #define CORE_OVERFLOW_GUARD(i, n) (i)
    #else
        #define CORE_OVERFLOW_GUARD(i, n) (min(i, (n) - 1))   // # Intel hotfix: prevent compiler-generated overflow
    #endif

        // compatibility for Intel and macOS
        v_v4VarColor   = vec4(0.0);
        for(int i = 0; i < CORE_NUM_TEXTURES_2D; ++i) v_av2TexCoord[CORE_OVERFLOW_GUARD(i, CORE_NUM_TEXTURES_2D)] = vec2(0.0);
        for(int i = 0; i < CORE_NUM_LIGHTS;      ++i) v_av4LightPos[CORE_OVERFLOW_GUARD(i, CORE_NUM_LIGHTS)]      = vec4(0.0);
        for(int i = 0; i < CORE_NUM_LIGHTS;      ++i) v_av4LightDir[CORE_OVERFLOW_GUARD(i, CORE_NUM_LIGHTS)]      = vec4(0.0);
        v_v3TangentPos = vec3(0.0);
        v_v3TangentCam = vec3(0.0);

    #if defined(_CORE_OPTION_INSTANCING_)
        v_v4VarColor = a_v4DivColor;
    #endif

        a_v2LowPosition = a_v3RawPosition.xy;
        a_v2LowTexCoord = vec2(0.5 + a_v3RawPosition.x, 0.5 - a_v3RawPosition.y);

        VertexMain();

    #if defined(GL_ES)
        v_v3TangentPos = v_v3TangentCam - v_v3TangentPos;
    #endif
    }

#endif // _CORE_VERTEX_SHADER_