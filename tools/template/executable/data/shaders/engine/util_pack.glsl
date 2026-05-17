///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////


// ****************************************************************
// vector normal pack and unpack
vec2 corePackNormalSphere(const in vec3 v)
{
    float A = inversesqrt(v.z * 8.0 + 8.0);
    return v.xy * A + 0.5;
}

vec3 coreUnpackNormalSphere(const in vec2 v)
{
    vec2  A = v * 4.0 - 2.0;
    float B = coreLengthSq(A);
    float C = sqrt(1.0 - B / 4.0);
    return vec3(A * C, 1.0 - B / 2.0);
}

vec3 coreUnpackNormalMap(const in vec2 v)
{
    vec2 A = v * 2.0 - 1.0;
    return normalize(vec3(A, sqrt(1.0 - coreLengthSq(A))));
}

vec3 coreUnpackNormalMapDeriv(const in vec2 v)
{
    vec2 A = v * 2.0 - 1.0;
    return normalize(vec3(A, 1.0));
}

vec2 corePackNormalOcta(const in vec3 v)
{
    vec2 A = v.xy / (abs(v.x) + abs(v.y) + abs(v.z));
    A = (v.z >= 0.0) ? A : ((vec2(1.0) - abs(A.yx)) * coreSign(A));
    return A;
}

vec3 coreUnpackNormalOcta(const in vec2 v)
{
    vec3 A = vec3(v, 1.0 - abs(v.x) - abs(v.y));
    A.xy += coreSign(A.xy) * -coreSaturate(-A.z);
    return normalize(A);
}


// ****************************************************************
// value pack and unpack
#if defined(CORE_GL_gpu_shader4)

    uint corePackUnorm4x8(const in vec4 x)
    {
    #if defined(GL_ARB_shading_language_packing)
        return packUnorm4x8(x);
    #else
        return (uint(x.a * 255.0) << 24) +
               (uint(x.b * 255.0) << 16) +
               (uint(x.g * 255.0) <<  8) +
               (uint(x.r * 255.0));
    #endif
    }

    vec4 coreUnpackUnorm4x8(const in uint x)
    {
    #if defined(GL_ARB_shading_language_packing)
        return unpackUnorm4x8(x);
    #else
        return vec4(float( x        & 0xFFu),
                    float((x >>  8) & 0xFFu),
                    float((x >> 16) & 0xFFu),
                    float((x >> 24) & 0xFFu)) * 0.003921569;
    #endif
    }

#endif