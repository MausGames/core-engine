///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////


#if defined(_CORE_GEOMETRY_SHADER_)

    // shader input
    in b_Varying
    {
        flat vec4 v_v4VarColor;
        vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D];
        vec4 v_av4LightPos[CORE_NUM_LIGHTS];
        vec4 v_av4LightDir[CORE_NUM_LIGHTS];
        vec3 v_v3TangentPos;
        vec3 v_v3TangentCam;
    }
    In[];

    // shader output
    out b_Varying
    {
        flat vec4 v_v4VarColor;
        vec2 v_av2TexCoord[CORE_NUM_TEXTURES_2D];
        vec4 v_av4LightPos[CORE_NUM_LIGHTS];
        vec4 v_av4LightDir[CORE_NUM_LIGHTS];
        vec3 v_v3TangentPos;
        vec3 v_v3TangentCam;
    }
    Out;

    // main function
    void GeometryMain();
    void ShaderMain()
    {
        GeometryMain();
    }

#endif