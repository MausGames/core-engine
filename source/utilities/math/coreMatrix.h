///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MATRIX_H_
#define _CORE_GUARD_MATRIX_H_

// TODO 5: do not multiply always full matrices, implement "modifying" functions
// TODO 3: implement "Rotation from One Vector to Another" matrix (what about orientation ?)
// TODO 3: the mXXX() conversion functions create a lot of move instructions (seen before using FORCE_INLINE constructors)

// NOTE: row major

#define CONSTEXPR inline


// ****************************************************************
/* 3x2-matrix class (storage only) */
class coreMatrix3x2 final
{
public:
    coreFloat _11, _12;
    coreFloat _21, _22;
    coreFloat _31, _32;


public:
    coreMatrix3x2() = default;
    constexpr FORCE_INLINE coreMatrix3x2(const coreFloat f11, const coreFloat f12,
                                         const coreFloat f21, const coreFloat f22,
                                         const coreFloat f31, const coreFloat f32)noexcept;

    ENABLE_COPY(coreMatrix3x2)
    ENABLE_COMPARISON(coreMatrix3x2)

    /* static functions */
    static constexpr coreMatrix3x2 Identity();
};


// ****************************************************************
/* 4x3-matrix class (storage only) */
class coreMatrix4x3 final
{
public:
    coreFloat _11, _12, _13;
    coreFloat _21, _22, _23;
    coreFloat _31, _32, _33;
    coreFloat _41, _42, _43;


public:
    coreMatrix4x3() = default;
    constexpr FORCE_INLINE coreMatrix4x3(const coreFloat f11, const coreFloat f12, const coreFloat f13,
                                         const coreFloat f21, const coreFloat f22, const coreFloat f23,
                                         const coreFloat f31, const coreFloat f32, const coreFloat f33,
                                         const coreFloat f41, const coreFloat f42, const coreFloat f43)noexcept;

    ENABLE_COPY(coreMatrix4x3)
    ENABLE_COMPARISON(coreMatrix4x3)

    /* static functions */
    static constexpr coreMatrix4x3 Identity();
};


// ****************************************************************
/* 2x2-matrix class */
class coreMatrix2 final
{
public:
    coreFloat _11, _12;
    coreFloat _21, _22;


public:
    coreMatrix2() = default;
    constexpr FORCE_INLINE coreMatrix2(const coreFloat f11, const coreFloat f12,
                                       const coreFloat f21, const coreFloat f22)noexcept;

    ENABLE_COPY(coreMatrix2)
    ENABLE_COMPARISON(coreMatrix2)

    /* matrix operations */
    constexpr coreMatrix2  operator +  (const coreMatrix2& m)const;
    constexpr coreMatrix2  operator -  (const coreMatrix2& m)const;
    constexpr coreMatrix2  operator *  (const coreMatrix2& m)const;
    inline    coreMatrix2& operator += (const coreMatrix2& m) {return (*this = *this + m);}
    inline    coreMatrix2& operator -= (const coreMatrix2& m) {return (*this = *this - m);}
    inline    coreMatrix2& operator *= (const coreMatrix2& m) {return (*this = *this * m);}

    /* scalar operations */
    constexpr coreMatrix2  operator *  (const coreFloat f)const;
    constexpr coreMatrix2  operator /  (const coreFloat f)const {return  *this * RCP(f);}
    inline    coreMatrix2& operator *= (const coreFloat f)      {return (*this = *this * f);}
    inline    coreMatrix2& operator /= (const coreFloat f)      {return (*this = *this / f);}

    /* convert matrix */
    inline          coreFloat& arr(const coreUintW i)      {ASSERT(i < 4u) return (&_11)[i];}
    constexpr const coreFloat& arr(const coreUintW i)const {ASSERT(i < 4u) return (&_11)[i];}
    constexpr const coreFloat* ptr()const                  {return (&_11);}

    /* transpose matrix */
    constexpr coreMatrix2 Transposed()const;

    /* invert matrix */
    constexpr coreMatrix2 Inverted()const;

    /* process matrix */
    template <typename F, typename... A> inline coreMatrix2 Processed(F&& nFunction, A&&... vArgs)const                                                      {coreMatrix2 M; for(coreUintW i = 0u; i < 4u; ++i) M.arr(i) = nFunction(this->arr(i), std::forward<A>(vArgs)...); return M;}
    inline coreMatrix2 Processed(coreFloat (*nFunction) (const coreFloat&))const                                                                             {coreMatrix2 M; for(coreUintW i = 0u; i < 4u; ++i) M.arr(i) = nFunction(this->arr(i));                            return M;}
    inline coreMatrix2 Processed(coreFloat (*nFunction) (const coreFloat&, const coreFloat&),                   const coreFloat f1)const                     {coreMatrix2 M; for(coreUintW i = 0u; i < 4u; ++i) M.arr(i) = nFunction(this->arr(i), f1);                        return M;}
    inline coreMatrix2 Processed(coreFloat (*nFunction) (const coreFloat&, const coreFloat&, const coreFloat&), const coreFloat f1, const coreFloat f2)const {coreMatrix2 M; for(coreUintW i = 0u; i < 4u; ++i) M.arr(i) = nFunction(this->arr(i), f1, f2);                    return M;}

    /* direct functions */
    constexpr coreFloat Determinant()const;

    /* static functions */
    static constexpr coreMatrix2 Identity();
};


// ****************************************************************
/* 3x3-matrix class */
class coreMatrix3 final
{
public:
    coreFloat _11, _12, _13;
    coreFloat _21, _22, _23;
    coreFloat _31, _32, _33;


public:
    coreMatrix3() = default;
    constexpr FORCE_INLINE coreMatrix3(const coreFloat f11, const coreFloat f12, const coreFloat f13,
                                       const coreFloat f21, const coreFloat f22, const coreFloat f23,
                                       const coreFloat f31, const coreFloat f32, const coreFloat f33)noexcept;
    constexpr FORCE_INLINE explicit coreMatrix3(const coreMatrix3x2& m)noexcept;
    constexpr FORCE_INLINE explicit coreMatrix3(const coreMatrix2&   m)noexcept;

    ENABLE_COPY(coreMatrix3)
    ENABLE_COMPARISON(coreMatrix3)

    /* matrix operations */
    constexpr coreMatrix3  operator +  (const coreMatrix3& m)const;
    constexpr coreMatrix3  operator -  (const coreMatrix3& m)const;
    constexpr coreMatrix3  operator *  (const coreMatrix3& m)const;
    inline    coreMatrix3& operator += (const coreMatrix3& m) {return (*this = *this + m);}
    inline    coreMatrix3& operator -= (const coreMatrix3& m) {return (*this = *this - m);}
    inline    coreMatrix3& operator *= (const coreMatrix3& m) {return (*this = *this * m);}

