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
// 2d-vector class
class coreVector2 final
{
public:
    union
    {
        struct
        {
            float x, y;
        };
        struct
        {
            float u, v;
        };
        struct
        {
            float s, t;
        };
        float m[2];
    };


public:
    constexpr_func coreVector2()noexcept                                 : x (0.0f), y (0.0f) {}
    constexpr_func coreVector2(const float& fx, const float& fy)noexcept : x (fx),   y (fy)   {}

    //! compare operators
    //! @{
    inline bool operator == (const coreVector2& v)const noexcept {return std::memcmp(this, &v, sizeof(coreVector2)) ? false :  true;}
    inline bool operator != (const coreVector2& v)const noexcept {return std::memcmp(this, &v, sizeof(coreVector2)) ?  true : false;}
    //! @}

    //! vector calculation operators
    //! @{
    constexpr_func coreVector2 operator + (const coreVector2& v)const noexcept {return coreVector2(x+v.x, y+v.y);}
    constexpr_func coreVector2 operator - (const coreVector2& v)const noexcept {return coreVector2(x-v.x, y-v.y);}
    constexpr_func coreVector2 operator * (const coreVector2& v)const noexcept {return coreVector2(x*v.x, y*v.y);}
    constexpr_func coreVector2 operator / (const coreVector2& v)const noexcept {return coreVector2(x/v.x, y/v.y);}
    inline void operator += (const coreVector2& v)noexcept                     {*this = *this + v;}
    inline void operator -= (const coreVector2& v)noexcept                     {*this = *this - v;}
    inline void operator *= (const coreVector2& v)noexcept                     {*this = *this * v;}
    inline void operator /= (const coreVector2& v)noexcept                     {*this = *this / v;}
    //! @}

    //! scalar calculation operators
    //! @{
    constexpr_func coreVector2 operator + (const float& f)const noexcept                        {return coreVector2(x+f, y+f);}
    constexpr_func coreVector2 operator - (const float& f)const noexcept                        {return coreVector2(x-f, y-f);}
    constexpr_func coreVector2 operator * (const float& f)const noexcept                        {return coreVector2(x*f, y*f);}
    constexpr_func coreVector2 operator / (const float& f)const noexcept                        {return *this * (1.0f/f);}
    inline void operator += (const float& f)noexcept                                            {*this = *this + f;}
    inline void operator -= (const float& f)noexcept                                            {*this = *this - f;}
    inline void operator *= (const float& f)noexcept                                            {*this = *this * f;}
    inline void operator /= (const float& f)noexcept                                            {*this = *this / f;}
    friend constexpr_func coreVector2 operator + (const float& f, const coreVector2& v)noexcept {return v + f;}
    friend constexpr_func coreVector2 operator * (const float& f, const coreVector2& v)noexcept {return v * f;}
    //! @}

    //! matrix calculation operators
    //! @{
    inline coreVector2 operator * (const coreMatrix4& m)const noexcept;
    inline void operator *= (const coreMatrix4& m)noexcept {*this = *this * m;}
    //! @}

    //! invert vector
    //! @{
    constexpr_func coreVector2 operator - ()const noexcept {return coreVector2(-x, -y);}
    //! @}

    //! convert vector
    //! @{
    constexpr_func operator const float* ()const noexcept {return r_cast<const float*>(this);}
    constexpr_func coreVector2 yx()const noexcept         {return coreVector2(y, x);}
    //! @}

    //! normalize vector
    //! @{
    inline coreVector2& Normalize()noexcept;
    inline coreVector2 Normalized()const noexcept {return coreVector2(*this).Normalize();}
    //! @}

    //! direct functions
    //! @{
    inline float Length()const noexcept              {return coreMath::Sqrt(this->LengthSq());}
    constexpr_func float LengthSq()const noexcept    {return (x*x + y*y);}
    constexpr_func float Min()const noexcept         {return coreMath::Min(x, y);}
    constexpr_func float Max()const noexcept         {return coreMath::Max(x, y);}
    constexpr_func float AspectRatio()const noexcept {return (x/y);}
    inline float Angle()const noexcept               {if(y == 0.0f) return (x < 0.0f) ? TO_RAD(90.0f) : TO_RAD(270.0f); return coreMath::Atan(x/y) + ((y <= 0.0f) ? PI : 0.0f);}
    //! @}

