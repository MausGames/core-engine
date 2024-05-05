///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////


#if defined(CORE_GL_uniform_buffer_object)

    // ambient uniforms
    layout(std140) uniform b_Ambient
    {
        layoutEx(align = 16) highp   vec4 u_av4LightPos  [CORE_NUM_LIGHTS];
        layoutEx(align = 16) mediump vec4 u_av4LightDir  [CORE_NUM_LIGHTS];
        layoutEx(align = 16) mediump vec4 u_av4LightValue[CORE_NUM_LIGHTS];
    };

#else

    // ambient uniforms
    uniform highp   vec4 u_av4LightPos  [CORE_NUM_LIGHTS];
    uniform mediump vec4 u_av4LightDir  [CORE_NUM_LIGHTS];
    uniform mediump vec4 u_av4LightValue[CORE_NUM_LIGHTS];

#endif

#if defined(_CORE_VERTEX_SHADER_)

    // transform lighting properties into tangent-space
    void coreLightingTransform(const in vec3 v3Position)
    {
        mat3 TBN = coreTangentSpaceMatrix();
        for(int i = 0; i < CORE_NUM_LIGHTS; ++i)
        {
            v_av4LightPos[i] = vec4(TBN *  u_av4LightPos[i].xyz, u_av4LightPos[i].w);
            v_av4LightDir[i] = vec4(TBN * -u_av4LightDir[i].xyz, u_av4LightDir[i].w);
        }
        v_v3TangentPos = TBN * v3Position;
        v_v3TangentCam = TBN * u_v3CamPosition;
    }
    void coreLightingTransformRaw() {coreLightingTransform(coreObject3DTransformRaw());}
    void coreLightingTransformLow() {coreLightingTransform(coreObject3DTransformLow());}

#endif