//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// addition with vector
coreVector2 coreVector2::operator + (const coreVector2& v)const noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE2())
    {
        static align16(float) afOutput[4];
        _mm_store_ps(afOutput, _mm_add_ps(_mm_setr_ps(x, y, 0.0f, 0.0f), _mm_setr_ps(v.x, v.y, 0.0f, 0.0f)));

        return coreVector2(afOutput[0], afOutput[1]);
    }

#endif

    // normal
    return coreVector2(x+v.x, y+v.y);
}


// ****************************************************************
// subtraction with vector
coreVector2 coreVector2::operator - (const coreVector2& v)const noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE2())
    {
        static align16(float) afOutput[4];
        _mm_store_ps(afOutput, _mm_sub_ps(_mm_setr_ps(x, y, 0.0f, 0.0f), _mm_setr_ps(v.x, v.y, 0.0f, 0.0f)));

        return coreVector2(afOutput[0], afOutput[1]);
    }

#endif

    // normal
    return coreVector2(x-v.x, y-v.y);
}


// ****************************************************************
// multiplication with vector
coreVector2 coreVector2::operator * (const coreVector2& v)const noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE2())
    {
        static align16(float) afOutput[4];
        _mm_store_ps(afOutput, _mm_mul_ps(_mm_setr_ps(x, y, 0.0f, 0.0f), _mm_setr_ps(v.x, v.y, 0.0f, 0.0f)));

        return coreVector2(afOutput[0], afOutput[1]);
    }

#endif

    // normal
    return coreVector2(x*v.x, y*v.y);
}


// ****************************************************************
// division with vector
coreVector2 coreVector2::operator / (const coreVector2& v)const noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE2())
    {
        static align16(float) afOutput[4];
        _mm_store_ps(afOutput, _mm_div_ps(_mm_setr_ps(x, y, 0.0f, 0.0f), _mm_setr_ps(v.x, v.y, 0.0f, 0.0f)));

        return coreVector2(afOutput[0], afOutput[1]);
    }

#endif

    // normal
    return coreVector2(x/v.x, y/v.y);
}


// ****************************************************************
// multiplication with matrix
coreVector2 coreVector2::operator * (const coreMatrix& m)const noexcept
{
    // TODO: add SSE-support
    const float w = 1.0f / (x*m._14 + y*m._24 + m._44);

#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE41())
    {
        const __m128 A = _mm_setr_ps(x, y, 0.0f, w);
        static coreMatrix mInput; mInput = m.Transposed();

        float afOutput[2];
        for(int i = 0; i < 2; ++i)
            _mm_store_ss(&afOutput[i], _mm_dp_ps(A, _mm_load_ps(&mInput.m[i][0]), 0xF1));

        return coreVector2(afOutput[0], afOutput[1]);
    }

#endif

    // normal
    return coreVector2(x*m._11 + y*m._21 + w*m._41,
                       x*m._12 + y*m._22 + w*m._42);
}


// ****************************************************************
// normalize vector
coreVector2& coreVector2::Normalize()noexcept
{
    float fLength = this->LengthSq();
    if(fLength == 1.0f) return *this;
    if(fLength == 0.0f) return *this;

#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE2()) _mm_store_ss(&fLength, _mm_rsqrt_ss(_mm_load_ss(&fLength)));
    else

#endif

    // normal
    fLength = 1.0f/coreMath::Sqrt(fLength);

    *this *= fLength;
    return *this;
}


// ****************************************************************
// get squared length
float coreVector2::LengthSq()const noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE41())
    {
        const __m128 A = _mm_setr_ps(x, y, 0.0f, 0.0f);

        float fOutput;
        _mm_store_ss(&fOutput, _mm_dp_ps(A, A, 0x31));

        return fOutput;
    }

#endif

    // normal
    return (x*x + y*y);
}


// ****************************************************************
// get dot product
float coreVector2::Dot(const coreVector2& vInA, const coreVector2& vInB)noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE41())
    {
        float fOutput;
        _mm_store_ss(&fOutput, _mm_dp_ps(_mm_setr_ps(vInA.x, vInA.y, 0.0f, 0.0f), _mm_setr_ps(vInB.x, vInB.y, 0.0f, 0.0f), 0x31));

        return fOutput;
    }

