//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MATRIX_H_
#define _CORE_GUARD_MATRIX_H_


// ****************************************************************
// 3x3-matrix class
// TODO: implement all missing methods
// TODO: use 3x3 matrix where only 3 dimensions are required (Object2D, Rotation)
class coreMatrix3 final
{
public:
    union
    {
        struct
        {
            float _11, _12, _13;
            float _21, _22, _23;
            float _31, _32, _33;
        };
        float m[3][3];
    };


public:
    constexpr_func coreMatrix3()noexcept;
    constexpr_func coreMatrix3(const float& f11, const float& f12, const float& f13,
                               const float& f21, const float& f22, const float& f23,
                               const float& f31, const float& f32, const float& f33)noexcept;

    //! scalar calculation operators
    //! @{
    constexpr_func coreMatrix3 operator * (const float& f)const noexcept;
    constexpr_func coreMatrix3 operator / (const float& f)const noexcept                        {return *this * (1.0f/f);}
    inline void operator *= (const float& f)noexcept                                            {*this = *this * f;}
    inline void operator /= (const float& f)noexcept                                            {*this = *this / f;}
    friend constexpr_func coreMatrix3 operator * (const float& f, const coreMatrix3& v)noexcept {return v * f;}
    //! @}

    //! convert matrix
    //! @{
    constexpr_func operator const float* ()const noexcept {return r_cast<const float*>(this);}
    //! @}

    //! transpose matrix
    //! @{
    inline coreMatrix3& Transpose()noexcept;
    inline coreMatrix3 Transposed()const noexcept {return coreMatrix3(*this).Transpose();}
    //! @}

    //! inverse matrix
    //! @{
    inline coreMatrix3& Inverse()noexcept;
    inline coreMatrix3 Inversed()const noexcept {return coreMatrix3(*this).Inverse();}
    //! @}

    //! direct functions
    //! @{
    constexpr_func float Determinant()const noexcept;
    //! @}
};


// ****************************************************************
// 4x4-matrix class
// TODO: do not multiply always the full matrices (e.g. translation is only addition to 3 values)
class coreMatrix4 final
{
public:
    union
    {
        struct
        {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };


public:
    constexpr_func coreMatrix4()noexcept;
    constexpr_func coreMatrix4(const float& f11, const float& f12, const float& f13, const float& f14,
                               const float& f21, const float& f22, const float& f23, const float& f24,
                               const float& f31, const float& f32, const float& f33, const float& f34,
                               const float& f41, const float& f42, const float& f43, const float& f44)noexcept;

    //! compare operators
    //! @{
    inline bool operator == (const coreMatrix4& v)const noexcept {return std::memcmp(this, &v, sizeof(coreMatrix4)) ? false :  true;}
    inline bool operator != (const coreMatrix4& v)const noexcept {return std::memcmp(this, &v, sizeof(coreMatrix4)) ?  true : false;}
    //! @}

    //! matrix calculation operators
    //! @{
    constexpr_func coreMatrix4 operator + (const coreMatrix4& v)const noexcept;
    constexpr_func coreMatrix4 operator - (const coreMatrix4& v)const noexcept;
    constexpr_func coreMatrix4 operator * (const coreMatrix4& v)const noexcept;
    inline void operator += (const coreMatrix4& v)noexcept {*this = *this + v;}
    inline void operator -= (const coreMatrix4& v)noexcept {*this = *this - v;}
    inline void operator *= (const coreMatrix4& v)noexcept {*this = *this * v;}
    //! @}

    //! scalar calculation operators
    //! @{
    constexpr_func coreMatrix4 operator * (const float& f)const noexcept;
    constexpr_func coreMatrix4 operator / (const float& f)const noexcept                        {return *this * (1.0f/f);}
    inline void operator *= (const float& f)noexcept                                            {*this = *this * f;}
    inline void operator /= (const float& f)noexcept                                            {*this = *this / f;}
    friend constexpr_func coreMatrix4 operator * (const float& f, const coreMatrix4& v)noexcept {return v * f;}
    //! @}