    /* scalar operations */
    constexpr coreMatrix3  operator *  (const coreFloat f)const;
    constexpr coreMatrix3  operator /  (const coreFloat f)const {return  *this * RCP(f);}
    inline    coreMatrix3& operator *= (const coreFloat f)      {return (*this = *this * f);}
    inline    coreMatrix3& operator /= (const coreFloat f)      {return (*this = *this / f);}

    /* convert matrix */
    inline          coreFloat& arr (const coreUintW i)      {ASSERT(i < 9u) return (&_11)[i];}
    constexpr const coreFloat& arr (const coreUintW i)const {ASSERT(i < 9u) return (&_11)[i];}
    constexpr const coreFloat* ptr ()const                  {return (&_11);}
    CONSTEXPR coreMatrix3x2    m3x2()const                  {return coreMatrix3x2(_11, _12, _21, _22, _31, _32);}
    CONSTEXPR coreMatrix2      m12 ()const                  {return coreMatrix2  (_11, _12, _21, _22);}
    CONSTEXPR coreMatrix2      m13 ()const                  {return coreMatrix2  (_11, _13, _31, _33);}
    CONSTEXPR coreMatrix2      m23 ()const                  {return coreMatrix2  (_22, _23, _32, _33);}

    /* transpose matrix */
    constexpr coreMatrix3 Transposed()const;

    /* invert matrix */
    constexpr coreMatrix3 Inverted()const;

    /* process matrix */
    template <typename F, typename... A> inline coreMatrix3 Processed(F&& nFunction, A&&... vArgs)const                                                      {coreMatrix3 M; for(coreUintW i = 0u; i < 9u; ++i) M.arr(i) = nFunction(this->arr(i), std::forward<A>(vArgs)...); return M;}
    inline coreMatrix3 Processed(coreFloat (*nFunction) (const coreFloat&))const                                                                             {coreMatrix3 M; for(coreUintW i = 0u; i < 9u; ++i) M.arr(i) = nFunction(this->arr(i));                            return M;}
    inline coreMatrix3 Processed(coreFloat (*nFunction) (const coreFloat&, const coreFloat&),                   const coreFloat f1)const                     {coreMatrix3 M; for(coreUintW i = 0u; i < 9u; ++i) M.arr(i) = nFunction(this->arr(i), f1);                        return M;}
    inline coreMatrix3 Processed(coreFloat (*nFunction) (const coreFloat&, const coreFloat&, const coreFloat&), const coreFloat f1, const coreFloat f2)const {coreMatrix3 M; for(coreUintW i = 0u; i < 9u; ++i) M.arr(i) = nFunction(this->arr(i), f1, f2);                    return M;}

    /* component functions */
    constexpr coreVector2 GetPosition ()const {return coreVector2(_31, _32);}
    constexpr coreVector2 GetSize     ()const {return coreVector2(coreVector2(_11, _21).Length(), coreVector2(_12, _22).Length());}
    inline    coreVector2 GetSize90   ()const {return coreVector2(ABS(_11) + ABS(_21),            ABS(_12) + ABS(_22));}
    constexpr coreVector2 GetDirection()const {return coreVector2(_12, _11).Normalized();}

    /* direct functions */
    constexpr coreFloat Determinant()const;

    /* static functions */
    static constexpr coreMatrix3 Identity   ();
    static constexpr coreMatrix3 Translation(const coreVector2 vPosition);
    static constexpr coreMatrix3 Scaling    (const coreVector2 vSize);
    static constexpr coreMatrix3 Rotation   (const coreVector2 vDirection);
    static inline    coreMatrix3 Rotation   (const coreFloat   fAngle);
    static constexpr coreMatrix3 ShearXY    (const coreFloat   fFactor);
    static constexpr coreMatrix3 ShearYX    (const coreFloat   fFactor);

    /* quaternion functions */
    constexpr        coreVector4 ToQuat  ()const;
    static constexpr coreMatrix3 FromQuat(const coreVector4 v);
};


// ****************************************************************
/* 4x4-matrix class */
class coreMatrix4 final
{
public:
    coreFloat _11, _12, _13, _14;
    coreFloat _21, _22, _23, _24;
    coreFloat _31, _32, _33, _34;
    coreFloat _41, _42, _43, _44;


public:
    coreMatrix4() = default;
    constexpr FORCE_INLINE coreMatrix4(const coreFloat f11, const coreFloat f12, const coreFloat f13, const coreFloat f14,
                                       const coreFloat f21, const coreFloat f22, const coreFloat f23, const coreFloat f24,
                                       const coreFloat f31, const coreFloat f32, const coreFloat f33, const coreFloat f34,
                                       const coreFloat f41, const coreFloat f42, const coreFloat f43, const coreFloat f44)noexcept;
    constexpr FORCE_INLINE explicit coreMatrix4(const coreMatrix4x3& m)noexcept;
    constexpr FORCE_INLINE explicit coreMatrix4(const coreMatrix3x2& m)noexcept;
    constexpr FORCE_INLINE explicit coreMatrix4(const coreMatrix3&   m)noexcept;
    constexpr FORCE_INLINE explicit coreMatrix4(const coreMatrix2&   m)noexcept;

    ENABLE_COPY(coreMatrix4)
    ENABLE_COMPARISON(coreMatrix4)

    /* matrix operations */
    constexpr coreMatrix4  operator +  (const coreMatrix4& m)const;
    constexpr coreMatrix4  operator -  (const coreMatrix4& m)const;
    constexpr coreMatrix4  operator *  (const coreMatrix4& m)const;
    inline    coreMatrix4& operator += (const coreMatrix4& m) {return (*this = *this + m);}
    inline    coreMatrix4& operator -= (const coreMatrix4& m) {return (*this = *this - m);}
    inline    coreMatrix4& operator *= (const coreMatrix4& m) {return (*this = *this * m);}

    /* scalar operations */
    constexpr coreMatrix4  operator *  (const coreFloat f)const;
    constexpr coreMatrix4  operator /  (const coreFloat f)const {return  *this * RCP(f);}
    inline    coreMatrix4& operator *= (const coreFloat f)      {return (*this = *this * f);}
    inline    coreMatrix4& operator /= (const coreFloat f)      {return (*this = *this / f);}

    /* convert matrix */
    inline          coreFloat& arr (const coreUintW i)      {ASSERT(i < 16u) return (&_11)[i];}
    constexpr const coreFloat& arr (const coreUintW i)const {ASSERT(i < 16u) return (&_11)[i];}
    constexpr const coreFloat* ptr ()const                  {return (&_11);}
    CONSTEXPR coreMatrix4x3    m4x3()const                  {return coreMatrix4x3(_11, _12, _13, _21, _22, _23, _31, _32, _33, _41, _42, _43);}
    CONSTEXPR coreMatrix3x2    m3x2()const                  {return coreMatrix3x2(_11, _12, _21, _22, _31, _32);}
    CONSTEXPR coreMatrix3      m123()const                  {return coreMatrix3  (_11, _12, _13, _21, _22, _23, _31, _32, _33);}
    CONSTEXPR coreMatrix3      m124()const                  {return coreMatrix3  (_11, _12, _14, _21, _22, _24, _41, _42, _44);}
    CONSTEXPR coreMatrix3      m134()const                  {return coreMatrix3  (_11, _13, _14, _31, _33, _34, _41, _43, _44);}
    CONSTEXPR coreMatrix3      m234()const                  {return coreMatrix3  (_22, _23, _24, _32, _33, _34, _42, _43, _44);}

