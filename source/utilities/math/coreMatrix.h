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

// TODO: do not multiply always full matrices


// ****************************************************************
/* 3x3-matrix class */
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

    /*! compare operations */
    //! @{
    inline bool operator == (const coreMatrix3& v)const {return std::memcmp(this, &v, sizeof(coreMatrix3)) ? false :  true;}
    inline bool operator != (const coreMatrix3& v)const {return std::memcmp(this, &v, sizeof(coreMatrix3)) ?  true : false;}
    //! @}

    /*! matrix operations */
    //! @{
    constexpr_func coreMatrix3 operator +  (const coreMatrix3& v)const;
    constexpr_func coreMatrix3 operator -  (const coreMatrix3& v)const;
    constexpr_func coreMatrix3 operator *  (const coreMatrix3& v)const;
    inline         void        operator += (const coreMatrix3& v) {*this = *this + v;}
    inline         void        operator -= (const coreMatrix3& v) {*this = *this - v;}
    inline         void        operator *= (const coreMatrix3& v) {*this = *this * v;}
    //! @}

    /*! scalar operations */
    //! @{
    constexpr_func        coreMatrix3 operator *  (const float& f)const;
    inline                coreMatrix3 operator /  (const float& f)const                  {return  *this * RCP(f);}
    inline                void        operator *= (const float& f)                       {*this = *this * f;}
    inline                void        operator /= (const float& f)                       {*this = *this / f;}
    friend constexpr_func coreMatrix3 operator *  (const float& f, const coreMatrix3& v) {return v * f;}
    //! @}

    /*! convert matrix */
    //! @{
    constexpr_obj operator const float* ()const {return r_cast<const float*>(this);}
    //! @}

    /*! transpose matrix */
    //! @{
    inline coreMatrix3& Transpose();
    inline coreMatrix3  Transposed()const {return coreMatrix3(*this).Transpose();}
    //! @}

    /*! invert matrix */
    //! @{
    inline coreMatrix3& Invert();
    inline coreMatrix3  Inverted()const {return coreMatrix3(*this).Invert();}
    //! @}

    /*! direct functions */
    //! @{
    constexpr_func float Determinant()const;
    //! @}
};


