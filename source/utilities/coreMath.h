//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef GUARD_CORE_MATH_H
#define GUARD_CORE_MATH_H


// ****************************************************************
// math definitions
#define PI        (3.1415926535897932384626433832795f)         //!< Archimedes' constant
#define EU        (2.7182818284590452353602874713527f)         //!< Euler's number
#define P3        (0.3333333333333333333333333333333f)         //!< periodic value of 1.0f/3.0f
#define P6        (0.6666666666666666666666666666667f)         //!< periodic value of 2.0f/3.0f
#define TO_RAD(x) ((x) * 0.0174532925199432957692369076848f)   //!< convert degree to radian
#define TO_DEG(x) ((x) * 57.295779513082320876798154814105f)   //!< convert radian to degree

#define LERP(x,y,s)  coreMath::Lerp(x,y,s)
#define LERPS(x,y,s) coreMath::LerpSmooth(x,y,s)
#define LERPB(x,y,s) coreMath::LerpBreak(x,y,s)
#define MIN(x,y)     coreMath::Min(x,y)
#define MAX(x,y)     coreMath::Max(x,y)
#define CLAMP(a,x,y) coreMath::Clamp(a,x,y)
#define ABS(x)       coreMath::Abs(x)
#define SIG(x)       coreMath::Sig(x)


// ****************************************************************
// math collection
class coreMath
{
public:
    //! convenient logical operations
    //! @{
    template <typename T> static inline constexpr T Lerp(const T& x, const T& y, const float& s)       {return x + (y - x) * s;}
    template <typename T> static inline constexpr T LerpSmooth(const T& x, const T& y, const float& s) {return Lerp(x, y, 0.5f-0.5f*Cos(s*PI));}
    template <typename T> static inline constexpr T LerpBreak(const T& x, const T& y, const float& s)  {return Lerp(x, y, Sin(s*0.5f*PI));}
    template <typename T> static inline constexpr T Min(const T& x, const T& y)                        {return (x < y) ? x : y;}
    template <typename T> static inline constexpr T Max(const T& x, const T& y)                        {return (x > y) ? x : y;}
    template <typename T> static inline constexpr T Clamp(const T& a, const T& x, const T& y)          {return Min(Max(a, x), y);}
    static inline constexpr int Abs(const int& x)                                                      {return (x < 0) ? -x : x;}
    static inline constexpr float Abs(const float& x)                                                  {return (x < 0.0f) ? -x : x;}
    static inline constexpr int Sig(const int& x)                                                      {return (x < 0) ? -1 : 1;}
    static inline constexpr float Sig(const float& x)                                                  {return (x < 0.0f) ? -1.0f : 1.0f;}
    //! @}

    //! calculate square root
    //! @{
    static float Sqrt(float fInput);
    //! @}

    //! calculate trigonometric values
    //! @{
    static float Sin(const float& fInput);
    static float Cos(const float& fInput);
    //! @}

    //! check if inside field-of-view
    //! @{
    static bool CheckFOV(const coreVector3& vPosition, const float& fFOV, const coreVector3& vCamPosition, const coreVector3& vCamDirection);
    //! @}

    //! calculate next power-of-two
    //! @{
    static inline coreUint NextPOT(const coreUint& iInput) {coreUint k = 1; while(k < iInput) k = k<<1; return k;}
    //! @}

    //! convert color format
    //! @{
    static coreVector3 HSVtoRGB(const coreVector3& vHSV);
    static coreVector3 RGBtoHSV(const coreVector3& vRGB);
    //! @}
};


#endif // GUARD_CORE_MATH_H