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

// TODO: maybe implement real quaternion class later with animations


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
        float m[2];
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
    constexpr_obj operator const float* ()const {return r_cast<const float*>(this);}
    constexpr_func coreVector2 yx()const        {return coreVector2(y, x);}
    //! @}

    /*! normalize vector */
    //! @{
    inline coreVector2& Normalize   ();
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
    //! @}

    /*! static functions */
    //! @{
    static constexpr_func float       Dot      (const coreVector2& vInA, const coreVector2& vInB) {return (vInA.x*vInB.x + vInA.y*vInB.y);}
    static inline         coreVector2 Direction(const float& fAngle)                              {return coreVector2(-SIN(fAngle), COS(fAngle));}
    static inline         coreVector2 Rand     ();
    static inline         coreVector2 Rand     (const float& fMax);
    static inline         coreVector2 Rand     (const float& fMin,  const float& fMax);
    static inline         coreVector2 Rand     (const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY);
    static inline         coreVector2 Reflect  (const coreVector2& vVelocity, const coreVector2& vNormal);
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
        float m[3];
    };


public:
    constexpr_func coreVector3()noexcept                                                  : x (0.0f), y (0.0f), z (0.0f) {}
    constexpr_func coreVector3(const coreVector2& c, const float& fz)noexcept             : x (c.x),  y (c.y),  z (fz)   {}
    constexpr_func coreVector3(const float& fx, const coreVector2& c)noexcept             : x (fx),   y (c.x),  z (c.y)  {}
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
    constexpr_obj operator const float* ()const {return r_cast<const float*>(this);}
    constexpr_func coreVector2 xy()const        {return coreVector2(x, y);}
    constexpr_func coreVector2 xz()const        {return coreVector2(x, z);}
    constexpr_func coreVector2 yx()const        {return coreVector2(y, x);}
    constexpr_func coreVector2 yz()const        {return coreVector2(y, z);}
    constexpr_func coreVector2 zx()const        {return coreVector2(z, x);}
    constexpr_func coreVector2 zy()const        {return coreVector2(z, y);}
    inline void xy(const coreVector2& v)        {x = v.x; y = v.y;}
    inline void xz(const coreVector2& v)        {x = v.x; z = v.y;}
    inline void yx(const coreVector2& v)        {y = v.x; x = v.y;}
    inline void yz(const coreVector2& v)        {y = v.x; z = v.y;}
    inline void zx(const coreVector2& v)        {z = v.x; x = v.y;}
    inline void zy(const coreVector2& v)        {z = v.x; y = v.y;}
    //! @}

    /*! normalize vector */
    //! @{
    inline coreVector3& Normalize   ();
    inline coreVector3  Normalized  ()const {return coreVector3(*this).Normalize();}
    constexpr_func bool IsNormalized()const {return coreMath::InRange(this->LengthSq(), 1.0f, CORE_MATH_PRECISION);}
    //! @}

    /*! direct functions */
    //! @{
    inline         float Length  ()const {return SQRT(this->LengthSq());}
    constexpr_func float LengthSq()const {return (x*x + y*y + z*z);}
    constexpr_func float Min     ()const {return MIN(x, y, z);}
    constexpr_func float Max     ()const {return MAX(x, y, z);}
    //! @}

    /*! static functions */
    //! @{
    static constexpr_func float       Dot    (const coreVector3& vInA, const coreVector3& vInB) {return (vInA.x*vInB.x + vInA.y*vInB.y + vInA.z*vInB.z);}
    static constexpr_func coreVector3 Cross  (const coreVector3& vInA, const coreVector3& vInB);
    static inline         coreVector3 Rand   ();
    static inline         coreVector3 Rand   (const float& fMax);
    static inline         coreVector3 Rand   (const float& fMin,  const float& fMax);
    static inline         coreVector3 Rand   (const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY, const float& fMinZ, const float& fMaxZ);
    static inline         coreVector3 Reflect(const coreVector3& vVelocity, const coreVector3& vNormal);
    static inline         bool        Visible(const coreVector3& vPosition, const float& fFOV, const coreVector3& vViewPosition, const coreVector3& vViewDirection);
    //! @}

    /*! color functions */
    //! @{
    static constexpr_func coreVector3 ColorUnpack(const coreUint& iNumber);
    constexpr_func        coreUint    ColorPack  ()const;
    inline                coreVector3 HSVtoRGB   ()const;
    inline                coreVector3 RGBtoHSV   ()const;
    //! @}
};