#endif

    // normal
    return (vInA.x*vInB.x + vInA.y*vInB.y);
}


// ****************************************************************
// get random vector
coreVector2 coreVector2::Rand()noexcept
{
    return coreVector2(Core::Rand->Float(-1.0f, 1.0f),
                       Core::Rand->Float(-1.0f, 1.0f)).Normalize();
}

coreVector2 coreVector2::Rand(const float& fMin, const float& fMax)noexcept
{
    return coreVector2(Core::Rand->Float(-1.0f, 1.0f),
                       Core::Rand->Float(-1.0f, 1.0f)).Normalize() * Core::Rand->Float(fMin, fMax);
}

coreVector2 coreVector2::Rand(const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY)noexcept
{
    return coreVector2(Core::Rand->Float(fMinX, fMaxX),
                       Core::Rand->Float(fMinY, fMaxY));
}


// ****************************************************************
// get reflected vector
coreVector2 coreVector2::Reflect(const coreVector2& vVelocity, const coreVector2& vNormal)noexcept
{
    const float fDot = coreVector2::Dot(vVelocity, vNormal);

    if(fDot > 0.0f) return vVelocity;
    return vVelocity - 2.0f*vNormal*fDot;
}


// ****************************************************************
// addition with vector
coreVector3 coreVector3::operator + (const coreVector3& v)const noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE2())
    {
        static align16(float) afOutput[4];
        _mm_store_ps(afOutput, _mm_add_ps(_mm_setr_ps(x, y, z, 0.0f), _mm_setr_ps(v.x, v.y, v.z, 0.0f)));

        return coreVector3(afOutput[0], afOutput[1], afOutput[2]);
    }

#endif

    // normal
    return coreVector3(x+v.x, y+v.y, z+v.z);
}


// ****************************************************************
// subtraction with vector
coreVector3 coreVector3::operator - (const coreVector3& v)const noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE2())
    {
        static align16(float) afOutput[4];
        _mm_store_ps(afOutput, _mm_sub_ps(_mm_setr_ps(x, y, z, 0.0f), _mm_setr_ps(v.x, v.y, v.z, 0.0f)));

        return coreVector3(afOutput[0], afOutput[1], afOutput[2]);
    }

#endif

    // normal
    return coreVector3(x-v.x, y-v.y, z-v.z);
}


// ****************************************************************
// multiplication with vector
coreVector3 coreVector3::operator * (const coreVector3& v)const noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE2())
    {
        static align16(float) afOutput[4];
        _mm_store_ps(afOutput, _mm_mul_ps(_mm_setr_ps(x, y, z, 0.0f), _mm_setr_ps(v.x, v.y, v.z, 0.0f)));

        return coreVector3(afOutput[0], afOutput[1], afOutput[2]);
    }

#endif

    // normal
    return coreVector3(x*v.x, y*v.y, z*v.z);
}


// ****************************************************************
// division with vector
coreVector3 coreVector3::operator / (const coreVector3& v)const noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE2())
    {
        static align16(float) afOutput[4];
        _mm_store_ps(afOutput, _mm_div_ps(_mm_setr_ps(x, y, z, 0.0f), _mm_setr_ps(v.x, v.y, v.z, 0.0f)));

        return coreVector3(afOutput[0], afOutput[1], afOutput[2]);
    }

#endif

    // normal
    return coreVector3(x/v.x, y/v.y, z/v.z);
}


// ****************************************************************
// multiplication with matrix
coreVector3 coreVector3::operator * (const coreMatrix& m)const noexcept
{
    // TODO: add SSE-support
    const float w = 1.0f / (x*m._14 + y*m._24 + z*m._34 + m._44);

#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE41())
    {
        const __m128 A = _mm_setr_ps(x, y, z, w);
        static coreMatrix mInput; mInput = m.Transposed();

        float afOutput[3];
        for(int i = 0; i < 3; ++i)
            _mm_store_ss(&afOutput[i], _mm_dp_ps(A, _mm_load_ps(&mInput.m[i][0]), 0xF1));

        return coreVector3(afOutput[0], afOutput[1], afOutput[2]);
    }

#endif

    // normal
    return coreVector3(x*m._11 + y*m._21 + z*m._31 + w*m._41,
                       x*m._12 + y*m._22 + z*m._32 + w*m._42,
                       x*m._13 + y*m._23 + z*m._33 + w*m._43);
}


