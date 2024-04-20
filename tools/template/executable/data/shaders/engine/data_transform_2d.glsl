///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////


#if defined(CORE_GL_uniform_buffer_object)

    // transformation uniforms
    layout(std140) uniform b_Transform2D
    {
        layoutEx(align = 16) highp mat4 u_m4Ortho;
        layoutEx(align = 16) highp vec4 u_v4Resolution;
    };

#else

    // transformation uniforms
    uniform highp mat4 u_m4Ortho;
    uniform highp vec4 u_v4Resolution;

#endif

#if defined(_CORE_VERTEX_SHADER_)

    // default position transformation
    vec4 coreObject2DPosition() {return vec4((u_m3ScreenView * vec3(a_v2LowPosition, 1.0)).xy, 1.0, 1.0);}

    // default texture coordinate transformation
    vec2 coreObject2DTexCoord() {return a_v2LowTexCoord * u_v2TexSize + u_v2TexOffset;}

#endif // _CORE_VERTEX_SHADER_