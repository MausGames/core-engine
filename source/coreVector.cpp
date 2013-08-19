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
// constructor
coreVector2::coreVector2()
: x (0.0f), y (0.0f)
{
}

coreVector2::coreVector2(const coreVector2& c)
: x (c.x), y (c.y)
{
}

coreVector2::coreVector2(coreVector2&& c)
: x (c.x), y (c.y)
{
    c.x = 0;
    c.y = 0;
}

coreVector2::coreVector2(const float& fx, const float& fy)
: x (fx), y (fy)
{
}


// ****************************************************************
// assign vector
coreVector2& coreVector2::operator = (const coreVector2& c)
{
    x = c.x;
    y = c.y;

    return *this;
}

coreVector2& coreVector2::operator = (coreVector2&& c)
{
    if(this != &c)
    {
        x = c.x; c.x = 0; 
        y = c.y; c.y = 0;
    }
    return *this;
}
 

// ****************************************************************
// addition with vector
coreVector2 coreVector2::operator + (const coreVector2& c)const
{
    // optimized
    if(Core::System->SupportSSE2())
    {
        float __align16 afOutput[4];
        _mm_store_ps(afOutput, _mm_add_ps(_mm_setr_ps(x, y, 0.0f, 0.0f), _mm_setr_ps(c.x, c.y, 0.0f, 0.0f)));

        return coreVector2(afOutput[0], afOutput[1]);
    }

    // normal
    return coreVector2(x+c.x, y+c.y);
}


// ****************************************************************
// subtraction with vector
coreVector2 coreVector2::operator - (const coreVector2& c)const
{
    // optimized
    if(Core::System->SupportSSE2())
    {
        float __align16 afOutput[4];
        _mm_store_ps(afOutput, _mm_sub_ps(_mm_setr_ps(x, y, 0.0f, 0.0f), _mm_setr_ps(c.x, c.y, 0.0f, 0.0f)));

        return coreVector2(afOutput[0], afOutput[1]);
    }

    // normal
    return coreVector2(x-c.x, y-c.y);
}


// ****************************************************************
// multiplication with vector
coreVector2 coreVector2::operator * (const coreVector2& c)const
{
    // optimized
    if(Core::System->SupportSSE2())
    {
        float __align16 afOutput[4];
        _mm_store_ps(afOutput, _mm_mul_ps(_mm_setr_ps(x, y, 0.0f, 0.0f), _mm_setr_ps(c.x, c.y, 0.0f, 0.0f)));

        return coreVector2(afOutput[0], afOutput[1]);
    }

    // normal
    return coreVector2(x*c.x, y*c.y);
}


// ****************************************************************
// division with vector
coreVector2 coreVector2::operator / (const coreVector2& c)const
{
    // optimized
    if(Core::System->SupportSSE2())
    {
        float __align16 afOutput[4];
        _mm_store_ps(afOutput, _mm_div_ps(_mm_setr_ps(x, y, 0.0f, 0.0f), _mm_setr_ps(c.x, c.y, 0.0f, 0.0f)));

        return coreVector2(afOutput[0], afOutput[1]);
    }

    // normal
    return coreVector2(x/c.x, y/c.y);
}


// ****************************************************************
// multiplication with matrix
coreVector2 coreVector2::operator * (const coreMatrix& m)const
{
    // TODO: add SSE-support
    const float w = 1.0f / (x*m._14 + y*m._24 + m._44);

    // optimized
    if(Core::System->SupportSSE41())
    {
        float afOutput[2];

        const __m128 A = _mm_setr_ps(x, y, 0.0f, w);
        const coreMatrix mInput = m.Transposed();

        for(int i = 0; i < 2; ++i)
        {
            const __m128 B = _mm_load_ps(&mInput.m[i][0]);
            _mm_store_ss(&afOutput[i], _mm_dp_ps(A, B, 0xF1));
        }

        return coreVector2(afOutput[0], afOutput[1]);
    }

    // normal
    return coreVector2(x*m._11 + y*m._21 + w*m._41,
                       x*m._12 + y*m._22 + w*m._42);
}


