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

// TODO: SIN and COS with MacLaurin or Taylor series (no lookup-table, because memory access may be equally slow)
// TODO: check out _mm_ceil_ss and _mm_floor_ss (SSE4) ?
// TODO: FUNC_CONST and FORCEINLINE on every function in this class ?
// TODO: add integer-log (macro)
// TODO: use std::common_type for return values
// TODO: CeilPot and FloorPot with BitScan
// TODO: BSWAP, __builtin_bswap16, __builtin_bswap32, __builtin_bswap64

// NOTE: {(x < y) ? x : y} -> int: cmp,cmovl -> float: _mm_min_ss


// ****************************************************************
/* math definitions */
#define CORE_MATH_PRECISION (0.001f)                               //!< default floating-point precision

#define PI    (3.1415926535897932384626433832795f)                 //!< Archimedes' constant
#define EU    (2.7182818284590452353602874713527f)                 //!< Euler's number
#define SQRT2 (1.4142135623730950488016887242097f)                 //!< principal square root of 2

#define DEG_TO_RAD(x) ((x) * 0.0174532925199432957692369076848f)   //!< convert degrees to radians
#define RAD_TO_DEG(x) ((x) * 57.295779513082320876798154814105f)   //!< convert radians to degrees
#define KM_TO_MI(x)   ((x) * 0.621371192237f)                      //!< convert kilometers to miles
#define MI_TO_KM(x)   ((x) * 1.609344000000f)                      //!< convert miles to kilometers

#define MIN    coreMath::Min
#define MAX    coreMath::Max
#define MED    coreMath::Med
#define CLAMP  coreMath::Clamp
#define SIGN   coreMath::Sign
#define ABS    coreMath::Abs
#define POW2   coreMath::Pow2
#define POW3   coreMath::Pow3
#define LERP   coreMath::Lerp
#define LERPS  coreMath::LerpSmooth
#define LERPB  coreMath::LerpBreak
#define LERPH3 coreMath::LerpHermit3
#define LERPH5 coreMath::LerpHermit5
#define LOG    coreMath::Log
#define POW    coreMath::Pow
#define FMOD   coreMath::Fmod
#define EXP2   coreMath::Exp2
#define TRUNC  coreMath::Trunc
#define FRACT  coreMath::Fract
#define SQRT   coreMath::Sqrt
#define RSQRT  coreMath::Rsqrt
#define RCP    coreMath::Rcp
#define SIN    coreMath::Sin
#define COS    coreMath::Cos
#define TAN    coreMath::Tan
#define ASIN   coreMath::Asin
#define ACOS   coreMath::Acos
#define ATAN   coreMath::Atan
#define COT    coreMath::Cot
#define CEIL   coreMath::Ceil
#define FLOOR  coreMath::Floor
#define ROUND  coreMath::Round


// ****************************************************************
/* math utility collection */
class INTERFACE coreMath final
{
public:
    DISABLE_CONSTRUCTION(coreMath)

