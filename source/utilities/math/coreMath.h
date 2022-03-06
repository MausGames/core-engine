///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MATH_H_
#define _CORE_GUARD_MATH_H_

// TODO 5: SIN and COS with MacLaurin or Taylor series (no lookup-table, because memory access may be equally slow)
// TODO 5: FUNC_CONST and FORCEINLINE on every function in this class (beware of errno changes, maybe not const) ?
// TODO 5: add integer-log (macro)
// TODO 5: use std::common_type for return values

// NOTE: {(x < y) ? x : y} -> int: cmp,cmovl -> float: _mm_min_ss


// ****************************************************************
/* math definitions */
#define CORE_MATH_PRECISION (0.0005f)                              // default floating-point precision

#define PI    (3.1415926535897932384626433832795f)                 // Archimedes' constant
#define EU    (2.7182818284590452353602874713527f)                 // Euler's number
#define GR    (1.6180339887498948482045868343656f)                 // golden ratio ((a+b)/a = a/b)
#define GA    (2.3999632297286533222315555066336f)                 // golden angle (radians)
#define SQRT2 (1.4142135623730950488016887242097f)                 // principal square root of 2
#define SQRT3 (1.7320508075688772935274463415059f)                 // principal square root of 3

#define DEG_TO_RAD(x) ((x) * 0.0174532925199432957692369076848f)   // convert degrees to radians
#define RAD_TO_DEG(x) ((x) * 57.295779513082320876798154814105f)   // convert radians to degrees
#define KM_TO_MI(x)   ((x) * 0.621371192237f)                      // convert kilometers to miles
#define MI_TO_KM(x)   ((x) * 1.609344000000f)                      // convert miles to kilometers
#define M_TO_FT(x)    ((x) * 0.304800000f)                         // convert meters to feet
#define FT_TO_M(x)    ((x) * 3.280839895f)                         // convert feet to meters

#define MIN    coreMath::Min                                       // for easier distinction in formulas
#define MAX    coreMath::Max
#define MED    coreMath::Med
#define CLAMP  coreMath::Clamp
#define SIGN   coreMath::Sign
#define SIGNUM coreMath::Signum
#define ABS    coreMath::Abs
#define POW2   coreMath::Pow2
#define POW3   coreMath::Pow3
#define LERP   coreMath::Lerp
#define LERPS  coreMath::LerpSmooth
#define LERPB  coreMath::LerpBreak
#define LERPBR coreMath::LerpBreakRev
#define LERPH3 coreMath::LerpHermite3
#define LERPH5 coreMath::LerpHermite5
#define STEP   coreMath::Step
#define STEPH3 coreMath::StepHermite3
#define STEPH5 coreMath::StepHermite5
#define FMOD   coreMath::Fmod
#define TRUNC  coreMath::Trunc
#define FRACT  coreMath::Fract
#define CBRT   coreMath::Cbrt
#define SQRT   coreMath::Sqrt
#define RSQRT  coreMath::Rsqrt
#define RCP    coreMath::Rcp
#define POW    coreMath::Pow
#define LOG    coreMath::Log
#define LOGB   coreMath::LogB
#define LOG2   coreMath::Log2
#define LOG10  coreMath::Log10
#define EXP    coreMath::Exp
#define EXP2   coreMath::Exp2
#define EXP10  coreMath::Exp10
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

#if defined(_CORE_LIBCPP_)
    namespace std {template <typename T, typename S> FORCE_INLINE constexpr T bit_cast(const S& tValue) {return __builtin_bit_cast(T, tValue);}}
#endif


// ****************************************************************
/* math utility collection */
class INTERFACE coreMath final
{
public:
    DISABLE_CONSTRUCTION(coreMath)

