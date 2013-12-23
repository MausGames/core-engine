//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
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
    constexpr_func coreVector2()noexcept;
    constexpr_func coreVector2(const float& fx, const float& fy)noexcept;

    //! compare operators
    //! @{
    inline bool operator == (const coreVector2& v)const noexcept {return ((x == v.x) && (y == v.y));}
    inline bool operator != (const coreVector2& v)const noexcept {return ((x != v.x) || (y != v.y));}
    //! @}

    //! vector calculation operators
    //! @{
    coreVector2 operator + (const coreVector2& v)const noexcept;
    coreVector2 operator - (const coreVector2& v)const noexcept;
    coreVector2 operator * (const coreVector2& v)const noexcept;
    coreVector2 operator / (const coreVector2& v)const noexcept;
    inline void operator += (const coreVector2& v)noexcept {*this = *this + v;}
    inline void operator -= (const coreVector2& v)noexcept {*this = *this - v;}
    inline void operator *= (const coreVector2& v)noexcept {*this = *this * v;}
    inline void operator /= (const coreVector2& v)noexcept {*this = *this / v;}
    //! @}

    //! scalar calculation operators
    //! @{
    inline coreVector2 operator + (const float& f)const noexcept                        {return *this + coreVector2(f, f);}
    inline coreVector2 operator - (const float& f)const noexcept                        {return *this - coreVector2(f, f);}
    inline coreVector2 operator * (const float& f)const noexcept                        {return *this * coreVector2(f, f);}
    inline coreVector2 operator / (const float& f)const noexcept                        {return *this / coreVector2(f, f);}
    inline void operator += (const float& f)noexcept                                    {*this = *this + f;}
    inline void operator -= (const float& f)noexcept                                    {*this = *this - f;}
    inline void operator *= (const float& f)noexcept                                    {*this = *this * f;}
    inline void operator /= (const float& f)noexcept                                    {*this = *this / f;}
    friend inline coreVector2 operator + (const float& f, const coreVector2& v)noexcept {return v + f;}
    friend inline coreVector2 operator * (const float& f, const coreVector2& v)noexcept {return v * f;}
    //! @}

    //! matrix calculation operators
    //! @{
    coreVector2 operator * (const coreMatrix4& m)const noexcept;
    inline void operator *= (const coreMatrix4& m)noexcept {*this = *this * m;}
    //! @}

    //! invert vector
    //! @{
    inline coreVector2 operator - ()const noexcept {return coreVector2(-x, -y);}
    //! @}

    //! convert vector
    //! @{
    inline operator const float* ()const noexcept {return r_cast<const float*>(this);}
    inline coreVector2 yx()const noexcept         {return coreVector2(y, x);}
    //! @}

    //! normalize vector
    //! @{
    coreVector2& Normalize()noexcept;
    inline coreVector2 Normalized()const noexcept {return coreVector2(*this).Normalize();}
    //! @}

    //! direct functions
    //! @{
    float LengthSq()const noexcept;
    inline float Length()const noexcept      {return coreMath::Sqrt(this->LengthSq());}
    inline float Min()const noexcept         {return coreMath::Min(x, y);}
    inline float Max()const noexcept         {return coreMath::Max(x, y);}
    inline float Angle()const noexcept       {if(y == 0.0f) return (x < 0.0f) ? TO_RAD(90.0f) : TO_RAD(270.0f); return coreMath::Atan(x/y) + ((y <= 0.0f) ? PI : 0.0f);}
    inline float AspectRatio()const noexcept {return x/y;}
    //! @}

    //! static functions
    //! @{
    static float Dot(const coreVector2& vInA, const coreVector2& vInB)noexcept;
    static coreVector2 Rand()noexcept;
    static coreVector2 Rand(const float& fMin, const float& fMax)noexcept;
    static coreVector2 Rand(const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY)noexcept;
    static coreVector2 Reflect(const coreVector2& vVelocity, const coreVector2& vNormal)noexcept;
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
    constexpr_func coreVector3()noexcept;
    constexpr_func coreVector3(const coreVector2& c, const float& fz)noexcept;
    constexpr_func coreVector3(const float& fx, const float& fy, const float& fz)noexcept;

    //! compare operators
    //! @{
    inline bool operator == (const coreVector3& v)const noexcept {return ((x == v.x) && (y == v.y) && (z == v.z));}
    inline bool operator != (const coreVector3& v)const noexcept {return ((x != v.x) || (y != v.y) || (z != v.z));}
    //! @}

    //! vector calculation operators
    //! @{
    coreVector3 operator + (const coreVector3& v)const noexcept;
    coreVector3 operator - (const coreVector3& v)const noexcept;
    coreVector3 operator * (const coreVector3& v)const noexcept;
    coreVector3 operator / (const coreVector3& v)const noexcept;
    inline void operator += (const coreVector3& v)noexcept {*this = *this + v;}
    inline void operator -= (const coreVector3& v)noexcept {*this = *this - v;}
    inline void operator *= (const coreVector3& v)noexcept {*this = *this * v;}
    inline void operator /= (const coreVector3& v)noexcept {*this = *this / v;}
    //! @}

    //! scalar calculation operators
    //! @{
    inline coreVector3 operator + (const float& f)const noexcept                        {return *this + coreVector3(f, f, f);}
    inline coreVector3 operator - (const float& f)const noexcept                        {return *this - coreVector3(f, f, f);}
    inline coreVector3 operator * (const float& f)const noexcept                        {return *this * coreVector3(f, f, f);}
    inline coreVector3 operator / (const float& f)const noexcept                        {return *this / coreVector3(f, f, f);}
    inline void operator += (const float& f)noexcept                                    {*this = *this + f;}
    inline void operator -= (const float& f)noexcept                                    {*this = *this - f;}
    inline void operator *= (const float& f)noexcept                                    {*this = *this * f;}
    inline void operator /= (const float& f)noexcept                                    {*this = *this / f;}
    friend inline coreVector3 operator + (const float& f, const coreVector3& v)noexcept {return v + f;}
    friend inline coreVector3 operator * (const float& f, const coreVector3& v)noexcept {return v * f;}
    //! @}

    //! matrix calculation operators
    //! @{
    coreVector3 operator * (const coreMatrix4& m)const noexcept;
    inline void operator *= (const coreMatrix4& m)noexcept {*this = *this * m;}
    //! @}

    //! invert vector
    //! @{
    inline coreVector3 operator - ()const noexcept {return coreVector3(-x, -y, -z);}
    //! @}

    //! convert vector
    //! @{
    inline operator const float* ()const noexcept {return r_cast<const float*>(this);}
    inline const coreVector2& xy()const noexcept  {return *(r_cast<const coreVector2*>(this));}
    inline coreVector2 xz()const noexcept         {return coreVector2(x, z);}
    inline coreVector2 yx()const noexcept         {return coreVector2(y, x);}
    inline coreVector2 yz()const noexcept         {return coreVector2(y, z);}
    inline coreVector2 zx()const noexcept         {return coreVector2(z, x);}
    inline coreVector2 zy()const noexcept         {return coreVector2(z, y);}
    inline void xy(const coreVector2& v)noexcept  {x = v.x; y = v.y;}
    //! @}

    //! normalize vector
    //! @{
    coreVector3& Normalize()noexcept;
    inline coreVector3 Normalized()const noexcept {return coreVector3(*this).Normalize();}
    //! @}

    //! direct functions
    //! @{
    float LengthSq()const noexcept;
    inline float Length()const noexcept {return coreMath::Sqrt(this->LengthSq());}
    inline float Min()const noexcept    {return coreMath::Min(coreMath::Min(x, y), z);}
    inline float Max()const noexcept    {return coreMath::Max(coreMath::Max(x, y), z);}
    //! @}

    //! static functions
    //! @{
    static float Dot(const coreVector3& vInA, const coreVector3& vInB)noexcept;
    static coreVector3 Rand()noexcept;
    static coreVector3 Rand(const float& fMin, const float& fMax)noexcept;
    static coreVector3 Rand(const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY, const float& fMinZ, const float& fMaxZ)noexcept;
    static coreVector3 Reflect(const coreVector3& vVelocity, const coreVector3& vNormal)noexcept;
    static coreVector3 Cross(const coreVector3& vInA, const coreVector3& vInB)noexcept;
    static constexpr_func coreVector3 ColorCode(const coreUint& iCode)noexcept;
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
    constexpr_func coreVector4()noexcept;
    constexpr_func coreVector4(const coreVector3& c, const float& fw)noexcept;
    constexpr_func coreVector4(const coreVector2& c, const float& fz, const float& fw)noexcept;
    constexpr_func coreVector4(const float& fx, const float& fy, const float& fz, const float& fw)noexcept;

    //! compare operators
    //! @{
    inline bool operator == (const coreVector4& v)const noexcept {return ((x == v.x) && (y == v.y) && (z == v.z) && (w == v.w));}
    inline bool operator != (const coreVector4& v)const noexcept {return ((x != v.x) || (y != v.y) || (z != v.z) || (w != v.w));}
    //! @}

    //! vector calculation operators
    //! @{
    coreVector4 operator + (const coreVector4& v)const noexcept;
    coreVector4 operator - (const coreVector4& v)const noexcept;
    coreVector4 operator * (const coreVector4& v)const noexcept;
    coreVector4 operator / (const coreVector4& v)const noexcept;
    inline void operator += (const coreVector4& v)noexcept {*this = *this + v;}
    inline void operator -= (const coreVector4& v)noexcept {*this = *this - v;}
    inline void operator *= (const coreVector4& v)noexcept {*this = *this * v;}
    inline void operator /= (const coreVector4& v)noexcept {*this = *this / v;}
    //! @}

    //! scalar calculation operators
    //! @{
    inline coreVector4 operator + (const float& f)const noexcept                        {return *this + coreVector4(f, f, f, f);}
    inline coreVector4 operator - (const float& f)const noexcept                        {return *this - coreVector4(f, f, f, f);}
    inline coreVector4 operator * (const float& f)const noexcept                        {return *this * coreVector4(f, f, f, f);}
    inline coreVector4 operator / (const float& f)const noexcept                        {return *this / coreVector4(f, f, f, f);}
    inline void operator += (const float& f)noexcept                                    {*this = *this + f;}
    inline void operator -= (const float& f)noexcept                                    {*this = *this - f;}
    inline void operator *= (const float& f)noexcept                                    {*this = *this * f;}
    inline void operator /= (const float& f)noexcept                                    {*this = *this / f;}
    friend inline coreVector4 operator + (const float& f, const coreVector4& v)noexcept {return v + f;}
    friend inline coreVector4 operator * (const float& f, const coreVector4& v)noexcept {return v * f;}
    //! @}

    //! matrix calculation operators
    //! @{
    coreVector4 operator * (const coreMatrix4& m)const noexcept;
    inline void operator *= (const coreMatrix4& m)noexcept {*this = *this * m;}
    //! @}

    //! invert vector
    //! @{
    inline coreVector4 operator - ()const noexcept {return coreVector4(-x, -y, -z, -w);}
    //! @}

    //! convert vector
    //! @{
    inline operator const float* ()const noexcept {return r_cast<const float*>(this);}
    inline coreVector3 xyzw()const noexcept       {return coreVector3(x, y, z)*w;}
    inline const coreVector3& xyz()const noexcept {return *(r_cast<const coreVector3*>(this));}
    inline const coreVector2& xy()const noexcept  {return *(r_cast<const coreVector2*>(this));}
    inline void xyz(const coreVector3& v)noexcept {x = v.x; y = v.y; z = v.z;}
    inline void xy(const coreVector2& v)noexcept  {x = v.x; y = v.y;}
    //! @}

    //! direct functions
    //! @{
    float LengthSq()const noexcept;
    inline float Length()const noexcept {return coreMath::Sqrt(this->LengthSq());}
    inline float Min()const noexcept    {return coreMath::Min(coreMath::Min(coreMath::Min(x, y), z), w);}
    inline float Max()const noexcept    {return coreMath::Max(coreMath::Max(coreMath::Max(x, y), z), w);}
    //! @}

    //! color functions
    //! @{
    static constexpr_func coreVector4 ColorUnpack(const coreUint& iNumber)noexcept;
    constexpr_func coreUint ColorPack()const noexcept;
    //! @}

    //! quaternion functions
    //! @{
    static coreVector4 QuatMul(const coreVector4& vInA, const coreVector4& vInB)noexcept;
    inline coreVector4 QuatConjugation()const noexcept               {return coreVector4(-x, -y, -z, w);}
    inline coreVector4 QuatInverse()const noexcept                   {return coreVector4(-x, -y, -z, w) / this->LengthSq();}
    inline coreVector3 QuatApply(const coreVector3& v)const noexcept {return QuatMul(QuatMul((*this), coreVector4(v,0.0f)), this->QuatConjugation()).xyz();}
    //! @}
};


