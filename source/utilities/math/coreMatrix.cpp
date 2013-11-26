//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// compare with matrix
bool coreMatrix4::operator == (const coreMatrix4& v)const noexcept
{
    // TODO: add SSE-support
    return ((_11 == v._11) && (_12 == v._12) && (_13 == v._13) && (_14 == v._14) &&
            (_21 == v._21) && (_22 == v._22) && (_23 == v._23) && (_24 == v._24) &&
            (_31 == v._31) && (_32 == v._32) && (_33 == v._33) && (_34 == v._34) &&
            (_41 == v._41) && (_42 == v._42) && (_43 == v._43) && (_44 == v._44));
}


// ****************************************************************
// inverse compare with matrix
bool coreMatrix4::operator != (const coreMatrix4& v)const noexcept
{
    // TODO: add SSE-support
    return ((_11 != v._11) || (_12 != v._12) || (_13 != v._13) || (_14 != v._14) ||
            (_21 != v._21) || (_22 != v._22) || (_23 != v._23) || (_24 != v._24) ||
            (_31 != v._31) || (_32 != v._32) || (_33 != v._33) || (_34 != v._34) ||
            (_41 != v._41) || (_42 != v._42) || (_43 != v._43) || (_44 != v._44));
}


// ****************************************************************
// addition with matrix
coreMatrix4 coreMatrix4::operator + (const coreMatrix4& v)const noexcept
{
    // TODO: add SSE-support
    return coreMatrix4(_11+v._11, _12+v._12, _13+v._13, _14+v._14,
                       _21+v._21, _22+v._22, _23+v._23, _24+v._24,
                       _31+v._31, _32+v._32, _33+v._33, _34+v._34,
                       _41+v._41, _42+v._42, _43+v._43, _44+v._44);
}


// ****************************************************************
// subtraction with matrix
coreMatrix4 coreMatrix4::operator - (const coreMatrix4& v)const noexcept
{
    // TODO: add SSE-support
    return coreMatrix4(_11-v._11, _12-v._12, _13-v._13, _14-v._14,
                       _21-v._21, _22-v._22, _23-v._23, _24-v._24,
                       _31-v._31, _32-v._32, _33-v._33, _34-v._34,
                       _41-v._41, _42-v._42, _43-v._43, _44-v._44);
}


// ****************************************************************
// multiplication with matrix
coreMatrix4 coreMatrix4::operator * (const coreMatrix4& v)const noexcept
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE2())
    {
        sse_save(coreMatrix4) mInput;
        mInput = v;

        const __m128 A = _mm_load_ps(mInput.m[0]);
        const __m128 B = _mm_load_ps(mInput.m[1]);
        const __m128 C = _mm_load_ps(mInput.m[2]);
        const __m128 D = _mm_load_ps(mInput.m[3]);

        sse_save(coreMatrix4) mOutput;
        for(int i = 0; i < 4; ++i) 
        {
            _mm_store_ps(mOutput.m[i], _mm_add_ps(_mm_add_ps(_mm_mul_ps(_mm_set1_ps(m[i][0]), A), _mm_mul_ps(_mm_set1_ps(m[i][1]), B)),
                                                  _mm_add_ps(_mm_mul_ps(_mm_set1_ps(m[i][2]), C), _mm_mul_ps(_mm_set1_ps(m[i][3]), D))));
        }

        return mOutput;
    }

#endif

    // normal
    return coreMatrix4(_11*v._11 + _12*v._21 + _13*v._31 + _14*v._41, _11*v._12 + _12*v._22 + _13*v._32 + _14*v._42,
                       _11*v._13 + _12*v._23 + _13*v._33 + _14*v._43, _11*v._14 + _12*v._24 + _13*v._34 + _14*v._44,
                       _21*v._11 + _22*v._21 + _23*v._31 + _24*v._41, _21*v._12 + _22*v._22 + _23*v._32 + _24*v._42,
                       _21*v._13 + _22*v._23 + _23*v._33 + _24*v._43, _21*v._14 + _22*v._24 + _23*v._34 + _24*v._44,
                       _31*v._11 + _32*v._21 + _33*v._31 + _34*v._41, _31*v._12 + _32*v._22 + _33*v._32 + _34*v._42,
                       _31*v._13 + _32*v._23 + _33*v._33 + _34*v._43, _31*v._14 + _32*v._24 + _33*v._34 + _34*v._44,
                       _41*v._11 + _42*v._21 + _43*v._31 + _44*v._41, _41*v._12 + _42*v._22 + _43*v._32 + _44*v._42,
                       _41*v._13 + _42*v._23 + _43*v._33 + _44*v._43, _41*v._14 + _42*v._24 + _43*v._34 + _44*v._44);
}


