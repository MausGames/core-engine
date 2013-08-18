#pragma once


// ****************************************************************
// two-dimensional vector
class coreVector2
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
        float m[2];
    };


public:
    coreVector2();
    coreVector2(const coreVector2& c);
    coreVector2(coreVector2&& c);
    coreVector2(const float& fx, const float& fy);

    // assignment operators
    coreVector2& operator = (const coreVector2& c);
    coreVector2& operator = (coreVector2&& c);

    // compare operators
    inline bool operator == (const coreVector2& c)const {return ((x == c.x) && (y == c.y));}
    inline bool operator != (const coreVector2& c)const {return ((x != c.x) || (y != c.y));}

    // vector calculation operators
    coreVector2 operator + (const coreVector2& c)const;
    coreVector2 operator - (const coreVector2& c)const;
    coreVector2 operator * (const coreVector2& c)const;
    coreVector2 operator / (const coreVector2& c)const;
    inline void operator += (const coreVector2& c) {*this = *this + c;}
    inline void operator -= (const coreVector2& c) {*this = *this - c;}
    inline void operator *= (const coreVector2& c) {*this = *this * c;}
    inline void operator /= (const coreVector2& c) {*this = *this / c;}

    // scalar calculation operators
    inline coreVector2 operator + (const float& f)const                         {return *this + coreVector2(f, f);}
    inline coreVector2 operator - (const float& f)const                         {return *this - coreVector2(f, f);}
    inline coreVector2 operator * (const float& f)const                         {return *this * coreVector2(f, f);}
    inline coreVector2 operator / (const float& f)const                         {return *this / coreVector2(f, f);}
    inline void operator += (const float& f)                                    {*this = *this + f;}
    inline void operator -= (const float& f)                                    {*this = *this - f;}
    inline void operator *= (const float& f)                                    {*this = *this * f;}
    inline void operator /= (const float& f)                                    {*this = *this / f;}
    friend inline coreVector2 operator + (const float& f, const coreVector2& c) {return c + f;}
    friend inline coreVector2 operator * (const float& f, const coreVector2& c) {return c * f;}

    // matrix calculation operators
    coreVector2 operator * (const coreMatrix& m)const;
    inline void operator *= (const coreMatrix& m) {*this = *this * m;}

    // invert vector
    inline coreVector2 operator - ()const {return coreVector2(-x, -y);}

    // convert vector
    inline coreVector2 yx()const {return coreVector2(y, x);}

    // normalize vector
    coreVector2& Normalize();
    coreVector2 Normalized()const {return coreVector2(*this).Normalize();}

    // direct functions
    float LengthSq()const;
    inline float Length()const      {return coreMath::Sqrt(this->LengthSq());}
    inline float Min()const         {return coreMath::Min(x, y);}
    inline float Max()const         {return coreMath::Max(x, y);}
    inline float Angle()const       {if(y == 0.0f) return DEG_TO_RAD((x < 0.0f) ? 90.0f : 270.0f); return atanf(x/y) + ((y <= 0.0f) ? PI : 0.0f);}
    inline float AspectRatio()const {return x/y;}

    // static functions
    static float Dot(const coreVector2& vInA, const coreVector2& vInB);
    static coreVector2 Rand();
    static coreVector2 Rand(const float& fMin, const float& fMax);
    static coreVector2 Rand(const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY);
    static coreVector2 Reflect(const coreVector2& vVelocity, const coreVector2& vNormal);
    static inline coreVector2 Direction(const float& fAngle) {return coreVector2(-coreMath::Sin(fAngle), coreMath::Cos(fAngle));}
};