// ****************************************************************
// normalize vector
coreVector2& coreVector2::Normalize()
{
    float fLength = this->LengthSq();
    if(fLength == 1.0f) return *this;
    if(fLength == 0.0f) return *this;

    // optimized
    if(Core::System->SupportSSE2()) _mm_store_ss(&fLength, _mm_rsqrt_ss(_mm_load_ss(&fLength)));

    // normal
    else fLength = 1.0f/coreMath::Sqrt(fLength);

    *this *= fLength;
    return *this;
}


// ****************************************************************
// get squared length 
float coreVector2::LengthSq()const
{
    // optimized
    if(Core::System->SupportSSE41())
    {
        const __m128 A = _mm_setr_ps(x, y, 0.0f, 0.0f);
        
        float fOutput;
        _mm_store_ss(&fOutput, _mm_dp_ps(A, A, 0x31));

        return fOutput;
    }

    // normal
    return (x*x + y*y);
}


// ****************************************************************
// get dot product
float coreVector2::Dot(const coreVector2& vInA, const coreVector2& vInB)
{
    // optimized
    if(Core::System->SupportSSE41())
    {
        float fOutput;
        _mm_store_ss(&fOutput, _mm_dp_ps(_mm_setr_ps(vInA.x, vInA.y, 0.0f, 0.0f), _mm_setr_ps(vInB.x, vInB.y, 0.0f, 0.0f), 0x31));

        return fOutput;
    }

    // normal
    return (vInA.x*vInB.x + vInA.y*vInB.y);
}


// ****************************************************************
// get random vector
coreVector2 coreVector2::Rand()
{
    return coreVector2(Core::Rand->Float(-1.0f, 1.0f), 
                       Core::Rand->Float(-1.0f, 1.0f)).Normalize();
}

coreVector2 coreVector2::Rand(const float& fMin, const float& fMax)
{
    return coreVector2(Core::Rand->Float(-1.0f, 1.0f), 
                       Core::Rand->Float(-1.0f, 1.0f)).Normalize() * Core::Rand->Float(fMin, fMax);
}

coreVector2 coreVector2::Rand(const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY)
{
    return coreVector2(Core::Rand->Float(fMinX, fMaxX), 
                       Core::Rand->Float(fMinY, fMaxY));
}


// ****************************************************************
// get reflected vector
coreVector2 coreVector2::Reflect(const coreVector2& vVelocity, const coreVector2& vNormal) 
{
    const float fDot = coreVector2::Dot(vVelocity, vNormal);

    if(fDot > 0.0f) return vVelocity;
    return vVelocity - 2.0f*vNormal*fDot;
}


// ****************************************************************
// constructor
coreVector3::coreVector3()
: x (0.0f), y (0.0f), z (0.0f)
{
}

coreVector3::coreVector3(const coreVector3& c)
: x (c.x), y (c.y), z (c.z)
{
}

coreVector3::coreVector3(const coreVector2& c)
: x (c.x), y (c.y), z (0.0f)
{
}

coreVector3::coreVector3(coreVector3&& c)
: x (c.x), y (c.y), z (c.z)
{
    c.x = 0;
    c.y = 0;
    c.z = 0;
}

coreVector3::coreVector3(coreVector2&& c)
: x (c.x), y (c.y), z (0.0f)
{
    c.x = 0;
    c.y = 0;
}

coreVector3::coreVector3(const float& fx, const float& fy, const float& fz)
: x (fx), y (fy), z (fz)
{
}


// ****************************************************************
// assign vector
coreVector3& coreVector3::operator = (const coreVector3& c)
{
    x = c.x;
    y = c.y;
    z = c.z;

    return *this;
}

coreVector3& coreVector3::operator = (coreVector3&& c)
{
    if(this != &c)
    {
        x = c.x; c.x = 0; 
        y = c.y; c.y = 0;
        z = c.z; c.z = 0;
    }
    return *this;
}