// ****************************************************************
// normalize vector
coreVector3& coreVector3::Normalize()noexcept
{
    float fLength = this->LengthSq();
    if(fLength == 1.0f) return *this;
    if(fLength == 0.0f) return *this;

#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE2()) _mm_store_ss(&fLength, _mm_rsqrt_ss(_mm_load_ss(&fLength)));
    else

#endif

    // normal
    fLength = 1.0f/coreMath::Sqrt(fLength);

    *this *= fLength;
    return *this;
}


// ****************************************************************
// get squared length
float coreVector3::LengthSq()const noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE41())
    {
        const __m128 A = _mm_setr_ps(x, y, z, 0.0f);

        float fOutput;
        _mm_store_ss(&fOutput, _mm_dp_ps(A, A, 0x71));

        return fOutput;
    }

#endif

    // normal
    return (x*x + y*y + z*z);
}


// ****************************************************************
// get dot product
float coreVector3::Dot(const coreVector3& vInA, const coreVector3& vInB)noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE41())
    {
        float fOutput;
        _mm_store_ss(&fOutput, _mm_dp_ps(_mm_setr_ps(vInA.x, vInA.y, vInA.z, 0.0f), _mm_setr_ps(vInB.x, vInB.y, vInB.z, 0.0f), 0x71));

        return fOutput;
    }

#endif

    // normal
    return (vInA.x*vInB.x + vInA.y*vInB.y + vInA.z*vInB.z);
}


// ****************************************************************
// get random vector
coreVector3 coreVector3::Rand()noexcept
{
    return coreVector3(Core::Rand->Float(-1.0f, 1.0f),
                       Core::Rand->Float(-1.0f, 1.0f),
                       Core::Rand->Float(-1.0f, 1.0f)).Normalize();
}

coreVector3 coreVector3::Rand(const float& fMin, const float& fMax)noexcept
{
    return coreVector3(Core::Rand->Float(-1.0f, 1.0f),
                       Core::Rand->Float(-1.0f, 1.0f),
                       Core::Rand->Float(-1.0f, 1.0f)).Normalize() * Core::Rand->Float(fMin, fMax);
}

coreVector3 coreVector3::Rand(const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY, const float& fMinZ, const float& fMaxZ)noexcept
{
    return coreVector3(Core::Rand->Float(fMinX, fMaxX),
                       Core::Rand->Float(fMinY, fMaxY),
                       Core::Rand->Float(fMinZ, fMaxZ));
}


// ****************************************************************
// get reflected vector
coreVector3 coreVector3::Reflect(const coreVector3& vVelocity, const coreVector3& vNormal)noexcept
{
    const float fDot = coreVector3::Dot(vVelocity, vNormal);

    if(fDot > 0.0f) return vVelocity;
    return vVelocity - 2.0f*vNormal*fDot;
}


// ****************************************************************
// get cross product
coreVector3 coreVector3::Cross(const coreVector3& vInA, const coreVector3& vInB)noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE2())
    {
        const __m128 A = _mm_setr_ps(vInA.x, vInA.y, vInA.z, 0.0f);
        const __m128 B = _mm_setr_ps(vInB.x, vInB.y, vInB.z, 0.0f);

        static align16(float) afOutput[4];
        _mm_store_ps(afOutput, _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(A, A, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(B, B, _MM_SHUFFLE(3, 1, 0, 2))),
                                          _mm_mul_ps(_mm_shuffle_ps(A, A, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(B, B, _MM_SHUFFLE(3, 0, 2, 1)))));

        return coreVector3(afOutput[0], afOutput[1], afOutput[2]);
    }

#endif

    // normal
    return coreVector3(vInA.y*vInB.z - vInA.z*vInB.y,
                       vInA.z*vInB.x - vInA.x*vInB.z,
                       vInA.x*vInB.y - vInA.y*vInB.x);
}