    /* transpose matrix */
    constexpr coreMatrix4 Transposed()const;

    /* invert matrix */
    constexpr coreMatrix4 Inverted()const;

    /* process matrix */
    template <typename F, typename... A> inline coreMatrix4 Processed(F&& nFunction, A&&... vArgs)const                                                      {coreMatrix4 M; for(coreUintW i = 0u; i < 16u; ++i) M.arr(i) = nFunction(this->arr(i), std::forward<A>(vArgs)...); return M;}
    inline coreMatrix4 Processed(coreFloat (*nFunction) (const coreFloat&))const                                                                             {coreMatrix4 M; for(coreUintW i = 0u; i < 16u; ++i) M.arr(i) = nFunction(this->arr(i));                            return M;}
    inline coreMatrix4 Processed(coreFloat (*nFunction) (const coreFloat&, const coreFloat&),                   const coreFloat f1)const                     {coreMatrix4 M; for(coreUintW i = 0u; i < 16u; ++i) M.arr(i) = nFunction(this->arr(i), f1);                        return M;}
    inline coreMatrix4 Processed(coreFloat (*nFunction) (const coreFloat&, const coreFloat&, const coreFloat&), const coreFloat f1, const coreFloat f2)const {coreMatrix4 M; for(coreUintW i = 0u; i < 16u; ++i) M.arr(i) = nFunction(this->arr(i), f1, f2);                    return M;}

    /* component functions */
    constexpr coreVector3 GetPosition()const {return coreVector3(_31, _32, _33);}
    constexpr coreVector3 GetSize    ()const {return coreVector3(coreVector3(_11, _21, _31).Length(), coreVector3(_12, _22, _32).Length(), coreVector3(_13, _23, _33).Length());}
    inline    coreVector3 GetSize90  ()const {return coreVector3(ABS(_11) + ABS(_21) + ABS(_31),      ABS(_12) + ABS(_22) + ABS(_32),      ABS(_13) + ABS(_23) + ABS(_33));}

    /* direct functions */
    constexpr coreFloat Determinant()const;

    /* static functions */
    static constexpr coreMatrix4 Identity    ();
    static constexpr coreMatrix4 Translation (const coreVector3 vPosition);
    static constexpr coreMatrix4 Scaling     (const coreVector3 vSize);
    static constexpr coreMatrix4 RotationX   (const coreVector2 vDirection);
    static inline    coreMatrix4 RotationX   (const coreFloat   fAngle);
    static constexpr coreMatrix4 RotationY   (const coreVector2 vDirection);
    static inline    coreMatrix4 RotationY   (const coreFloat   fAngle);
    static constexpr coreMatrix4 RotationZ   (const coreVector2 vDirection);
    static inline    coreMatrix4 RotationZ   (const coreFloat   fAngle);
    static inline    coreMatrix4 RotationAxis(const coreFloat   fAngle, const coreVector3 vAxis);
    static constexpr coreMatrix4 ShearXY     (const coreFloat   fFactor);
    static constexpr coreMatrix4 ShearXZ     (const coreFloat   fFactor);
    static constexpr coreMatrix4 ShearYX     (const coreFloat   fFactor);
    static constexpr coreMatrix4 ShearYZ     (const coreFloat   fFactor);
    static constexpr coreMatrix4 ShearZX     (const coreFloat   fFactor);
    static constexpr coreMatrix4 ShearZY     (const coreFloat   fFactor);
    static constexpr coreMatrix4 Orientation (const coreVector3 vDirection, const coreVector3 vOrientation);
    static inline    coreMatrix4 Perspective (const coreVector2 vResolution, const coreFloat fFOV, const coreFloat fNearClip, const coreFloat fFarClip);
    static constexpr coreMatrix4 Ortho       (const coreVector2 vResolution);
    static constexpr coreMatrix4 Ortho       (const coreFloat   fLeft, const coreFloat fRight, const coreFloat fBottom, const coreFloat fTop, const coreFloat fNearClip, const coreFloat fFarClip);
    static constexpr coreMatrix4 Camera      (const coreVector3 vPosition, const coreVector3 vDirection, const coreVector3 vOrientation);
};


// ****************************************************************
/* global scalar operations */
CONSTEXPR coreMatrix2 operator * (const coreFloat f, const coreMatrix2& m) {return m * f;}
CONSTEXPR coreMatrix3 operator * (const coreFloat f, const coreMatrix3& m) {return m * f;}
CONSTEXPR coreMatrix4 operator * (const coreFloat f, const coreMatrix4& m) {return m * f;}


// ****************************************************************
/* constructor */
constexpr FORCE_INLINE coreMatrix3x2::coreMatrix3x2(const coreFloat f11, const coreFloat f12,
                                                    const coreFloat f21, const coreFloat f22,
                                                    const coreFloat f31, const coreFloat f32)noexcept
: _11 (f11), _12 (f12)
, _21 (f21), _22 (f22)
, _31 (f31), _32 (f32)
{
}


// ****************************************************************
/* get identity matrix */
constexpr coreMatrix3x2 coreMatrix3x2::Identity()
{
    return coreMatrix3x2(1.0f, 0.0f,
                         0.0f, 1.0f,
                         0.0f, 0.0f);
}


// ****************************************************************
/* constructor */
constexpr FORCE_INLINE coreMatrix4x3::coreMatrix4x3(const coreFloat f11, const coreFloat f12, const coreFloat f13,
                                                    const coreFloat f21, const coreFloat f22, const coreFloat f23,
                                                    const coreFloat f31, const coreFloat f32, const coreFloat f33,
                                                    const coreFloat f41, const coreFloat f42, const coreFloat f43)noexcept
: _11 (f11), _12 (f12), _13 (f13)
, _21 (f21), _22 (f22), _23 (f23)
, _31 (f31), _32 (f32), _33 (f33)
, _41 (f41), _42 (f42), _43 (f43)
{
}


// ****************************************************************
/* get identity matrix */
constexpr coreMatrix4x3 coreMatrix4x3::Identity()
{
    return coreMatrix4x3(1.0f, 0.0f, 0.0f,
                         0.0f, 1.0f, 0.0f,
                         0.0f, 0.0f, 1.0f,
                         0.0f, 0.0f, 0.0f);
}


// ****************************************************************
/* constructor */
constexpr FORCE_INLINE coreMatrix2::coreMatrix2(const coreFloat f11, const coreFloat f12,
                                                const coreFloat f21, const coreFloat f22)noexcept
: _11 (f11), _12 (f12)
, _21 (f21), _22 (f22)
{
}