    /* special operations */
    template <typename T, typename S = T, typename... A>  static constexpr T Min  (const T& x, const S& y, A&&... vArgs) {return MIN(x, MIN(y, std::forward<A>(vArgs)...));}
    template <typename T, typename S = T, typename... A>  static constexpr T Max  (const T& x, const S& y, A&&... vArgs) {return MAX(x, MAX(y, std::forward<A>(vArgs)...));}
    template <typename T, typename S = T>                 static constexpr T Min  (const T& x, const S& y)               {return (x < y) ? T(x) : T(y);}
    template <typename T, typename S = T>                 static constexpr T Max  (const T& x, const S& y)               {return (x > y) ? T(x) : T(y);}
    template <typename T, typename S = T, typename R = T> static constexpr T Med  (const T& x, const S& y, const R& z)   {return MAX(MIN(MAX(x, y), z), MIN(x, y));}
    template <typename T, typename S = T, typename R = T> static constexpr T Clamp(const T& x, const S& a, const R& b)   {return MIN(MAX(x, a), b);}
    template <typename T> static constexpr T Sign        (const T& x)                                                    {return (x < T(0)) ? T(-1) : T(1);}   // never return 0
    template <typename T> static constexpr T Signum      (const T& x)                                                    {return (x) ? SIGN(x) : T(0);}
    template <typename T> static inline    T Abs         (const T& x)                                                    {return std::abs(x);}
    template <typename T> static constexpr T Pow2        (const T& x)                                                    {return x * x;}
    template <typename T> static constexpr T Pow3        (const T& x)                                                    {return x * x * x;}
    template <typename T> static constexpr T Lerp        (const T& x, const T& y, const coreFloat s)                     {return x * (1.0f - s) + y * s;}   // better precision than (x + (y - x) * s)
    template <typename T> static inline    T LerpSmooth  (const T& x, const T& y, const coreFloat s)                     {return LERP(x, y, 0.5f - 0.5f * COS(s * PI));}
    template <typename T> static inline    T LerpBreak   (const T& x, const T& y, const coreFloat s)                     {return LERP(x, y, SIN(s * (PI * 0.5f)));}
    template <typename T> static inline    T LerpBreakRev(const T& x, const T& y, const coreFloat s)                     {return LERP(y, x, COS(s * (PI * 0.5f)));}
    template <typename T> static constexpr T LerpHermite3(const T& x, const T& y, const coreFloat s)                     {return LERP(x, y, (3.0f - 2.0f * s) * s * s);}
    template <typename T> static constexpr T LerpHermite5(const T& x, const T& y, const coreFloat s)                     {return LERP(x, y, (10.0f + (-15.0f + 6.0f * s) * s) * s * s * s);}
    static constexpr coreFloat               Step        (const coreFloat a, const coreFloat b, const coreFloat x)       {return CLAMP((x - a) * RCP(b - a), 0.0f, 1.0f);}   // linearstep
    static constexpr coreFloat               StepHermite3(const coreFloat a, const coreFloat b, const coreFloat x)       {return LERPH3(0.0f, 1.0f, STEP(a, b, x));}         // smoothstep
    static constexpr coreFloat               StepHermite5(const coreFloat a, const coreFloat b, const coreFloat x)       {return LERPH5(0.0f, 1.0f, STEP(a, b, x));}         // smootherstep

    /* base operations */
    static inline    coreFloat Fmod (const coreFloat fNum, const coreFloat fDenom) {return std::fmod (fNum, fDenom);}
    static inline    coreFloat Trunc(const coreFloat fInput)                       {return std::trunc(fInput);}
    static inline    coreFloat Fract(const coreFloat fInput)                       {return fInput - TRUNC(fInput);}   // FMOD(x, 1.0f)
    static inline    coreFloat Cbrt (const coreFloat fInput)                       {return std::cbrt (fInput);}
    static constexpr coreFloat Sqrt (const coreFloat fInput);
    static constexpr coreFloat Rsqrt(const coreFloat fInput);
    static constexpr coreFloat Rcp  (const coreFloat fInput);