// ****************************************************************
/* 4d-vector class */
class coreVector4 final
{
public:
    union
    {
        struct {float x, y, z, w;};
        struct {float r, g, b, a;};
        float m[4];
    };


public:
    constexpr_func coreVector4()noexcept                                                                   : x (0.0f), y (0.0f), z (0.0f), w (0.0f) {}
    constexpr_func coreVector4(const coreVector3& c, const float& fw)noexcept                              : x (c.x),  y (c.y),  z (c.z),  w (fw)   {}
    constexpr_func coreVector4(const float& fx, const coreVector3& c)noexcept                              : x (fx),   y (c.x),  z (c.y),  w (c.z)  {}
    constexpr_func coreVector4(const coreVector2& c, const float& fz, const float& fw)noexcept             : x (c.x),  y (c.y),  z (fz),   w (fw)   {}
    constexpr_func coreVector4(const float& fx, const coreVector2& c, const float& fw)noexcept             : x (fx),   y (c.x),  z (c.y),  w (fw)   {}
    constexpr_func coreVector4(const float& fx, const float& fy, const coreVector2& c)noexcept             : x (fx),   y (fy),   z (c.x),  w (c.y)  {}
    constexpr_func coreVector4(const coreVector2& c1, const coreVector2& c2)noexcept                       : x (c1.x), y (c1.y), z (c2.x), w (c2.y) {}
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
    constexpr_obj operator const float* ()const {return r_cast<const float*>(this);}
    constexpr_func coreVector3 xyzw()const      {return coreVector3(x, y, z)*w;}
    constexpr_func coreVector3 xyz ()const      {return coreVector3(x, y, z);}
    constexpr_func coreVector2 xy  ()const      {return coreVector2(x, y);}
    constexpr_func coreVector2 zw  ()const      {return coreVector2(z, w);}
    inline void xyz(const coreVector3& v)       {x = v.x; y = v.y; z = v.z;}
    inline void xy (const coreVector2& v)       {x = v.x; y = v.y;}
    inline void zw (const coreVector2& v)       {z = v.x; w = v.y;}
    //! @}

    /*! direct functions */
    //! @{
    inline         float Length  ()const {return SQRT(this->LengthSq());}
    constexpr_func float LengthSq()const {return (x*x + y*y + z*z + w*w);}
    constexpr_func float Min     ()const {return MIN(x, y, z, w);}
    constexpr_func float Max     ()const {return MAX(x, y, z, w);}
    //! @}

    /*! color functions */
    //! @{
    static constexpr_func coreVector4 ColorUnpack(const coreUint& iNumber);
    constexpr_func        coreUint    ColorPack  ()const;
    //! @}

    /*! quaternion functions */
    //! @{
    static constexpr_func coreVector4 QuatMul        (const coreVector4& vInA, const coreVector4& vInB);
    constexpr_func        coreVector4 QuatConjugation()const                     {return coreVector4(-x, -y, -z, w);}
    inline                coreVector4 QuatInverse    ()const                     {return coreVector4(-x, -y, -z, w) * RCP(this->LengthSq());}
    constexpr_func        coreVector3 QuatApply      (const coreVector3& v)const {return QuatMul(QuatMul((*this), coreVector4(v, 0.0f)), this->QuatConjugation()).xyz();}
    //! @}
};


// ****************************************************************
/* normalize vector */
inline coreVector2& coreVector2::Normalize()
{
    *this *= RSQRT(this->LengthSq());
    return *this;
}


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
/* calculate cross product */
constexpr_func coreVector3 coreVector3::Cross(const coreVector3& vInA, const coreVector3& vInB)
{
    return coreVector3(vInA.y*vInB.z - vInA.z*vInB.y,
                       vInA.z*vInB.x - vInA.x*vInB.z,
                       vInA.x*vInB.y - vInA.y*vInB.x);
}

// ****************************************************************
/* normalize vector */
inline coreVector3& coreVector3::Normalize()
{
    *this *= RSQRT(this->LengthSq());
    return *this;
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
/* convert RBG color-code to color-vector */
constexpr_func coreVector3 coreVector3::ColorUnpack(const coreUint& iNumber)
{
    return coreVector3(float( iNumber        & 0xFF), 
                       float((iNumber >>  8) & 0xFF), 
                       float((iNumber >> 16) & 0xFF)) * 0.003921569f;
}


// ****************************************************************
/* convert color-vector to RBG color-code */
constexpr_func coreUint coreVector3::ColorPack()const
{
    return (coreUint(b * 255.0f) << 16) +
           (coreUint(g * 255.0f) <<  8) +
           (coreUint(r * 255.0f));
};


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
    const float q = V - t;
    const float o = p + t;

    switch((int)h)
    {
    case 1:  return coreVector3(q, V, p);
    case 2:  return coreVector3(p, V, o);
    case 3:  return coreVector3(p, q, V);
    case 4:  return coreVector3(o, p, V);
    case 5:  return coreVector3(V, p, q);
    default: return coreVector3(V, o, p);
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
/* multiplicate two quaternions */
constexpr_func coreVector4 coreVector4::QuatMul(const coreVector4& vInA, const coreVector4& vInB)
{
    return coreVector4(vInA.x*vInB.w + vInA.w*vInB.x + vInA.y*vInB.z - vInA.z*vInB.y,
                       vInA.y*vInB.w + vInA.w*vInB.y + vInA.z*vInB.x - vInA.x*vInB.z,
                       vInA.z*vInB.w + vInA.w*vInB.z + vInA.x*vInB.y - vInA.y*vInB.x,
                       vInA.w*vInB.w - vInA.x*vInB.x - vInA.y*vInB.y - vInA.z*vInB.z);
}


// ****************************************************************
/* convert RBGA color-code to color-vector */
constexpr_func coreVector4 coreVector4::ColorUnpack(const coreUint& iNumber)
{
    return coreVector4(float( iNumber        & 0xFF), 
                       float((iNumber >>  8) & 0xFF), 
                       float((iNumber >> 16) & 0xFF), 
                       float((iNumber >> 24) & 0xFF)) * 0.003921569f;
}


// ****************************************************************
/* convert color-vector to RGBA color-code */
constexpr_func coreUint coreVector4::ColorPack()const 
{
    return (coreUint(a * 255.0f) << 24) +
           (coreUint(b * 255.0f) << 16) +
           (coreUint(g * 255.0f) <<  8) +
           (coreUint(r * 255.0f));
};


#endif /* _CORE_GUARD_VECTOR_H_ */