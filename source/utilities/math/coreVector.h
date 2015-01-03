//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_VECTOR_H_
#define _CORE_GUARD_VECTOR_H_


// ****************************************************************
/* 2d-vector class */
class coreVector2 final
{
public:
    union
    {
        struct {float x, y;};
        struct {float u, v;};
        struct {float s, t;};
        float arr[2];
    };


public:
    constexpr_func coreVector2()noexcept                                 : x (0.0f), y (0.0f) {}
    constexpr_func coreVector2(const float& fx, const float& fy)noexcept : x (fx),   y (fy)   {}

    /*! compare operations */
    //! @{
    inline bool operator == (const coreVector2& v)const {return std::memcmp(this, &v, sizeof(coreVector2)) ? false :  true;}
    inline bool operator != (const coreVector2& v)const {return std::memcmp(this, &v, sizeof(coreVector2)) ?  true : false;}
    //! @}

    /*! vector operations */
    //! @{
    constexpr_func coreVector2 operator +  (const coreVector2& v)const {return coreVector2(x+v.x, y+v.y);}
    constexpr_func coreVector2 operator -  (const coreVector2& v)const {return coreVector2(x-v.x, y-v.y);}
    constexpr_func coreVector2 operator *  (const coreVector2& v)const {return coreVector2(x*v.x, y*v.y);}
    inline         coreVector2 operator /  (const coreVector2& v)const {return coreVector2(x*RCP(v.x), y*RCP(v.y));}
    inline         void        operator += (const coreVector2& v)      {*this = *this + v;}
    inline         void        operator -= (const coreVector2& v)      {*this = *this - v;}
    inline         void        operator *= (const coreVector2& v)      {*this = *this * v;}
    inline         void        operator /= (const coreVector2& v)      {*this = *this / v;}
    //! @}

    /*! scalar operations */
    //! @{
    constexpr_func        coreVector2 operator +  (const float& f)const                  {return coreVector2(x+f, y+f);}
    constexpr_func        coreVector2 operator -  (const float& f)const                  {return coreVector2(x-f, y-f);}
    constexpr_func        coreVector2 operator *  (const float& f)const                  {return coreVector2(x*f, y*f);}
    inline                coreVector2 operator /  (const float& f)const                  {return  *this * RCP(f);}
    inline                void        operator += (const float& f)                       {*this = *this + f;}
    inline                void        operator -= (const float& f)                       {*this = *this - f;}
    inline                void        operator *= (const float& f)                       {*this = *this * f;}
    inline                void        operator /= (const float& f)                       {*this = *this / f;}
    friend constexpr_func coreVector2 operator +  (const float& f, const coreVector2& v) {return v + f;}
    friend constexpr_func coreVector2 operator *  (const float& f, const coreVector2& v) {return v * f;}
    //! @}

    /*! matrix operations */
    //! @{
    inline coreVector2 operator *  (const coreMatrix4& m)const;
    inline void        operator *= (const coreMatrix4& m) {*this = *this * m;}
    //! @}

    /*! invert vector */
    //! @{
    constexpr_func coreVector2 operator - ()const {return coreVector2(-x, -y);}
    //! @}

    /*! convert vector */
    //! @{
    constexpr_weak operator const float* ()const {return r_cast<const float*>(this);}
    constexpr_func coreVector2 yx()const         {return coreVector2(y, x);}
    //! @}

    /*! normalize vector */
    //! @{
    inline coreVector2& Normalize   ()      {*this *= RSQRT(this->LengthSq()); return *this;}
    inline coreVector2  Normalized  ()const {return coreVector2(*this).Normalize();}
    constexpr_func bool IsNormalized()const {return coreMath::InRange(this->LengthSq(), 1.0f, CORE_MATH_PRECISION);}
    //! @}

    /*! direct functions */
    //! @{
    inline         float Length     ()const {return SQRT(this->LengthSq());}
    constexpr_func float LengthSq   ()const {return (x*x + y*y);}
    constexpr_func float Min        ()const {return MIN(x, y);}
    constexpr_func float Max        ()const {return MAX(x, y);}
    inline         float AspectRatio()const {return (x * RCP(y));}
    inline         float Angle      ()const {return y ? (ATAN(this->AspectRatio()) + ((y < 0.0f) ? PI*1.0f : PI*0.0f)) : ((x < 0.0f) ? PI*0.5f : PI*1.5f);}
    constexpr_func bool  IsNull     ()const {return coreMath::InRange(this->LengthSq(), 0.0f, CORE_MATH_PRECISION);}
    //! @}