// ****************************************************************
// addition with vector
coreVector3 coreVector3::operator + (const coreVector3& c)const
{
    // optimized
    if(Core::System->SupportSSE2())
    {
        float __align16 afOutput[4];
        _mm_store_ps(afOutput, _mm_add_ps(_mm_setr_ps(x, y, z, 0.0f), _mm_setr_ps(c.x, c.y, c.z, 0.0f)));

        return coreVector3(afOutput[0], afOutput[1], afOutput[2]);
    }

    // normal
    return coreVector3(x+c.x, y+c.y, z+c.z);
}


// ****************************************************************
// subtraction with vector
coreVector3 coreVector3::operator - (const coreVector3& c)const
{
    // optimized
    if(Core::System->SupportSSE2())
    {
        float __align16 afOutput[4];
        _mm_store_ps(afOutput, _mm_sub_ps(_mm_setr_ps(x, y, z, 0.0f), _mm_setr_ps(c.x, c.y, c.z, 0.0f)));

        return coreVector3(afOutput[0], afOutput[1], afOutput[2]);
    }

    // normal
    return coreVector3(x-c.x, y-c.y, z-c.z);
}


// ****************************************************************
// multiplication with vector
coreVector3 coreVector3::operator * (const coreVector3& c)const
{
    // optimized
    if(Core::System->SupportSSE2())
    {
        float __align16 afOutput[4];
        _mm_store_ps(afOutput, _mm_mul_ps(_mm_setr_ps(x, y, z, 0.0f), _mm_setr_ps(c.x, c.y, c.z, 0.0f)));

        return coreVector3(afOutput[0], afOutput[1], afOutput[2]);
    }

    // normal
    return coreVector3(x*c.x, y*c.y, z*c.z);
}


// ****************************************************************
// division with vector
coreVector3 coreVector3::operator / (const coreVector3& c)const
{
    // optimized
    if(Core::System->SupportSSE2())
    {
        float __align16 afOutput[4];
        _mm_store_ps(afOutput, _mm_div_ps(_mm_setr_ps(x, y, z, 0.0f), _mm_setr_ps(c.x, c.y, c.z, 0.0f)));

        return coreVector3(afOutput[0], afOutput[1], afOutput[2]);
    }

    // normal
    return coreVector3(x/c.x, y/c.y, z/c.z);
}


// ****************************************************************
// multiplication with matrix
coreVector3 coreVector3::operator * (const coreMatrix& m)const
{
    // TODO: add SSE-support
    const float w = 1.0f / (x*m._14 + y*m._24 + z*m._34 + m._44);

    // optimized
    if(Core::System->SupportSSE41())
    {
        float afOutput[3];

        const __m128 A = _mm_setr_ps(x, y, z, w);
        const coreMatrix mInput = m.Transposed();

        for(int i = 0; i < 3; ++i)
        {
            const __m128 B = _mm_load_ps(&mInput.m[i][0]);
            _mm_store_ss(&afOutput[i], _mm_dp_ps(A, B, 0xF1));
        }

        return coreVector3(afOutput[0], afOutput[1], afOutput[2]);
    }

    // normal
    return coreVector3(x*m._11 + y*m._21 + z*m._31 + w*m._41,
                       x*m._12 + y*m._22 + z*m._32 + w*m._42,
                       x*m._13 + y*m._23 + z*m._33 + w*m._43);
}


// ****************************************************************
// normalize vector
coreVector3& coreVector3::Normalize()
{
    float fLength = this->LengthSq();
    if(fLength == 1.0f) return *this;
    if(fLength == 0.0f) return *this;

    // optimized
    if(Core::System->SupportSSE2()) _mm_store_ss(&fLength, _mm_rsqrt_ss(_mm_load_ss(&fLength)));

    // normal
    else fLength = 1.0f/coreMath::Sqrt(fLength);

    *this *= fLength;
    return *this;
}


// ****************************************************************
// get squared length 
float coreVector3::LengthSq()const
{
    // optimized
    if(Core::System->SupportSSE41())
    {
        const __m128 A = _mm_setr_ps(x, y, z, 0.0f);

        float fOutput;
        _mm_store_ss(&fOutput, _mm_dp_ps(A, A, 0x71));

        return fOutput;
    }

    // normal
    return (x*x + y*y + z*z);
}