// ****************************************************************
/* 4x4-matrix class */
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

    /*! compare operations */
    //! @{
    inline bool operator == (const coreMatrix4& v)const {return std::memcmp(this, &v, sizeof(coreMatrix4)) ? false :  true;}
    inline bool operator != (const coreMatrix4& v)const {return std::memcmp(this, &v, sizeof(coreMatrix4)) ?  true : false;}
    //! @}

    /*! matrix operations */
    //! @{
    constexpr_func coreMatrix4 operator +  (const coreMatrix4& v)const;
    constexpr_func coreMatrix4 operator -  (const coreMatrix4& v)const;
    constexpr_func coreMatrix4 operator *  (const coreMatrix4& v)const hot_func;
    inline         void        operator += (const coreMatrix4& v) {*this = *this + v;}
    inline         void        operator -= (const coreMatrix4& v) {*this = *this - v;}
    inline         void        operator *= (const coreMatrix4& v) {*this = *this * v;}
    //! @}

    /*! scalar operations */
    //! @{
    constexpr_func        coreMatrix4 operator *  (const float& f)const;
    inline                coreMatrix4 operator /  (const float& f)const                  {return  *this * RCP(f);}
    inline                void        operator *= (const float& f)                       {*this = *this * f;}
    inline                void        operator /= (const float& f)                       {*this = *this / f;}
    friend constexpr_func coreMatrix4 operator *  (const float& f, const coreMatrix4& v) {return v * f;}
    //! @}

    /*! convert matrix */
    //! @{
    constexpr_obj operator const float* ()const {return r_cast<const float*>(this);}
    constexpr_func coreMatrix3 m123()const      {return coreMatrix3(_11, _12, _13, _21, _22, _23, _31, _32, _33);}
    constexpr_func coreMatrix3 m124()const      {return coreMatrix3(_11, _12, _14, _21, _22, _24, _41, _42, _44);}
    constexpr_func coreMatrix3 m134()const      {return coreMatrix3(_11, _13, _14, _31, _33, _34, _41, _43, _44);}
    constexpr_func coreMatrix3 m234()const      {return coreMatrix3(_22, _23, _24, _32, _33, _34, _42, _43, _44);}
    //! @}

    /*! transpose matrix */
    //! @{
    inline coreMatrix4& Transpose();
    inline coreMatrix4  Transposed()const {return coreMatrix4(*this).Transpose();}
    //! @}

    /*! invert matrix */
    //! @{
    inline coreMatrix4& Invert();
    inline coreMatrix4  Inverted()const {return coreMatrix4(*this).Invert();}
    //! @}

    /*! direct functions */
    //! @{
    constexpr_func float Determinant()const;
    //! @}

    /*! static functions */
    //! @{
    static constexpr_func coreMatrix4 Identity   ();
    static constexpr_func coreMatrix4 Translation(const coreVector3& vPosition);
    static constexpr_func coreMatrix4 Scaling    (const coreVector3& vSize);
    static inline         coreMatrix4 RotationX  (const coreVector2& vDirection);
    static inline         coreMatrix4 RotationX  (const float&       fAngle);
    static inline         coreMatrix4 RotationY  (const coreVector2& vDirection);
    static inline         coreMatrix4 RotationY  (const float&       fAngle);
    static inline         coreMatrix4 RotationZ  (const coreVector2& vDirection);
    static inline         coreMatrix4 RotationZ  (const float&       fAngle);
    static inline         coreMatrix4 Orientation(const coreVector3& vDirection, const coreVector3& vOrientation);
    static inline         coreMatrix4 Perspective(const coreVector2& vResolution, const float& fFOV, const float& fNearClip, const float& fFarClip);
    static inline         coreMatrix4 Ortho      (const coreVector2& vResolution);
    static inline         coreMatrix4 Camera     (const coreVector3& vPosition, const coreVector3& vDirection, const coreVector3& vOrientation);
    //! @}
};



// ****************************************************************
/* constructor */
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
/* addition with matrix */
constexpr_func coreMatrix3 coreMatrix3::operator + (const coreMatrix3& v)const
{
    return coreMatrix3(_11+v._11, _12+v._12, _13+v._13,
                       _21+v._21, _22+v._22, _23+v._23,
                       _31+v._31, _32+v._32, _33+v._33);
}


// ****************************************************************
/* subtraction with matrix */
constexpr_func coreMatrix3 coreMatrix3::operator - (const coreMatrix3& v)const
{
    return coreMatrix3(_11-v._11, _12-v._12, _13-v._13,
                       _21-v._21, _22-v._22, _23-v._23,
                       _31-v._31, _32-v._32, _33-v._33);
}


// ****************************************************************
/* multiplication with matrix */
constexpr_func coreMatrix3 coreMatrix3::operator * (const coreMatrix3& v)const
{
    return coreMatrix3(_11*v._11 + _12*v._21 + _13*v._31, _11*v._12 + _12*v._22 + _13*v._32, _11*v._13 + _12*v._23 + _13*v._33,
                       _21*v._11 + _22*v._21 + _23*v._31, _21*v._12 + _22*v._22 + _23*v._32, _21*v._13 + _22*v._23 + _23*v._33,
                       _31*v._11 + _32*v._21 + _33*v._31, _31*v._12 + _32*v._22 + _33*v._32, _31*v._13 + _32*v._23 + _33*v._33);
}


// ****************************************************************
/* multiplication with scalar */
constexpr_func coreMatrix3 coreMatrix3::operator * (const float& f)const
{
    return coreMatrix3(_11*f, _12*f, _13*f,
                       _21*f, _22*f, _23*f,
                       _31*f, _32*f, _33*f);
}


// ****************************************************************
/* transpose matrix */
inline coreMatrix3& coreMatrix3::Transpose()
{
    *this = coreMatrix3(_11, _21, _31,
                        _12, _22, _32,
                        _13, _23, _33);

    return *this;
}