// ****************************************************************
/* addition with matrix */
constexpr coreMatrix2 coreMatrix2::operator + (const coreMatrix2& m)const
{
    return coreMatrix2(_11+m._11, _12+m._12,
                       _21+m._21, _22+m._22);
}


// ****************************************************************
/* subtraction with matrix */
constexpr coreMatrix2 coreMatrix2::operator - (const coreMatrix2& m)const
{
    return coreMatrix2(_11-m._11, _12-m._12,
                       _21-m._21, _22-m._22);
}


// ****************************************************************
/* multiplication with matrix */
constexpr coreMatrix2 coreMatrix2::operator * (const coreMatrix2& m)const
{
    return coreMatrix2(_11*m._11 + _12*m._21, _11*m._12 + _12*m._22,
                       _21*m._11 + _22*m._21, _21*m._12 + _22*m._22);
}


// ****************************************************************
/* multiplication with scalar */
constexpr coreMatrix2 coreMatrix2::operator * (const coreFloat f)const
{
    return coreMatrix2(_11*f, _12*f,
                       _21*f, _22*f);
}


// ****************************************************************
/* transpose matrix */
constexpr coreMatrix2 coreMatrix2::Transposed()const
{
    return coreMatrix2(_11, _21,
                       _12, _22);
}


// ****************************************************************
/* invert matrix */
constexpr coreMatrix2 coreMatrix2::Inverted()const
{
    return coreMatrix2( _22, -_12,
                       -_21,  _11) / (_11*_22 - _12*_21);
}


// ****************************************************************
/* calculate determinant */
constexpr coreFloat coreMatrix2::Determinant()const
{
    return _11*_22 - _12*_21;
}


// ****************************************************************
/* get identity matrix */
constexpr coreMatrix2 coreMatrix2::Identity()
{
    return coreMatrix2(1.0f, 0.0f,
                       0.0f, 1.0f);
}


// ****************************************************************
/* constructor */
constexpr FORCE_INLINE coreMatrix3::coreMatrix3(const coreFloat f11, const coreFloat f12, const coreFloat f13,
                                                const coreFloat f21, const coreFloat f22, const coreFloat f23,
                                                const coreFloat f31, const coreFloat f32, const coreFloat f33)noexcept
: _11 (f11), _12 (f12), _13 (f13)
, _21 (f21), _22 (f22), _23 (f23)
, _31 (f31), _32 (f32), _33 (f33)
{
}

constexpr FORCE_INLINE coreMatrix3::coreMatrix3(const coreMatrix3x2& m)noexcept
: _11 (m._11), _12 (m._12), _13 (0.0f)
, _21 (m._21), _22 (m._22), _23 (0.0f)
, _31 (m._31), _32 (m._32), _33 (1.0f)
{
}

constexpr FORCE_INLINE coreMatrix3::coreMatrix3(const coreMatrix2& m)noexcept
: _11 (m._11), _12 (m._12), _13 (0.0f)
, _21 (m._21), _22 (m._22), _23 (0.0f)
, _31  (0.0f), _32  (0.0f), _33 (1.0f)
{
}


// ****************************************************************
/* addition with matrix */
constexpr coreMatrix3 coreMatrix3::operator + (const coreMatrix3& m)const
{
    return coreMatrix3(_11+m._11, _12+m._12, _13+m._13,
                       _21+m._21, _22+m._22, _23+m._23,
                       _31+m._31, _32+m._32, _33+m._33);
}


// ****************************************************************
/* subtraction with matrix */
constexpr coreMatrix3 coreMatrix3::operator - (const coreMatrix3& m)const
{
    return coreMatrix3(_11-m._11, _12-m._12, _13-m._13,
                       _21-m._21, _22-m._22, _23-m._23,
                       _31-m._31, _32-m._32, _33-m._33);
}


// ****************************************************************
/* multiplication with matrix */
constexpr coreMatrix3 coreMatrix3::operator * (const coreMatrix3& m)const
{
    return coreMatrix3(_11*m._11 + _12*m._21 + _13*m._31, _11*m._12 + _12*m._22 + _13*m._32, _11*m._13 + _12*m._23 + _13*m._33,
                       _21*m._11 + _22*m._21 + _23*m._31, _21*m._12 + _22*m._22 + _23*m._32, _21*m._13 + _22*m._23 + _23*m._33,
                       _31*m._11 + _32*m._21 + _33*m._31, _31*m._12 + _32*m._22 + _33*m._32, _31*m._13 + _32*m._23 + _33*m._33);
}


// ****************************************************************
/* multiplication with scalar */
constexpr coreMatrix3 coreMatrix3::operator * (const coreFloat f)const
{
    return coreMatrix3(_11*f, _12*f, _13*f,
                       _21*f, _22*f, _23*f,
                       _31*f, _32*f, _33*f);
}


// ****************************************************************
/* transpose matrix */
constexpr coreMatrix3 coreMatrix3::Transposed()const
{
    return coreMatrix3(_11, _21, _31,
                       _12, _22, _32,
                       _13, _23, _33);
}


// ****************************************************************
/* invert matrix */
constexpr coreMatrix3 coreMatrix3::Inverted()const
{
    const coreFloat A = _22*_33 - _23*_32;
    const coreFloat B = _23*_31 - _21*_33;
    const coreFloat C = _21*_32 - _22*_31;

    return coreMatrix3(A, _13*_32 - _12*_33, _12*_23 - _13*_22,
                       B, _11*_33 - _13*_31, _13*_21 - _11*_23,
                       C, _12*_31 - _11*_32, _11*_22 - _12*_21) / (_11*A + _12*B + _13*C);
}


// ****************************************************************
/* calculate determinant */
constexpr coreFloat coreMatrix3::Determinant()const
{
    return _11*(_22*_33 - _23*_32) + _12*(_23*_31 - _21*_33) + _13*(_21*_32 - _22*_31);
}


// ****************************************************************
/* get identity matrix */
constexpr coreMatrix3 coreMatrix3::Identity()
{
    return coreMatrix3(1.0f, 0.0f, 0.0f,
                       0.0f, 1.0f, 0.0f,
                       0.0f, 0.0f, 1.0f);
}


// ****************************************************************
/* get translation matrix */
constexpr coreMatrix3 coreMatrix3::Translation(const coreVector2 vPosition)
{
    return coreMatrix3(       1.0f,        0.0f, 0.0f,
                              0.0f,        1.0f, 0.0f,
                       vPosition.x, vPosition.y, 1.0f);
}


// ****************************************************************
/* get scale matrix */
constexpr coreMatrix3 coreMatrix3::Scaling(const coreVector2 vSize)
{
    return coreMatrix3(vSize.x,    0.0f, 0.0f,
                          0.0f, vSize.y, 0.0f,
                          0.0f,    0.0f, 1.0f);
}