    /*! static functions */
    //! @{
    static constexpr_func float       Dot      (const coreVector2& v1, const coreVector2& v2) {return (v1.x*v2.x + v1.y*v2.y);}
    static inline         coreVector2 Direction(const float& fAngle)                          {return coreVector2(-SIN(fAngle), COS(fAngle));}
    static inline         coreVector2 Rand     ();
    static inline         coreVector2 Rand     (const float& fMax);
    static inline         coreVector2 Rand     (const float& fMin,  const float& fMax);
    static inline         coreVector2 Rand     (const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY);
    static inline         coreVector2 Reflect  (const coreVector2& vVelocity, const coreVector2& vNormal);
    //! @}

    /*! packing functions */
    //! @{
    constexpr_func        coreUint    PackUnorm2x16  ()const;
    constexpr_func        coreUint    PackSnorm2x16  ()const;
    static constexpr_func coreVector2 UnpackUnorm2x16(const coreUint& iNumber);
    static inline         coreVector2 UnpackSnorm2x16(const coreUint& iNumber);
    //! @}
};


// ****************************************************************
/* 3d-vector class */
class coreVector3 final
{
public:
    union
    {
        struct {float x, y, z;};
        struct {float r, g, b;};
        float arr[3];
    };


public:
    constexpr_func coreVector3()noexcept                                                  : x (0.0f), y (0.0f), z (0.0f) {}
    constexpr_func coreVector3(const coreVector2& v, const float& fz)noexcept             : x (v.x),  y (v.y),  z (fz)   {}
    constexpr_func coreVector3(const float& fx, const coreVector2& v)noexcept             : x (fx),   y (v.x),  z (v.y)  {}
    constexpr_func coreVector3(const float& fx, const float& fy, const float& fz)noexcept : x (fx),   y (fy),   z (fz)   {}

    /*! compare operations */
    //! @{
    inline bool operator == (const coreVector3& v)const {return std::memcmp(this, &v, sizeof(coreVector3)) ? false :  true;}
    inline bool operator != (const coreVector3& v)const {return std::memcmp(this, &v, sizeof(coreVector3)) ?  true : false;}
    //! @}

    /*! vector operations */
    //! @{
    constexpr_func coreVector3 operator +  (const coreVector3& v)const {return coreVector3(x+v.x, y+v.y, z+v.z);}
    constexpr_func coreVector3 operator -  (const coreVector3& v)const {return coreVector3(x-v.x, y-v.y, z-v.z);}
    constexpr_func coreVector3 operator *  (const coreVector3& v)const {return coreVector3(x*v.x, y*v.y, z*v.z);}
    inline         coreVector3 operator /  (const coreVector3& v)const {return coreVector3(x*RCP(v.x), y*RCP(v.y), z*RCP(v.z));}
    inline         void        operator += (const coreVector3& v)      {*this = *this + v;}
    inline         void        operator -= (const coreVector3& v)      {*this = *this - v;}
    inline         void        operator *= (const coreVector3& v)      {*this = *this * v;}
    inline         void        operator /= (const coreVector3& v)      {*this = *this / v;}
    //! @}

    /*! scalar operations */
    //! @{
    constexpr_func        coreVector3 operator +  (const float& f)const                  {return coreVector3(x+f, y+f, z+f);}
    constexpr_func        coreVector3 operator -  (const float& f)const                  {return coreVector3(x-f, y-f, z-f);}
    constexpr_func        coreVector3 operator *  (const float& f)const                  {return coreVector3(x*f, y*f, z*f);}
    inline                coreVector3 operator /  (const float& f)const                  {return  *this * RCP(f);}
    inline                void        operator += (const float& f)                       {*this = *this + f;}
    inline                void        operator -= (const float& f)                       {*this = *this - f;}
    inline                void        operator *= (const float& f)                       {*this = *this * f;}
    inline                void        operator /= (const float& f)                       {*this = *this / f;}
    friend constexpr_func coreVector3 operator +  (const float& f, const coreVector3& v) {return v + f;}
    friend constexpr_func coreVector3 operator *  (const float& f, const coreVector3& v) {return v * f;}
    //! @}

    /*! matrix operations */
    //! @{
    inline coreVector3 operator *  (const coreMatrix4& m)const;
    inline void        operator *= (const coreMatrix4& m) {*this = *this * m;}
    //! @}

