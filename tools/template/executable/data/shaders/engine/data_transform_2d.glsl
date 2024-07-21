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

    // default model transformation
    vec2 coreObject2DTransform(const in vec2 v2Vector)
    {
    #if defined(_CORE_OPTION_NO_ROTATION_)
        return v2Vector * u_v2TwoSize + u_v3TwoPosition.xy;
    #else
        return mat2(u_v2TwoRotation.y, -u_v2TwoRotation.x, u_v2TwoRotation.x, u_v2TwoRotation.y) * (v2Vector * u_v2TwoSize) + u_v3TwoPosition.xy;
    #endif
    }
    vec2 coreObject2DTransform() {return coreObject2DTransform(a_v2LowPosition);}

    // default position transformation
    vec4 coreObject2DPosition() {return u_m4Ortho * vec4(coreObject2DTransform(), u_v3TwoPosition.z, 1.0);}

    // default texture coordinate transformation
    vec2 coreObject2DTexCoord() {return a_v2LowTexCoord * u_v2TexSize + u_v2TexOffset;}

#endif