// ****************************************************************
// multiplication with scalar
coreMatrix4 coreMatrix4::operator * (const float& f)const noexcept
{
    // TODO: add SSE-support
    return coreMatrix4(_11*f, _12*f, _13*f, _14*f,
                       _21*f, _22*f, _23*f, _24*f,
                       _31*f, _32*f, _33*f, _34*f,
                       _41*f, _42*f, _43*f, _44*f);
}


// ****************************************************************
// division with scalar
coreMatrix4 coreMatrix4::operator / (const float& f)const noexcept
{
    // TODO: add SSE-support
    return (*this)*(1.0f/f);
}


// ****************************************************************
// transpose matrix
coreMatrix4& coreMatrix4::Transpose()noexcept
{
    *this = coreMatrix4(_11, _21, _31, _41,
                        _12, _22, _32, _42,
                        _13, _23, _33, _43,
                        _14, _24, _34, _44);

    return *this;
}


// ****************************************************************
// invert matrix
coreMatrix4& coreMatrix4::Invert()noexcept
{
    // TODO: add SSE-support
    *this = coreMatrix4(_23*_34*_42 - _24*_33*_42 + _24*_32*_43 - _22*_34*_43 - _23*_32*_44 + _22*_33*_44,
                        _14*_33*_42 - _13*_34*_42 - _14*_32*_43 + _12*_34*_43 + _13*_32*_44 - _12*_33*_44,
                        _13*_24*_42 - _14*_23*_42 + _14*_22*_43 - _12*_24*_43 - _13*_22*_44 + _12*_23*_44,
                        _14*_23*_32 - _13*_24*_32 - _14*_22*_33 + _12*_24*_33 + _13*_22*_34 - _12*_23*_34,
                        _24*_33*_41 - _23*_34*_41 - _24*_31*_43 + _21*_34*_43 + _23*_31*_44 - _21*_33*_44,
                        _13*_34*_41 - _14*_33*_41 + _14*_31*_43 - _11*_34*_43 - _13*_31*_44 + _11*_33*_44,
                        _14*_23*_41 - _13*_24*_41 - _14*_21*_43 + _11*_24*_43 + _13*_21*_44 - _11*_23*_44,
                        _13*_24*_31 - _14*_23*_31 + _14*_21*_33 - _11*_24*_33 - _13*_21*_34 + _11*_23*_34,
                        _22*_34*_41 - _24*_32*_41 + _24*_31*_42 - _21*_34*_42 - _22*_31*_44 + _21*_32*_44,
                        _14*_32*_41 - _12*_34*_41 - _14*_31*_42 + _11*_34*_42 + _12*_31*_44 - _11*_32*_44,
                        _12*_24*_41 - _14*_22*_41 + _14*_21*_42 - _11*_24*_42 - _12*_21*_44 + _11*_22*_44,
                        _14*_22*_31 - _12*_24*_31 - _14*_21*_32 + _11*_24*_32 + _12*_21*_34 - _11*_22*_34,
                        _23*_32*_41 - _22*_33*_41 - _23*_31*_42 + _21*_33*_42 + _22*_31*_43 - _21*_32*_43,
                        _12*_33*_41 - _13*_32*_41 + _13*_31*_42 - _11*_33*_42 - _12*_31*_43 + _11*_32*_43,
                        _13*_22*_41 - _12*_23*_41 - _13*_21*_42 + _11*_23*_42 + _12*_21*_43 - _11*_22*_43,
                        _12*_23*_31 - _13*_22*_31 + _13*_21*_32 - _11*_23*_32 - _12*_21*_33 + _11*_22*_33)
                        / this->Determinant();

    return *this;
}


// ****************************************************************
// get determinant
float coreMatrix4::Determinant()const noexcept
{
    // TODO: add SSE-support
    return _14*_23*_32*_41 - _13*_24*_32*_41 - _14*_22*_33*_41 + _12*_24*_33*_41 +
           _13*_22*_34*_41 - _12*_23*_34*_41 - _14*_23*_31*_42 + _13*_24*_31*_42 +
           _14*_21*_33*_42 - _11*_24*_33*_42 - _13*_21*_34*_42 + _11*_23*_34*_42 +
           _14*_22*_31*_43 - _12*_24*_31*_43 - _14*_21*_32*_43 + _11*_24*_32*_43 +
           _12*_21*_34*_43 - _11*_22*_34*_43 - _13*_22*_31*_44 + _12*_23*_31*_44 +
           _13*_21*_32*_44 - _11*_23*_32*_44 - _12*_21*_33*_44 + _11*_22*_33*_44;
}