    /* exponential operations */
    static inline coreFloat Pow  (const coreFloat fBase, const coreFloat fExp)  {return std::pow  (fBase, fExp);}
    static inline coreFloat LogB (const coreFloat fVal,  const coreFloat fBase) {return LOG(fVal) * RCP(LOG(fBase));}
    static inline coreFloat Log  (const coreFloat fInput)                       {return std::log  (fInput);}
    static inline coreFloat Log2 (const coreFloat fInput)                       {return std::log2 (fInput);}
    static inline coreFloat Log10(const coreFloat fInput)                       {return std::log10(fInput);}
    static inline coreFloat Exp  (const coreFloat fInput)                       {return std::exp  (fInput);}
    static inline coreFloat Exp2 (const coreFloat fInput)                       {return std::exp2 (fInput);}
    static inline coreFloat Exp10(const coreFloat fInput)                       {return POW(10.0f, fInput);}

    /* trigonometric operations */
    static inline coreFloat Sin (const coreFloat fInput) {return std::sin (fInput);}
    static inline coreFloat Cos (const coreFloat fInput) {return std::cos (fInput);}
    static inline coreFloat Tan (const coreFloat fInput) {return std::tan (fInput);}
    static inline coreFloat Asin(const coreFloat fInput) {return std::asin(fInput);}
    static inline coreFloat Acos(const coreFloat fInput) {return std::acos(fInput);}
    static inline coreFloat Atan(const coreFloat fInput) {return std::atan(fInput);}
    static inline coreFloat Cot (const coreFloat fInput) {return TAN(PI*0.5f - fInput);}

    /* rounding operations */
    static inline coreFloat Ceil (const coreFloat fInput)                                            {return std::ceil     (fInput);}
    static inline coreFloat Floor(const coreFloat fInput)                                            {return std::floor    (fInput);}
    static inline coreFloat Round(const coreFloat fInput)                                            {return std::round    (fInput);}
    template <typename T> static constexpr T  CeilPot      (const T& tInput)                         {return std::bit_ceil (tInput);}
    template <typename T> static constexpr T  FloorPot     (const T& tInput)                         {return std::bit_floor(tInput);}
    template <typename T> static constexpr T  CeilAlign    (const T& tInput, const coreUintW iAlign) {const coreUintW k = iAlign - 1u; return (tInput + k) & ~k;}
    template <typename T> static constexpr T  FloorAlign   (const T& tInput, const coreUintW iAlign) {const coreUintW k = iAlign - 1u; return (tInput)     & ~k;}
    template <typename T> static constexpr T* CeilAlignPtr (const T* tInput, const coreUintW iAlign) {const coreUintW k = iAlign - 1u; return s_cast<T*>(I_TO_P((P_TO_UI(tInput) + k) & ~k));}
    template <typename T> static constexpr T* FloorAlignPtr(const T* tInput, const coreUintW iAlign) {const coreUintW k = iAlign - 1u; return s_cast<T*>(I_TO_P((P_TO_UI(tInput))     & ~k));}

    /* analyzing operations */
    template <typename T> static constexpr coreBool IsPot (const T& x)                                               {return x && !(x & (x - T(1)));}
    template <typename T> static constexpr coreBool IsNear(const T& x, const T& c, const T& r = CORE_MATH_PRECISION) {return POW2(x - c) <= POW2(r);}

    /* bit operations */
    static constexpr coreUint32 PopCount      (const coreUint64 iInput);
    static constexpr coreUint32 BitScanFwd    (const coreUint64 iInput);
    static constexpr coreUint32 BitScanRev    (const coreUint64 iInput);
    static constexpr coreUint32 RotateLeft32  (const coreUint32 iInput, const coreUint8 iShift);
    static constexpr coreUint64 RotateLeft64  (const coreUint64 iInput, const coreUint8 iShift);
    static constexpr coreUint32 RotateRight32 (const coreUint32 iInput, const coreUint8 iShift);
    static constexpr coreUint64 RotateRight64 (const coreUint64 iInput, const coreUint8 iShift);
    static constexpr coreUint8  ReverseBits8  (const coreUint8  iInput);
    static constexpr coreUint16 ReverseBits16 (const coreUint16 iInput);
    static constexpr coreUint32 ReverseBits32 (const coreUint32 iInput);
    static constexpr coreUint64 ReverseBits64 (const coreUint64 iInput);
    static constexpr coreUint16 ReverseBytes16(const coreUint16 iInput);
    static constexpr coreUint32 ReverseBytes32(const coreUint32 iInput);
    static constexpr coreUint64 ReverseBytes64(const coreUint64 iInput);