// ****************************************************************
// three-dimensional vector
class coreVector3
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
    coreVector3();
    coreVector3(const coreVector3& c);
    coreVector3(const coreVector2& c);
    coreVector3(coreVector3&& c);
    coreVector3(coreVector2&& c);
    coreVector3(const float& fx, const float& fy, const float& fz);

    // assignment operators
    coreVector3& operator = (const coreVector3& c);
    coreVector3& operator = (coreVector3&& c);

    // compare operators
    inline bool operator == (const coreVector3& c)const {return ((x == c.x) && (y == c.y) && (z == c.z));}
    inline bool operator != (const coreVector3& c)const {return ((x != c.x) || (y != c.y) || (z != c.z));}

    // vector calculation operators
    coreVector3 operator + (const coreVector3& c)const;
    coreVector3 operator - (const coreVector3& c)const;
    coreVector3 operator * (const coreVector3& c)const;
    coreVector3 operator / (const coreVector3& c)const;
    inline void operator += (const coreVector3& c) {*this = *this + c;}
    inline void operator -= (const coreVector3& c) {*this = *this - c;}
    inline void operator *= (const coreVector3& c) {*this = *this * c;}
    inline void operator /= (const coreVector3& c) {*this = *this / c;}

    // scalar calculation operators
    inline coreVector3 operator + (const float& f)const                         {return *this + coreVector3(f, f, f);}
    inline coreVector3 operator - (const float& f)const                         {return *this - coreVector3(f, f, f);}
    inline coreVector3 operator * (const float& f)const                         {return *this * coreVector3(f, f, f);}
    inline coreVector3 operator / (const float& f)const                         {return *this / coreVector3(f, f, f);}
    inline void operator += (const float& f)                                    {*this = *this + f;}
    inline void operator -= (const float& f)                                    {*this = *this - f;}
    inline void operator *= (const float& f)                                    {*this = *this * f;}
    inline void operator /= (const float& f)                                    {*this = *this / f;}
    friend inline coreVector3 operator + (const float& f, const coreVector3& c) {return c + f;}
    friend inline coreVector3 operator * (const float& f, const coreVector3& c) {return c * f;}

    // matrix calculation operators
    coreVector3 operator * (const coreMatrix& m)const;
    inline void operator *= (const coreMatrix& m) {*this = *this * m;}

    // invert vector
    inline coreVector3 operator - ()const {return coreVector3(-x, -y, -z);}

    // convert vector
    inline operator coreVector2& () {return *(reinterpret_cast<coreVector2*>(this));}
    inline coreVector2 xy()const    {return coreVector2(x, y);}
    inline coreVector2 xz()const    {return coreVector2(x, z);}
    inline coreVector2 yx()const    {return coreVector2(y, x);}
    inline coreVector2 yz()const    {return coreVector2(y, z);}
    inline coreVector2 zx()const    {return coreVector2(z, x);}
    inline coreVector2 zy()const    {return coreVector2(z, y);}

    // normalize vector
    coreVector3& Normalize();
    coreVector3 Normalized()const {return coreVector3(*this).Normalize();}

    // direct functions
    float LengthSq()const;
    inline float Length()const {return coreMath::Sqrt(this->LengthSq());}
    inline float Min()const    {return coreMath::Min(coreMath::Min(x, y), z);}
    inline float Max()const    {return coreMath::Max(coreMath::Max(x, y), z);}

    // static functions
    static float Dot(const coreVector3& vInA, const coreVector3& vInB);
    static coreVector3 Rand();
    static coreVector3 Rand(const float& fMin, const float& fMax);
    static coreVector3 Rand(const float& fMinX, const float& fMaxX, const float& fMinY, const float& fMaxY, const float& fMinZ, const float& fMaxZ);
    static coreVector3 Reflect(const coreVector3& vVelocity, const coreVector3& vNormal);
    static coreVector3 Cross(const coreVector3& vInA, const coreVector3& vInB);
};


// ****************************************************************
// four-dimensional vector
class coreVector4
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
    coreVector4();
    coreVector4(const coreVector4& c);
    coreVector4(const coreVector3& c);
    coreVector4(const coreVector2& c);
    coreVector4(coreVector4&& c);
    coreVector4(coreVector3&& c);
    coreVector4(coreVector2&& c);
    coreVector4(const float& fx, const float& fy, const float& fz, const float& fw);

    // assignment operators
    coreVector4& operator = (const coreVector4& c);
    coreVector4& operator = (coreVector4&& c);

    // compare operators
    inline bool operator == (const coreVector4& c)const {return ((x == c.x) && (y == c.y) && (z == c.z) && (z == c.w));}
    inline bool operator != (const coreVector4& c)const {return ((x != c.x) || (y != c.y) || (z != c.z) || (z != c.w));}

    // vector calculation operators
    coreVector4 operator + (const coreVector4& c)const;
    coreVector4 operator - (const coreVector4& c)const;
    coreVector4 operator * (const coreVector4& c)const;
    coreVector4 operator / (const coreVector4& c)const;
    inline void operator += (const coreVector4& c) {*this = *this + c;}
    inline void operator -= (const coreVector4& c) {*this = *this - c;}
    inline void operator *= (const coreVector4& c) {*this = *this * c;}
    inline void operator /= (const coreVector4& c) {*this = *this / c;}

    // scalar calculation operators
    inline coreVector4 operator + (const float& f)const                         {return *this + coreVector4(f, f, f, f);}
    inline coreVector4 operator - (const float& f)const                         {return *this - coreVector4(f, f, f, f);}
    inline coreVector4 operator * (const float& f)const                         {return *this * coreVector4(f, f, f, f);}
    inline coreVector4 operator / (const float& f)const                         {return *this / coreVector4(f, f, f, f);}
    inline void operator += (const float& f)                                    {*this = *this + f;}
    inline void operator -= (const float& f)                                    {*this = *this - f;}
    inline void operator *= (const float& f)                                    {*this = *this * f;}
    inline void operator /= (const float& f)                                    {*this = *this / f;}
    friend inline coreVector4 operator + (const float& f, const coreVector4& c) {return c + f;}
    friend inline coreVector4 operator * (const float& f, const coreVector4& c) {return c * f;}

    // matrix calculation operators
    coreVector4 operator * (const coreMatrix& m)const;
    inline void operator *= (const coreMatrix& m) {*this = *this * m;}

    // invert vector
    inline coreVector4 operator - ()const {return coreVector4(-x, -y, -z, w);}

    // convert vector
    inline operator coreVector3& () {return *(reinterpret_cast<coreVector3*>(this));}
    inline operator coreVector2& () {return *(reinterpret_cast<coreVector2*>(this));}
    inline coreVector3 xyz()const   {return coreVector3(x, y, z)*w;}
};