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
coreMatrix::coreMatrix()
: _11 (1.0f), _12 (0.0f), _13 (0.0f), _14 (0.0f)
, _21 (0.0f), _22 (1.0f), _23 (0.0f), _24 (0.0f)
, _31 (0.0f), _32 (0.0f), _33 (1.0f), _34 (0.0f)
, _41 (0.0f), _42 (0.0f), _43 (0.0f), _44 (1.0f)
{
}

coreMatrix::coreMatrix(const float& f11, const float& f12, const float& f13, const float& f14,
                       const float& f21, const float& f22, const float& f23, const float& f24,
                       const float& f31, const float& f32, const float& f33, const float& f34,
                       const float& f41, const float& f42, const float& f43, const float& f44)
: _11 (f11), _12 (f12), _13 (f13), _14 (f14)
, _21 (f21), _22 (f22), _23 (f23), _24 (f24)
, _31 (f31), _32 (f32), _33 (f33), _34 (f34)
, _41 (f41), _42 (f42), _43 (f43), _44 (f44)
{
}


// ****************************************************************
// compare with matrix
bool coreMatrix::operator == (const coreMatrix& v)const
{
    // TODO: add SSE-support
    return ((_11 == v._11) && (_12 == v._12) && (_13 == v._13) && (_14 == v._14) &&
            (_21 == v._21) && (_22 == v._22) && (_23 == v._23) && (_24 == v._24) &&
            (_31 == v._31) && (_32 == v._32) && (_33 == v._33) && (_34 == v._34) &&
            (_41 == v._41) && (_42 == v._42) && (_43 == v._43) && (_44 == v._44));
}


// ****************************************************************
// inverse compare with matrix
bool coreMatrix::operator != (const coreMatrix& v)const
{
    // TODO: add SSE-support
    return ((_11 != v._11) || (_12 != v._12) || (_13 != v._13) || (_14 != v._14) ||
            (_21 != v._21) || (_22 != v._22) || (_23 != v._23) || (_24 != v._24) ||
            (_31 != v._31) || (_32 != v._32) || (_33 != v._33) || (_34 != v._34) ||
            (_41 != v._41) || (_42 != v._42) || (_43 != v._43) || (_44 != v._44));
}


// ****************************************************************
// addition with matrix
coreMatrix coreMatrix::operator + (const coreMatrix& v)const
{
    // TODO: add SSE-support
    return coreMatrix(_11+v._11, _12+v._12, _13+v._13, _14+v._14,
                      _21+v._21, _22+v._22, _23+v._23, _24+v._24,
                      _31+v._31, _32+v._32, _33+v._33, _34+v._34,
                      _41+v._41, _42+v._42, _43+v._43, _44+v._44);
}


// ****************************************************************
// subtraction with matrix
coreMatrix coreMatrix::operator - (const coreMatrix& v)const
{
    // TODO: add SSE-support
    return coreMatrix(_11-v._11, _12-v._12, _13-v._13, _14-v._14,
                      _21-v._21, _22-v._22, _23-v._23, _24-v._24,
                      _31-v._31, _32-v._32, _33-v._33, _34-v._34,
                      _41-v._41, _42-v._42, _43-v._43, _44-v._44);
}