    //! convert matrix
    //! @{
    constexpr_func operator const float* ()const noexcept {return r_cast<const float*>(this);}
    constexpr_func coreMatrix3 m123()const noexcept       {return coreMatrix3(_11, _12, _13, _21, _22, _23, _31, _32, _33);}
    constexpr_func coreMatrix3 m124()const noexcept       {return coreMatrix3(_11, _12, _14, _21, _22, _24, _41, _42, _44);}
    constexpr_func coreMatrix3 m134()const noexcept       {return coreMatrix3(_11, _13, _14, _31, _33, _34, _41, _43, _44);}
    constexpr_func coreMatrix3 m234()const noexcept       {return coreMatrix3(_22, _23, _24, _32, _33, _34, _42, _43, _44);}
    //! @}

    //! transpose matrix
    //! @{
    inline coreMatrix4& Transpose()noexcept;
    inline coreMatrix4 Transposed()const noexcept {return coreMatrix4(*this).Transpose();}
    //! @}

    //! inverse matrix
    //! @{
    inline coreMatrix4& Inverse()noexcept;
    inline coreMatrix4 Inversed()const noexcept {return coreMatrix4(*this).Inverse();}
    //! @}

    //! direct functions
    //! @{
    constexpr_func float Determinant()const noexcept;
    //! @}

