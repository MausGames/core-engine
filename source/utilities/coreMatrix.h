//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef GUARD_CORE_MATRIX_H
#define GUARD_CORE_MATRIX_H


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
    coreMatrix();
    coreMatrix(const float& f11, const float& f12, const float& f13, const float& f14,
               const float& f21, const float& f22, const float& f23, const float& f24,
               const float& f31, const float& f32, const float& f33, const float& f34,
               const float& f41, const float& f42, const float& f43, const float& f44);

    //! compare operators
    //! @{
    bool operator == (const coreMatrix& v)const;
    bool operator != (const coreMatrix& v)const;
    //! @}

    //! matrix calculation operators
    //! @{
    coreMatrix operator + (const coreMatrix& v)const;
    coreMatrix operator - (const coreMatrix& v)const;
    coreMatrix operator * (const coreMatrix& v)const;
    inline void operator += (const coreMatrix& v) {*this = *this + v;}
    inline void operator -= (const coreMatrix& v) {*this = *this - v;}
    inline void operator *= (const coreMatrix& v) {*this = *this * v;}
    //! @}

    //! scalar calculation operators
    //! @{
    coreMatrix operator * (const float& f)const;
    coreMatrix operator / (const float& f)const;
    inline void operator *= (const float& f)                                  {*this = *this * f;}
    inline void operator /= (const float& f)                                  {*this = *this / f;} //! @} @{
    friend inline coreMatrix operator * (const float& f, const coreMatrix& v) {return v * f;}
    //! @}

    //! convert matrix
    //! @{
    inline operator const float* ()const {return reinterpret_cast<const float*>(this);}
    //! @}

    //! transpose matrix
    //! @{
    coreMatrix& Transpose();
    coreMatrix Transposed()const {return coreMatrix(*this).Transpose();}
    //! @}

    //! invert matrix
    //! @{
    coreMatrix& Invert();
    coreMatrix Inverted()const {return coreMatrix(*this).Invert();}
    //! @}

    //! direct functions
    //! @{
    float Determinant()const;
    //! @}

    //! static functions
    //! @{
    static coreMatrix Identity();
    static coreMatrix Translation(const coreVector3& vPosition);
    static coreMatrix Scaling(const coreVector3& vSize);
    static coreMatrix RotationX(const coreVector2& vDirection);
    static coreMatrix RotationX(const float& fAngle);
    static coreMatrix RotationY(const coreVector2& vDirection);
    static coreMatrix RotationY(const float& fAngle);
    static coreMatrix RotationZ(const coreVector2& vDirection);
    static coreMatrix RotationZ(const float& fAngle);
    static coreMatrix Orientation(const coreVector3& vDirection, const coreVector3& vOrintation);
    static coreMatrix Perspective(const coreVector2& vResolution, const float& fFOV, const float& fNearClip, const float& fFarClip);
    static coreMatrix Ortho(const coreVector2& vResolution);
    static coreMatrix Camera(const coreVector3& vPosition, const coreVector3& vDirection, const coreVector3& vOrintation);
    //! @}
};


#endif // GUARD_CORE_MATRIX_H