    /*! invert vector */
    //! @{
    constexpr_func coreVector3 operator - ()const {return coreVector3(-x, -y, -z);}
    //! @}

    /*! convert vector */
    //! @{
    constexpr_weak operator const float* ()const {return r_cast<const float*>(this);}
    constexpr_func coreVector2 xy()const         {return coreVector2(x, y);}
    constexpr_func coreVector2 xz()const         {return coreVector2(x, z);}
    constexpr_func coreVector2 yx()const         {return coreVector2(y, x);}
    constexpr_func coreVector2 yz()const         {return coreVector2(y, z);}
    constexpr_func coreVector2 zx()const         {return coreVector2(z, x);}
    constexpr_func coreVector2 zy()const         {return coreVector2(z, y);}
    inline void xy(const coreVector2& v)         {x = v.x; y = v.y;}
    inline void xz(const coreVector2& v)         {x = v.x; z = v.y;}
    inline void yx(const coreVector2& v)         {y = v.x; x = v.y;}
    inline void yz(const coreVector2& v)         {y = v.x; z = v.y;}
    inline void zx(const coreVector2& v)         {z = v.x; x = v.y;}
    inline void zy(const coreVector2& v)         {z = v.x; y = v.y;}
    //! @}

    /*! normalize vector */
    //! @{
    inline coreVector3& Normalize   ()      {*this *= RSQRT(this->LengthSq()); return *this;}
    inline coreVector3  Normalized  ()const {return coreVector3(*this).Normalize();}
    constexpr_func bool IsNormalized()const {return coreMath::InRange(this->LengthSq(), 1.0f, CORE_MATH_PRECISION);}
    //! @}

    /*! direct functions */
    //! @{
    inline         float Length  ()const {return SQRT(this->LengthSq());}
    constexpr_func float LengthSq()const {return (x*x + y*y + z*z);}
    constexpr_func float Min     ()const {return MIN(x, y, z);}
    constexpr_func float Max     ()const {return MAX(x, y, z);}
    constexpr_func bool  IsNull  ()const {return coreMath::InRange(this->LengthSq(), 0.0f, CORE_MATH_PRECISION);}
    //! @}

    /*! static functions */
    //! @{
    static constexpr_func float       Dot    (const coreVector3& v1, const coreVector3& v2) {return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);}
    static constexpr_func coreVector3 Cross  (const coreVector3& v1, const coreVector3& v2);
    static inline         coreVector3 Rand   ();
    static inline         coreVector3 Rand   (const float& fMax);
    static inline         coreVector3 Rand   (const float& fMin,  const float& fMax);
    static inline         coreVector3 Rand   (const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY, const float& fMinZ, const float& fMaxZ);
    static inline         coreVector3 Reflect(const coreVector3& vVelocity, const coreVector3& vNormal);
    static inline         bool        Visible(const coreVector3& vPosition, const float& fFOV, const coreVector3& vViewPosition, const coreVector3& vViewDirection);
    //! @}

    /*! color functions */
    //! @{
    inline coreVector3 HSVtoRGB()const;
    inline coreVector3 RGBtoHSV()const;
    //! @}
};


// ****************************************************************
/* 4d-vector and quaternion class */
class coreVector4 final
{
public:
    union
    {
        struct {float x, y, z, w;};
        struct {float r, g, b, a;};
        float arr[4];
    };


public:
    constexpr_func coreVector4()noexcept                                                                   : x (0.0f), y (0.0f), z (0.0f), w (0.0f) {}
    constexpr_func coreVector4(const coreVector3& v, const float& fw)noexcept                              : x (v.x),  y (v.y),  z (v.z),  w (fw)   {}
    constexpr_func coreVector4(const float& fx, const coreVector3& v)noexcept                              : x (fx),   y (v.x),  z (v.y),  w (v.z)  {}
    constexpr_func coreVector4(const coreVector2& v, const float& fz, const float& fw)noexcept             : x (v.x),  y (v.y),  z (fz),   w (fw)   {}
    constexpr_func coreVector4(const float& fx, const coreVector2& v, const float& fw)noexcept             : x (fx),   y (v.x),  z (v.y),  w (fw)   {}
    constexpr_func coreVector4(const float& fx, const float& fy, const coreVector2& v)noexcept             : x (fx),   y (fy),   z (v.x),  w (v.y)  {}
    constexpr_func coreVector4(const coreVector2& v1, const coreVector2& v2)noexcept                       : x (v1.x), y (v1.y), z (v2.x), w (v2.y) {}
    constexpr_func coreVector4(const float& fx, const float& fy, const float& fz, const float& fw)noexcept : x (fx),   y (fy),   z (fz),   w (fw)   {}