// ****************************************************************
/* invert matrix */
inline coreMatrix3& coreMatrix3::Invert()
{
    const float A = _22*_33 - _23*_32;
    const float B = _23*_31 - _21*_33;
    const float C = _21*_32 - _22*_31;

    *this = coreMatrix3(A, _13*_32 - _12*_33, _12*_23 - _13*_22,
                        B, _11*_33 - _13*_31, _13*_21 - _11*_23,
                        C, _12*_31 - _11*_32, _11*_22 - _12*_21)
                        / (_11*A + _12*B + _13*C);

    return *this;
}


// ****************************************************************
/* calculate determinant */
constexpr_func float coreMatrix3::Determinant()const
{
    return _11*(_22*_33 - _23*_32) + _12*(_23*_31 - _21*_33) + _13*(_21*_32 - _22*_31);
}


// ****************************************************************
/* constructor */
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
/* addition with matrix */
constexpr_func coreMatrix4 coreMatrix4::operator + (const coreMatrix4& v)const
{
    return coreMatrix4(_11+v._11, _12+v._12, _13+v._13, _14+v._14,
                       _21+v._21, _22+v._22, _23+v._23, _24+v._24,
                       _31+v._31, _32+v._32, _33+v._33, _34+v._34,
                       _41+v._41, _42+v._42, _43+v._43, _44+v._44);
}


// ****************************************************************
/* subtraction with matrix */
constexpr_func coreMatrix4 coreMatrix4::operator - (const coreMatrix4& v)const
{
    return coreMatrix4(_11-v._11, _12-v._12, _13-v._13, _14-v._14,
                       _21-v._21, _22-v._22, _23-v._23, _24-v._24,
                       _31-v._31, _32-v._32, _33-v._33, _34-v._34,
                       _41-v._41, _42-v._42, _43-v._43, _44-v._44);
}


// ****************************************************************
/* multiplication with matrix */
constexpr_func coreMatrix4 coreMatrix4::operator * (const coreMatrix4& v)const
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
/* multiplication with scalar */
constexpr_func coreMatrix4 coreMatrix4::operator * (const float& f)const
{
    return coreMatrix4(_11*f, _12*f, _13*f, _14*f,
                       _21*f, _22*f, _23*f, _24*f,
                       _31*f, _32*f, _33*f, _34*f,
                       _41*f, _42*f, _43*f, _44*f);
}


// ****************************************************************
/* transpose matrix */
inline coreMatrix4& coreMatrix4::Transpose()
{
    *this = coreMatrix4(_11, _21, _31, _41,
                        _12, _22, _32, _42,
                        _13, _23, _33, _43,
                        _14, _24, _34, _44);

    return *this;
}


// ****************************************************************
/* invert matrix */
inline coreMatrix4& coreMatrix4::Invert()
{
    const float A = _11*_22 - _12*_21;
    const float B = _11*_23 - _13*_21;
    const float C = _11*_24 - _14*_21;
    const float D = _12*_23 - _13*_22;
    const float E = _12*_24 - _14*_22;
    const float F = _13*_24 - _14*_23;
    const float G = _31*_42 - _32*_41;
    const float H = _31*_43 - _33*_41;
    const float I = _31*_44 - _34*_41;
    const float J = _32*_43 - _33*_42;
    const float K = _32*_44 - _34*_42;
    const float L = _33*_44 - _34*_43;

    *this = coreMatrix4(_22*L - _23*K + _24*J, _13*K - _12*L - _14*J, _42*F - _43*E + _44*D, _33*E - _32*F - _34*D,
                        _23*I - _21*L - _24*H, _11*L - _13*I + _14*H, _43*C - _41*F - _44*B, _31*F - _33*C + _34*B,
                        _21*K - _22*I + _24*G, _12*I - _11*K - _14*G, _41*E - _42*C + _44*A, _32*C - _31*E - _34*A,
                        _22*H - _21*J - _23*G, _11*J - _12*H + _13*G, _42*B - _41*D - _43*A, _31*D - _32*B + _33*A)
                        / (A*L - B*K + C*J + D*I - E*H + F*G);

    return *this;
}