    //! static functions
    //! @{
    static constexpr_func float Dot(const coreVector2& vInA, const coreVector2& vInB)noexcept {return (vInA.x*vInB.x + vInA.y*vInB.y);}
    static inline coreVector2 Rand()noexcept;
    static inline coreVector2 Rand(const float& fMin, const float& fMax)noexcept;
    static inline coreVector2 Rand(const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY)noexcept;
    static inline coreVector2 Reflect(const coreVector2& vVelocity, const coreVector2& vNormal)noexcept;
    static inline coreVector2 Direction(const float& fAngle)noexcept {return coreVector2(-coreMath::Sin(fAngle), coreMath::Cos(fAngle));}
    //! @}
};


// ****************************************************************
// 3d-vector class
class coreVector3 final
{
public:
    union
    {
        struct
        {
            float x, y, z;
        };
        struct
        {
            float r, g, b;
        };
        float m[3];
    };


public:
    constexpr_func coreVector3()noexcept                                                  : x (0.0f), y (0.0f), z (0.0f) {}
    constexpr_func coreVector3(const coreVector2& c, const float& fz)noexcept             : x (c.x),  y (c.y),  z (fz)   {}
    constexpr_func coreVector3(const float& fx, const coreVector2& c)noexcept             : x (fx),   y (c.x),  z (c.y)  {}
    constexpr_func coreVector3(const float& fx, const float& fy, const float& fz)noexcept : x (fx),   y (fy),   z (fz)   {}

    //! compare operators
    //! @{
    inline bool operator == (const coreVector3& v)const noexcept {return std::memcmp(this, &v, sizeof(coreVector3)) ? false :  true;}
    inline bool operator != (const coreVector3& v)const noexcept {return std::memcmp(this, &v, sizeof(coreVector3)) ?  true : false;}
    //! @}

    //! vector calculation operators
    //! @{
    constexpr_func coreVector3 operator + (const coreVector3& v)const noexcept {return coreVector3(x+v.x, y+v.y, z+v.z);}
    constexpr_func coreVector3 operator - (const coreVector3& v)const noexcept {return coreVector3(x-v.x, y-v.y, z-v.z);}
    constexpr_func coreVector3 operator * (const coreVector3& v)const noexcept {return coreVector3(x*v.x, y*v.y, z*v.z);}
    constexpr_func coreVector3 operator / (const coreVector3& v)const noexcept {return coreVector3(x/v.x, y/v.y, z/v.z);}
    inline void operator += (const coreVector3& v)noexcept                     {*this = *this + v;}
    inline void operator -= (const coreVector3& v)noexcept                     {*this = *this - v;}
    inline void operator *= (const coreVector3& v)noexcept                     {*this = *this * v;}
    inline void operator /= (const coreVector3& v)noexcept                     {*this = *this / v;}
    //! @}

    //! scalar calculation operators
    //! @{
    constexpr_func coreVector3 operator + (const float& f)const noexcept                        {return coreVector3(x+f, y+f, z+f);}
    constexpr_func coreVector3 operator - (const float& f)const noexcept                        {return coreVector3(x-f, y-f, z-f);}
    constexpr_func coreVector3 operator * (const float& f)const noexcept                        {return coreVector3(x*f, y*f, z*f);}
    constexpr_func coreVector3 operator / (const float& f)const noexcept                        {return *this * (1.0f/f);}
    inline void operator += (const float& f)noexcept                                            {*this = *this + f;}
    inline void operator -= (const float& f)noexcept                                            {*this = *this - f;}
    inline void operator *= (const float& f)noexcept                                            {*this = *this * f;}
    inline void operator /= (const float& f)noexcept                                            {*this = *this / f;}
    friend constexpr_func coreVector3 operator + (const float& f, const coreVector3& v)noexcept {return v + f;}
    friend constexpr_func coreVector3 operator * (const float& f, const coreVector3& v)noexcept {return v * f;}
    //! @}

    //! matrix calculation operators
    //! @{
    inline coreVector3 operator * (const coreMatrix4& m)const noexcept;
    inline void operator *= (const coreMatrix4& m)noexcept {*this = *this * m;}
    //! @}

    //! invert vector
    //! @{
    constexpr_func coreVector3 operator - ()const noexcept {return coreVector3(-x, -y, -z);}
    //! @}