// ****************************************************************
// constructor
constexpr_func coreVector2::coreVector2()noexcept
: x (0.0f), y (0.0f)
{
}

constexpr_func coreVector2::coreVector2(const float& fx, const float& fy)noexcept
: x (fx), y (fy)
{
}


// ****************************************************************
// constructor
constexpr_func coreVector3::coreVector3()noexcept
: x (0.0f), y (0.0f), z (0.0f)
{
}

constexpr_func coreVector3::coreVector3(const coreVector2& c, const float& fz)noexcept
: x (c.x), y (c.y), z (fz)
{
}

constexpr_func coreVector3::coreVector3(const float& fx, const float& fy, const float& fz)noexcept
: x (fx), y (fy), z (fz)
{
}


// ****************************************************************
// convert color-code to vector
constexpr_func coreVector3 coreVector3::ColorCode(const coreUint& iCode)noexcept
{
    return coreVector3(float((iCode >> 16) % 256)*0.003921569f, 
                       float((iCode >>  8) % 256)*0.003921569f, 
                       float( iCode        % 256)*0.003921569f);
}


// ****************************************************************
// constructor
constexpr_func coreVector4::coreVector4()noexcept
: x (0.0f), y (0.0f), z (0.0f), w (0.0f)
{
}

constexpr_func coreVector4::coreVector4(const coreVector3& c, const float& fw)noexcept
: x (c.x), y (c.y), z (c.z), w (fw)
{
}

constexpr_func coreVector4::coreVector4(const coreVector2& c, const float& fz, const float& fw)noexcept
: x (c.x), y (c.y), z (fz), w (fw)
{
}

constexpr_func coreVector4::coreVector4(const float& fx, const float& fy, const float& fz, const float& fw)noexcept
: x (fx), y (fy), z (fz), w (fw)
{
}


// ****************************************************************
// convert color-code to color-vector
constexpr_func coreVector4 coreVector4::ColorUnpack(const coreUint& iNumber)noexcept
{
    return coreVector4(float((iNumber >> 24) % 256)*0.003921569f, 
                       float((iNumber >> 16) % 256)*0.003921569f, 
                       float((iNumber >>  8) % 256)*0.003921569f, 
                       float( iNumber        % 256)*0.003921569f);
}


// ****************************************************************
// convert color-vector to color-code
constexpr_func coreUint coreVector4::ColorPack()const noexcept
{
    return (coreUint(r * 255.0f) << 24) +
           (coreUint(g * 255.0f) << 16) +
           (coreUint(b * 255.0f) <<  8) +
           (coreUint(a * 255.0f));
};


#endif // _CORE_GUARD_VECTOR_H_