    /* converting operations */
    static constexpr coreUint32 FloatToBits(const coreFloat  fInput);
    static constexpr coreFloat  BitsToFloat(const coreUint32 iInput);
    static constexpr coreUint16 Float32To16(const coreFloat  fInput);
    static constexpr coreFloat  Float16To32(const coreUint16 iInput);

    /* miscellaneous functions */
    static inline void EnableExceptions();
    static inline void EnableRoundToNearest();
    static inline void DisableDenormals();
};


// ****************************************************************
/* calculate square root */
constexpr coreFloat coreMath::Sqrt(const coreFloat fInput)
{
    ASSERT(fInput >= 0.0f)

    if(!std::is_constant_evaluated())
    {
#if defined(_CORE_SSE_)

        // optimized calculation with SSE
        return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(fInput)));

#elif defined(_CORE_NEON_)

        // optimized calculation with NEON
        return vget_lane_f32(vsqrt_f32(vdup_n_f32(fInput)), 0);

#else

        // normal calculation
        return std::sqrt(fInput);

#endif
    }

    // compile-time calculation
    coreFloat fPrev   = 0.0f;
    coreFloat fOutput = fInput;
    while(fPrev != fOutput)
    {
        fPrev   = fOutput;
        fOutput = 0.5f * (fOutput + fInput / fOutput);
    }
    return fOutput;
}


// ****************************************************************
/* calculate approximate inverse square root */
constexpr coreFloat coreMath::Rsqrt(const coreFloat fInput)
{
    ASSERT(fInput > 0.0f)

    if(!std::is_constant_evaluated())
    {
#if defined(_CORE_SSE_)

        // optimized calculation with SSE
        const coreFloat A = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(fInput)));
        return 0.5f * A * (3.0f - (fInput * A) * A);

#elif defined(_CORE_NEON_)

        // optimized calculation with NEON
        const coreFloat A = vrsqrtes_f32(fInput);
        return 0.5f * A * (3.0f - (fInput * A) * A);

#endif
    }

    // normal calculation
    return 1.0f / SQRT(fInput);
}


// ****************************************************************
/* calculate approximate reciprocal */
constexpr coreFloat coreMath::Rcp(const coreFloat fInput)
{
    ASSERT(fInput)

    if(!std::is_constant_evaluated())
    {
#if defined(_CORE_SSE_)

        // optimized calculation with SSE
        const coreFloat A = _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ss(fInput)));
        return A * (2.0f - fInput * A);

#elif defined(_CORE_NEON_)

        // optimized calculation with NEON
        const coreFloat A = vrecpes_f32(fInput);
        return A * (2.0f - fInput * A);

#endif
    }

    // normal calculation
    return 1.0f / fInput;
}


// ****************************************************************
/* count the number of one-bits (population count) */
constexpr coreUint32 coreMath::PopCount(coreUint64 iInput)
{
#if defined(_CORE_SSE_)

    if(!std::is_constant_evaluated() && coreCPUID::POPCNT())
    {
        // optimized calculation with POPCNT
    #if defined(_CORE_64BIT_)
        return _mm_popcnt_u64(iInput);
    #else
        return _mm_popcnt_u32(iInput) + _mm_popcnt_u32(iInput >> 32u);
    #endif
    }

#endif

    // normal calculation
    iInput = (iInput)                       - ((iInput >> 1u) & 0x5555555555555555u);
    iInput = (iInput & 0x3333333333333333u) + ((iInput >> 2u) & 0x3333333333333333u);
    return (((iInput + (iInput >> 4u)) & 0x0F0F0F0F0F0F0F0Fu) * 0x0101010101010101u) >> 56u;
}