// ****************************************************************
// get dot product
float coreVector3::Dot(const coreVector3& vInA, const coreVector3& vInB)
{
    // optimized
    if(Core::System->SupportSSE41())
    {
        float fOutput;
        _mm_store_ss(&fOutput, _mm_dp_ps(_mm_setr_ps(vInA.x, vInA.y, vInA.z, 0.0f), _mm_setr_ps(vInB.x, vInB.y, vInB.z, 0.0f), 0x71));

        return fOutput;
    }

    // normal
    return (vInA.x*vInB.x + vInA.y*vInB.y + vInA.z*vInB.z);
}


// ****************************************************************
// get random vector
coreVector3 coreVector3::Rand()
{
    return coreVector3(Core::Rand->Float(-1.0f, 1.0f), 
                       Core::Rand->Float(-1.0f, 1.0f),
                       Core::Rand->Float(-1.0f, 1.0f)).Normalize();
}

coreVector3 coreVector3::Rand(const float& fMin, const float& fMax)
{
    return coreVector3(Core::Rand->Float(-1.0f, 1.0f), 
                       Core::Rand->Float(-1.0f, 1.0f),
                       Core::Rand->Float(-1.0f, 1.0f)).Normalize() * Core::Rand->Float(fMin, fMax);
}

coreVector3 coreVector3::Rand(const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY, const float& fMinZ, const float& fMaxZ)
{
    return coreVector3(Core::Rand->Float(fMinX, fMaxX), 
                       Core::Rand->Float(fMinY, fMaxY),
                       Core::Rand->Float(fMinZ, fMaxZ));
}


// ****************************************************************
// get reflected vector
coreVector3 coreVector3::Reflect(const coreVector3& vVelocity, const coreVector3& vNormal) 
{
    const float fDot = coreVector3::Dot(vVelocity, vNormal);

    if(fDot > 0.0f) return vVelocity;
    return vVelocity - 2.0f*vNormal*fDot;
}