// ****************************************************************
// multiplication with matrix
coreMatrix coreMatrix::operator * (const coreMatrix& v)const
{
#if defined(_CORE_SSE_)

    // optimized
    if(Core::System->SupportSSE41())
    {
        const coreMatrix mThis  = *this;
        const coreMatrix mInput = v.Transposed();
        coreMatrix mOutput;

        for(int i = 0; i < 4; ++i)
            for(int j = 0; j < 4; ++j)
                _mm_store_ss(&mOutput.m[i][j], _mm_dp_ps(_mm_load_ps(&mThis.m[i][0]), _mm_load_ps(&mInput.m[j][0]), 0xF1));

        return mOutput;
    }

#endif

    // normal
    return coreMatrix(_11*v._11 + _12*v._21 + _13*v._31 + _14*v._41, _11*v._12 + _12*v._22 + _13*v._32 + _14*v._42,
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
coreMatrix coreMatrix::operator * (const float& f)const
{
    // TODO: add SSE-support
    return coreMatrix(_11*f, _12*f, _13*f, _14*f,
                      _21*f, _22*f, _23*f, _24*f,
                      _31*f, _32*f, _33*f, _34*f,
                      _41*f, _42*f, _43*f, _44*f);
}


// ****************************************************************
// division with scalar
coreMatrix coreMatrix::operator / (const float& f)const
{
    // TODO: add SSE-support
    return (*this)*(1.0f/f);
}


// ****************************************************************
// transpose matrix
coreMatrix& coreMatrix::Transpose()
{
    *this = coreMatrix(_11, _21, _31, _41,
                       _12, _22, _32, _42,
                       _13, _23, _33, _43,
                       _14, _24, _34, _44);

    return *this;
}


// ****************************************************************
// invert matrix
coreMatrix& coreMatrix::Invert()
{
    // TODO: add SSE-support
    *this = coreMatrix(_23*_34*_42 - _24*_33*_42 + _24*_32*_43 - _22*_34*_43 - _23*_32*_44 + _22*_33*_44,
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
float coreMatrix::Determinant()const
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
// get identity matrix
coreMatrix coreMatrix::Identity()
{
    return coreMatrix(1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);
}


// ****************************************************************
// get translation matrix
coreMatrix coreMatrix::Translation(const coreVector3& vPosition)
{
    return coreMatrix(       1.0f,        0.0f,        0.0f, 0.0f,
                             0.0f,        1.0f,        0.0f, 0.0f,
                             0.0f,        0.0f,        1.0f, 0.0f,
                      vPosition.x, vPosition.y, vPosition.z, 1.0f);
}


// ****************************************************************
// get scale matrix
coreMatrix coreMatrix::Scaling(const coreVector3& vSize)
{
    return coreMatrix(vSize.x,    0.0f,    0.0f, 0.0f,
                         0.0f, vSize.y,    0.0f, 0.0f,
                         0.0f,    0.0f, vSize.z, 0.0f,
                         0.0f,    0.0f,    0.0f, 1.0f);
}


// ****************************************************************
// get rotation matrix around X
coreMatrix coreMatrix::RotationX(const coreVector2& vDirection)
{
    if(!vDirection.x && !vDirection.y) return coreMatrix::Identity();

    const coreVector2 N = vDirection.Normalized();

    return coreMatrix(1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f,  N.y,  N.x, 0.0f,
                      0.0f, -N.x,  N.y, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);
}

coreMatrix coreMatrix::RotationX(const float& fAngle)
{
    return coreMatrix::RotationX(coreVector2::Direction(fAngle));
}


// ****************************************************************
// get rotation matrix around Y
coreMatrix coreMatrix::RotationY(const coreVector2& vDirection)
{
    if(!vDirection.x && !vDirection.y) return coreMatrix::Identity();

    const coreVector2 N = vDirection.Normalized();

    return coreMatrix( N.y, 0.0f, -N.x, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                       N.x, 0.0f,  N.y, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);
}

coreMatrix coreMatrix::RotationY(const float& fAngle)
{
    return coreMatrix::RotationY(coreVector2::Direction(fAngle));
}


// ****************************************************************
// get rotation matrix around Z
coreMatrix coreMatrix::RotationZ(const coreVector2& vDirection)
{
    if(!vDirection.x && !vDirection.y) return coreMatrix::Identity();

    const coreVector2 N = vDirection.Normalized();

    return coreMatrix( N.y,  N.x, 0.0f, 0.0f,
                      -N.x,  N.y, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);
}

coreMatrix coreMatrix::RotationZ(const float& fAngle)
{
    return coreMatrix::RotationZ(coreVector2::Direction(fAngle));
}


// ****************************************************************
// get orientation matrix
coreMatrix coreMatrix::Orientation(const coreVector3& vDirection, const coreVector3& vOrientation)
{
    const coreVector3 F =  vDirection.Normalized();
    const coreVector3 O = -vOrientation.Normalized();
    const coreVector3 S =  coreVector3::Cross(O, F).Normalize();
    const coreVector3 U =  coreVector3::Cross(S, F).Normalize();

    return coreMatrix( S.x,  U.x, -F.x, 0.0f,
                       S.y,  U.y, -F.y, 0.0f,
                       S.z,  U.z, -F.z, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);
}


// ****************************************************************
// get perspective matrix
coreMatrix coreMatrix::Perspective(const coreVector2& vResolution, const float& fFOV, const float& fNearClip, const float& fFarClip)
{
    const float  V = 1.0f / tanf(0.5f*fFOV);
    const float  A = vResolution.yx().AspectRatio();
    const float& N = fNearClip;
    const float& F = fFarClip;

    const float INF = 1.0f / (N-F);

    return coreMatrix( V*A, 0.0f,         0.0f,  0.0f,
                      0.0f,    V,         0.0f,  0.0f,
                      0.0f, 0.0f,    (N+F)*INF, -1.0f,
                      0.0f, 0.0f, 2.0f*N*F*INF,  0.0f);
}


// ****************************************************************
// get ortho matrix
coreMatrix coreMatrix::Ortho(const coreVector2& vResolution)
{
    const float L = -vResolution.x*0.5f;
    const float R =  vResolution.x*0.5f;
    const float B = -vResolution.y*0.5f;
    const float T =  vResolution.y*0.5f;
    const float N = -32.0f;
    const float F = 128.0f;

    const float IRL = 1.0f / (R-L);
    const float ITN = 1.0f / (T-B);
    const float IFN = 1.0f / (F-N);

    return coreMatrix(  2.0f*IRL,       0.0f,       0.0f, 0.0f,
                            0.0f,   2.0f*ITN,       0.0f, 0.0f,
                            0.0f,       0.0f,  -2.0f*IFN, 0.0f,
                      -(R+L)*IRL, -(T+B)*ITN, -(F+N)*IFN, 1.0f);
}


// ****************************************************************
// get camera matrix
coreMatrix coreMatrix::Camera(const coreVector3& vPosition, const coreVector3& vDirection, const coreVector3& vOrientation)
{
    return coreMatrix::Translation(-vPosition) * coreMatrix::Orientation(vDirection, vOrientation);
}