// ****************************************************************
/* get index of the least significant one-bit */
constexpr coreUint32 coreMath::BitScanFwd(const coreUint64 iInput)
{
    if(!iInput) return 64u;

#if defined(_CORE_MSVC_)

    if(!std::is_constant_evaluated())
    {
        // calculation with MSVC intrinsic
        DWORD iOutput; BitScanForward64(&iOutput, iInput);
        return iOutput;
    }

#else

    // calculation with other intrinsic
    return __builtin_ctzll(iInput);

#endif

    // normal calculation
    constexpr coreUint32 aiTable[] =
    {
        0u, 47u,  1u, 56u, 48u, 27u,  2u, 60u,
       57u, 49u, 41u, 37u, 28u, 16u,  3u, 61u,
       54u, 58u, 35u, 52u, 50u, 42u, 21u, 44u,
       38u, 32u, 29u, 23u, 17u, 11u,  4u, 62u,
       46u, 55u, 26u, 59u, 40u, 36u, 15u, 53u,
       34u, 51u, 20u, 43u, 31u, 22u, 10u, 45u,
       25u, 39u, 14u, 33u, 19u, 30u,  9u, 24u,
       13u, 18u,  8u, 12u,  7u,  6u,  5u, 63u
    };

    return aiTable[((iInput ^ (iInput - 1u)) * 0x03F79D71B4CB0A89u) >> 58u];
}


// ****************************************************************
/* get index of the most significant one-bit */
constexpr coreUint32 coreMath::BitScanRev(const coreUint64 iInput)
{
    if(!iInput) return 64u;

#if defined(_CORE_MSVC_)

    if(!std::is_constant_evaluated())
    {
        // calculation with MSVC intrinsic
        DWORD iOutput; BitScanReverse64(&iOutput, iInput);
        return iOutput;
    }

#else

    // calculation with other intrinsic
    return 63u - __builtin_clzll(iInput);

#endif

    // normal calculation
    constexpr coreUint32 aiTable[] =
    {
        0u, 47u,  1u, 56u, 48u, 27u,  2u, 60u,
       57u, 49u, 41u, 37u, 28u, 16u,  3u, 61u,
       54u, 58u, 35u, 52u, 50u, 42u, 21u, 44u,
       38u, 32u, 29u, 23u, 17u, 11u,  4u, 62u,
       46u, 55u, 26u, 59u, 40u, 36u, 15u, 53u,
       34u, 51u, 20u, 43u, 31u, 22u, 10u, 45u,
       25u, 39u, 14u, 33u, 19u, 30u,  9u, 24u,
       13u, 18u,  8u, 12u,  7u,  6u,  5u, 63u
    };

    coreUint64 A = iInput;
    A |= A >> 1u; A |= A >>  2u; A |= A >>  4u;
    A |= A >> 8u; A |= A >> 16u; A |= A >> 32u;
    return aiTable[(A * 0x03F79D71B4CB0A89u) >> 58u];
}


// ****************************************************************
/* rotate bits to the left in a 32-bit sequence */
constexpr coreUint32 coreMath::RotateLeft32(const coreUint32 iInput, const coreUint8 iShift)
{
    ASSERT(iShift <= 32u)

#if defined(_CORE_MSVC_)

    if(!std::is_constant_evaluated())
    {
        // calculation with MSVC intrinsic
        return _rotl(iInput, iShift);
    }

#elif defined(_CORE_CLANG_)

    // calculation with Clang intrinsic
    return __builtin_rotateleft32(iInput, iShift);

#endif

    // normal calculation
    return (iInput << iShift) | (iInput >> (32u - iShift));
}


// ****************************************************************
/* rotate bits to the left in a 64-bit sequence */
constexpr coreUint64 coreMath::RotateLeft64(const coreUint64 iInput, const coreUint8 iShift)
{
    ASSERT(iShift <= 64u)

#if defined(_CORE_MSVC_)

    if(!std::is_constant_evaluated())
    {
        // calculation with MSVC intrinsic
        return _rotl64(iInput, iShift);
    }

#elif defined(_CORE_CLANG_)

    // calculation with Clang intrinsic
    return __builtin_rotateleft64(iInput, iShift);

#endif

    // normal calculation
    return (iInput << iShift) | (iInput >> (64u - iShift));
}