// ****************************************************************
// get cross product
coreVector3 coreVector3::Cross(const coreVector3& vInA, const coreVector3& vInB)
{
    // optimized
    if(Core::System->SupportSSE2())
    {
        const __m128 A = _mm_setr_ps(vInA.x, vInA.y, vInA.z, 0.0f);
        const __m128 B = _mm_setr_ps(vInB.x, vInB.y, vInB.z, 0.0f);

        float __align16 afOutput[4];
        _mm_store_ps(afOutput, _mm_sub_ps(_mm_mul_ps(_mm_shuffle_ps(A, A, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(B, B, _MM_SHUFFLE(3, 1, 0, 2))), 
                                          _mm_mul_ps(_mm_shuffle_ps(A, A, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(B, B, _MM_SHUFFLE(3, 0, 2, 1)))));

        return coreVector3(afOutput[0], afOutput[1], afOutput[2]);
    }

    // normal
    return coreVector3(vInA.y*vInB.z - vInA.z*vInB.y, 
                       vInA.z*vInB.x - vInA.x*vInB.z, 
                       vInA.x*vInB.y - vInA.y*vInB.x);
}


// ****************************************************************
// constructor
coreVector4::coreVector4()
: x (0.0f), y (0.0f), z (0.0f), w (0.0f)
{
}

coreVector4::coreVector4(const coreVector4& c)
: x (c.x), y (c.y), z (c.z), w (c.w)
{
}

coreVector4::coreVector4(const coreVector3& c)
: x (c.x), y (c.y), z (c.z), w (0.0f)
{
}

coreVector4::coreVector4(const coreVector2& c)
: x (c.x), y (c.y), z (0.0f), w (0.0f)
{
}

coreVector4::coreVector4(coreVector4&& c)
: x (c.x), y (c.y), z (c.z), w (c.w)
{
    c.x = 0;
    c.y = 0;
    c.z = 0;
    c.w = 0;
}

coreVector4::coreVector4(coreVector3&& c)
: x (c.x), y (c.y), z (c.z), w (0.0f)
{
    c.x = 0;
    c.y = 0;
    c.z = 0;
}

coreVector4::coreVector4(coreVector2&& c)
: x (c.x), y (c.y), z (0.0f), w (0.0f)
{
    c.x = 0;
    c.y = 0;
}

coreVector4::coreVector4(const float& fx, const float& fy, const float& fz, const float& fw)
: x (fx), y (fy), z (fz), w (fw)
{
}


// ****************************************************************
// assign vector
coreVector4& coreVector4::operator = (const coreVector4& c)
{
    x = c.x;
    y = c.y;
    z = c.z;
    w = c.w;

    return *this;
}

coreVector4& coreVector4::operator = (coreVector4&& c)
{
    if(this != &c)
    {
        x = c.x; c.x = 0; 
        y = c.y; c.y = 0;
        z = c.z; c.z = 0;
        w = c.w; c.w = 0;
    }
    return *this;
}


// ****************************************************************
// addition with vector
coreVector4 coreVector4::operator + (const coreVector4& c)const
{
    // optimized
    if(Core::System->SupportSSE2())
    {
        float __align16 afOutput[4];
        _mm_store_ps(afOutput, _mm_add_ps(_mm_setr_ps(x, y, z, w), _mm_setr_ps(c.x, c.y, c.z, c.w)));

        return coreVector4(afOutput[0], afOutput[1], afOutput[2], afOutput[3]);
    }

    // normal
    return coreVector4(x+c.x, y+c.y, z+c.z, w+c.w);
}


// ****************************************************************
// subtraction with vector
coreVector4 coreVector4::operator - (const coreVector4& c)const
{
    // optimized
    if(Core::System->SupportSSE2())
    {
        float __align16 afOutput[4];
        _mm_store_ps(afOutput, _mm_sub_ps(_mm_setr_ps(x, y, z, w), _mm_setr_ps(c.x, c.y, c.z, c.w)));

        return coreVector4(afOutput[0], afOutput[1], afOutput[2], afOutput[3]);
    }

    // normal
    return coreVector4(x-c.x, y-c.y, z-c.z, w-c.w);
}


// ****************************************************************
// multiplication with vector
coreVector4 coreVector4::operator * (const coreVector4& c)const
{
    // optimized
    if(Core::System->SupportSSE2())
    {
        float __align16 afOutput[4];
        _mm_store_ps(afOutput, _mm_mul_ps(_mm_setr_ps(x, y, z, w), _mm_setr_ps(c.x, c.y, c.z, c.w)));

        return coreVector4(afOutput[0], afOutput[1], afOutput[2], afOutput[3]);
    }

    // normal
    return coreVector4(x*c.x, y*c.y, z*c.z, w*c.w);
}


// ****************************************************************
// division with vector
coreVector4 coreVector4::operator / (const coreVector4& c)const
{
    // optimized
    if(Core::System->SupportSSE2())
    {
        float __align16 afOutput[4];
        _mm_store_ps(afOutput, _mm_div_ps(_mm_setr_ps(x, y, z, w), _mm_setr_ps(c.x, c.y, c.z, c.w)));

        return coreVector4(afOutput[0], afOutput[1], afOutput[2], afOutput[3]);
    }

    // normal
    return coreVector4(x/c.x, y/c.y, z/c.z, w/c.w);
}


// ****************************************************************
// multiplication with matrix
coreVector4 coreVector4::operator * (const coreMatrix& m)const
{
    // optimized
    if(Core::System->SupportSSE41())
    {
        float afOutput[4];

        const __m128 A = _mm_setr_ps(x, y, z, w);
        const coreMatrix mInput = m.Transposed();

        for(int i = 0; i < 4; ++i)
        {
            const __m128 B = _mm_load_ps(&mInput.m[i][0]);
            _mm_store_ss(&afOutput[i], _mm_dp_ps(A, B, 0xF1));
        }

        return coreVector4(afOutput[0], afOutput[1], afOutput[2], afOutput[3]);
    }

    // normal
    return coreVector4(x*m._11 + y*m._21 + z*m._31 + w*m._41,
                       x*m._12 + y*m._22 + z*m._32 + w*m._42,
                       x*m._13 + y*m._23 + z*m._33 + w*m._43,
                       x*m._14 + y*m._24 + z*m._34 + w*m._44);
}