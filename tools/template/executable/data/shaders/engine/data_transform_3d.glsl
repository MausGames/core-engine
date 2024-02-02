///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "util_base.glsl"
#include "util_matrix.glsl"


#if (CORE_GL_VERSION >= 130) || (CORE_GL_ES_VERSION >= 300)

    // transformation uniforms
    layout(std140) uniform b_Transform3D
    {
        layoutEx(align = 16) highp mat4 u_m4ViewProj;
        layoutEx(align = 16) highp mat4 u_m4Camera;
        layoutEx(align = 16) highp mat4 u_m4Perspective;
        layoutEx(align = 16) highp vec3 u_v3CamPosition;
    };

#else

    // transformation uniforms
    uniform highp mat4 u_m4ViewProj;
    uniform highp mat4 u_m4Camera;
    uniform highp mat4 u_m4Perspective;
    uniform highp vec3 u_v3CamPosition;

#endif

#if defined(_CORE_VERTEX_SHADER_)

    // default model transformation
    vec3 coreObject3DTransform(const in vec3 v3Vector)
    {
    #if defined(_CORE_OPTION_NO_ROTATION_)
        return v3Vector * u_v3Size + u_v3Position;
    #else
        return coreQuatApply(u_v4Rotation, v3Vector * u_v3Size) + u_v3Position;
    #endif
    }
    vec3 coreObject3DTransformRaw() {return coreObject3DTransform(a_v3RawPosition);}
    vec3 coreObject3DTransformLow() {return coreObject3DTransform(vec3(a_v2LowPosition, 0.0));}

    // default particle range calculation
    vec3 coreParticleRange()
    {
    #if defined(_CORE_OPTION_NO_ROTATION_)
        return coreTranspose(coreMat4to3(u_m4Camera)) * vec3(a_v2LowPosition * a_v1DivScale, 0.0);
    #else
        float v1Sin = sin(a_v1DivAngle);
        float v1Cos = cos(a_v1DivAngle);
        return coreTranspose(coreMat4to3(u_m4Camera)) * vec3(mat2(v1Cos, v1Sin, -v1Sin, v1Cos) * (a_v2LowPosition * a_v1DivScale), 0.0);
    #endif
    }

    // default position transformation
    vec4 coreObject3DPositionRaw() {return u_m4ViewProj * vec4(coreObject3DTransformRaw(), 1.0);}
    vec4 coreObject3DPositionLow() {return u_m4ViewProj * vec4(coreObject3DTransformLow(), 1.0);}
    vec4 coreParticlePosition()    {return u_m4ViewProj * vec4(coreParticleRange() + a_v3DivPosition, 1.0);}

    // default texture coordinate transformation
    vec2 coreObject3DTexCoordRaw() {return a_v2RawTexCoord * u_v2TexSize + u_v2TexOffset;}
    vec2 coreObject3DTexCoordLow() {return a_v2LowTexCoord * u_v2TexSize + u_v2TexOffset;}
    vec2 coreParticleTexCoord()    {return a_v2LowTexCoord;}

    // calculate tangent-space matrix
    mat3 coreTangentSpaceMatrix(const in vec4 v4Rotation, const in vec3 v3Normal, const in vec4 v4Tangent)
    {
    #if defined(_CORE_OPTION_NO_ROTATION_)
        vec3 N = v3Normal;
        vec3 T = v4Tangent.xyz;
    #else
        vec3 N = coreQuatApply(v4Rotation, v3Normal);
        vec3 T = coreQuatApply(v4Rotation, v4Tangent.xyz);
    #endif
        vec3 B = cross(N, T) * v4Tangent.w;
        return coreTranspose(mat3(T, B, N));
    }
    mat3 coreTangentSpaceMatrix() {return coreTangentSpaceMatrix(u_v4Rotation, a_v3RawNormal, a_v4RawTangent);}

#endif // _CORE_VERTEX_SHADER_