// ****************************************************************
/* rotate bits to the right in a 32-bit sequence */
constexpr coreUint32 coreMath::RotateRight32(const coreUint32 iInput, const coreUint8 iShift)
{
    ASSERT(iShift <= 32u)

#if defined(_CORE_MSVC_)

    if(!std::is_constant_evaluated())
    {
        // calculation with MSVC intrinsic
        return _rotr(iInput, iShift);
    }

#elif defined(_CORE_CLANG_)

    // calculation with Clang intrinsic
    return __builtin_rotateright32(iInput, iShift);

#endif

    // normal calculation
    return (iInput >> iShift) | (iInput << (32u - iShift));
}


// ****************************************************************
/* rotate bits to the right in a 64-bit sequence */
constexpr coreUint64 coreMath::RotateRight64(const coreUint64 iInput, const coreUint8 iShift)
{
    ASSERT(iShift <= 64u)

#if defined(_CORE_MSVC_)

    if(!std::is_constant_evaluated())
    {
        // calculation with MSVC intrinsic
        return _rotr64(iInput, iShift);
    }

#elif defined(_CORE_CLANG_)

    // calculation with Clang intrinsic
    return __builtin_rotateright64(iInput, iShift);

#endif

    // normal calculation
    return (iInput >> iShift) | (iInput << (64u - iShift));
}


// ****************************************************************
/* reverse bit-order of a 8-bit sequence */
constexpr coreUint8 coreMath::ReverseBits8(const coreUint8 iInput)
{
#if defined(_CORE_CLANG_)

    // calculation with Clang intrinsic
    return __builtin_bitreverse8(iInput);

#else

    // normal calculation
    coreUint16 iOutput = iInput;
    iOutput = ((iOutput >> 4u) & 0x0Fu) | ((iOutput << 4u) & 0xF0u);
    iOutput = ((iOutput >> 2u) & 0x33u) | ((iOutput << 2u) & 0xCCu);
    iOutput = ((iOutput >> 1u) & 0x55u) | ((iOutput << 1u) & 0xAAu);
    return iOutput;

#endif
}


// ****************************************************************
/* reverse bit-order of a 16-bit sequence */
constexpr coreUint16 coreMath::ReverseBits16(const coreUint16 iInput)
{
#if defined(_CORE_CLANG_)

    // calculation with Clang intrinsic
    return __builtin_bitreverse16(iInput);

#else

    // normal calculation
    coreUint16 iOutput = iInput;
    iOutput = ((iOutput >> 8u) & 0x00FFu) | ((iOutput << 8u) & 0xFF00u);
    iOutput = ((iOutput >> 4u) & 0x0F0Fu) | ((iOutput << 4u) & 0xF0F0u);
    iOutput = ((iOutput >> 2u) & 0x3333u) | ((iOutput << 2u) & 0xCCCCu);
    iOutput = ((iOutput >> 1u) & 0x5555u) | ((iOutput << 1u) & 0xAAAAu);
    return iOutput;

#endif
}


// ****************************************************************
/* reverse bit-order of a 32-bit sequence */
constexpr coreUint32 coreMath::ReverseBits32(const coreUint32 iInput)
{
#if defined(_CORE_CLANG_)

    // calculation with Clang intrinsic
    return __builtin_bitreverse32(iInput);

#else

    // normal calculation
    coreUint32 iOutput = iInput;
    iOutput = ((iOutput >> 16u) & 0x0000FFFFu) | ((iOutput << 16u) & 0xFFFF0000u);
    iOutput = ((iOutput >>  8u) & 0x00FF00FFu) | ((iOutput <<  8u) & 0xFF00FF00u);
    iOutput = ((iOutput >>  4u) & 0x0F0F0F0Fu) | ((iOutput <<  4u) & 0xF0F0F0F0u);
    iOutput = ((iOutput >>  2u) & 0x33333333u) | ((iOutput <<  2u) & 0xCCCCCCCCu);
    iOutput = ((iOutput >>  1u) & 0x55555555u) | ((iOutput <<  1u) & 0xAAAAAAAAu);
    return iOutput;

#endif
}


