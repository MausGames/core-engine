//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MATRIX_H_
#define _CORE_GUARD_MATRIX_H_


// ****************************************************************
// 4x4 matrix class
class coreMatrix final
{
public:
    align16(union)
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
    constexpr coreMatrix()noexcept;
    constexpr coreMatrix(const float& f11, const float& f12, const float& f13, const float& f14,
                         const float& f21, const float& f22, const float& f23, const float& f24,
                         const float& f31, const float& f32, const float& f33, const float& f34,
                         const float& f41, const float& f42, const float& f43, const float& f44)noexcept;

    //! compare operators
    //! @{
    bool operator == (const coreMatrix& v)const noexcept;
    bool operator != (const coreMatrix& v)const noexcept;
    //! @}

    //! matrix calculation operators
    //! @{
    coreMatrix operator + (const coreMatrix& v)const noexcept;
    coreMatrix operator - (const coreMatrix& v)const noexcept;
    coreMatrix operator * (const coreMatrix& v)const noexcept;
    inline void operator += (const coreMatrix& v)noexcept {*this = *this + v;}
    inline void operator -= (const coreMatrix& v)noexcept {*this = *this - v;}
    inline void operator *= (const coreMatrix& v)noexcept {*this = *this * v;}
    //! @}

    //! scalar calculation operators
    //! @{
    coreMatrix operator * (const float& f)const noexcept;
    coreMatrix operator / (const float& f)const noexcept;
    inline void operator *= (const float& f)noexcept                                  {*this = *this * f;}
    inline void operator /= (const float& f)noexcept                                  {*this = *this / f;}
    friend inline coreMatrix operator * (const float& f, const coreMatrix& v)noexcept {return v * f;}
    //! @}

    //! convert matrix
    //! @{
    inline operator const float* ()const noexcept {return reinterpret_cast<const float*>(this);}
    //! @}

    //! transpose matrix
    //! @{
    coreMatrix& Transpose()noexcept;
    coreMatrix Transposed()const noexcept {return coreMatrix(*this).Transpose();}
    //! @}

    //! invert matrix
    //! @{
    coreMatrix& Invert()noexcept;
    coreMatrix Inverted()const noexcept {return coreMatrix(*this).Invert();}
    //! @}

    //! direct functions
    //! @{
    float Determinant()const noexcept;
    //! @}

    //! static functions
    //! @{
    static coreMatrix Identity()noexcept;
    static coreMatrix Translation(const coreVector3& vPosition)noexcept;
    static coreMatrix Scaling(const coreVector3& vSize)noexcept;
    static coreMatrix RotationX(const coreVector2& vDirection)noexcept;
    static coreMatrix RotationX(const float& fAngle)noexcept;
    static coreMatrix RotationY(const coreVector2& vDirection)noexcept;
    static coreMatrix RotationY(const float& fAngle)noexcept;
    static coreMatrix RotationZ(const coreVector2& vDirection)noexcept;
    static coreMatrix RotationZ(const float& fAngle)noexcept;
    static coreMatrix Orientation(const coreVector3& vDirection, const coreVector3& vOrintation)noexcept;
    static coreMatrix Perspective(const coreVector2& vResolution, const float& fFOV, const float& fNearClip, const float& fFarClip)noexcept;
    static coreMatrix Ortho(const coreVector2& vResolution)noexcept;
    static coreMatrix Camera(const coreVector3& vPosition, const coreVector3& vDirection, const coreVector3& vOrintation)noexcept;
    //! @}
};


// ****************************************************************
// constructor
constexpr coreMatrix::coreMatrix()noexcept
: _11 (1.0f), _12 (0.0f), _13 (0.0f), _14 (0.0f)
, _21 (0.0f), _22 (1.0f), _23 (0.0f), _24 (0.0f)
, _31 (0.0f), _32 (0.0f), _33 (1.0f), _34 (0.0f)
, _41 (0.0f), _42 (0.0f), _43 (0.0f), _44 (1.0f)
{
}

constexpr coreMatrix::coreMatrix(const float& f11, const float& f12, const float& f13, const float& f14,
                                 const float& f21, const float& f22, const float& f23, const float& f24,
                                 const float& f31, const float& f32, const float& f33, const float& f34,
                                 const float& f41, const float& f42, const float& f43, const float& f44)noexcept
: _11 (f11), _12 (f12), _13 (f13), _14 (f14)
, _21 (f21), _22 (f22), _23 (f23), _24 (f24)
, _31 (f31), _32 (f32), _33 (f33), _34 (f34)
, _41 (f41), _42 (f42), _43 (f43), _44 (f44)
{
}


#endif // _CORE_GUARD_MATRIX_H_