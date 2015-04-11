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
#define CLAMP coreMath::Clamp
#define SIGN  coreMath::Sign
#define ABS   coreMath::Abs
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
#define ASIN  coreMath::Asin
#define ACOS  coreMath::Acos
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
    template <typename T, typename S, typename... A> static constexpr_func T Min(const T& x, const S& y, A&&... vArgs) {return MIN(x, MIN(y, std::forward<A>(vArgs)...));}
    template <typename T, typename S, typename... A> static constexpr_func T Max(const T& x, const S& y, A&&... vArgs) {return MAX(x, MAX(y, std::forward<A>(vArgs)...));}
    template <typename T, typename S> static constexpr_func T Min  (const T& x, const S& y)                            {return (x < y) ? x : y;}
    template <typename T, typename S> static constexpr_func T Max  (const T& x, const S& y)                            {return (x > y) ? x : y;}
    template <typename T, typename S> static constexpr_func T Clamp(const T& x, const S& a, const S& b)                {return MIN(MAX(x, a), b);}
    template <typename T> static constexpr_func T        Sign      (const T& x)                                        {return std::copysign(T(1), x);}
    template <typename T> static constexpr_func T        Abs       (const T& x)                                        {return std::abs(x);}
    template <typename T> static constexpr_func T        Lerp      (const T& x, const T& y, const coreFloat& s)        {return x + (y - x) * s;}
    template <typename T> static inline         T        LerpSmooth(const T& x, const T& y, const coreFloat& s)        {return LERP(x, y, 0.5f - 0.5f * COS(s*PI));}
    template <typename T> static inline         T        LerpBreak (const T& x, const T& y, const coreFloat& s)        {return LERP(x, y, SIN(s*PI*0.5f));}
    template <typename T> static constexpr_func coreBool InRange   (const T& x, const T& c, const T& r)                {return ((c-r) <= x && x <= (c+r));}
    template <typename T> static inline         coreBool IsPOT     (const T& x)                                        {return !(x & (x - 1));}
    //! @}

    /*! elementary operations */
    //! @{
    template <coreUintW iBase> static inline coreFloat Log(const coreFloat& fInput) {return std::log(fInput) / std::log(I_TO_F(iBase));}
    static inline coreFloat Fract(const coreFloat& fInput)                          {return fInput - I_TO_F(F_TO_SI(fInput));}
    static inline coreFloat Sqrt (const coreFloat& fInput)                          {return fInput ? (fInput * RSQRT(fInput)) : 0.0f;}
    static inline coreFloat Rsqrt(coreFloat fInput);
    static inline coreFloat Rcp  (coreFloat fInput);
    //! @}

    /*! trigonometric operations */
    //! @{
    static inline coreFloat Sin (const coreFloat& fInput) {return std::sin (fInput);}
    static inline coreFloat Cos (const coreFloat& fInput) {return std::cos (fInput);}
    static inline coreFloat Tan (const coreFloat& fInput) {return std::tan (fInput);}
    static inline coreFloat Asin(const coreFloat& fInput) {return std::asin(fInput);}
    static inline coreFloat Acos(const coreFloat& fInput) {return std::acos(fInput);}
    static inline coreFloat Atan(const coreFloat& fInput) {return std::atan(fInput);}
    static inline coreFloat Cot (const coreFloat& fInput) {return TAN(PI*0.5f - fInput);}
    //! @}

    /*! rounding operations */
    //! @{
    template <coreUintW iByte, typename T> static inline T CeilAlign (const T& iInput) {const T  k = iInput - 1; return k - (k % iByte) + iByte;}
    template <coreUintW iByte, typename T> static inline T FloorAlign(const T& iInput) {const T& k = iInput;     return k - (k % iByte);}
    template                  <typename T> static inline T CeilPOT   (const T& iInput) {T k = 2; while(k <  iInput) k <<= 1; return k;}
    template                  <typename T> static inline T FloorPOT  (const T& iInput) {T k = 2; while(k <= iInput) k <<= 1; return k >> 1;}
    static inline coreFloat Ceil (const coreFloat& fInput)                             {return std::ceil (fInput);}
    static inline coreFloat Floor(const coreFloat& fInput)                             {return std::floor(fInput);}
    //! @}
};


// ****************************************************************
/* template specializations */
template <> inline coreFloat coreMath::Log< 2u>(const coreFloat& fInput) {return std::log2 (fInput);}
template <> inline coreFloat coreMath::Log<10u>(const coreFloat& fInput) {return std::log10(fInput);}


// ****************************************************************
/* calculate inverse square root */
inline coreFloat coreMath::Rsqrt(coreFloat fInput)
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
        uint* piPointer        = r_cast<uint*>(&fInput);
        *piPointer             = 0x5F3759DFu - (*piPointer >> 1u);

        fInput *= 1.5f - fInput*fInput*fHalfValue;
        fInput *= 1.5f - fInput*fInput*fHalfValue;
    */

#endif

    return fInput;
}


// ****************************************************************
/* calculate approximate reciprocal */
inline coreFloat coreMath::Rcp(coreFloat fInput)
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
        const float fValue = fInput;
        uint* piPointer    = r_cast<uint*>(&fInput);
        *piPointer         = 0x7EEEEEEEu - *piPointer;

        fInput *= 2.0f - fInput*fValue;
        fInput *= 2.0f - fInput*fValue;
    */

#endif

    return fInput;
}


#endif /* _CORE_GUARD_MATH_H_ */