// ****************************************************************
/* reverse bit-order of a 64-bit sequence */
constexpr coreUint64 coreMath::ReverseBits64(const coreUint64 iInput)
{
#if defined(_CORE_CLANG_)

    // calculation with Clang intrinsic
    return __builtin_bitreverse64(iInput);

#else

    // normal calculation
    coreUint64 iOutput = iInput;
    iOutput = ((iOutput >> 32u) & 0x00000000FFFFFFFFu) | ((iOutput << 32u) & 0xFFFFFFFF00000000u);
    iOutput = ((iOutput >> 16u) & 0x0000FFFF0000FFFFu) | ((iOutput << 16u) & 0xFFFF0000FFFF0000u);
    iOutput = ((iOutput >>  8u) & 0x00FF00FF00FF00FFu) | ((iOutput <<  8u) & 0xFF00FF00FF00FF00u);
    iOutput = ((iOutput >>  4u) & 0x0F0F0F0F0F0F0F0Fu) | ((iOutput <<  4u) & 0xF0F0F0F0F0F0F0F0u);
    iOutput = ((iOutput >>  2u) & 0x3333333333333333u) | ((iOutput <<  2u) & 0xCCCCCCCCCCCCCCCCu);
    iOutput = ((iOutput >>  1u) & 0x5555555555555555u) | ((iOutput <<  1u) & 0xAAAAAAAAAAAAAAAAu);
    return iOutput;

#endif
}


// ****************************************************************
/* reverse byte-order of a 2-byte sequence */
constexpr coreUint16 coreMath::ReverseBytes16(const coreUint16 iInput)
{
#if defined(_CORE_MSVC_)

    if(!std::is_constant_evaluated())
    {
        // calculation with MSVC intrinsic
        return _byteswap_ushort(iInput);
    }

#else

    // calculation with other intrinsic
    return __builtin_bswap16(iInput);

#endif

    // normal calculation
    coreUint16 iOutput = iInput;
    iOutput = ((iOutput >> 8u) & 0x00FFu) | ((iOutput << 8u) & 0xFF00u);
    return iOutput;
}


// ****************************************************************
/* reverse byte-order of a 4-byte sequence */
constexpr coreUint32 coreMath::ReverseBytes32(const coreUint32 iInput)
{
#if defined(_CORE_MSVC_)

    if(!std::is_constant_evaluated())
    {
        // calculation with MSVC intrinsic
        return _byteswap_ulong(iInput);
    }

#else

    // calculation with other intrinsic
    return __builtin_bswap32(iInput);

#endif

    // normal calculation
    coreUint32 iOutput = iInput;
    iOutput = ((iOutput >> 16u) & 0x0000FFFFu) | ((iOutput << 16u) & 0xFFFF0000u);
    iOutput = ((iOutput >>  8u) & 0x00FF00FFu) | ((iOutput <<  8u) & 0xFF00FF00u);
    return iOutput;
}


// ****************************************************************
/* reverse byte-order of a 8-byte sequence */
constexpr coreUint64 coreMath::ReverseBytes64(const coreUint64 iInput)
{
#if defined(_CORE_MSVC_)

    if(!std::is_constant_evaluated())
    {
        // calculation with MSVC intrinsic
        return _byteswap_uint64(iInput);
    }

#else

    // calculation with other intrinsic
    return __builtin_bswap64(iInput);

#endif

    // normal calculation
    coreUint64 iOutput = iInput;
    iOutput = ((iOutput >> 32u) & 0x00000000FFFFFFFFu) | ((iOutput << 32u) & 0xFFFFFFFF00000000u);
    iOutput = ((iOutput >> 16u) & 0x0000FFFF0000FFFFu) | ((iOutput << 16u) & 0xFFFF0000FFFF0000u);
    iOutput = ((iOutput >>  8u) & 0x00FF00FF00FF00FFu) | ((iOutput <<  8u) & 0xFF00FF00FF00FF00u);
    return iOutput;
}


// ****************************************************************
/* safely convert float into bit-representation */
constexpr coreUint32 coreMath::FloatToBits(const coreFloat fInput)
{
    return std::bit_cast<coreUint32>(fInput);
}