    /*! special operations */
    //! @{
    template <typename T, typename S = T, typename... A>  static constexpr T Min  (const T& x, const S& y, A&&... vArgs)  {return MIN(x, MIN(y, std::forward<A>(vArgs)...));}
    template <typename T, typename S = T, typename... A>  static constexpr T Max  (const T& x, const S& y, A&&... vArgs)  {return MAX(x, MAX(y, std::forward<A>(vArgs)...));}
    template <typename T, typename S = T>                 static constexpr T Min  (const T& x, const S& y)                {return (x < y) ? x : y;}
    template <typename T, typename S = T>                 static constexpr T Max  (const T& x, const S& y)                {return (x > y) ? x : y;}
    template <typename T, typename S = T, typename R = T> static constexpr T Med  (const T& x, const S& y, const R& z)    {return MAX(MIN(MAX(x, y), z), MIN(x, y));}
    template <typename T, typename S = T, typename R = T> static constexpr T Clamp(const T& x, const S& a, const R& b)    {return MIN(MAX(x, a), b);}
    template <typename T> static constexpr T        Sign       (const T& x)                                               {return (x < T(0)) ? T(-1) : T(1);}
    template <typename T> static inline    T        Abs        (const T& x)                                               {return std::abs(x);}
    template <typename T> static constexpr T        Pow2       (const T& x)                                               {return x * x;}
    template <typename T> static constexpr T        Pow3       (const T& x)                                               {return x * x * x;}
    template <typename T> static constexpr T        Lerp       (const T& x, const T& y, const coreFloat s)                {return x + (y - x) * s;}
    template <typename T> static inline    T        LerpSmooth (const T& x, const T& y, const coreFloat s)                {return LERP(x, y, 0.5f - 0.5f * COS(s * PI));}
    template <typename T> static inline    T        LerpBreak  (const T& x, const T& y, const coreFloat s)                {return LERP(x, y, SIN(s * PI * 0.5f));}
    template <typename T> static constexpr T        LerpHermit3(const T& x, const T& y, const coreFloat s)                {return LERP(x, y, (3.0f - 2.0f * s) * s * s);}
    template <typename T> static constexpr T        LerpHermit5(const T& x, const T& y, const coreFloat s)                {return LERP(x, y, (10.0f + (-15.0f + 6.0f * s) * s) * s * s * s);}
    template <typename T> static constexpr coreBool IsNear     (const T& x, const T& c, const T& r = CORE_MATH_PRECISION) {return POW2(x - c) <= POW2(r);}
    template <typename T> static constexpr coreBool IsPot      (const T& x)                                               {return x && !(x & (x - T(1)));}
    //! @}

    /*! elementary operations */
    //! @{
    template <coreUintW iBase> static inline coreFloat Log(const coreFloat fInput) {return std::log  (fInput) / std::log(I_TO_F(iBase));}
    static inline coreFloat Pow  (const coreFloat fBase,   const coreFloat fExp)   {return std::pow  (fBase, fExp);}
    static inline coreFloat Fmod (const coreFloat fNum,    const coreFloat fDenom) {return std::fmod (fNum, fDenom);}
    static inline coreFloat Exp2 (const coreFloat fInput)                          {return std::exp2 (fInput);}
    static inline coreFloat Trunc(const coreFloat fInput)                          {return std::trunc(fInput);}
    static inline coreFloat Fract(const coreFloat fInput)                          {return fInput - TRUNC(fInput);}
    static inline coreFloat Sqrt (const coreFloat fInput);
    static inline coreFloat Rsqrt(const coreFloat fInput);
    static inline coreFloat Rcp  (const coreFloat fInput);
    //! @}

    /*! trigonometric operations */
    //! @{
    static inline coreFloat Sin (const coreFloat fInput) {return std::sin (fInput);}
    static inline coreFloat Cos (const coreFloat fInput) {return std::cos (fInput);}
    static inline coreFloat Tan (const coreFloat fInput) {return std::tan (fInput);}
    static inline coreFloat Asin(const coreFloat fInput) {return std::asin(fInput);}
    static inline coreFloat Acos(const coreFloat fInput) {return std::acos(fInput);}
    static inline coreFloat Atan(const coreFloat fInput) {return std::atan(fInput);}
    static inline coreFloat Cot (const coreFloat fInput) {return TAN(PI*0.5f - fInput);}
    //! @}

    /*! rounding operations */
    //! @{
    template <typename T> static constexpr T* CeilAlignPtr (const T* tInput, const coreUintW iAlign) {const coreUintW k = iAlign - 1u; return r_cast<T*>(I_TO_P((P_TO_UI(tInput) + k) & ~k));}
    template <typename T> static constexpr T* FloorAlignPtr(const T* tInput, const coreUintW iAlign) {const coreUintW k = iAlign - 1u; return r_cast<T*>(I_TO_P((P_TO_UI(tInput))     & ~k));}
    template <typename T> static constexpr T  CeilAlign    (const T& tInput, const coreUintW iAlign) {const T  k = tInput - T(1); return k - (k % iAlign) + iAlign;}
    template <typename T> static constexpr T  FloorAlign   (const T& tInput, const coreUintW iAlign) {const T& k = tInput;        return k - (k % iAlign);}
    template <typename T> static constexpr T  CeilPot      (const T& tInput)                         {T k = T(1); while(k <  tInput) k <<= T(1); return k;}
    template <typename T> static constexpr T  FloorPot     (const T& tInput)                         {T k = T(2); while(k <= tInput) k <<= T(1); return k >> T(1);}
    static inline coreFloat Ceil (const coreFloat fInput)                                            {return std::ceil (fInput);}
    static inline coreFloat Floor(const coreFloat fInput)                                            {return std::floor(fInput);}
    static inline coreFloat Round(const coreFloat fInput)                                            {return std::round(fInput);}
    //! @}

