//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MATH_H_
#define _CORE_GUARD_MATH_H_


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
#define SIGN(x)      coreMath::Sign(x)


// ****************************************************************
// math utility collection
class coreMath
{
public:
    //! convenient logical operations
    //! @{
    template <typename T> static constexpr_func T Lerp(const T& x, const T& y, const float& s)noexcept       {return x + (y - x) * s;}
    template <typename T> static constexpr_func T LerpSmooth(const T& x, const T& y, const float& s)noexcept {return Lerp(x, y, 0.5f-0.5f*Cos(s*PI));}
    template <typename T> static constexpr_func T LerpBreak(const T& x, const T& y, const float& s)noexcept  {return Lerp(x, y, Sin(s*0.5f*PI));}
    template <typename T> static constexpr_func T Min(const T& x, const T& y)noexcept                        {return (x < y) ? x : y;}
    template <typename T> static constexpr_func T Max(const T& x, const T& y)noexcept                        {return (x > y) ? x : y;}
    template <typename T> static constexpr_func T Clamp(const T& a, const T& x, const T& y)noexcept          {return Min(Max(a, x), y);}
    static constexpr_func int Abs(const int& x)noexcept                                                      {return (x < 0) ? -x : x;}
    static constexpr_func float Abs(const float& x)noexcept                                                  {return (x < 0.0f) ? -x : x;}
    static constexpr_func int Sign(const int& x)noexcept                                                     {return (x < 0) ? -1 : 1;}
    static constexpr_func float Sign(const float& x)noexcept                                                 {return (x < 0.0f) ? -1.0f : 1.0f;}
    //! @}

    //! calculate square root
    //! @{
    static inline float Sqrt(const float& fInput)noexcept {return fInput*Rsqrt(fInput);}
    static inline float Rsqrt(float fInput)noexcept;
    //! @}

    //! calculate trigonometric values
    //! @{
    static inline float Sin(const float& fInput)noexcept  {return std::sin(fInput);}
    static inline float Cos(const float& fInput)noexcept  {return std::cos(fInput);}
    static inline float Tan(const float& fInput)noexcept  {return std::tan(fInput);}
    static inline float Atan(const float& fInput)noexcept {return std::atan(fInput);}
    static inline float Cot(const float& fInput)noexcept  {return std::tan(PI*0.5f - fInput);}
    //! @}

    //! calculate next power-of-two
    //! @{
    static inline coreUint NextPOT(const coreUint& iInput)noexcept {coreUint k = 1; while(k < iInput) k = k << 1; return k;}
    //! @}

    //! check if inside field-of-view
    //! @{
    static bool CheckFOV(const coreVector3& vPosition, const float& fFOV, const coreVector3& vCamPosition, const coreVector3& vCamDirection)noexcept;
    //! @}

    //! convert color format
    //! @{
    static coreVector3 HSVtoRGB(const coreVector3& vHSV)noexcept;
    static coreVector3 RGBtoHSV(const coreVector3& vRGB)noexcept;
    //! @}
};


// ****************************************************************
// calculate inverse square root
inline float coreMath::Rsqrt(float fInput)noexcept
{
    SDL_assert(fInput >= 0.0f);

#if defined(_CORE_SSE_)

    // optimized calculation with SSE
    _mm_store_ss(&fInput, _mm_rsqrt_ss(_mm_load_ss(&fInput)));

#else

    // normal calculation
    if(!fInput) return 0.0f;

    const float fHalfValue = fInput*0.5f;
    coreUint* piPointer    = (coreUint*)&fInput;
    *piPointer             = 0x5f3759df - (*piPointer >> 1);

    fInput *= 1.5f - fInput*fInput*fHalfValue;
    fInput *= 1.5f - fInput*fInput*fHalfValue;

#endif

    return fInput;
}


#endif // _CORE_GUARD_MATH_H_