// ****************************************************************
/* get rotation matrix */
constexpr coreMatrix3 coreMatrix3::Rotation(const coreVector2 vDirection)
{
    ASSERT(vDirection.IsNormalized())
    return coreMatrix3( vDirection.y, vDirection.x, 0.0f,
                       -vDirection.x, vDirection.y, 0.0f,
                                0.0f,         0.0f, 1.0f);
}

inline coreMatrix3 coreMatrix3::Rotation(const coreFloat fAngle)
{
    return coreMatrix3::Rotation(coreVector2::Direction(fAngle));
}


// ****************************************************************
/* get shear matrix along X sheared by Y */
constexpr coreMatrix3 coreMatrix3::ShearXY(const coreFloat fFactor)
{
    return coreMatrix3(   1.0f, 0.0f, 0.0f,
                       fFactor, 1.0f, 0.0f,
                          0.0f, 0.0f, 1.0f);
}


// ****************************************************************
/* get shear matrix along Y sheared by X */
constexpr coreMatrix3 coreMatrix3::ShearYX(const coreFloat fFactor)
{
    return coreMatrix3(1.0f, fFactor, 0.0f,
                       0.0f,    1.0f, 0.0f,
                       0.0f,    0.0f, 1.0f);
}


// ****************************************************************
/* convert matrix to quaternion (# only use precise calculations) */
constexpr coreVector4 coreMatrix3::ToQuat()const
{
    const coreFloat fTrace = _11 + _22 + _33;

    if(fTrace > 0.0f)
    {
        const coreFloat F = 0.5f / SQRT(fTrace + 1.0f);
        return coreVector4((_23 - _32) * F,
                           (_31 - _13) * F,
                           (_12 - _21) * F,
                                 0.25f / F);
    }
    else
    {
        if((_11 > _22) && (_11 > _33))
        {
            const coreFloat F = 0.5f / SQRT(_11 - _22 - _33 + 1.0f);
            return coreVector4(      0.25f / F,
                               (_21 + _12) * F,
                               (_31 + _13) * F,
                               (_23 - _32) * F);
        }
        else if(_22 > _33)
        {
            const coreFloat F = 0.5f / SQRT(_22 - _11 - _33 + 1.0f);
            return coreVector4((_21 + _12) * F,
                                     0.25f / F,
                               (_32 + _23) * F,
                               (_31 - _13) * F);
        }
        else
        {
            const coreFloat F = 0.5f / SQRT(_33 - _11 - _22 + 1.0f);
            return coreVector4((_31 + _13) * F,
                               (_32 + _23) * F,
                                     0.25f / F,
                               (_12 - _21) * F);
        }
    }
}


// ****************************************************************
/* convert quaternion to matrix */
constexpr coreMatrix3 coreMatrix3::FromQuat(const coreVector4 v)
{
    const coreFloat XX = v.x*v.x;
    const coreFloat XY = v.x*v.y;
    const coreFloat XZ = v.x*v.z;
    const coreFloat XW = v.x*v.w;
    const coreFloat YY = v.y*v.y;
    const coreFloat YZ = v.y*v.z;
    const coreFloat YW = v.y*v.w;
    const coreFloat ZZ = v.z*v.z;
    const coreFloat ZW = v.z*v.w;

    return coreMatrix3(1.0f - 2.0f * (YY + ZZ),        2.0f * (XY + ZW),        2.0f * (XZ - YW),
                              2.0f * (XY - ZW), 1.0f - 2.0f * (XX + ZZ),        2.0f * (YZ + XW),
                              2.0f * (XZ + YW),        2.0f * (YZ - XW), 1.0f - 2.0f * (XX + YY));
}


// ****************************************************************
/* constructor */
constexpr FORCE_INLINE coreMatrix4::coreMatrix4(const coreFloat f11, const coreFloat f12, const coreFloat f13, const coreFloat f14,
                                                const coreFloat f21, const coreFloat f22, const coreFloat f23, const coreFloat f24,
                                                const coreFloat f31, const coreFloat f32, const coreFloat f33, const coreFloat f34,
                                                const coreFloat f41, const coreFloat f42, const coreFloat f43, const coreFloat f44)noexcept
: _11 (f11), _12 (f12), _13 (f13), _14 (f14)
, _21 (f21), _22 (f22), _23 (f23), _24 (f24)
, _31 (f31), _32 (f32), _33 (f33), _34 (f34)
, _41 (f41), _42 (f42), _43 (f43), _44 (f44)
{
}

constexpr FORCE_INLINE coreMatrix4::coreMatrix4(const coreMatrix4x3& m)noexcept
: _11 (m._11), _12 (m._12), _13 (m._13), _14 (0.0f)
, _21 (m._21), _22 (m._22), _23 (m._23), _24 (0.0f)
, _31 (m._31), _32 (m._32), _33 (m._33), _34 (0.0f)
, _41 (m._41), _42 (m._42), _43 (m._43), _44 (1.0f)
{
}

constexpr FORCE_INLINE coreMatrix4::coreMatrix4(const coreMatrix3x2& m)noexcept
: _11 (m._11), _12 (m._12), _13 (0.0f), _14 (0.0f)
, _21 (m._21), _22 (m._22), _23 (0.0f), _24 (0.0f)
, _31 (m._31), _32 (m._32), _33 (1.0f), _34 (0.0f)
, _41  (0.0f), _42  (0.0f), _43 (0.0f), _44 (1.0f)
{
}

constexpr FORCE_INLINE coreMatrix4::coreMatrix4(const coreMatrix3& m)noexcept
: _11 (m._11), _12 (m._12), _13 (m._13), _14 (0.0f)
, _21 (m._21), _22 (m._22), _23 (m._23), _24 (0.0f)
, _31 (m._31), _32 (m._32), _33 (m._33), _34 (0.0f)
, _41  (0.0f), _42  (0.0f), _43  (0.0f), _44 (1.0f)
{
}

constexpr FORCE_INLINE coreMatrix4::coreMatrix4(const coreMatrix2& m)noexcept
: _11 (m._11), _12 (m._12), _13 (0.0f), _14 (0.0f)
, _21 (m._21), _22 (m._22), _23 (0.0f), _24 (0.0f)
, _31  (0.0f), _32  (0.0f), _33 (1.0f), _34 (0.0f)
, _41  (0.0f), _42  (0.0f), _43 (0.0f), _44 (1.0f)
{
}


// ****************************************************************
/* addition with matrix */
constexpr coreMatrix4 coreMatrix4::operator + (const coreMatrix4& m)const
{
    return coreMatrix4(_11+m._11, _12+m._12, _13+m._13, _14+m._14,
                       _21+m._21, _22+m._22, _23+m._23, _24+m._24,
                       _31+m._31, _32+m._32, _33+m._33, _34+m._34,
                       _41+m._41, _42+m._42, _43+m._43, _44+m._44);
}