    //! convert vector
    //! @{
    constexpr_func operator const float* ()const noexcept {return r_cast<const float*>(this);}
    constexpr_func const coreVector2& xy()const noexcept  {return *(r_cast<const coreVector2*>(this));}
    constexpr_func const coreVector2& yz()const noexcept  {return *(r_cast<const coreVector2*>(&y));}
    constexpr_func coreVector2 xz()const noexcept         {return coreVector2(x, z);}
    constexpr_func coreVector2 yx()const noexcept         {return coreVector2(y, x);}
    constexpr_func coreVector2 zx()const noexcept         {return coreVector2(z, x);}
    constexpr_func coreVector2 zy()const noexcept         {return coreVector2(z, y);}
    inline void xy(const coreVector2& v)noexcept          {x = v.x; y = v.y;}
    inline void yz(const coreVector2& v)noexcept          {y = v.x; z = v.y;}
    inline void xz(const coreVector2& v)noexcept          {x = v.x; z = v.y;}
    //! @}

    //! normalize vector
    //! @{
    inline coreVector3& Normalize()noexcept;
    inline coreVector3 Normalized()const noexcept {return coreVector3(*this).Normalize();}
    //! @}

    //! direct functions
    //! @{
    inline float Length()const noexcept           {return coreMath::Sqrt(this->LengthSq());}
    constexpr_func float LengthSq()const noexcept {return (x*x + y*y + z*z);}
    constexpr_func float Min()const noexcept      {return coreMath::Min(coreMath::Min(x, y), z);}
    constexpr_func float Max()const noexcept      {return coreMath::Max(coreMath::Max(x, y), z);}
    //! @}

    //! static functions
    //! @{
    static constexpr_func float Dot(const coreVector3& vInA, const coreVector3& vInB)noexcept {return (vInA.x*vInB.x + vInA.y*vInB.y + vInA.z*vInB.z);}
    static constexpr_func coreVector3 Cross(const coreVector3& vInA, const coreVector3& vInB)noexcept;
    static inline coreVector3 Rand()noexcept;
    static inline coreVector3 Rand(const float& fMin, const float& fMax)noexcept;
    static inline coreVector3 Rand(const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY, const float& fMinZ, const float& fMaxZ)noexcept;
    static inline coreVector3 Reflect(const coreVector3& vVelocity, const coreVector3& vNormal)noexcept;
    //! @}

    //! color functions
    //! @{
    static constexpr_func coreVector3 ColorUnpack(const coreUint& iNumber)noexcept;
    constexpr_func coreUint ColorPack()const noexcept;
    //! @}
};


// ****************************************************************
// 4d-vector class
// TODO: maybe implement real quaternion class later with animations
class coreVector4 final
{
public:
    union
    {
        struct
        {
            float x, y, z, w;
        };
        struct
        {
            float r, g, b, a;
        };
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

    //! compare operators
    //! @{
    inline bool operator == (const coreVector4& v)const noexcept {return std::memcmp(this, &v, sizeof(coreVector4)) ? false :  true;}
    inline bool operator != (const coreVector4& v)const noexcept {return std::memcmp(this, &v, sizeof(coreVector4)) ?  true : false;}
    //! @}

    //! vector calculation operators
    //! @{
    constexpr_func coreVector4 operator + (const coreVector4& v)const noexcept {return coreVector4(x+v.x, y+v.y, z+v.z, w+v.w);}
    constexpr_func coreVector4 operator - (const coreVector4& v)const noexcept {return coreVector4(x-v.x, y-v.y, z-v.z, w-v.w);}
    constexpr_func coreVector4 operator * (const coreVector4& v)const noexcept {return coreVector4(x*v.x, y*v.y, z*v.z, w*v.w);}
    constexpr_func coreVector4 operator / (const coreVector4& v)const noexcept {return coreVector4(x/v.x, y/v.y, z/v.z, w/v.w);}
    inline void operator += (const coreVector4& v)noexcept                     {*this = *this + v;}
    inline void operator -= (const coreVector4& v)noexcept                     {*this = *this - v;}
    inline void operator *= (const coreVector4& v)noexcept                     {*this = *this * v;}
    inline void operator /= (const coreVector4& v)noexcept                     {*this = *this / v;}
    //! @}