    /*! compare operations */
    //! @{
    inline bool operator == (const coreVector4& v)const {return std::memcmp(this, &v, sizeof(coreVector4)) ? false :  true;}
    inline bool operator != (const coreVector4& v)const {return std::memcmp(this, &v, sizeof(coreVector4)) ?  true : false;}
    //! @}

    /*! vector operations */
    //! @{
    constexpr_func coreVector4 operator +  (const coreVector4& v)const {return coreVector4(x+v.x, y+v.y, z+v.z, w+v.w);}
    constexpr_func coreVector4 operator -  (const coreVector4& v)const {return coreVector4(x-v.x, y-v.y, z-v.z, w-v.w);}
    constexpr_func coreVector4 operator *  (const coreVector4& v)const {return coreVector4(x*v.x, y*v.y, z*v.z, w*v.w);}
    inline         coreVector4 operator /  (const coreVector4& v)const {return coreVector4(x*RCP(v.x), y*RCP(v.y), z*RCP(v.z), w*RCP(v.w));}
    inline         void        operator += (const coreVector4& v)      {*this = *this + v;}
    inline         void        operator -= (const coreVector4& v)      {*this = *this - v;}
    inline         void        operator *= (const coreVector4& v)      {*this = *this * v;}
    inline         void        operator /= (const coreVector4& v)      {*this = *this / v;}
    //! @}

    /*! scalar operations */
    //! @{
    constexpr_func        coreVector4 operator +  (const float& f)const                  {return coreVector4(x+f, y+f, z+f, w+f);}
    constexpr_func        coreVector4 operator -  (const float& f)const                  {return coreVector4(x-f, y-f, z-f, w-f);}
    constexpr_func        coreVector4 operator *  (const float& f)const                  {return coreVector4(x*f, y*f, z*f, w*f);}
    inline                coreVector4 operator /  (const float& f)const                  {return  *this * RCP(f);}
    inline                void        operator += (const float& f)                       {*this = *this + f;}
    inline                void        operator -= (const float& f)                       {*this = *this - f;}
    inline                void        operator *= (const float& f)                       {*this = *this * f;}
    inline                void        operator /= (const float& f)                       {*this = *this / f;}
    friend constexpr_func coreVector4 operator +  (const float& f, const coreVector4& v) {return v + f;}
    friend constexpr_func coreVector4 operator *  (const float& f, const coreVector4& v) {return v * f;}
    //! @}

    /*! matrix operations */
    //! @{
    constexpr_func coreVector4 operator *  (const coreMatrix4& m)const;
    inline         void        operator *= (const coreMatrix4& m) {*this = *this * m;}
    //! @}

    /*! invert vector */
    //! @{
    constexpr_func coreVector4 operator - ()const {return coreVector4(-x, -y, -z, -w);}
    //! @}

    /*! convert vector */
    //! @{
    constexpr_weak operator const float* ()const {return r_cast<const float*>(this);}
    constexpr_func coreVector3 xyzw()const       {return coreVector3(x, y, z)*w;}
    constexpr_func coreVector3 xyz ()const       {return coreVector3(x, y, z);}
    constexpr_func coreVector2 xy  ()const       {return coreVector2(x, y);}
    constexpr_func coreVector2 zw  ()const       {return coreVector2(z, w);}
    inline void xyz(const coreVector3& v)        {x = v.x; y = v.y; z = v.z;}
    inline void xy (const coreVector2& v)        {x = v.x; y = v.y;}
    inline void zw (const coreVector2& v)        {z = v.x; w = v.y;}
    //! @}

    /*! direct functions */
    //! @{
    inline         float Length  ()const {return SQRT(this->LengthSq());}
    constexpr_func float LengthSq()const {return (x*x + y*y + z*z + w*w);}
    constexpr_func float Min     ()const {return MIN(x, y, z, w);}
    constexpr_func float Max     ()const {return MAX(x, y, z, w);}
    constexpr_func bool  IsNull  ()const {return coreMath::InRange(this->LengthSq(), 0.0f, CORE_MATH_PRECISION);}
    //! @}