// ****************************************************************
/* subtraction with matrix */
constexpr coreMatrix4 coreMatrix4::operator - (const coreMatrix4& m)const
{
    return coreMatrix4(_11-m._11, _12-m._12, _13-m._13, _14-m._14,
                       _21-m._21, _22-m._22, _23-m._23, _24-m._24,
                       _31-m._31, _32-m._32, _33-m._33, _34-m._34,
                       _41-m._41, _42-m._42, _43-m._43, _44-m._44);
}


// ****************************************************************
/* multiplication with matrix */
constexpr coreMatrix4 coreMatrix4::operator * (const coreMatrix4& m)const
{
    return coreMatrix4(_11*m._11 + _12*m._21 + _13*m._31 + _14*m._41, _11*m._12 + _12*m._22 + _13*m._32 + _14*m._42,
                       _11*m._13 + _12*m._23 + _13*m._33 + _14*m._43, _11*m._14 + _12*m._24 + _13*m._34 + _14*m._44,
                       _21*m._11 + _22*m._21 + _23*m._31 + _24*m._41, _21*m._12 + _22*m._22 + _23*m._32 + _24*m._42,
                       _21*m._13 + _22*m._23 + _23*m._33 + _24*m._43, _21*m._14 + _22*m._24 + _23*m._34 + _24*m._44,
                       _31*m._11 + _32*m._21 + _33*m._31 + _34*m._41, _31*m._12 + _32*m._22 + _33*m._32 + _34*m._42,
                       _31*m._13 + _32*m._23 + _33*m._33 + _34*m._43, _31*m._14 + _32*m._24 + _33*m._34 + _34*m._44,
                       _41*m._11 + _42*m._21 + _43*m._31 + _44*m._41, _41*m._12 + _42*m._22 + _43*m._32 + _44*m._42,
                       _41*m._13 + _42*m._23 + _43*m._33 + _44*m._43, _41*m._14 + _42*m._24 + _43*m._34 + _44*m._44);
}


// ****************************************************************
/* multiplication with scalar */
constexpr coreMatrix4 coreMatrix4::operator * (const coreFloat f)const
{
    return coreMatrix4(_11*f, _12*f, _13*f, _14*f,
                       _21*f, _22*f, _23*f, _24*f,
                       _31*f, _32*f, _33*f, _34*f,
                       _41*f, _42*f, _43*f, _44*f);
}


// ****************************************************************
/* transpose matrix */
constexpr coreMatrix4 coreMatrix4::Transposed()const
{
    return coreMatrix4(_11, _21, _31, _41,
                       _12, _22, _32, _42,
                       _13, _23, _33, _43,
                       _14, _24, _34, _44);
}


// ****************************************************************
/* invert matrix */
constexpr coreMatrix4 coreMatrix4::Inverted()const
{
    const coreFloat A = _11*_22 - _12*_21;
    const coreFloat B = _11*_23 - _13*_21;
    const coreFloat C = _11*_24 - _14*_21;
    const coreFloat D = _12*_23 - _13*_22;
    const coreFloat E = _12*_24 - _14*_22;
    const coreFloat F = _13*_24 - _14*_23;
    const coreFloat G = _31*_42 - _32*_41;
    const coreFloat H = _31*_43 - _33*_41;
    const coreFloat I = _31*_44 - _34*_41;
    const coreFloat J = _32*_43 - _33*_42;
    const coreFloat K = _32*_44 - _34*_42;
    const coreFloat L = _33*_44 - _34*_43;

    return coreMatrix4(_22*L - _23*K + _24*J, _13*K - _12*L - _14*J, _42*F - _43*E + _44*D, _33*E - _32*F - _34*D,
                       _23*I - _21*L - _24*H, _11*L - _13*I + _14*H, _43*C - _41*F - _44*B, _31*F - _33*C + _34*B,
                       _21*K - _22*I + _24*G, _12*I - _11*K - _14*G, _41*E - _42*C + _44*A, _32*C - _31*E - _34*A,
                       _22*H - _21*J - _23*G, _11*J - _12*H + _13*G, _42*B - _41*D - _43*A, _31*D - _32*B + _33*A) / (A*L - B*K + C*J + D*I - E*H + F*G);
}


// ****************************************************************
/* calculate determinant */
constexpr coreFloat coreMatrix4::Determinant()const
{
    return (_11*_22 - _12*_21) * (_33*_44 - _34*_43) - (_11*_23 - _13*_21) * (_32*_44 - _34*_42) +
           (_11*_24 - _14*_21) * (_32*_43 - _33*_42) + (_12*_23 - _13*_22) * (_31*_44 - _34*_41) -
           (_12*_24 - _14*_22) * (_31*_43 - _33*_41) + (_13*_24 - _14*_23) * (_31*_42 - _32*_41);
}


// ****************************************************************
/* get identity matrix */
constexpr coreMatrix4 coreMatrix4::Identity()
{
    return coreMatrix4(1.0f, 0.0f, 0.0f, 0.0f,
                       0.0f, 1.0f, 0.0f, 0.0f,
                       0.0f, 0.0f, 1.0f, 0.0f,
                       0.0f, 0.0f, 0.0f, 1.0f);
}


// ****************************************************************
/* get translation matrix */
constexpr coreMatrix4 coreMatrix4::Translation(const coreVector3 vPosition)
{
    return coreMatrix4(       1.0f,        0.0f,        0.0f, 0.0f,
                              0.0f,        1.0f,        0.0f, 0.0f,
                              0.0f,        0.0f,        1.0f, 0.0f,
                       vPosition.x, vPosition.y, vPosition.z, 1.0f);
}


// ****************************************************************
/* get scale matrix */
constexpr coreMatrix4 coreMatrix4::Scaling(const coreVector3 vSize)
{
    return coreMatrix4(vSize.x,    0.0f,    0.0f, 0.0f,
                          0.0f, vSize.y,    0.0f, 0.0f,
                          0.0f,    0.0f, vSize.z, 0.0f,
                          0.0f,    0.0f,    0.0f, 1.0f);
}


// ****************************************************************
/* get rotation matrix around X */
constexpr coreMatrix4 coreMatrix4::RotationX(const coreVector2 vDirection)
{
    ASSERT(vDirection.IsNormalized())
    return coreMatrix4(1.0f,          0.0f,          0.0f, 0.0f,
                       0.0f,  vDirection.y,  vDirection.x, 0.0f,
                       0.0f, -vDirection.x,  vDirection.y, 0.0f,
                       0.0f,          0.0f,          0.0f, 1.0f);
}

inline coreMatrix4 coreMatrix4::RotationX(const coreFloat fAngle)
{
    return coreMatrix4::RotationX(coreVector2::Direction(fAngle));
}


// ****************************************************************
/* get rotation matrix around Y */
constexpr coreMatrix4 coreMatrix4::RotationY(const coreVector2 vDirection)
{
    ASSERT(vDirection.IsNormalized())
    return coreMatrix4(vDirection.y, 0.0f, -vDirection.x, 0.0f,
                               0.0f, 1.0f,          0.0f, 0.0f,
                       vDirection.x, 0.0f,  vDirection.y, 0.0f,
                               0.0f, 0.0f,          0.0f, 1.0f);
}