    /*! bit operations */
    //! @{
    static inline coreUint32 PopCount     (coreUint32 iInput);
    static inline coreUint32 BitScanFwd   (coreUint32 iInput);
    static inline coreUint32 BitScanRev   (coreUint32 iInput);
    static inline coreUint8  ReverseBits8 (coreUint8  iInput);
    static inline coreUint16 ReverseBits16(coreUint16 iInput);
    static inline coreUint32 ReverseBits32(coreUint32 iInput);
    static inline coreUint64 ReverseBits64(coreUint64 iInput);
    //! @}

    /*! converting operations */
    //! @{
    static inline coreUint32 FloatToBits(const coreFloat  fInput);
    static inline coreFloat  BitsToFloat(const coreUint32 iInput);
    static inline coreUint16 Float32To16(const coreFloat  fInput);
    static inline coreFloat  Float16To32(const coreUint16 iInput);
    //! @}

    /*! miscellaneous functions */
    //! @{
    static inline void DisableDenormals();
    static inline void EnableExceptions();
    //! @}
};


// ****************************************************************
/* template specializations */
template <> inline coreFloat coreMath::Log<2u> (const coreFloat fInput) {return std::log2 (fInput);}
template <> inline coreFloat coreMath::Log<10u>(const coreFloat fInput) {return std::log10(fInput);}


// ****************************************************************
/* calculate square root */
inline coreFloat coreMath::Sqrt(const coreFloat fInput)
{
    ASSERT(fInput >= 0.0f)

#if defined(_CORE_SSE_)

    // optimized calculation with SSE2
    return fInput ? (fInput * RSQRT(fInput)) : 0.0f;

#else

    // normal calculation
    return std::sqrt(fInput);

#endif
}


// ****************************************************************
/* calculate inverse square root */
inline coreFloat coreMath::Rsqrt(const coreFloat fInput)
{
    ASSERT(fInput > 0.0f)

#if defined(_CORE_SSE_)

    // optimized calculation with SSE2
    return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(fInput)));

#else

    // normal calculation
    return 1.0f / std::sqrt(fInput);

#endif
}


// ****************************************************************
/* calculate approximate reciprocal */
inline coreFloat coreMath::Rcp(const coreFloat fInput)
{
    ASSERT(fInput)

#if defined(_CORE_SSE_)

    // optimized calculation with SSE2
    return _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ss(fInput)));

#else

    // normal calculation
    return 1.0f / fInput;

#endif
}


// ****************************************************************
/* count the number of one-bits (population count) */
inline coreUint32 coreMath::PopCount(coreUint32 iInput)
{
#if defined(_CORE_SSE_)

    if(coreCPUID::POPCNT())
    {
        // optimized calculation with POPCNT
        return __popcnt(iInput);
    }

#endif

    // normal calculation
    iInput = (iInput)               - ((iInput >> 1u) & 0x55555555u);
    iInput = (iInput & 0x33333333u) + ((iInput >> 2u) & 0x33333333u);
    return (((iInput + (iInput >> 4u)) & 0xF0F0F0Fu) * 0x1010101u) >> 24u;
}