    //! scalar calculation operators
    //! @{
    constexpr_func coreVector4 operator + (const float& f)const noexcept                        {return coreVector4(x+f, y+f, z+f, w+f);}
    constexpr_func coreVector4 operator - (const float& f)const noexcept                        {return coreVector4(x-f, y-f, z-f, w-f);}
    constexpr_func coreVector4 operator * (const float& f)const noexcept                        {return coreVector4(x*f, y*f, z*f, w*f);}
    constexpr_func coreVector4 operator / (const float& f)const noexcept                        {return *this * (1.0f/f);}
    inline void operator += (const float& f)noexcept                                            {*this = *this + f;}
    inline void operator -= (const float& f)noexcept                                            {*this = *this - f;}
    inline void operator *= (const float& f)noexcept                                            {*this = *this * f;}
    inline void operator /= (const float& f)noexcept                                            {*this = *this / f;}
    friend constexpr_func coreVector4 operator + (const float& f, const coreVector4& v)noexcept {return v + f;}
    friend constexpr_func coreVector4 operator * (const float& f, const coreVector4& v)noexcept {return v * f;}
    //! @}

    //! matrix calculation operators
    //! @{
    constexpr_func coreVector4 operator * (const coreMatrix4& m)const noexcept;
    inline void operator *= (const coreMatrix4& m)noexcept {*this = *this * m;}
    //! @}

    //! invert vector
    //! @{
    constexpr_func coreVector4 operator - ()const noexcept {return coreVector4(-x, -y, -z, -w);}
    //! @}

    //! convert vector
    //! @{
    constexpr_func operator const float* ()const noexcept {return r_cast<const float*>(this);}
    constexpr_func coreVector3 xyzw()const noexcept       {return coreVector3(x, y, z)*w;}
    constexpr_func const coreVector3& xyz()const noexcept {return *(r_cast<const coreVector3*>(this));}
    constexpr_func const coreVector2& xy()const noexcept  {return *(r_cast<const coreVector2*>(this));}
    constexpr_func const coreVector2& zw()const noexcept  {return *(r_cast<const coreVector2*>(&z));}
    inline void xyz(const coreVector3& v)noexcept         {x = v.x; y = v.y; z = v.z;}
    inline void xy(const coreVector2& v)noexcept          {x = v.x; y = v.y;}
    inline void zw(const coreVector2& v)noexcept          {z = v.x; w = v.y;}
    //! @}

    //! direct functions
    //! @{
    inline float Length()const noexcept           {return coreMath::Sqrt(this->LengthSq());}
    constexpr_func float LengthSq()const noexcept {return (x*x + y*y + z*z + w*w);}
    constexpr_func float Min()const noexcept      {return coreMath::Min(coreMath::Min(coreMath::Min(x, y), z), w);}
    constexpr_func float Max()const noexcept      {return coreMath::Max(coreMath::Max(coreMath::Max(x, y), z), w);}
    //! @}

    //! color functions
    //! @{
    static constexpr_func coreVector4 ColorUnpack(const coreUint& iNumber)noexcept;
    constexpr_func coreUint ColorPack()const noexcept;
    //! @}