// ****************************************************************
/* safely convert bit-representation into float */
constexpr coreFloat coreMath::BitsToFloat(const coreUint32 iInput)
{
    return std::bit_cast<coreFloat>(iInput);
}


// ****************************************************************
/* convert single-precision float into half-precision */
constexpr coreUint16 coreMath::Float32To16(const coreFloat fInput)
{
#if defined(_CORE_SSE_)

    if(!std::is_constant_evaluated() && coreCPUID::F16C())
    {
        // optimized calculation with F16C
        return _mm_cvtsi128_si32(_mm_cvtps_ph(_mm_set_ss(fInput), _MM_FROUND_CUR_DIRECTION));
    }

#elif defined(_CORE_NEON_)

    // optimized calculation with NEON
    return std::bit_cast<coreUint16>(__fp16(fInput));

#endif

    // normal calculation
    const coreUint32 A = coreMath::FloatToBits(fInput);
    return ((A & 0x7FFFFFFFu) > 0x38000000u) ? ((((A & 0x7FFFFFFFu) >> 13u) - 0x0001C000u) |
                                                 ((A & 0x80000000u) >> 16u)) & 0xFFFFu : 0u;
}


// ****************************************************************
/* convert half-precision float into single-precision */
constexpr coreFloat coreMath::Float16To32(const coreUint16 iInput)
{
#if defined(_CORE_SSE_)

    if(!std::is_constant_evaluated() && coreCPUID::F16C())
    {
        // optimized calculation with F16C
        return _mm_cvtss_f32(_mm_cvtph_ps(_mm_cvtsi32_si128(iInput)));
    }

#elif defined(_CORE_NEON_)

    // optimized calculation with NEON
    return coreFloat(std::bit_cast<__fp16>(iInput));

#endif

    // normal calculation
    const coreUint32 A = (iInput & 0x7C00u) ? (((coreUint32(iInput & 0x7FFFu) << 13u) + 0x38000000u) |
                                                (coreUint32(iInput & 0x8000u) << 16u)) : 0u;
    return coreMath::BitsToFloat(A);
}


// ****************************************************************
/* enable relevant floating-point exceptions (per thread) */
inline void coreMath::EnableExceptions()
{
#if defined(_CORE_DEBUG_)

    #if defined(_CORE_SSE_)

        // enable in the MXCSR control register (for SSE)
        _MM_SET_EXCEPTION_MASK(~(_MM_MASK_OVERFLOW | _MM_MASK_DIV_ZERO | _MM_MASK_INVALID) & _MM_MASK_MASK);

    #endif

    #if defined(_CORE_WINDOWS_)

        // enable with Windows function (for x87 and SSE)
        _controlfp(~(_EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID) & _MCW_EM, _MCW_EM);

    #elif defined(_CORE_LINUX_)

        // enable with Linux/glibc function (for x87 and SSE)
        feenableexcept(FE_OVERFLOW | FE_DIVBYZERO | FE_INVALID);

    #endif

#endif
}


// ****************************************************************
/* enable floating-point rounding to nearest even value (per thread) */
inline void coreMath::EnableRoundToNearest()
{
#if defined(_CORE_SSE_)

    // enable in the MXCSR control register (for SSE)
    _MM_SET_ROUNDING_MODE(_MM_ROUND_NEAREST);

#endif

#if defined(_CORE_WINDOWS_)

    // enable with Windows function (for x87 and SSE)
    _controlfp(_RC_NEAR, _MCW_RC);

#endif

    // enable with standard function
    std::fesetround(FE_TONEAREST);
}


// ****************************************************************
/* disable denormal results and inputs (per thread) */
inline void coreMath::DisableDenormals()
{
#if defined(_CORE_SSE_)

    // disable in the MXCSR control register (for SSE)
    _MM_SET_FLUSH_ZERO_MODE    (_MM_FLUSH_ZERO_ON);       // calculation results
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);   // instruction inputs

#endif

#if defined(_CORE_WINDOWS_)

    // disable with Windows function (for x87 and SSE)
    _controlfp(_DN_FLUSH, _MCW_DN);

#endif
}


#endif /* _CORE_GUARD_MATH_H_ */