// ****************************************************************
/* get index of the least significant one-bit */
inline coreUint32 coreMath::BitScanFwd(coreUint32 iInput)
{
    if(!iInput) return 32u;

#if defined(_CORE_MSVC_)

    // calculation with MSVC intrinsic
    DWORD iOutput; _BitScanForward(&iOutput, iInput);
    return iOutput;

#else

    // calculation with other intrinsic
    return __builtin_ctz(iInput);

#endif
}


// ****************************************************************
/* get index of the most significant one-bit */
inline coreUint32 coreMath::BitScanRev(coreUint32 iInput)
{
    if(!iInput) return 32u;

#if defined(_CORE_MSVC_)

    // calculation with MSVC intrinsic
    DWORD iOutput; _BitScanReverse(&iOutput, iInput);
    return iOutput;

#else

    // calculation with other intrinsic
    return 31u - __builtin_clz(iInput);

#endif
}


// ****************************************************************
/* reverse bit-order of a 8-bit sequence */
inline coreUint8 coreMath::ReverseBits8(coreUint8 iInput)
{
#if defined(_CORE_CLANG_)

    // calculation with Clang intrinsic
    return __builtin_bitreverse8(iInput);

#else

    // normal calculation
    iInput = ((iInput >> 4u) & 0x0Fu) | ((iInput << 4u) & 0xF0u);
    iInput = ((iInput >> 2u) & 0x33u) | ((iInput << 2u) & 0xCCu);
    iInput = ((iInput >> 1u) & 0x55u) | ((iInput << 1u) & 0xAAu);
    return iInput;

#endif
}


// ****************************************************************
/* reverse bit-order of a 16-bit sequence */
inline coreUint16 coreMath::ReverseBits16(coreUint16 iInput)
{
#if defined(_CORE_CLANG_)

    // calculation with Clang intrinsic
    return __builtin_bitreverse16(iInput);

#else

    // normal calculation
    iInput = ((iInput >> 8u) & 0x00FFu) | ((iInput << 8u) & 0xFF00u);
    iInput = ((iInput >> 4u) & 0x0F0Fu) | ((iInput << 4u) & 0xF0F0u);
    iInput = ((iInput >> 2u) & 0x3333u) | ((iInput << 2u) & 0xCCCCu);
    iInput = ((iInput >> 1u) & 0x5555u) | ((iInput << 1u) & 0xAAAAu);
    return iInput;

#endif
}


// ****************************************************************
/* reverse bit-order of a 32-bit sequence */
inline coreUint32 coreMath::ReverseBits32(coreUint32 iInput)
{
#if defined(_CORE_CLANG_)

    // calculation with Clang intrinsic
    return __builtin_bitreverse32(iInput);

#else

    // normal calculation
    iInput = ((iInput >> 16u) & 0x0000FFFFu) | ((iInput << 16u) & 0xFFFF0000u);
    iInput = ((iInput >>  8u) & 0x00FF00FFu) | ((iInput <<  8u) & 0xFF00FF00u);
    iInput = ((iInput >>  4u) & 0x0F0F0F0Fu) | ((iInput <<  4u) & 0xF0F0F0F0u);
    iInput = ((iInput >>  2u) & 0x33333333u) | ((iInput <<  2u) & 0xCCCCCCCCu);
    iInput = ((iInput >>  1u) & 0x55555555u) | ((iInput <<  1u) & 0xAAAAAAAAu);
    return iInput;

#endif
}


// ****************************************************************
/* reverse bit-order of a 64-bit sequence */
inline coreUint64 coreMath::ReverseBits64(coreUint64 iInput)
{
#if defined(_CORE_CLANG_)

    // calculation with Clang intrinsic
    return __builtin_bitreverse64(iInput);

#else

    // normal calculation
    iInput = ((iInput >> 32u) & 0x00000000FFFFFFFFu) | ((iInput << 32u) & 0xFFFFFFFF00000000u);
    iInput = ((iInput >> 16u) & 0x0000FFFF0000FFFFu) | ((iInput << 16u) & 0xFFFF0000FFFF0000u);
    iInput = ((iInput >>  8u) & 0x00FF00FF00FF00FFu) | ((iInput <<  8u) & 0xFF00FF00FF00FF00u);
    iInput = ((iInput >>  4u) & 0x0F0F0F0F0F0F0F0Fu) | ((iInput <<  4u) & 0xF0F0F0F0F0F0F0F0u);
    iInput = ((iInput >>  2u) & 0x3333333333333333u) | ((iInput <<  2u) & 0xCCCCCCCCCCCCCCCCu);
    iInput = ((iInput >>  1u) & 0x5555555555555555u) | ((iInput <<  1u) & 0xAAAAAAAAAAAAAAAAu);
    return iInput;

#endif
}