    //! static functions
    //! @{
    static constexpr_func coreMatrix4 Identity()noexcept;
    static constexpr_func coreMatrix4 Translation(const coreVector3& vPosition)noexcept;
    static constexpr_func coreMatrix4 Scaling(const coreVector3& vSize)noexcept;
    static inline coreMatrix4 RotationX(const coreVector2& vDirection)noexcept;
    static inline coreMatrix4 RotationX(const float& fAngle)noexcept;
    static inline coreMatrix4 RotationY(const coreVector2& vDirection)noexcept;
    static inline coreMatrix4 RotationY(const float& fAngle)noexcept;
    static inline coreMatrix4 RotationZ(const coreVector2& vDirection)noexcept;
    static inline coreMatrix4 RotationZ(const float& fAngle)noexcept;
    static inline coreMatrix4 Orientation(const coreVector3& vDirection, const coreVector3& vOrintation)noexcept;
    static inline coreMatrix4 Perspective(const coreVector2& vResolution, const float& fFOV, const float& fNearClip, const float& fFarClip)noexcept;
    static inline coreMatrix4 Ortho(const coreVector2& vResolution)noexcept;
    static inline coreMatrix4 Camera(const coreVector3& vPosition, const coreVector3& vDirection, const coreVector3& vOrintation)noexcept;
    //! @}
};



// ****************************************************************
// constructor
constexpr_func coreMatrix3::coreMatrix3()noexcept
: _11 (1.0f), _12 (0.0f), _13 (0.0f)
, _21 (0.0f), _22 (1.0f), _23 (0.0f)
, _31 (0.0f), _32 (0.0f), _33 (1.0f)
{
}

constexpr_func coreMatrix3::coreMatrix3(const float& f11, const float& f12, const float& f13,
                                        const float& f21, const float& f22, const float& f23,
                                        const float& f31, const float& f32, const float& f33)noexcept
: _11 (f11), _12 (f12), _13 (f13)
, _21 (f21), _22 (f22), _23 (f23)
, _31 (f31), _32 (f32), _33 (f33)
{
}


// ****************************************************************
// multiplication with scalar
constexpr_func coreMatrix3 coreMatrix3::operator * (const float& f)const noexcept
{
    return coreMatrix3(_11*f, _12*f, _13*f,
                       _21*f, _22*f, _23*f,
                       _31*f, _32*f, _33*f);
}


// ****************************************************************
// transpose matrix
inline coreMatrix3& coreMatrix3::Transpose()noexcept
{
    *this = coreMatrix3(_11, _21, _31,
                        _12, _22, _32,
                        _13, _23, _33);

    return *this;
}


// ****************************************************************
// inverse matrix
inline coreMatrix3& coreMatrix3::Inverse()noexcept
{
    *this = coreMatrix3(_22*_33 - _23*_32, _13*_32 - _12*_33, _12*_23 - _13*_22,
                        _23*_31 - _21*_33, _11*_33 - _13*_31, _13*_21 - _11*_23,
                        _21*_32 - _22*_31, _13*_31 - _11*_32, _11*_22 - _12*_21)
                        / this->Determinant();

    return *this;
}


// ****************************************************************
// calculate determinant
constexpr_func float coreMatrix3::Determinant()const noexcept
{
    return _11*_22*_33 + _12*_23*_31 + _13*_21*_32 - _13*_22*_31 - _12*_21*_33 - _11*_23*_32;
}


// ****************************************************************
// constructor
constexpr_func coreMatrix4::coreMatrix4()noexcept
: _11 (1.0f), _12 (0.0f), _13 (0.0f), _14 (0.0f)
, _21 (0.0f), _22 (1.0f), _23 (0.0f), _24 (0.0f)
, _31 (0.0f), _32 (0.0f), _33 (1.0f), _34 (0.0f)
, _41 (0.0f), _42 (0.0f), _43 (0.0f), _44 (1.0f)
{
}

constexpr_func coreMatrix4::coreMatrix4(const float& f11, const float& f12, const float& f13, const float& f14,
                                        const float& f21, const float& f22, const float& f23, const float& f24,
                                        const float& f31, const float& f32, const float& f33, const float& f34,
                                        const float& f41, const float& f42, const float& f43, const float& f44)noexcept
: _11 (f11), _12 (f12), _13 (f13), _14 (f14)
, _21 (f21), _22 (f22), _23 (f23), _24 (f24)
, _31 (f31), _32 (f32), _33 (f33), _34 (f34)
, _41 (f41), _42 (f42), _43 (f43), _44 (f44)
{
}


// ****************************************************************
// addition with matrix
constexpr_func coreMatrix4 coreMatrix4::operator + (const coreMatrix4& v)const noexcept
{
    return coreMatrix4(_11+v._11, _12+v._12, _13+v._13, _14+v._14,
                       _21+v._21, _22+v._22, _23+v._23, _24+v._24,
                       _31+v._31, _32+v._32, _33+v._33, _34+v._34,
                       _41+v._41, _42+v._42, _43+v._43, _44+v._44);
}


// ****************************************************************
// subtraction with matrix
constexpr_func coreMatrix4 coreMatrix4::operator - (const coreMatrix4& v)const noexcept
{
    return coreMatrix4(_11-v._11, _12-v._12, _13-v._13, _14-v._14,
                       _21-v._21, _22-v._22, _23-v._23, _24-v._24,
                       _31-v._31, _32-v._32, _33-v._33, _34-v._34,
                       _41-v._41, _42-v._42, _43-v._43, _44-v._44);
}


// ****************************************************************
// multiplication with matrix
constexpr_func coreMatrix4 coreMatrix4::operator * (const coreMatrix4& v)const noexcept
{
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
constexpr_func coreMatrix4 coreMatrix4::operator * (const float& f)const noexcept
{
    return coreMatrix4(_11*f, _12*f, _13*f, _14*f,
                       _21*f, _22*f, _23*f, _24*f,
                       _31*f, _32*f, _33*f, _34*f,
                       _41*f, _42*f, _43*f, _44*f);
}


// ****************************************************************
// transpose matrix
inline coreMatrix4& coreMatrix4::Transpose()noexcept
{
    *this = coreMatrix4(_11, _21, _31, _41,
                        _12, _22, _32, _42,
                        _13, _23, _33, _43,
                        _14, _24, _34, _44);

    return *this;
}


// ****************************************************************
// inverse matrix
inline coreMatrix4& coreMatrix4::Inverse()noexcept
{
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
// calculate determinant
constexpr_func float coreMatrix4::Determinant()const noexcept
{
    return _14*_23*_32*_41 - _13*_24*_32*_41 - _14*_22*_33*_41 + _12*_24*_33*_41 +
           _13*_22*_34*_41 - _12*_23*_34*_41 - _14*_23*_31*_42 + _13*_24*_31*_42 +
           _14*_21*_33*_42 - _11*_24*_33*_42 - _13*_21*_34*_42 + _11*_23*_34*_42 +
           _14*_22*_31*_43 - _12*_24*_31*_43 - _14*_21*_32*_43 + _11*_24*_32*_43 +
           _12*_21*_34*_43 - _11*_22*_34*_43 - _13*_22*_31*_44 + _12*_23*_31*_44 +
           _13*_21*_32*_44 - _11*_23*_32*_44 - _12*_21*_33*_44 + _11*_22*_33*_44;
}


// ****************************************************************
// get identity matrix
constexpr_func coreMatrix4 coreMatrix4::Identity()noexcept
{
    return coreMatrix4(1.0f, 0.0f, 0.0f, 0.0f,
                       0.0f, 1.0f, 0.0f, 0.0f,
                       0.0f, 0.0f, 1.0f, 0.0f,
                       0.0f, 0.0f, 0.0f, 1.0f);
}


// ****************************************************************
// get translation matrix
constexpr_func coreMatrix4 coreMatrix4::Translation(const coreVector3& vPosition)noexcept
{
    return coreMatrix4(       1.0f,        0.0f,        0.0f, 0.0f,
                              0.0f,        1.0f,        0.0f, 0.0f,
                              0.0f,        0.0f,        1.0f, 0.0f,
                       vPosition.x, vPosition.y, vPosition.z, 1.0f);
}


// ****************************************************************
// get scale matrix
constexpr_func coreMatrix4 coreMatrix4::Scaling(const coreVector3& vSize)noexcept
{
    return coreMatrix4(vSize.x,    0.0f,    0.0f, 0.0f,
                          0.0f, vSize.y,    0.0f, 0.0f,
                          0.0f,    0.0f, vSize.z, 0.0f,
                          0.0f,    0.0f,    0.0f, 1.0f);
}


// ****************************************************************
// get rotation matrix around X
inline coreMatrix4 coreMatrix4::RotationX(const coreVector2& vDirection)noexcept
{
    const coreVector2 N = vDirection.Normalized();
    return coreMatrix4(1.0f, 0.0f, 0.0f, 0.0f,
                       0.0f,  N.y,  N.x, 0.0f,
                       0.0f, -N.x,  N.y, 0.0f,
                       0.0f, 0.0f, 0.0f, 1.0f);
}

inline coreMatrix4 coreMatrix4::RotationX(const float& fAngle)noexcept
{
    return coreMatrix4::RotationX(coreVector2::Direction(fAngle));
}


// ****************************************************************
// get rotation matrix around Y
inline coreMatrix4 coreMatrix4::RotationY(const coreVector2& vDirection)noexcept
{
    const coreVector2 N = vDirection.Normalized();
    return coreMatrix4( N.y, 0.0f, -N.x, 0.0f,
                       0.0f, 1.0f, 0.0f, 0.0f,
                        N.x, 0.0f,  N.y, 0.0f,
                       0.0f, 0.0f, 0.0f, 1.0f);
}

inline coreMatrix4 coreMatrix4::RotationY(const float& fAngle)noexcept
{
    return coreMatrix4::RotationY(coreVector2::Direction(fAngle));
}


// ****************************************************************
// get rotation matrix around Z
inline coreMatrix4 coreMatrix4::RotationZ(const coreVector2& vDirection)noexcept
{
    const coreVector2 N = vDirection.Normalized();
    return coreMatrix4( N.y,  N.x, 0.0f, 0.0f,
                       -N.x,  N.y, 0.0f, 0.0f,
                       0.0f, 0.0f, 1.0f, 0.0f,
                       0.0f, 0.0f, 0.0f, 1.0f);
}

inline coreMatrix4 coreMatrix4::RotationZ(const float& fAngle)noexcept
{
    return coreMatrix4::RotationZ(coreVector2::Direction(fAngle));
}


// ****************************************************************
// calculate orientation matrix
inline coreMatrix4 coreMatrix4::Orientation(const coreVector3& vDirection, const coreVector3& vOrientation)noexcept
{
    const coreVector3 F = -vDirection.Normalized();
    const coreVector3 O =  vOrientation.Normalized();
    const coreVector3 S =  coreVector3::Cross(O, F).Normalize();
    const coreVector3 U = -coreVector3::Cross(S, F).Normalize();

    return coreMatrix4( S.x,  S.y,  S.z, 0.0f,
                        U.x,  U.y,  U.z, 0.0f,
                        F.x,  F.y,  F.z, 0.0f,
                       0.0f, 0.0f, 0.0f, 1.0f);
}


// ****************************************************************
// calculate perspective matrix
inline coreMatrix4 coreMatrix4::Perspective(const coreVector2& vResolution, const float& fFOV, const float& fNearClip, const float& fFarClip)noexcept
{
    const float  V = coreMath::Cot(fFOV*0.5f);
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
// calculate ortho matrix
inline coreMatrix4 coreMatrix4::Ortho(const coreVector2& vResolution)noexcept
{
    constexpr_var float N = -32.0f;
    constexpr_var float F = 128.0f;

    const         float IRL = 1.0f / vResolution.x;
    const         float ITB = 1.0f / vResolution.y;
    constexpr_var float IFN = 1.0f / (F-N);

    return coreMatrix4(2.0f*IRL,     0.0f,       0.0f, 0.0f,
                           0.0f, 2.0f*ITB,       0.0f, 0.0f,
                           0.0f,     0.0f,  -2.0f*IFN, 0.0f,
                           0.0f,     0.0f, -(F+N)*IFN, 1.0f);
}


// ****************************************************************
// calculate camera matrix
// TODO: you know, what to do (fix camera and orientation)
inline coreMatrix4 coreMatrix4::Camera(const coreVector3& vPosition, const coreVector3& vDirection, const coreVector3& vOrientation)noexcept
{
    const coreVector3 F =  vDirection.Normalized();
    const coreVector3 O = -vOrientation.Normalized();
    const coreVector3 S =  coreVector3::Cross(O, F).Normalize();
    const coreVector3 U =  coreVector3::Cross(S, F).Normalize();

    return coreMatrix4::Translation(-vPosition) * coreMatrix4( S.x,  U.x, -F.x, 0.0f,
                                                               S.y,  U.y, -F.y, 0.0f,
                                                               S.z,  U.z, -F.z, 0.0f,
                                                              0.0f, 0.0f, 0.0f, 1.0f);
}


// ****************************************************************
// multiplication with matrix
inline coreVector2 coreVector2::operator * (const coreMatrix4& m)const noexcept
{
    const float w = 1.0f / (x*m._14 + y*m._24 + m._44);
    return coreVector2(x*m._11 + y*m._21 + w*m._41,
                       x*m._12 + y*m._22 + w*m._42);
}


// ****************************************************************
// multiplication with matrix
inline coreVector3 coreVector3::operator * (const coreMatrix4& m)const noexcept
{
    const float w = 1.0f / (x*m._14 + y*m._24 + z*m._34 + m._44);
    return coreVector3(x*m._11 + y*m._21 + z*m._31 + w*m._41,
                       x*m._12 + y*m._22 + z*m._32 + w*m._42,
                       x*m._13 + y*m._23 + z*m._33 + w*m._43);
}


// ****************************************************************
// multiplication with matrix
constexpr_func coreVector4 coreVector4::operator * (const coreMatrix4& m)const noexcept
{
    return coreVector4(x*m._11 + y*m._21 + z*m._31 + w*m._41,
                       x*m._12 + y*m._22 + z*m._32 + w*m._42,
                       x*m._13 + y*m._23 + z*m._33 + w*m._43,
                       x*m._14 + y*m._24 + z*m._34 + w*m._44);
}


#endif // _CORE_GUARD_MATRIX_H_