inline coreMatrix4 coreMatrix4::RotationY(const coreFloat fAngle)
{
    return coreMatrix4::RotationY(coreVector2::Direction(fAngle));
}


// ****************************************************************
/* get rotation matrix around Z */
constexpr coreMatrix4 coreMatrix4::RotationZ(const coreVector2 vDirection)
{
    ASSERT(vDirection.IsNormalized())
    return coreMatrix4( vDirection.y, vDirection.x, 0.0f, 0.0f,
                       -vDirection.x, vDirection.y, 0.0f, 0.0f,
                                0.0f,         0.0f, 1.0f, 0.0f,
                                0.0f,         0.0f, 0.0f, 1.0f);
}

inline coreMatrix4 coreMatrix4::RotationZ(const coreFloat fAngle)
{
    return coreMatrix4::RotationZ(coreVector2::Direction(fAngle));
}


// ****************************************************************
/* get rotation matrix around arbitrary axis */
inline coreMatrix4 coreMatrix4::RotationAxis(const coreFloat fAngle, const coreVector3 vAxis)
{
    ASSERT(vAxis.IsNormalized())

    const coreFloat C = COS(fAngle);
    const coreFloat S = SIN(fAngle);
    const coreFloat I = 1.0f - C;

    const coreFloat XX = vAxis.x * vAxis.x * I;
    const coreFloat YY = vAxis.y * vAxis.y * I;
    const coreFloat ZZ = vAxis.z * vAxis.z * I;

    const coreFloat XY = vAxis.x * vAxis.y * I;
    const coreFloat XZ = vAxis.x * vAxis.z * I;
    const coreFloat YZ = vAxis.y * vAxis.z * I;

    const coreFloat SX = vAxis.x * S;
    const coreFloat SY = vAxis.y * S;
    const coreFloat SZ = vAxis.z * S;

    return coreMatrix4(XX +  C, XY + SZ, XZ - SY, 0.0f,
                       XY - SZ, YY +  C, YZ + SX, 0.0f,
                       XZ + SY, YZ - SX, ZZ +  C, 0.0f,
                          0.0f,    0.0f,    0.0f, 1.0f);
}


// ****************************************************************
/* get shear matrix along X sheared by Y */
constexpr coreMatrix4 coreMatrix4::ShearXY(const coreFloat fFactor)
{
    return coreMatrix4(   1.0f, 0.0f, 0.0f, 0.0f,
                       fFactor, 1.0f, 0.0f, 0.0f,
                          0.0f, 0.0f, 1.0f, 0.0f,
                          0.0f, 0.0f, 0.0f, 1.0f);
}


// ****************************************************************
/* get shear matrix along X sheared by Z */
constexpr coreMatrix4 coreMatrix4::ShearXZ(const coreFloat fFactor)
{
    return coreMatrix4(   1.0f, 0.0f, 0.0f, 0.0f,
                          0.0f, 1.0f, 0.0f, 0.0f,
                       fFactor, 0.0f, 1.0f, 0.0f,
                          0.0f, 0.0f, 0.0f, 1.0f);
}


// ****************************************************************
/* get shear matrix along Y sheared by X */
constexpr coreMatrix4 coreMatrix4::ShearYX(const coreFloat fFactor)
{
    return coreMatrix4(1.0f, fFactor, 0.0f, 0.0f,
                       0.0f,    1.0f, 0.0f, 0.0f,
                       0.0f,    0.0f, 1.0f, 0.0f,
                       0.0f,    0.0f, 0.0f, 1.0f);
}


// ****************************************************************
/* get shear matrix along Y sheared by Z */
constexpr coreMatrix4 coreMatrix4::ShearYZ(const coreFloat fFactor)
{
    return coreMatrix4(1.0f,    0.0f, 0.0f, 0.0f,
                       0.0f,    1.0f, 0.0f, 0.0f,
                       0.0f, fFactor, 1.0f, 0.0f,
                       0.0f,    0.0f, 0.0f, 1.0f);
}


// ****************************************************************
/* get shear matrix along Z sheared by X */
constexpr coreMatrix4 coreMatrix4::ShearZX(const coreFloat fFactor)
{
    return coreMatrix4(1.0f, 0.0f, fFactor, 0.0f,
                       0.0f, 1.0f,    0.0f, 0.0f,
                       0.0f, 0.0f,    1.0f, 0.0f,
                       0.0f, 0.0f,    0.0f, 1.0f);
}


// ****************************************************************
/* get shear matrix along Z sheared by Y */
constexpr coreMatrix4 coreMatrix4::ShearZY(const coreFloat fFactor)
{
    return coreMatrix4(1.0f, 0.0f,    0.0f, 0.0f,
                       0.0f, 1.0f, fFactor, 0.0f,
                       0.0f, 0.0f,    1.0f, 0.0f,
                       0.0f, 0.0f,    0.0f, 1.0f);
}


// ****************************************************************
/* calculate orientation matrix */
constexpr coreMatrix4 coreMatrix4::Orientation(const coreVector3 vDirection, const coreVector3 vOrientation)
{
    ASSERT(vDirection.IsNormalized() && vOrientation.IsNormalized())

    const coreVector3 D = vDirection;
    const coreVector3 O = vOrientation;
    const coreVector3 S = coreVector3::Cross(D, O).Normalized();
    const coreVector3 U = coreVector3::Cross(S, D);

    return coreMatrix4( S.x,  S.y,  S.z, 0.0f,
                        D.x,  D.y,  D.z, 0.0f,
                        U.x,  U.y,  U.z, 0.0f,
                       0.0f, 0.0f, 0.0f, 1.0f);
}


// ****************************************************************
/* calculate perspective projection matrix */
inline coreMatrix4 coreMatrix4::Perspective(const coreVector2 vResolution, const coreFloat fFOV, const coreFloat fNearClip, const coreFloat fFarClip)
{
    const coreFloat   A = COT(fFOV * 0.5f);
    const coreVector2 B = vResolution.yx().LowRatio() * A;

    const coreFloat N = fNearClip;
    const coreFloat F = fFarClip;
    const coreFloat I = RCP(N-F);

    return coreMatrix4( B.x, 0.0f,       0.0f,  0.0f,
                       0.0f,  B.y,       0.0f,  0.0f,
                       0.0f, 0.0f,    (N+F)*I, -1.0f,
                       0.0f, 0.0f, 2.0f*N*F*I,  0.0f);
}


// ****************************************************************
/* calculate orthographic projection matrix */
constexpr coreMatrix4 coreMatrix4::Ortho(const coreVector2 vResolution)
{
    const coreFloat X = 2.0f * RCP(vResolution.x);
    const coreFloat Y = 2.0f * RCP(vResolution.y);

    return coreMatrix4(   X, 0.0f, 0.0f, 0.0f,
                       0.0f,    Y, 0.0f, 0.0f,
                       0.0f, 0.0f, 1.0f, 0.0f,
                       0.0f, 0.0f, 0.0f, 1.0f);
}