    /*! packing functions */
    //! @{
    constexpr_func        coreUint    PackUnorm4x8  ()const;
    constexpr_func        coreUint    PackSnorm4x8  ()const;
    constexpr_func        coreUint    PackSnorm210  ()const;
    static constexpr_func coreVector4 UnpackUnorm4x8(const coreUint& iNumber);
    static inline         coreVector4 UnpackSnorm4x8(const coreUint& iNumber);
    static inline         coreVector4 UnpackSnorm210(const coreUint& iNumber);
    //! @}

    /*! quaternion functions */
    //! @{
    static constexpr_func coreVector4 QuatMultiply (const coreVector4& v1, const coreVector4& v2);
    static constexpr_func coreVector4 QuatIdentity ()                          {return coreVector4(0.0f,0.0f,0.0f,1.0f);}
    constexpr_func        coreVector4 QuatConjugate()const                     {return coreVector4(-x, -y, -z, w);}
    inline                coreVector4 QuatInvert   ()const                     {return coreVector4(-x, -y, -z, w) * RCP(this->LengthSq());}
    constexpr_func        coreVector3 QuatApply    (const coreVector3& v)const {return v + 2.0f * coreVector3::Cross(this->xyz(), coreVector3::Cross(this->xyz(), v) + w * v);}
    //! @}
};


// ****************************************************************
/* generate random vector */
inline coreVector2 coreVector2::Rand()
{
    return coreVector2(Core::Rand->Float(2.0f) - 1.0f,
                       Core::Rand->Float(2.0f) - 1.0f).Normalize();
}

inline coreVector2 coreVector2::Rand(const float& fMax)
{
    return coreVector2(Core::Rand->Float(2.0f) - 1.0f,
                       Core::Rand->Float(2.0f) - 1.0f).Normalize() * Core::Rand->Float(fMax);
}

inline coreVector2 coreVector2::Rand(const float& fMin, const float& fMax)
{
    return coreVector2(Core::Rand->Float(2.0f) - 1.0f,
                       Core::Rand->Float(2.0f) - 1.0f).Normalize() * Core::Rand->Float(fMin, fMax);
}

inline coreVector2 coreVector2::Rand(const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY)
{
    return coreVector2(Core::Rand->Float(fMinX, fMaxX),
                       Core::Rand->Float(fMinY, fMaxY));
}


// ****************************************************************
/* calculate reflected vector */
inline coreVector2 coreVector2::Reflect(const coreVector2& vVelocity, const coreVector2& vNormal)
{
    const float fDot = coreVector2::Dot(vVelocity, vNormal);
    return (fDot > 0.0f) ? vVelocity : (vVelocity - 2.0f*vNormal*fDot);
}


// ****************************************************************
/* compress 0.0 to 1.0 vector into YX packed uint */
constexpr_func coreUint coreVector2::PackUnorm2x16()const
{
    return (F_TO_UI(y * 65535.0f) << 16) +
           (F_TO_UI(x * 65535.0f));
}


// ****************************************************************
/* compress -1.0 to 1.0 vector into YX packed uint */
constexpr_func coreUint coreVector2::PackSnorm2x16()const
{
    return (F_TO_UI((y < 0.0f) ? (65536.0f + y*32768.0f) : y*32767.0f) << 16) +
           (F_TO_UI((x < 0.0f) ? (65536.0f + x*32768.0f) : x*32767.0f));
};


// ****************************************************************
/* uncompress YX packed uint into 0.0 to 1.0 vector */
constexpr_func coreVector2 coreVector2::UnpackUnorm2x16(const coreUint& iNumber)
{
    return coreVector2(I_TO_F( iNumber        & 0xFFFFu),
                       I_TO_F((iNumber >> 16) & 0xFFFFu)) * 1.525902189e-5f;
}


// ****************************************************************
/* uncompress YX packed uint into -1.0 to 1.0 vector */
inline coreVector2 coreVector2::UnpackSnorm2x16(const coreUint& iNumber)
{
    const coreVector2 A = coreVector2(I_TO_F( iNumber        & 0xFFFFu),
                                      I_TO_F((iNumber >> 16) & 0xFFFFu));

    return coreVector2((A.x >= 32768.0f) ? ((A.x - 65536.0f)/32768.0f) : (A.x/32767.0f),
                       (A.y >= 32768.0f) ? ((A.y - 65536.0f)/32768.0f) : (A.y/32767.0f));
}