// ****************************************************************
/* calculate determinant */
constexpr_func float coreMatrix4::Determinant()const
{
    return (_11*_22 - _12*_21) * (_33*_44 - _34*_43) - (_11*_23 - _13*_21) * (_32*_44 - _34*_42) + 
           (_11*_24 - _14*_21) * (_32*_43 - _33*_42) + (_12*_23 - _13*_22) * (_31*_44 - _34*_41) - 
           (_12*_24 - _14*_22) * (_31*_43 - _33*_41) + (_13*-24 - _14*_23) * (_31*_42 - _32*_41);
}


// ****************************************************************
/* get identity matrix */
constexpr_func coreMatrix4 coreMatrix4::Identity()
{
    return coreMatrix4(1.0f, 0.0f, 0.0f, 0.0f,
                       0.0f, 1.0f, 0.0f, 0.0f,
                       0.0f, 0.0f, 1.0f, 0.0f,
                       0.0f, 0.0f, 0.0f, 1.0f);
}


// ****************************************************************
/* get translation matrix */
constexpr_func coreMatrix4 coreMatrix4::Translation(const coreVector3& vPosition)
{
    return coreMatrix4(       1.0f,        0.0f,        0.0f, 0.0f,
                              0.0f,        1.0f,        0.0f, 0.0f,
                              0.0f,        0.0f,        1.0f, 0.0f,
                       vPosition.x, vPosition.y, vPosition.z, 1.0f);
}


// ****************************************************************
/* get scale matrix */
constexpr_func coreMatrix4 coreMatrix4::Scaling(const coreVector3& vSize)
{
    return coreMatrix4(vSize.x,    0.0f,    0.0f, 0.0f,
                          0.0f, vSize.y,    0.0f, 0.0f,
                          0.0f,    0.0f, vSize.z, 0.0f,
                          0.0f,    0.0f,    0.0f, 1.0f);
}


// ****************************************************************
/* get rotation matrix around X */
inline coreMatrix4 coreMatrix4::RotationX(const coreVector2& vDirection)
{
    ASSERT(vDirection.IsNormalized())
    return coreMatrix4(1.0f,          0.0f,          0.0f, 0.0f,
                       0.0f,  vDirection.y,  vDirection.x, 0.0f,
                       0.0f, -vDirection.x,  vDirection.y, 0.0f,
                       0.0f,          0.0f,          0.0f, 1.0f);
}

inline coreMatrix4 coreMatrix4::RotationX(const float& fAngle)
{
    return coreMatrix4::RotationX(coreVector2::Direction(fAngle));
}


// ****************************************************************
/* get rotation matrix around Y */
inline coreMatrix4 coreMatrix4::RotationY(const coreVector2& vDirection)
{
    ASSERT(vDirection.IsNormalized())
    return coreMatrix4(vDirection.y, 0.0f, -vDirection.x, 0.0f,
                               0.0f, 1.0f,          0.0f, 0.0f,
                       vDirection.x, 0.0f,  vDirection.y, 0.0f,
                               0.0f, 0.0f,          0.0f, 1.0f);
}

inline coreMatrix4 coreMatrix4::RotationY(const float& fAngle)noexcept
{
    return coreMatrix4::RotationY(coreVector2::Direction(fAngle));
}


// ****************************************************************
/* get rotation matrix around Z */
inline coreMatrix4 coreMatrix4::RotationZ(const coreVector2& vDirection)
{
    ASSERT(vDirection.IsNormalized())
    return coreMatrix4( vDirection.y, vDirection.x, 0.0f, 0.0f,
                       -vDirection.x, vDirection.y, 0.0f, 0.0f,
                                0.0f,         0.0f, 1.0f, 0.0f,
                                0.0f,         0.0f, 0.0f, 1.0f);
}

inline coreMatrix4 coreMatrix4::RotationZ(const float& fAngle)
{
    return coreMatrix4::RotationZ(coreVector2::Direction(fAngle));
}