// ****************************************************************
// get rotation matrix around X
coreMatrix4 coreMatrix4::RotationX(const coreVector2& vDirection)noexcept
{
    if(!vDirection.x && !vDirection.y) return coreMatrix4::Identity();

    const coreVector2 N = vDirection.Normalized();
    return coreMatrix4(1.0f, 0.0f, 0.0f, 0.0f,
                       0.0f,  N.y,  N.x, 0.0f,
                       0.0f, -N.x,  N.y, 0.0f,
                       0.0f, 0.0f, 0.0f, 1.0f);
}

coreMatrix4 coreMatrix4::RotationX(const float& fAngle)noexcept
{
    return coreMatrix4::RotationX(coreVector2::Direction(fAngle));
}


// ****************************************************************
// get rotation matrix around Y
coreMatrix4 coreMatrix4::RotationY(const coreVector2& vDirection)noexcept
{
    if(!vDirection.x && !vDirection.y) return coreMatrix4::Identity();

    const coreVector2 N = vDirection.Normalized();
    return coreMatrix4( N.y, 0.0f, -N.x, 0.0f,
                       0.0f, 1.0f, 0.0f, 0.0f,
                        N.x, 0.0f,  N.y, 0.0f,
                       0.0f, 0.0f, 0.0f, 1.0f);
}

coreMatrix4 coreMatrix4::RotationY(const float& fAngle)noexcept
{
    return coreMatrix4::RotationY(coreVector2::Direction(fAngle));
}


// ****************************************************************
// get rotation matrix around Z
coreMatrix4 coreMatrix4::RotationZ(const coreVector2& vDirection)noexcept
{
    if(!vDirection.x && !vDirection.y) return coreMatrix4::Identity();

    const coreVector2 N = vDirection.Normalized();
    return coreMatrix4( N.y,  N.x, 0.0f, 0.0f,
                       -N.x,  N.y, 0.0f, 0.0f,
                       0.0f, 0.0f, 1.0f, 0.0f,
                       0.0f, 0.0f, 0.0f, 1.0f);
}

coreMatrix4 coreMatrix4::RotationZ(const float& fAngle)noexcept
{
    return coreMatrix4::RotationZ(coreVector2::Direction(fAngle));
}


// ****************************************************************
// get orientation matrix
coreMatrix4 coreMatrix4::Orientation(const coreVector3& vDirection, const coreVector3& vOrientation)noexcept
{
    const coreVector3 F =  vDirection.Normalized();
    const coreVector3 O = -vOrientation.Normalized();
    const coreVector3 S =  coreVector3::Cross(O, F).Normalize();
    const coreVector3 U =  coreVector3::Cross(S, F).Normalize();

    return coreMatrix4( S.x,  U.x, -F.x, 0.0f,
                        S.y,  U.y, -F.y, 0.0f,
                        S.z,  U.z, -F.z, 0.0f,
                       0.0f, 0.0f, 0.0f, 1.0f);
}


// ****************************************************************
// get perspective matrix
coreMatrix4 coreMatrix4::Perspective(const coreVector2& vResolution, const float& fFOV, const float& fNearClip, const float& fFarClip)noexcept
{
    const float  V = 1.0f / coreMath::Tan(0.5f*fFOV);
    const float  A = vResolution.yx().AspectRatio();
    const float& N = fNearClip;
    const float& F = fFarClip;

    const float INF = 1.0f / (N-F);

    return coreMatrix4( V*A, 0.0f,         0.0f,  0.0f,
                       0.0f,    V,         0.0f,  0.0f,
                       0.0f, 0.0f,    (N+F)*INF, -1.0f,
                       0.0f, 0.0f, 2.0f*N*F*INF,  0.0f);
}


// ****************************************************************
// get ortho matrix
coreMatrix4 coreMatrix4::Ortho(const coreVector2& vResolution)noexcept
{
    const         float L = -vResolution.x*0.5f;
    const         float R =  vResolution.x*0.5f;
    const         float B = -vResolution.y*0.5f;
    const         float T =  vResolution.y*0.5f;
    constexpr_var float N = -32.0f;
    constexpr_var float F = 128.0f;

    const         float IRL = 1.0f / (R-L);
    const         float ITN = 1.0f / (T-B);
    constexpr_var float IFN = 1.0f / (F-N);

    return coreMatrix4(  2.0f*IRL,       0.0f,       0.0f, 0.0f,
                             0.0f,   2.0f*ITN,       0.0f, 0.0f,
                             0.0f,       0.0f,  -2.0f*IFN, 0.0f,
                       -(R+L)*IRL, -(T+B)*ITN, -(F+N)*IFN, 1.0f);
}


// ****************************************************************
// get camera matrix
coreMatrix4 coreMatrix4::Camera(const coreVector3& vPosition, const coreVector3& vDirection, const coreVector3& vOrientation)noexcept
{
    return coreMatrix4::Translation(-vPosition) * coreMatrix4::Orientation(vDirection, vOrientation);
}