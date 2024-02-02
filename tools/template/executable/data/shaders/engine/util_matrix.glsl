///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////


// ****************************************************************
// matrix transpose
mat2 coreTranspose(const in mat2 m)
{
#if (CORE_GL_VERSION >= 120) || (CORE_GL_ES_VERSION >= 300)
    return transpose(m);
#else
    return mat2(m[0][0], m[1][0],
                m[0][1], m[1][1]);
#endif
}

mat3 coreTranspose(const in mat3 m)
{
#if (CORE_GL_VERSION >= 120) || (CORE_GL_ES_VERSION >= 300)
    return transpose(m);
#else
    return mat3(m[0][0], m[1][0], m[2][0],
                m[0][1], m[1][1], m[2][1],
                m[0][2], m[1][2], m[2][2]);
#endif
}

mat4 coreTranspose(const in mat4 m)
{
#if (CORE_GL_VERSION >= 120) || (CORE_GL_ES_VERSION >= 300)
    return transpose(m);
#else
    return mat4(m[0][0], m[1][0], m[2][0], m[3][0],
                m[0][1], m[1][1], m[2][1], m[3][1],
                m[0][2], m[1][2], m[2][2], m[3][2],
                m[0][3], m[1][3], m[2][3], m[3][3]);
#endif
}


// ****************************************************************
// matrix invert
mat3 coreInvert(const in mat3 m)
{
#if (CORE_GL_VERSION >= 150) || (CORE_GL_ES_VERSION >= 300)
    return inverse(m);
#else
    float A = m[1][1]*m[2][2] - m[1][2]*m[2][1];
    float B = m[1][2]*m[2][0] - m[1][0]*m[2][2];
    float C = m[1][0]*m[2][1] - m[1][1]*m[2][0];

    return mat3(A, m[0][2]*m[2][1] - m[0][1]*m[2][2], m[0][1]*m[1][2] - m[0][2]*m[1][1],
                B, m[0][0]*m[2][2] - m[0][2]*m[2][0], m[0][2]*m[1][0] - m[0][0]*m[1][2],
                C, m[0][1]*m[2][0] - m[0][0]*m[2][1], m[0][0]*m[1][1] - m[0][1]*m[1][0])
                * (1.0 / (m[0][0]*A + m[0][1]*B + m[0][2]*C));
#endif
}

mat4 coreInvert(const in mat4 m)
{
#if (CORE_GL_VERSION >= 150) || (CORE_GL_ES_VERSION >= 300)
    return inverse(m);
#else
    float A = m[0][0]*m[1][1] - m[0][1]*m[1][0];
    float B = m[0][0]*m[1][2] - m[0][2]*m[1][0];
    float C = m[0][0]*m[1][3] - m[0][3]*m[1][0];
    float D = m[0][1]*m[1][2] - m[0][2]*m[1][1];
    float E = m[0][1]*m[1][3] - m[0][3]*m[1][1];
    float F = m[0][2]*m[1][3] - m[0][3]*m[1][2];
    float G = m[2][0]*m[3][1] - m[2][1]*m[3][0];
    float H = m[2][0]*m[3][2] - m[2][2]*m[3][0];
    float I = m[2][0]*m[3][3] - m[2][3]*m[3][0];
    float J = m[2][1]*m[3][2] - m[2][2]*m[3][1];
    float K = m[2][1]*m[3][3] - m[2][3]*m[3][1];
    float L = m[2][2]*m[3][3] - m[2][3]*m[3][2];

    return mat4(m[1][1]*L - m[1][2]*K + m[1][3]*J, m[0][2]*K - m[0][1]*L - m[0][3]*J, m[3][1]*F - m[3][2]*E + m[3][3]*D, m[2][2]*E - m[2][1]*F - m[2][3]*D,
                m[1][2]*I - m[1][0]*L - m[1][3]*H, m[0][0]*L - m[0][2]*I + m[0][3]*H, m[3][2]*C - m[3][0]*F - m[3][3]*B, m[2][0]*F - m[2][2]*C + m[2][3]*B,
                m[1][0]*K - m[1][1]*I + m[1][3]*G, m[0][1]*I - m[0][0]*K - m[0][3]*G, m[3][0]*E - m[3][1]*C + m[3][3]*A, m[2][1]*C - m[2][0]*E - m[2][3]*A,
                m[1][1]*H - m[1][0]*J - m[1][2]*G, m[0][0]*J - m[0][1]*H + m[0][2]*G, m[3][1]*B - m[3][0]*D - m[3][2]*A, m[2][0]*D - m[2][1]*B + m[2][2]*A)
                * (1.0 / (A*L - B*K + C*J + D*I - E*H + F*G));
#endif
}


// ****************************************************************
// matrix convert
#if (CORE_GL_VERSION >= 120) || (CORE_GL_ES_VERSION >= 300)
    #define coreMat4to3(m) mat3(m)
    #define coreMat3to2(m) mat2(m)
#else
    #define coreMat4to3(m) mat3(m[0].xyz, m[1].xyz, m[2].xyz)
    #define coreMat3to2(m) mat2(m[0].xy,  m[1].xy)
#endif
#define coreMat4to2(m) coreMat3to2(m)