// ****************************************************************
/* calculate orientation matrix */
inline coreMatrix4 coreMatrix4::Orientation(const coreVector3& vDirection, const coreVector3& vOrientation)
{
    ASSERT(vDirection.IsNormalized() && vOrientation.IsNormalized())

    const coreVector3  D = -vDirection;
    const coreVector3& O =  vOrientation;
    const coreVector3  S =  coreVector3::Cross(O, D).Normalize();
    const coreVector3  U = -coreVector3::Cross(S, D).Normalize();

    return coreMatrix4( S.x,  S.y,  S.z, 0.0f,
                        U.x,  U.y,  U.z, 0.0f,
                        D.x,  D.y,  D.z, 0.0f,
                       0.0f, 0.0f, 0.0f, 1.0f);
}


// ****************************************************************
/* calculate perspective projection matrix */
inline coreMatrix4 coreMatrix4::Perspective(const coreVector2& vResolution, const float& fFOV, const float& fNearClip, const float& fFarClip)
{
    const float Y = COT(fFOV * 0.5f);
    const float X = Y * vResolution.yx().AspectRatio();

    const float& N = fNearClip;
    const float& F = fFarClip;
    const float  I = RCP(N-F);

    return coreMatrix4(   X, 0.0f,       0.0f,  0.0f,
                       0.0f,    Y,       0.0f,  0.0f,
                       0.0f, 0.0f,    (N+F)*I, -1.0f,
                       0.0f, 0.0f, 2.0f*N*F*I,  0.0f);
}


// ****************************************************************
/* calculate orthogonal projection matrix (simplified) */
inline coreMatrix4 coreMatrix4::Ortho(const coreVector2& vResolution)
{
    const float X = 2.0f * RCP(vResolution.x);
    const float Y = 2.0f * RCP(vResolution.y);

    return coreMatrix4(   X, 0.0f, 0.0f, 0.0f,
                       0.0f,    Y, 0.0f, 0.0f,
                       0.0f, 0.0f, 1.0f, 0.0f,
                       0.0f, 0.0f, 0.0f, 1.0f);
}


// ****************************************************************
/* calculate camera matrix */
inline coreMatrix4 coreMatrix4::Camera(const coreVector3& vPosition, const coreVector3& vDirection, const coreVector3& vOrientation)
{
    coreMatrix4 mCamera = coreMatrix4::Orientation(vDirection, vOrientation);
    mCamera._14 = -mCamera._11*vPosition.x - mCamera._12*vPosition.y - mCamera._13*vPosition.z;
    mCamera._24 = -mCamera._21*vPosition.x - mCamera._22*vPosition.y - mCamera._23*vPosition.z;
    mCamera._34 = -mCamera._31*vPosition.x - mCamera._32*vPosition.y - mCamera._33*vPosition.z;

    return mCamera.Transpose();
}


// ****************************************************************
/* multiplication with matrix */
inline coreVector2 coreVector2::operator * (const coreMatrix4& m)const
{
    const float w = RCP(x*m._14 + y*m._24 +   m._44);
    return  coreVector2(x*m._11 + y*m._21 + w*m._41,
                        x*m._12 + y*m._22 + w*m._42);
}


// ****************************************************************
/* multiplication with matrix */
inline coreVector3 coreVector3::operator * (const coreMatrix4& m)const
{
    const float w = RCP(x*m._14 + y*m._24 + z*m._34 +   m._44);
    return  coreVector3(x*m._11 + y*m._21 + z*m._31 + w*m._41,
                        x*m._12 + y*m._22 + z*m._32 + w*m._42,
                        x*m._13 + y*m._23 + z*m._33 + w*m._43);
}


// ****************************************************************
/* multiplication with matrix */
constexpr_func coreVector4 coreVector4::operator * (const coreMatrix4& m)const
{
    return coreVector4(x*m._11 + y*m._21 + z*m._31 + w*m._41,
                       x*m._12 + y*m._22 + z*m._32 + w*m._42,
                       x*m._13 + y*m._23 + z*m._33 + w*m._43,
                       x*m._14 + y*m._24 + z*m._34 + w*m._44);
}


#endif /* _CORE_GUARD_MATRIX_H_ */