// ****************************************************************
/* calculate cross product */
constexpr_func coreVector3 coreVector3::Cross(const coreVector3& v1, const coreVector3& v2)
{
    return coreVector3(v1.y*v2.z - v1.z*v2.y,
                       v1.z*v2.x - v1.x*v2.z,
                       v1.x*v2.y - v1.y*v2.x);
}


// ****************************************************************
/* generate random vector */
inline coreVector3 coreVector3::Rand()
{
    return coreVector3(Core::Rand->Float(2.0f) - 1.0f,
                       Core::Rand->Float(2.0f) - 1.0f,
                       Core::Rand->Float(2.0f) - 1.0f).Normalize();
}

inline coreVector3 coreVector3::Rand(const float& fMax)
{
    return coreVector3(Core::Rand->Float(2.0f) - 1.0f,
                       Core::Rand->Float(2.0f) - 1.0f,
                       Core::Rand->Float(2.0f) - 1.0f).Normalize() * Core::Rand->Float(fMax);
}

inline coreVector3 coreVector3::Rand(const float& fMin, const float& fMax)
{
    return coreVector3(Core::Rand->Float(2.0f) - 1.0f,
                       Core::Rand->Float(2.0f) - 1.0f,
                       Core::Rand->Float(2.0f) - 1.0f).Normalize() * Core::Rand->Float(fMin, fMax);
}

inline coreVector3 coreVector3::Rand(const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY, const float& fMinZ, const float& fMaxZ)
{
    return coreVector3(Core::Rand->Float(fMinX, fMaxX),
                       Core::Rand->Float(fMinY, fMaxY),
                       Core::Rand->Float(fMinZ, fMaxZ));
}


// ****************************************************************
/* calculate reflected vector */
inline coreVector3 coreVector3::Reflect(const coreVector3& vVelocity, const coreVector3& vNormal)
{
    const float fDot = coreVector3::Dot(vVelocity, vNormal);
    return (fDot > 0.0f) ? vVelocity : (vVelocity - 2.0f*vNormal*fDot);
}


// ****************************************************************
/* check if inside field-of-view */
bool coreVector3::Visible(const coreVector3& vPosition, const float& fFOV, const coreVector3& vViewPosition, const coreVector3& vViewDirection)
{
    // calculate relative position and angle
    const coreVector3 vRelative = (vPosition - vViewPosition).Normalize();
    const float fDot = coreVector3::Dot(vRelative, vViewDirection);

    // check result
    return (fDot < 0.0f) ? false : (fDot > COS(fFOV));
}


// ****************************************************************
/* convert HSV-color to RGB-color */
inline coreVector3 coreVector3::HSVtoRGB()const
{
    const float  H = r * 6.0f;
    const float& S = g;
    const float& V = b;

    const float h = FLOOR(H);

    const float s = V * S;
    const float t = s * (H - h);
    const float p = V - s;

    switch(F_TO_SI(h))
    {
    case 1:  return coreVector3(V - t, V,     p);
    case 2:  return coreVector3(p,     V,     p + t);
    case 3:  return coreVector3(p,     V - t, V);
    case 4:  return coreVector3(p + t, p,     V);
    case 5:  return coreVector3(V,     p,     V - t);
    default: return coreVector3(V,     p + t, p);
    }
}


// ****************************************************************
/* convert RGB-color to HSV-color */
inline coreVector3 coreVector3::RGBtoHSV()const
{
    const float& R = r;
    const float& G = g;
    const float& B = b;

    const float v = this->Max();
    const float d = v - this->Min();

    if(!d) return coreVector3(0.0f, 0.0f, v);

    const float s = d * RCP(v);

    if(R == v) return coreVector3((0.0f + (G - B) * RCP(d)) / 6.0f, s, v);
    if(G == v) return coreVector3((2.0f + (B - R) * RCP(d)) / 6.0f, s, v);
               return coreVector3((4.0f + (R - G) * RCP(d)) / 6.0f, s, v);
}


// ****************************************************************
/* compress 0.0 to 1.0 vector into WZYX packed uint */
constexpr_func coreUint coreVector4::PackUnorm4x8()const
{
    return (F_TO_UI(w * 255.0f) << 24) +
           (F_TO_UI(z * 255.0f) << 16) +
           (F_TO_UI(y * 255.0f) <<  8) +
           (F_TO_UI(x * 255.0f));
};


