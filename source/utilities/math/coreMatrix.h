//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
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

    //! convert matrix
    //! @{
    inline operator const float* ()const noexcept {return r_cast<const float*>(this);}
    //! @}
};


// ****************************************************************
// 4x4-matrix class
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
    bool operator == (const coreMatrix4& v)const noexcept;
    bool operator != (const coreMatrix4& v)const noexcept;
    //! @}

    //! matrix calculation operators
    //! @{
    coreMatrix4 operator + (const coreMatrix4& v)const noexcept;
    coreMatrix4 operator - (const coreMatrix4& v)const noexcept;
    coreMatrix4 operator * (const coreMatrix4& v)const noexcept;
    inline void operator += (const coreMatrix4& v)noexcept {*this = *this + v;}
    inline void operator -= (const coreMatrix4& v)noexcept {*this = *this - v;}
    inline void operator *= (const coreMatrix4& v)noexcept {*this = *this * v;}
    //! @}

    //! scalar calculation operators
    //! @{
    coreMatrix4 operator * (const float& f)const noexcept;
    coreMatrix4 operator / (const float& f)const noexcept;
    inline void operator *= (const float& f)noexcept                                    {*this = *this * f;}
    inline void operator /= (const float& f)noexcept                                    {*this = *this / f;}
    friend inline coreMatrix4 operator * (const float& f, const coreMatrix4& v)noexcept {return v * f;}
    //! @}

    //! convert matrix
    //! @{
    inline operator const float* ()const noexcept {return r_cast<const float*>(this);}
    inline coreMatrix3 m123()const noexcept       {return coreMatrix3(_11, _12, _13, _21, _22, _23, _31, _32, _33);}
    inline coreMatrix3 m124()const noexcept       {return coreMatrix3(_11, _12, _14, _21, _22, _24, _41, _42, _44);}
    inline coreMatrix3 m134()const noexcept       {return coreMatrix3(_11, _13, _14, _31, _33, _34, _41, _43, _44);}
    inline coreMatrix3 m234()const noexcept       {return coreMatrix3(_22, _23, _24, _32, _33, _34, _42, _43, _44);}
    //! @}

    //! transpose matrix
    //! @{
    coreMatrix4& Transpose()noexcept;
    inline coreMatrix4 Transposed()const noexcept {return coreMatrix4(*this).Transpose();}
    //! @}

    //! invert matrix
    //! @{
    coreMatrix4& Invert()noexcept;
    inline coreMatrix4 Inverted()const noexcept {return coreMatrix4(*this).Invert();}
    //! @}

    //! direct functions
    //! @{
    float Determinant()const noexcept;
    //! @}

    //! static functions
    //! @{
    static constexpr_func coreMatrix4 Identity()noexcept;
    static constexpr_func coreMatrix4 Translation(const coreVector3& vPosition)noexcept;
    static constexpr_func coreMatrix4 Scaling(const coreVector3& vSize)noexcept;
    static coreMatrix4 RotationX(const coreVector2& vDirection)noexcept;
    static coreMatrix4 RotationX(const float& fAngle)noexcept;
    static coreMatrix4 RotationY(const coreVector2& vDirection)noexcept;
    static coreMatrix4 RotationY(const float& fAngle)noexcept;
    static coreMatrix4 RotationZ(const coreVector2& vDirection)noexcept;
    static coreMatrix4 RotationZ(const float& fAngle)noexcept;
    static coreMatrix4 Orientation(const coreVector3& vDirection, const coreVector3& vOrintation)noexcept;
    static coreMatrix4 Perspective(const coreVector2& vResolution, const float& fFOV, const float& fNearClip, const float& fFarClip)noexcept;
    static coreMatrix4 Ortho(const coreVector2& vResolution)noexcept;
    static coreMatrix4 Camera(const coreVector3& vPosition, const coreVector3& vDirection, const coreVector3& vOrintation)noexcept;
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


#endif // _CORE_GUARD_MATRIX_H_