constexpr coreMatrix4 coreMatrix4::Ortho(const coreFloat fLeft, const coreFloat fRight, const coreFloat fBottom, const coreFloat fTop, const coreFloat fNearClip, const coreFloat fFarClip)
{
    const coreFloat X =  RCP(fRight   - fLeft);
    const coreFloat Y =  RCP(fTop     - fBottom);
    const coreFloat Z = -RCP(fFarClip - fNearClip);

    const coreFloat A = -(fRight   + fLeft);
    const coreFloat B = -(fTop     + fBottom);
    const coreFloat C =  (fFarClip + fNearClip);

    return coreMatrix4(X*2.0f,   0.0f,   0.0f, 0.0f,
                         0.0f, Y*2.0f,   0.0f, 0.0f,
                         0.0f,   0.0f, Z*2.0f, 0.0f,
                          X*A,    Y*B,    Z*C, 1.0f);
}


// ****************************************************************
/* calculate camera matrix */
constexpr coreMatrix4 coreMatrix4::Camera(const coreVector3 vPosition, const coreVector3 vDirection, const coreVector3 vOrientation)
{
    coreFloat fTemp;

    coreMatrix4 mCamera = coreMatrix4::Orientation(vDirection, vOrientation);
    fTemp = mCamera._21; mCamera._21 = mCamera._31; mCamera._31 = -fTemp;
    fTemp = mCamera._22; mCamera._22 = mCamera._32; mCamera._32 = -fTemp;
    fTemp = mCamera._23; mCamera._23 = mCamera._33; mCamera._33 = -fTemp;

    mCamera._14 = -(mCamera._11*vPosition.x + mCamera._12*vPosition.y + mCamera._13*vPosition.z);
    mCamera._24 = -(mCamera._21*vPosition.x + mCamera._22*vPosition.y + mCamera._23*vPosition.z);
    mCamera._34 = -(mCamera._31*vPosition.x + mCamera._32*vPosition.y + mCamera._33*vPosition.z);

    return mCamera.Transposed();
}


// ****************************************************************
/* multiplication with matrix */
constexpr coreVector2 coreVector2::operator * (const coreMatrix2& m)const
{
    return coreVector2(x*m._11 + y*m._21,
                       x*m._12 + y*m._22);
}

constexpr coreVector2 coreVector2::operator * (const coreMatrix3& m)const
{
    return coreVector2(x*m._11 + y*m._21 + m._31,
                       x*m._12 + y*m._22 + m._32) /
                      (x*m._13 + y*m._23 + m._33);
}

constexpr coreVector2 coreVector2::operator * (const coreMatrix4& m)const
{
    return coreVector2(x*m._11 + y*m._21 + m._41,
                       x*m._12 + y*m._22 + m._42) /
                      (x*m._14 + y*m._24 + m._44);
}


// ****************************************************************
/* multiplication with matrix */
constexpr coreVector3 coreVector3::operator * (const coreMatrix3& m)const
{
    return coreVector3(x*m._11 + y*m._21 + z*m._31,
                       x*m._12 + y*m._22 + z*m._32,
                       x*m._13 + y*m._23 + z*m._33);
}

constexpr coreVector3 coreVector3::operator * (const coreMatrix4& m)const
{
    return coreVector3(x*m._11 + y*m._21 + z*m._31 + m._41,
                       x*m._12 + y*m._22 + z*m._32 + m._42,
                       x*m._13 + y*m._23 + z*m._33 + m._43) /
                      (x*m._14 + y*m._24 + z*m._34 + m._44);
}


// ****************************************************************
/* multiplication with matrix */
constexpr coreVector4 coreVector4::operator * (const coreMatrix4& m)const
{
    return coreVector4(x*m._11 + y*m._21 + z*m._31 + w*m._41,
                       x*m._12 + y*m._22 + z*m._32 + w*m._42,
                       x*m._13 + y*m._23 + z*m._33 + w*m._43,
                       x*m._14 + y*m._24 + z*m._34 + w*m._44);
}


// ****************************************************************
/* convert RGB-color to YIQ-color (BT.601, NTSC) */
constexpr coreVector3 coreVector3::RgbToYiq()const
{
    return (*this) * coreMatrix3(0.299f,  0.587f,  0.114f,
                                 0.596f, -0.275f, -0.321f,
                                 0.212f, -0.523f,  0.311f);
}


// ****************************************************************
/* convert YIQ-color (BT.601, NTSC) to RGB-color */
constexpr coreVector3 coreVector3::YiqToRgb()const
{
    return (*this) * coreMatrix3(1.000f,  0.956f,  0.620f,
                                 1.000f, -0.272f, -0.647f,
                                 1.000f, -1.108f,  1.705f);
}


// ****************************************************************
/* convert RGB-color to YUV-color (BT.709) */
constexpr coreVector3 coreVector3::RgbToYuv()const
{
    return (*this) * coreMatrix3( 0.21260f,  0.71520f,  0.07220f,
                                 -0.09991f, -0.33609f,  0.43600f,
                                  0.61500f, -0.55861f, -0.05639f);
}


// ****************************************************************
/* convert YUV-color (BT.709) to RGB-color */
constexpr coreVector3 coreVector3::YuvToRgb()const
{
    return (*this) * coreMatrix3(1.00000f,  0.00000f,  1.28033f,
                                 1.00000f, -0.21482f, -0.38059f,
                                 1.00000f,  2.12798f,  0.00000f);
}


// ****************************************************************
/* convert RGB-color to YCbCr-color (BT.601, JPEG) */
constexpr coreVector3 coreVector3::RgbToYcbcr()const
{
    return (*this) * coreMatrix3( 0.299000f,  0.587000f,  0.114000f,
                                 -0.168736f, -0.331264f,  0.500000f,
                                  0.500000f, -0.418688f, -0.081312f) + coreVector3(0.0f,0.5f,0.5f);
}


// ****************************************************************
/* convert YCbCr-color (BT.601, JPEG) to RGB-color */
constexpr coreVector3 coreVector3::YcbcrToRgb()const
{
    return ((*this) - coreVector3(0.0f,0.5f,0.5f)) * coreMatrix3(1.00000f,  0.00000f,  1.40200f,
                                                                 1.00000f, -0.34414f, -0.71414f,
                                                                 1.00000f,  1.77200f,  0.00000f);
}


// ****************************************************************
/* additional checks */
STATIC_ASSERT(std::is_trivial<coreMatrix3x2>::value)
STATIC_ASSERT(std::is_trivial<coreMatrix4x3>::value)
STATIC_ASSERT(std::is_trivial<coreMatrix2>  ::value)
STATIC_ASSERT(std::is_trivial<coreMatrix3>  ::value)
STATIC_ASSERT(std::is_trivial<coreMatrix4>  ::value)


#endif /* _CORE_GUARD_MATRIX_H_ */