    //! quaternion functions
    //! @{
    static constexpr_func coreVector4 QuatMul(const coreVector4& vInA, const coreVector4& vInB)noexcept;
    constexpr_func coreVector4 QuatConjugation()const noexcept               {return coreVector4(-x, -y, -z, w);}
    constexpr_func coreVector4 QuatInverse()const noexcept                   {return coreVector4(-x, -y, -z, w) / this->LengthSq();}
    constexpr_func coreVector3 QuatApply(const coreVector3& v)const noexcept {return QuatMul(QuatMul((*this), coreVector4(v,0.0f)), this->QuatConjugation()).xyz();}
    //! @}
};


// ****************************************************************
// normalize vector
inline coreVector2& coreVector2::Normalize()noexcept
{
    const float fLength = this->LengthSq();
    if(fLength != 1.0f) *this *= coreMath::Rsqrt(fLength);

    return *this;
}


// ****************************************************************
// generate random vector
inline coreVector2 coreVector2::Rand()noexcept
{
    return coreVector2(Core::Rand->Float(-1.0f, 1.0f),
                       Core::Rand->Float(-1.0f, 1.0f)).Normalize();
}

inline coreVector2 coreVector2::Rand(const float& fMin, const float& fMax)noexcept
{
    return coreVector2(Core::Rand->Float(-1.0f, 1.0f),
                       Core::Rand->Float(-1.0f, 1.0f)).Normalize() * Core::Rand->Float(fMin, fMax);
}

inline coreVector2 coreVector2::Rand(const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY)noexcept
{
    return coreVector2(Core::Rand->Float(fMinX, fMaxX),
                       Core::Rand->Float(fMinY, fMaxY));
}


// ****************************************************************
// calculate reflected vector
inline coreVector2 coreVector2::Reflect(const coreVector2& vVelocity, const coreVector2& vNormal)noexcept
{
    const float fDot = coreVector2::Dot(vVelocity, vNormal);
    return (fDot > 0.0f) ? vVelocity : (vVelocity - 2.0f*vNormal*fDot);
}


// ****************************************************************
// calculate cross product
constexpr_func coreVector3 coreVector3::Cross(const coreVector3& vInA, const coreVector3& vInB)noexcept
{
    return coreVector3(vInA.y*vInB.z - vInA.z*vInB.y,
                       vInA.z*vInB.x - vInA.x*vInB.z,
                       vInA.x*vInB.y - vInA.y*vInB.x);
}

// ****************************************************************
// normalize vector
inline coreVector3& coreVector3::Normalize()noexcept
{
    const float fLength = this->LengthSq();
    if(fLength != 1.0f) *this *= coreMath::Rsqrt(fLength);

    return *this;
}


// ****************************************************************
// generate random vector
inline coreVector3 coreVector3::Rand()noexcept
{
    return coreVector3(Core::Rand->Float(-1.0f, 1.0f),
                       Core::Rand->Float(-1.0f, 1.0f),
                       Core::Rand->Float(-1.0f, 1.0f)).Normalize();
}

inline coreVector3 coreVector3::Rand(const float& fMin, const float& fMax)noexcept
{
    return coreVector3(Core::Rand->Float(-1.0f, 1.0f),
                       Core::Rand->Float(-1.0f, 1.0f),
                       Core::Rand->Float(-1.0f, 1.0f)).Normalize() * Core::Rand->Float(fMin, fMax);
}

inline coreVector3 coreVector3::Rand(const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY, const float& fMinZ, const float& fMaxZ)noexcept
{
    return coreVector3(Core::Rand->Float(fMinX, fMaxX),
                       Core::Rand->Float(fMinY, fMaxY),
                       Core::Rand->Float(fMinZ, fMaxZ));
}


// ****************************************************************
// calculate reflected vector
inline coreVector3 coreVector3::Reflect(const coreVector3& vVelocity, const coreVector3& vNormal)noexcept
{
    const float fDot = coreVector3::Dot(vVelocity, vNormal);
    return (fDot > 0.0f) ? vVelocity : (vVelocity - 2.0f*vNormal*fDot);
}


// ****************************************************************
// convert RBG color-code to color-vector
constexpr_func coreVector3 coreVector3::ColorUnpack(const coreUint& iNumber)noexcept
{
    return coreVector3(float((iNumber >> 16) % 256)*0.003921569f, 
                       float( iNumber        % 256)*0.003921569f, 
                       float((iNumber >>  8) % 256)*0.003921569f);
}


// ****************************************************************
// convert color-vector to RBG color-code
constexpr_func coreUint coreVector3::ColorPack()const noexcept
{
    return (coreUint(r * 255.0f) << 16) +
           (coreUint(b * 255.0f) <<  8) +
           (coreUint(g * 255.0f));
};


// ****************************************************************
// multiplicate two quaternions
constexpr_func coreVector4 coreVector4::QuatMul(const coreVector4& vInA, const coreVector4& vInB)noexcept
{
    return coreVector4(vInA.x*vInB.w + vInA.w*vInB.x + vInA.y*vInB.z - vInA.z*vInB.y,
                       vInA.y*vInB.w + vInA.w*vInB.y + vInA.z*vInB.x - vInA.x*vInB.z,
                       vInA.z*vInB.w + vInA.w*vInB.z + vInA.x*vInB.y - vInA.y*vInB.x,
                       vInA.w*vInB.w - vInA.x*vInB.x - vInA.y*vInB.y - vInA.z*vInB.z);
}


// ****************************************************************
// convert RBGA color-code to color-vector
constexpr_func coreVector4 coreVector4::ColorUnpack(const coreUint& iNumber)noexcept
{
    return coreVector4(float((iNumber >> 24) % 256)*0.003921569f, 
                       float((iNumber >>  8) % 256)*0.003921569f, 
                       float((iNumber >> 16) % 256)*0.003921569f, 
                       float( iNumber        % 256)*0.003921569f);
}


// ****************************************************************
// convert color-vector to RBGA color-code
constexpr_func coreUint coreVector4::ColorPack()const noexcept
{
    return (coreUint(r * 255.0f) << 24) +
           (coreUint(b * 255.0f) << 16) +
           (coreUint(g * 255.0f) <<  8) +
           (coreUint(a * 255.0f));
};


#endif // _CORE_GUARD_VECTOR_H_