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
    attribute vec3 a_v3DivTwoPosition;
    attribute vec2 a_v2DivTwoSize;
    attribute vec2 a_v2DivTwoRotation;
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
            flat   vec4 v_v4VarColor;
            smooth vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D];
            smooth vec4 v_av4LightPos[CORE_NUM_LIGHTS];
            smooth vec4 v_av4LightDir[CORE_NUM_LIGHTS];
            smooth vec3 v_v3TangentPos;
            smooth vec3 v_v3TangentCam;
        };

    #else

        // shader output
        flat   varying vec4 v_v4VarColor;
        smooth varying vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D];
        smooth varying vec4 v_av4LightPos[CORE_NUM_LIGHTS];
        smooth varying vec4 v_av4LightDir[CORE_NUM_LIGHTS];
        smooth varying vec3 v_v3TangentPos;
        smooth varying vec3 v_v3TangentCam;

    #endif

    // low-memory attributes
    vec2 a_v2LowPosition;
    vec2 a_v2LowTexCoord;

    // remapped variables
    #if defined(_CORE_OPTION_INSTANCING_)
        #define u_v3Position    (a_v3DivPosition)
        #define u_v3Size        (a_v3DivSize)
        #define u_v4Rotation    (a_v4DivRotation)
        #define u_v3TwoPosition (a_v3DivTwoPosition)
        #define u_v2TwoSize     (a_v2DivTwoSize)
        #define u_v2TwoRotation (a_v2DivTwoRotation)
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
        // prevent unused output variable errors
        if(false)
        {
            v_v4VarColor   = vec4(0.0);
            for(int i = 0; i < CORE_NUM_TEXTURES_2D; ++i) v_av2TexCoord[i] = vec2(0.0);
            for(int i = 0; i < CORE_NUM_LIGHTS;      ++i) v_av4LightPos[i] = vec4(0.0);
            for(int i = 0; i < CORE_NUM_LIGHTS;      ++i) v_av4LightDir[i] = vec4(0.0);
            v_v3TangentPos = vec3(0.0);
            v_v3TangentCam = vec3(0.0);
        }

    #if defined(_CORE_OPTION_INSTANCING_)

        // forward instancing attribute to fragment shader (# before main function)
        v_v4VarColor = a_v4DivColor;

    #endif

        // set low-memory model attributes
        a_v2LowPosition = a_v3RawPosition.xy;
        a_v2LowTexCoord = vec2(0.5 + a_v3RawPosition.x, 0.5 - a_v3RawPosition.y);

        // execute main function
        VertexMain();

    #if defined(GL_ES)

        // pre-calculate view-direction to reduce shader outputs
        v_v3TangentPos = v_v3TangentCam - v_v3TangentPos;

    #endif
    }

#endif