// ****************************************************************
/* compress -1.0 to 1.0 vector into WZYX packed uint */
constexpr_func coreUint coreVector4::PackSnorm4x8()const
{
    return (F_TO_UI((w < 0.0f) ? (256.0f + w*128.0f) : w*127.0f) << 24) +
           (F_TO_UI((z < 0.0f) ? (256.0f + z*128.0f) : z*127.0f) << 16) +
           (F_TO_UI((y < 0.0f) ? (256.0f + y*128.0f) : y*127.0f) <<  8) +
           (F_TO_UI((x < 0.0f) ? (256.0f + x*128.0f) : x*127.0f));
};


// ****************************************************************
/* compress -1.0 to 1.0 vector into 2_10_10_10_rev packed uint */
constexpr_func coreUint coreVector4::PackSnorm210()const
{
    return (F_TO_UI((w < 0.0f) ? (   4.0f + w*  2.0f) : w*  1.0f) << 30) +
           (F_TO_UI((z < 0.0f) ? (1024.0f + z*512.0f) : z*511.0f) << 20) +
           (F_TO_UI((y < 0.0f) ? (1024.0f + y*512.0f) : y*511.0f) << 10) +
           (F_TO_UI((x < 0.0f) ? (1024.0f + x*512.0f) : x*511.0f));
};


// ****************************************************************
/* uncompress WZYX packed uint into 0.0 to 1.0 vector */
constexpr_func coreVector4 coreVector4::UnpackUnorm4x8(const coreUint& iNumber)
{
    return coreVector4(I_TO_F( iNumber        & 0xFFu),
                       I_TO_F((iNumber >>  8) & 0xFFu),
                       I_TO_F((iNumber >> 16) & 0xFFu),
                       I_TO_F((iNumber >> 24) & 0xFFu)) * 0.003921569f;
}


// ****************************************************************
/* uncompress WZYX packed uint into -1.0 to 1.0 vector */
inline coreVector4 coreVector4::UnpackSnorm4x8(const coreUint& iNumber)
{
    const coreVector4 A = coreVector4(I_TO_F( iNumber        & 0xFFu),
                                      I_TO_F((iNumber >>  8) & 0xFFu),
                                      I_TO_F((iNumber >> 16) & 0xFFu),
                                      I_TO_F((iNumber >> 24) & 0xFFu));

    return coreVector4((A.x >= 128.0f) ? ((A.x - 256.0f)/128.0f) : (A.x/127.0f),
                       (A.y >= 128.0f) ? ((A.y - 256.0f)/128.0f) : (A.y/127.0f),
                       (A.z >= 128.0f) ? ((A.z - 256.0f)/128.0f) : (A.z/127.0f),
                       (A.w >= 128.0f) ? ((A.w - 256.0f)/128.0f) : (A.w/127.0f));
}


// ****************************************************************
/* uncompress 2_10_10_10_rev packed uint into -1.0 to 1.0 vector */
inline coreVector4 coreVector4::UnpackSnorm210(const coreUint& iNumber)
{
    const coreVector4 A = coreVector4(I_TO_F( iNumber        & 0x3FFu),
                                      I_TO_F((iNumber >> 10) & 0x3FFu),
                                      I_TO_F((iNumber >> 20) & 0x3FFu),
                                      I_TO_F((iNumber >> 30) & 0x003u));

    return coreVector4((A.x >= 512.0f) ? ((A.x - 1024.0f)/512.0f) : (A.x/511.0f),
                       (A.y >= 512.0f) ? ((A.y - 1024.0f)/512.0f) : (A.y/511.0f),
                       (A.z >= 512.0f) ? ((A.z - 1024.0f)/512.0f) : (A.z/511.0f),
                       (A.w >=   2.0f) ? ((A.w -    4.0f)/  2.0f) : (A.w/  1.0f));
}


// ****************************************************************
/* multiplicate two quaternions */
constexpr_func coreVector4 coreVector4::QuatMultiply(const coreVector4& v1, const coreVector4& v2)
{
    return coreVector4(v1.x*v2.w + v1.w*v2.x + v1.y*v2.z - v1.z*v2.y,
                       v1.y*v2.w + v1.w*v2.y + v1.z*v2.x - v1.x*v2.z,
                       v1.z*v2.w + v1.w*v2.z + v1.x*v2.y - v1.y*v2.x,
                       v1.w*v2.w - v1.x*v2.x - v1.y*v2.y - v1.z*v2.z);
}


#endif /* _CORE_GUARD_VECTOR_H_ */