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

// TODO: SIN and COS with pre-calculated table ? (1 for both, precision == memory) or SSE (already uses SSE with MSVC+/arch:SSE2 ?) what about sincos ?
// TODO: check out _mm_ceil_ss and _mm_floor_ss (SSE4), put SSE version checks static into coreData ?


// ****************************************************************
/* math definitions */
#define CORE_MATH_PRECISION (0.0001f)                              //!< default floating-point precision

#define PI (3.1415926535897932384626433832795f)                    //!< Archimedes' constant
#define EU (2.7182818284590452353602874713527f)                    //!< Euler's number

#define DEG_TO_RAD(x) ((x) * 0.0174532925199432957692369076848f)   //!< convert degrees to radians
#define RAD_TO_DEG(x) ((x) * 57.295779513082320876798154814105f)   //!< convert radians to degrees
#define KM_TO_MI(x)   ((x) * 0.621371192237f)                      //!< convert kilometers to miles
#define MI_TO_KM(x)   ((x) * 1.609344000000f)                      //!< convert miles to kilometers

#define MIN   coreMath::Min
#define MAX   coreMath::Max
#define SIGN  coreMath::Sign
#define ABS   coreMath::Abs
#define CLAMP coreMath::Clamp
#define LERP  coreMath::Lerp
#define LERPS coreMath::LerpSmooth
#define LERPB coreMath::LerpBreak
#define FRACT coreMath::Fract
#define SQRT  coreMath::Sqrt
#define RSQRT coreMath::Rsqrt
#define RCP   coreMath::Rcp
#define SIN   coreMath::Sin
#define COS   coreMath::Cos
#define TAN   coreMath::Tan
#define ATAN  coreMath::Atan
#define COT   coreMath::Cot
#define CEIL  coreMath::Ceil
#define FLOOR coreMath::Floor


// ****************************************************************
/* math utility collection */
class coreMath final
{
public:
    DISABLE_CONSTRUCTION(coreMath)

    /*! special operations */
    //! @{
    template <typename T, typename... A> static constexpr_func T Min(const T& x, const T& y, A&&... vArgs) {return MIN(x, MIN(y, std::forward<A>(vArgs)...));}
    template <typename T, typename... A> static constexpr_func T Max(const T& x, const T& y, A&&... vArgs) {return MAX(x, MAX(y, std::forward<A>(vArgs)...));}
    template <typename T> static constexpr_func T    Min       (const T& x, const T& y)                    {return (x <   y)  ?    x  :   y;}
    template <typename T> static constexpr_func T    Max       (const T& x, const T& y)                    {return (x >   y)  ?    x  :   y;}
    template <typename T> static constexpr_func T    Sign      (const T& x)                                {return (x < T(0)) ? T(-1) : T(1);}
    template <typename T> static constexpr_func T    Abs       (const T& x)                                {return std::abs(x);}
    template <typename T> static constexpr_func T    Clamp     (const T& x, const T& a, const T&     b)    {return MIN(MAX(x, a), b);}
    template <typename T> static constexpr_func T    Lerp      (const T& x, const T& y, const float& s)    {return x + (y - x) * s;}
    template <typename T> static inline         T    LerpSmooth(const T& x, const T& y, const float& s)    {return LERP(x, y, 0.5f - 0.5f * COS(s*PI));}
    template <typename T> static inline         T    LerpBreak (const T& x, const T& y, const float& s)    {return LERP(x, y, SIN(s*PI*0.5f));}
    template <typename T> static constexpr_func bool InRange   (const T& x, const T& c, const T&     r)    {return ((c-r) <= x && x <= (c+r));}
    //! @}

    /*! elementary operations */
    //! @{
    template <coreUint iBase> static inline float Log(const float& fInput) {return std::log(fInput) / std::log(I_TO_F(iBase));}
    static inline float Fract(const float& fInput)                         {return fInput - I_TO_F(F_TO_SI(fInput));}
    static inline float Sqrt (const float& fInput)                         {return fInput ? (fInput * RSQRT(fInput)) : 0.0f;}
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
    template <coreUint iByte> static inline coreUint CeilAlign (const coreUint& iInput) {const coreUint  k = iInput - 1; return k - (k % iByte) + iByte;}
    template <coreUint iByte> static inline coreUint FloorAlign(const coreUint& iInput) {const coreUint& k = iInput;     return k - (k % iByte);}
    static inline coreUint CeilPOT (const coreUint& iInput)                             {coreUint k = 2; while(k <  iInput) k <<= 1; return k;}
    static inline coreUint FloorPOT(const coreUint& iInput)                             {coreUint k = 2; while(k <= iInput) k <<= 1; return k >> 1;}
    static inline float    Ceil    (const float&    fInput)                             {return std::ceil (fInput);}
    static inline float    Floor   (const float&    fInput)                             {return std::floor(fInput);}
    //! @}
};


// ****************************************************************
/* calculate inverse square root */
inline float coreMath::Rsqrt(float fInput)
{
    ASSERT(fInput > 0.0f)

#if defined(_CORE_SSE_)

    // optimized calculation with SSE2
    _mm_store_ss(&fInput, _mm_rsqrt_ss(_mm_load_ss(&fInput)));

#else

    // normal calculation
    fInput = 1.0f / std::sqrt(fInput);

    // old approximation
    /*
        const float fHalfValue = fInput*0.5f;
        coreUint* piPointer    = r_cast<coreUint*>(&fInput);
        *piPointer             = 0x5F3759DF - (*piPointer >> 1);

        fInput *= 1.5f - fInput*fInput*fHalfValue;
        fInput *= 1.5f - fInput*fInput*fHalfValue;
    */

#endif

    return fInput;
}


// ****************************************************************
/* calculate approximate reciprocal */
inline float coreMath::Rcp(float fInput)
{
    ASSERT(fInput)

#if defined(_CORE_SSE_)

    // optimized calculation with SSE2
    _mm_store_ss(&fInput, _mm_rcp_ss(_mm_load_ss(&fInput)));

#else

    // normal calculation
    fInput = 1.0f / fInput;

    // old approximation
    /*
        const float fValue  = fInput;
        coreUint* piPointer = r_cast<coreUint*>(&fInput);
        *piPointer          = 0x7EEEEEEE - *piPointer;

        fInput *= 2.0f - fInput*fValue;
        fInput *= 2.0f - fInput*fValue;
    */

#endif

    return fInput;
}


#endif /* _CORE_GUARD_MATH_H_ */