// ****************************************************************
// addition with vector
coreVector4 coreVector4::operator + (const coreVector4& v)const noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE2())
    {
        static align16(float) afOutput[4];
        _mm_store_ps(afOutput, _mm_add_ps(_mm_setr_ps(x, y, z, w), _mm_setr_ps(v.x, v.y, v.z, v.w)));

        return coreVector4(afOutput[0], afOutput[1], afOutput[2], afOutput[3]);
    }

#endif

    // normal
    return coreVector4(x+v.x, y+v.y, z+v.z, w+v.w);
}


// ****************************************************************
// subtraction with vector
coreVector4 coreVector4::operator - (const coreVector4& v)const noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE2())
    {
        static align16(float) afOutput[4];
        _mm_store_ps(afOutput, _mm_sub_ps(_mm_setr_ps(x, y, z, w), _mm_setr_ps(v.x, v.y, v.z, v.w)));

        return coreVector4(afOutput[0], afOutput[1], afOutput[2], afOutput[3]);
    }

#endif

    // normal
    return coreVector4(x-v.x, y-v.y, z-v.z, w-v.w);
}


// ****************************************************************
// multiplication with vector
coreVector4 coreVector4::operator * (const coreVector4& v)const noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE2())
    {
        static align16(float) afOutput[4];
        _mm_store_ps(afOutput, _mm_mul_ps(_mm_setr_ps(x, y, z, w), _mm_setr_ps(v.x, v.y, v.z, v.w)));

        return coreVector4(afOutput[0], afOutput[1], afOutput[2], afOutput[3]);
    }

#endif

    // normal
    return coreVector4(x*v.x, y*v.y, z*v.z, w*v.w);
}


// ****************************************************************
// division with vector
coreVector4 coreVector4::operator / (const coreVector4& v)const noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE2())
    {
        static align16(float) afOutput[4];
        _mm_store_ps(afOutput, _mm_div_ps(_mm_setr_ps(x, y, z, w), _mm_setr_ps(v.x, v.y, v.z, v.w)));

        return coreVector4(afOutput[0], afOutput[1], afOutput[2], afOutput[3]);
    }

#endif

    // normal
    return coreVector4(x/v.x, y/v.y, z/v.z, w/v.w);
}


// ****************************************************************
// multiplication with matrix
coreVector4 coreVector4::operator * (const coreMatrix& m)const noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE41())
    {
        const __m128 A = _mm_setr_ps(x, y, z, w);
        static coreMatrix mInput; mInput = m.Transposed();

        float afOutput[4];
        for(int i = 0; i < 4; ++i)
            _mm_store_ss(&afOutput[i], _mm_dp_ps(A, _mm_load_ps(&mInput.m[i][0]), 0xF1));

        return coreVector4(afOutput[0], afOutput[1], afOutput[2], afOutput[3]);
    }

#endif

    // normal
    return coreVector4(x*m._11 + y*m._21 + z*m._31 + w*m._41,
                       x*m._12 + y*m._22 + z*m._32 + w*m._42,
                       x*m._13 + y*m._23 + z*m._33 + w*m._43,
                       x*m._14 + y*m._24 + z*m._34 + w*m._44);
}


// ****************************************************************
// get squared length
float coreVector4::LengthSq()const noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE41())
    {
        const __m128 A = _mm_setr_ps(x, y, z, w);

        float fOutput;
        _mm_store_ss(&fOutput, _mm_dp_ps(A, A, 0xF1));

        return fOutput;
    }

#endif

    // normal
    return (x*x + y*y + z*z + w*w);
}


// ****************************************************************
// multiplication of two quaternions
coreVector4 coreVector4::QuatMul(const coreVector4& vInA, const coreVector4& vInB)noexcept
{
    // TODO: add SSE-support
    return coreVector4(vInA.x*vInB.w + vInA.w*vInB.x + vInA.y*vInB.z - vInA.z*vInB.y,
                       vInA.y*vInB.w + vInA.w*vInB.y + vInA.z*vInB.x - vInA.x*vInB.z,
                       vInA.z*vInB.w + vInA.w*vInB.z + vInA.x*vInB.y - vInA.y*vInB.x,
                       vInA.w*vInB.w - vInA.x*vInB.x - vInA.y*vInB.y - vInA.z*vInB.z);
}