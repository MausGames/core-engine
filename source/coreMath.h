//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once


// ****************************************************************
// mathematical macros
#define PI            (3.1415926535897932384626433832795f)         // PI
#define DEG_TO_RAD(x) ((x) * 0.0174532925199432957692369076848f)   // convert degree to radian
#define RAD_TO_DEG(x) ((x) * 57.295779513082320876798154814105f)   // convert radian to degree
#define SIG(x)        ((x) < 0.0f ? -1.0f : 1.0f)                  // get sign of a number


// ****************************************************************
// fast access macros
#define LERP(x,y,s)   coreMath::Lerp(x,y,s)
#define MIN(x,y)      coreMath::Min(x,y)
#define MAX(x,y)      coreMath::Max(x,y)
#define CLAMP(a,x,y)  coreMath::Clamp(a,x,y)
#define ABS(x)        coreMath::Abs(x)


// ****************************************************************
// math collection
class coreMath
{
public:
    // convenient logical operations
    template <typename T> static inline T Lerp(const T& x, const T& y, const float& s) {return x + (y - x) * s;}
    template <typename T> static inline T Min(const T& x, const T& y)                  {if(x < y) return x; return y;}
    template <typename T> static inline T Max(const T& x, const T& y)                  {if(x > y) return x; return y;}
    template <typename T> static inline T Clamp(const T& a, const T& x, const T& y)    {return Min(Max(a, x), y);}
    static inline int Abs(const int& x)                                                {if(x < 0) return -x; return x;}
    static inline float Abs(const float& x)                                            {if(x < 0.0f) return -x; return x;}

    // calculate square root
    static float Sqrt(float fInput);

    // calculate trigonometric values
    static float Sin(const float& fInput);
    static float Cos(const float& fInput);
        
    // check if inside field-of-view
    static bool CheckFOV(const coreVector3& vPosition, const float& fFOV, const coreVector3& vCamPosition, const coreVector3& vCamDirection);

    // calculate next power-of-two
    static inline coreUint NextPOT(const coreUint& iInput) {coreUint k = 1; while(k < iInput) k = k<<1; return k;}

    // convert color format
    static coreVector3 HSVtoRGB(const coreVector3& vHSV);
    static coreVector3 RGBtoHSV(const coreVector3& vRGB);
};