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

// TODO: SIN and COS with precalculated table ? (1 for both, precision == memory) or SSE


// ****************************************************************
/* math definitions */
#define PI (3.1415926535897932384626433832795f)                    //!< Archimedes' constant
#define EU (2.7182818284590452353602874713527f)                    //!< Euler's number

#define DEG_TO_RAD(x) ((x) * 0.0174532925199432957692369076848f)   //!< convert degree to radian
#define RAD_TO_DEG(x) ((x) * 57.295779513082320876798154814105f)   //!< convert radian to degree

#define MIN(x,y,...) coreMath::Min(x, y, ##__VA_ARGS__)
#define MAX(x,y,...) coreMath::Max(x, y, ##__VA_ARGS__)
#define ABS(x)       coreMath::Abs(x)
#define SIGN(x)      coreMath::Sign(x)
#define CLAMP(a,x,y) coreMath::Clamp(a, x, y)
#define LERP(x,y,s)  coreMath::Lerp(x, y, s)
#define LERPS(x,y,s) coreMath::LerpSmooth(x, y, s)
#define LERPB(x,y,s) coreMath::LerpBreak(x, y, s)
#define FRACT(x)     coreMath::Fract(x)
#define SQRT(x)      coreMath::Sqrt(x)
#define RSQRT(x)     coreMath::Rsqrt(x)
#define RCP(x)       coreMath::Rcp(x)
#define SIN(x)       coreMath::Sin(x)
#define COS(x)       coreMath::Cos(x)
#define TAN(x)       coreMath::Tan(x)
#define ATAN(x)      coreMath::Atan(x)
#define COT(x)       coreMath::Cot(x)
#define CEIL(x)      coreMath::Ceil(x)
#define FLOOR(x)     coreMath::Floor(x)


// ****************************************************************
/* math utility collection */
class coreMath final
{
public:
    /*! special operations */
    //! @{
    template <typename T, typename... A> static constexpr_func T Min(const T& x, const T& y, A&&... vArgs) {return MIN(x, MIN(y, std::forward<A>(vArgs)...));}
    template <typename T, typename... A> static constexpr_func T Max(const T& x, const T& y, A&&... vArgs) {return MAX(x, MAX(y, std::forward<A>(vArgs)...));}
    template <typename T> static constexpr_func T    Min       (const T& x, const T& y)                    {return (x <   y)  ?    x  :   y;}
    template <typename T> static constexpr_func T    Max       (const T& x, const T& y)                    {return (x >   y)  ?    x  :   y;}
    template <typename T> static constexpr_func T    Abs       (const T& x)                                {return (x < T(0)) ?   -x  :   x;}
    template <typename T> static constexpr_func T    Sign      (const T& x)                                {return (x < T(0)) ? T(-1) : T(1);}
    template <typename T> static constexpr_func T    Clamp     (const T& a, const T& x, const T&     y)    {return MIN(MAX(a, x), y);}
    template <typename T> static constexpr_func T    Lerp      (const T& x, const T& y, const float& s)    {return x + (y - x) * s;}
    template <typename T> static inline         T    LerpSmooth(const T& x, const T& y, const float& s)    {return LERP(x, y, 0.5f - 0.5f * COS(s*PI));}
    template <typename T> static inline         T    LerpBreak (const T& x, const T& y, const float& s)    {return LERP(x, y, SIN(s*PI*0.5f));}
    template <typename T> static constexpr_func bool InRange   (const T& x, const T& c, const T&     r)    {return ((c-r) <= x && x <= (c+r));}
    //! @}

    /*! elementary operations */
    //! @{
    template <int iBase> static inline float Log(const float& fInput) {return std::log(fInput) / std::log((float)iBase);}
    static inline float Fract(const float& fInput)                    {return fInput - FLOOR(fInput);}
    static inline float Sqrt (const float& fInput)                    {return fInput * RSQRT(fInput);}
    static inline float Rsqrt(float fInput);
    static inline float Rcp  (float fInput);
    //! @}

    /*! trigonometric operations */
    //! @{
    static inline float Sin (const float& fInput) {return std::sin (fInput);}
    static inline float Cos (const float& fInput) {return std::cos (fInput);}
    static inline float Tan (const float& fInput) {return std::tan (fInput);}
    static inline float Atan(const float& fInput) {return std::atan(fInput);}
    static inline float Cot (const float& fInput) {return TAN(PI*0.5f - fInput);}
    //! @}

    /*! rounding operations */
    //! @{
    template <int iByte> static inline coreUint NextAlign(const int& iInput) {return coreMath::PrevAlign<iByte>(iInput) + iByte;}
    template <int iByte> static inline coreUint PrevAlign(const int& iInput) {const int k = iInput - 1; return k - (k % iByte);}
    static inline coreUint NextPOT(const coreUint& iInput)                   {coreUint k = 2; while(k < iInput) k <<= 1; return k;}
    static inline coreUint PrevPOT(const coreUint& iInput)                   {return coreMath::NextPOT(iInput) >> 1;}
    static inline float    Ceil   (const float&    fInput)                   {return std::ceil (fInput);}
    static inline float    Floor  (const float&    fInput)                   {return std::floor(fInput);}
    //! @}


private:
    DISABLE_TORS(coreMath)
};


// ****************************************************************
/* calculate inverse square root */
inline float coreMath::Rsqrt(float fInput)
{
    ASSERT(fInput > 0.0f)

#if defined(_CORE_SSE_)

    // optimized calculation with SSE
    _mm_store_ss(&fInput, _mm_rsqrt_ss(_mm_load_ss(&fInput)));

#else

    // normal calculation
    fInput = 1.0f / std::sqrt(fInput);

    //const float fHalfValue = fInput*0.5f;
    //coreUint* piPointer    = r_cast<coreUint*>(&fInput);
    //*piPointer             = 0x5F3759DF - (*piPointer >> 1);

    //fInput *= 1.5f - fInput*fInput*fHalfValue;
    //fInput *= 1.5f - fInput*fInput*fHalfValue;

#endif

    return fInput;
}


// ****************************************************************
/* calculate approximate reciprocal */
inline float coreMath::Rcp(float fInput)
{
    ASSERT(fInput)

#if defined(_CORE_SSE_)

    // optimized calculation with SSE
    _mm_store_ss(&fInput, _mm_rcp_ss(_mm_load_ss(&fInput)));

#else

    // normal calculation
    fInput = 1.0f / fInput;

    //const float fValue  = fInput;
    //coreUint* piPointer = r_cast<coreUint*>(&fInput);
    //*piPointer          = 0x7EEEEEEE - *piPointer;

    //fInput *= 2.0f - fInput*fValue;
    //fInput *= 2.0f - fInput*fValue;

#endif

    return fInput;
}


#endif /* _CORE_GUARD_MATH_H_ */