// ****************************************************************
/* safely convert float into bit-representation */
inline coreUint32 coreMath::FloatToBits(const coreFloat fInput)
{
    coreUint32 iOutput; std::memcpy(&iOutput, &fInput, sizeof(coreUint32));
    return iOutput;
}


// ****************************************************************
/* safely convert bit-representation into float */
inline coreFloat coreMath::BitsToFloat(const coreUint32 iInput)
{
    coreFloat fOutput; std::memcpy(&fOutput, &iInput, sizeof(coreFloat));
    return fOutput;
}


// ****************************************************************
/* convert single-precision float into half-precision */
inline coreUint16 coreMath::Float32To16(const coreFloat fInput)
{
#if defined(_CORE_SSE_)

    if(coreCPUID::F16C())
    {
        // optimized calculation with F16C
        return _mm_cvtsi128_si32(_mm_cvtps_ph(_mm_set_ss(fInput), _MM_FROUND_CUR_DIRECTION));
    }

#endif

    // normal calculation
    const coreUint32 A = coreMath::FloatToBits(fInput);
    return ((A & 0x7FFFFFFFu) > 0x38000000u) ? ((((A & 0x7FFFFFFFu) >> 13u) - 0x0001C000u) |
                                                 ((A & 0x80000000u) >> 16u)) & 0xFFFFu : 0u;
};


// ****************************************************************
/* convert half-precision float into single-precision */
inline coreFloat coreMath::Float16To32(const coreUint16 iInput)
{
#if defined(_CORE_SSE_)

    if(coreCPUID::F16C())
    {
        // optimized calculation with F16C
        return _mm_cvtss_f32(_mm_cvtph_ps(_mm_cvtsi32_si128(iInput)));
    }

#endif

    // normal calculation
    const coreUint32 A = (iInput & 0x7C00u) ? (((coreUint32(iInput & 0x7FFFu) << 13u) + 0x38000000u) |
                                                (coreUint32(iInput & 0x8000u) << 16u)) : 0u;
    return coreMath::BitsToFloat(A);
};


// ****************************************************************
/* disable denormal results and inputs (per thread) */
inline void coreMath::DisableDenormals()
{
#if defined(_CORE_SSE_)

    // disable in the MXCSR control register (only for SSE)
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);                                 // calculation results
    if(coreCPUID::SSE3()) _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);   // instruction inputs

#endif

#if defined(_CORE_WINDOWS_)

    // disable with Windows function (both for x87 and SSE)
    _controlfp(_DN_FLUSH, _MCW_DN);

#endif
}


// ****************************************************************
/* enable relevant floating-point exceptions (per thread) */
inline void coreMath::EnableExceptions()
{
#if defined(_CORE_DEBUG_)

    #if defined(_CORE_SSE_)

        // enable in the MXCSR control register (only for SSE)
        _MM_SET_EXCEPTION_MASK(~(_MM_MASK_OVERFLOW | _MM_MASK_DIV_ZERO | _MM_MASK_INVALID) & _MM_MASK_MASK);

    #endif

    #if defined(_CORE_WINDOWS_)

        // enable with Windows function (both for x87 and SSE)
        _controlfp(~(_EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID) & _MCW_EM, _MCW_EM);

    #endif

    #if defined(_CORE_GCC_)

        // enable with GCC function (both for x87 and SSE)
        feenableexcept(FE_OVERFLOW | FE_DIVBYZERO | FE_INVALID);

    #endif

#endif
}


#endif /* _CORE_GUARD_MATH_H_ */