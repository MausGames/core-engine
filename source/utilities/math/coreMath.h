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
// TODO 4: should HashCombine be moved to coreData? hashes are all defined in data-category
// TODO 3: std::byteswap (e.g. for MSVC constexpr)
// TODO 3: [CORE2] IsNear: (ABS(tValue1 - tValue2) <= tRange) ?

// NOTE: {(x < y) ? x : y} -> int: cmp,cmovl -> float: _mm_min_ss


// ****************************************************************
/* math definitions */
#define CORE_MATH_PRECISION (0.0005f)                              // default floating-point precision

#define PI    (3.1415926535897932384626433832795f)                 // Archimedes' constant
#define EU    (2.7182818284590452353602874713527f)                 // Euler's number
#define GR    (1.6180339887498948482045868343656f)                 // golden ratio ((a+b)/a = a/b)
#define GA    (2.3999632297286533222315555066336f)                 // golden angle (radians, PI * (3 - SQRT(5)))
#define SQRT2 (1.4142135623730950488016887242097f)                 // principal square root of 2
#define SQRT3 (1.7320508075688772935274463415059f)                 // principal square root of 3

#define DEG_TO_RAD(x) ((x) * 0.0174532925199432957692369076848f)   // convert degrees to radians
#define RAD_TO_DEG(x) ((x) * 57.295779513082320876798154814105f)   // convert radians to degrees
#define KM_TO_MI(x)   ((x) * 0.621371192237f)                      // convert kilometers to miles
#define MI_TO_KM(x)   ((x) * 1.609344000000f)                      // convert miles to kilometers
#define M_TO_FT(x)    ((x) * 0.304800000f)                         // convert meters to feet
#define FT_TO_M(x)    ((x) * 3.280839895f)                         // convert feet to meters

#define MIN    coreMath::Min                                       // for better visibility in formulas
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
#define LERPE  coreMath::LerpExp
#define LERPP  coreMath::LerpPow
#define STEP   coreMath::Step
#define STEPS  coreMath::StepSmooth
#define STEPB  coreMath::StepBreak
#define STEPBR coreMath::StepBreakRev
#define STEPH3 coreMath::StepHermite3
#define STEPH5 coreMath::StepHermite5
#define FMODR  coreMath::FmodRange
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

#define MIN1(x)    (MIN   (x, 1.0f))
#define MAX0(x)    (MAX   (x, 0.0f))
#define CLAMP01(x) (CLAMP (x, 0.0f, 1.0f))
#define BLENDS(x)  (LERPS (0.0f, 1.0f, x))
#define BLENDB(x)  (LERPB (0.0f, 1.0f, x))
#define BLENDBR(x) (LERPBR(0.0f, 1.0f, x))
#define BLENDH3(x) (LERPH3(0.0f, 1.0f, x))
#define BLENDH5(x) (LERPH5(0.0f, 1.0f, x))

#if defined(_CORE_MACOS_)
    namespace std {template <typename T, typename S> FORCE_INLINE constexpr T bit_cast(const S& tValue) {return __builtin_bit_cast(T, tValue);}}
#endif


// ****************************************************************
/* math utility collection */
class INTERFACE coreMath final
{
public:
    DISABLE_CONSTRUCTION(coreMath)

    /* special operations */
    template <typename T, typename... A> static constexpr T Min         (const T& x, const t_ident<T>& y, const t_ident<T>& z, A&&... vArgs) {return MIN(x, MIN(y, z, std::forward<A>(vArgs)...));}
    template <typename T, typename... A> static constexpr T Max         (const T& x, const t_ident<T>& y, const t_ident<T>& z, A&&... vArgs) {return MAX(x, MAX(y, z, std::forward<A>(vArgs)...));}
    template <typename T>                static constexpr T Min         (const T& x, const t_ident<T>& y)                                    {return (x < y) ? x : y;}
    template <typename T>                static constexpr T Max         (const T& x, const t_ident<T>& y)                                    {return (x > y) ? x : y;}
    template <typename T>                static constexpr T Med         (const T& x, const t_ident<T>& y, const t_ident<T>& z)               {return MAX(MIN(MAX(x, y), z), MIN(x, y));}
    template <typename T>                static constexpr T Clamp       (const T& x, const t_ident<T>& a, const t_ident<T>& b)               {return MIN(MAX(x, a), b);}
    template <typename T>                static constexpr T Sign        (const T& x)                                                         {return (x < T(0)) ? T(-1) : T(1);}          // never return 0
    template <typename T>                static constexpr T Signum      (const T& x)                                                         {return (x) ? SIGN(x) : T(0);}
    template <std::floating_point  T>    static inline    T Abs         (const T& x)                                                         {return std::abs(x);}
    template <std::signed_integral T>    static inline    T Abs         (const T& x)                                                         {return std::abs(MAX(x, -std::numeric_limits<T>::max()));}
    template <typename T>                static constexpr T Pow2        (const T& x)                                                         {return x * x;}
    template <typename T>                static constexpr T Pow3        (const T& x)                                                         {return x * x * x;}
    template <typename T>                static constexpr T Lerp        (const T& x, const T& y, const coreFloat s)                          {return x * (1.0f - s) + y * s;}             // better precision than (x + (y - x) * s)
    template <typename T>                static inline    T LerpSmooth  (const T& x, const T& y, const coreFloat s)                          {return LERP(x, y, 0.5f - 0.5f * COS(s * PI));}
    template <typename T>                static inline    T LerpBreak   (const T& x, const T& y, const coreFloat s)                          {return LERP(x, y, SIN(s * (PI * 0.5f)));}
    template <typename T>                static inline    T LerpBreakRev(const T& x, const T& y, const coreFloat s)                          {return LERP(y, x, COS(s * (PI * 0.5f)));}   // (y, x)
    template <typename T>                static constexpr T LerpHermite3(const T& x, const T& y, const coreFloat s)                          {return LERP(x, y, (3.0f - 2.0f * s) * s * s);}
    template <typename T>                static constexpr T LerpHermite5(const T& x, const T& y, const coreFloat s)                          {return LERP(x, y, (10.0f + (-15.0f + 6.0f * s) * s) * s * s * s);}
    template <typename T>                static inline    T LerpExp     (const T& x, const T& y, const coreFloat s)                          {return POW(x, 1.0f - s) * POW(y, s);}
    template <typename T>                static inline    T LerpPow     (const T& x, const T& y, const coreFloat s, const coreFloat k)       {return LERP(x, y, POW(s, k));}              // inverted curve with (k < 1)
    static constexpr coreFloat                              Step        (const coreFloat a, const coreFloat b, const coreFloat x)            {return CLAMP01((x - a) * RCP(b - a));}      // linearstep
    static inline    coreFloat                              StepSmooth  (const coreFloat a, const coreFloat b, const coreFloat x)            {return BLENDS (STEP(a, b, x));}
    static inline    coreFloat                              StepBreak   (const coreFloat a, const coreFloat b, const coreFloat x)            {return BLENDB (STEP(a, b, x));}
    static inline    coreFloat                              StepBreakRev(const coreFloat a, const coreFloat b, const coreFloat x)            {return BLENDBR(STEP(a, b, x));}
    static constexpr coreFloat                              StepHermite3(const coreFloat a, const coreFloat b, const coreFloat x)            {return BLENDH3(STEP(a, b, x));}             // smoothstep
    static constexpr coreFloat                              StepHermite5(const coreFloat a, const coreFloat b, const coreFloat x)            {return BLENDH5(STEP(a, b, x));}             // smootherstep

    /* base operations */
    template <std::floating_point T> static inline    T FmodRange(const T& tNum, const t_ident<T>& tFrom, const t_ident<T>& tTo);
    template <std::floating_point T> static inline    T Fmod     (const T& tNum, const t_ident<T>& tDenom) {return std::fmod (tNum, tDenom);}
    template <std::floating_point T> static inline    T Trunc    (const T& tInput)                         {return std::trunc(tInput);}
    template <std::floating_point T> static inline    T Fract    (const T& tInput)                         {return tInput - TRUNC(tInput);}   // FMOD(x, 1)
    template <std::floating_point T> static inline    T Cbrt     (const T& tInput)                         {return std::cbrt (tInput);}
    template <std::floating_point T> static inline    T Sqrt     (const T& tInput)                         {return std::sqrt (tInput);}
    template <std::floating_point T> static inline    T Rsqrt    (const T& tInput)                         {return T(1) / SQRT(tInput);}
    template <std::floating_point T> static constexpr T Rcp      (const T& tInput)                         {return T(1) / tInput;}
    static constexpr coreFloat                          Sqrt     (const coreFloat fInput);
    static constexpr coreFloat                          Rsqrt    (const coreFloat fInput);
    static constexpr coreFloat                          Rcp      (const coreFloat fInput);

    /* exponential operations */
    template <std::floating_point T> static inline T Pow  (const T& tBase,  const t_ident<T>& tExp)  {return std::pow  (tBase, tExp);}
    template <std::floating_point T> static inline T LogB (const T& tValue, const t_ident<T>& tBase) {return LOG(tValue) * RCP(LOG(tBase));}
    template <std::floating_point T> static inline T Log  (const T& tInput)                          {return std::log  (tInput);}
    template <std::floating_point T> static inline T Log2 (const T& tInput)                          {return std::log2 (tInput);}
    template <std::floating_point T> static inline T Log10(const T& tInput)                          {return std::log10(tInput);}
    template <std::floating_point T> static inline T Exp  (const T& tInput)                          {return std::exp  (tInput);}
    template <std::floating_point T> static inline T Exp2 (const T& tInput)                          {return std::exp2 (tInput);}
    template <std::floating_point T> static inline T Exp10(const T& tInput)                          {return POW(T(10), tInput);}

    /* trigonometric operations */
    template <std::floating_point T> static inline T Sin (const T& tInput) {return std::sin (tInput);}
    template <std::floating_point T> static inline T Cos (const T& tInput) {return std::cos (tInput);}
    template <std::floating_point T> static inline T Tan (const T& tInput) {return std::tan (tInput);}
    template <std::floating_point T> static inline T Asin(const T& tInput) {return std::asin(tInput);}
    template <std::floating_point T> static inline T Acos(const T& tInput) {return std::acos(tInput);}
    template <std::floating_point T> static inline T Atan(const T& tInput) {return std::atan(tInput);}
    template <std::floating_point T> static inline T Cot (const T& tInput) {return TAN(T(PI)/T(2) - tInput);}

    /* rounding operations */
    template <std::floating_point T> static inline    T  Ceil         (const T& tInput)                          {return std::ceil     (tInput);}
    template <std::floating_point T> static inline    T  Floor        (const T& tInput)                          {return std::floor    (tInput);}
    template <std::floating_point T> static inline    T  Round        (const T& tInput)                          {return std::round    (tInput);}
    template <std::floating_point T> static inline    T  CeilFactor   (const T& tInput, const T& tFactor)        {return CEIL (tInput * RCP(tFactor)) * tFactor;}
    template <std::floating_point T> static inline    T  FloorFactor  (const T& tInput, const T& tFactor)        {return FLOOR(tInput * RCP(tFactor)) * tFactor;}
    template <std::floating_point T> static inline    T  RoundFactor  (const T& tInput, const T& tFactor)        {return ROUND(tInput * RCP(tFactor)) * tFactor;}
    template <std::integral       T> static constexpr T  CeilPot      (const T& tInput)                          {ASSERT(tInput >= T(0)) return std::bit_ceil (std::make_unsigned_t<T>(tInput));}
    template <std::integral       T> static constexpr T  FloorPot     (const T& tInput)                          {ASSERT(tInput >= T(0)) return std::bit_floor(std::make_unsigned_t<T>(tInput));}
    template <std::integral       T> static constexpr T  CeilAlign    (const T& tInput,  const coreUintW iAlign) {ASSERT(tInput >= T(0)) const T k = tInput + iAlign - T(1); return k - (k % iAlign);}
    template <std::integral       T> static constexpr T  FloorAlign   (const T& tInput,  const coreUintW iAlign) {ASSERT(tInput >= T(0)) const T k = tInput;                 return k - (k % iAlign);}
    template <typename            T> static constexpr T* CeilAlignPtr (const T* ptInput, const coreUintW iAlign) {ASSERT(coreMath::IsPot(iAlign)) const coreUintW k = iAlign - 1u; return s_cast<T*>(I_TO_P((P_TO_UI(ptInput) + k) & ~k));}
    template <typename            T> static constexpr T* FloorAlignPtr(const T* ptInput, const coreUintW iAlign) {ASSERT(coreMath::IsPot(iAlign)) const coreUintW k = iAlign - 1u; return s_cast<T*>(I_TO_P((P_TO_UI(ptInput))     & ~k));}

    /* analyzing operations */
    template <std::integral T> static constexpr coreBool IsPot    (const T& tInput)                                                           {ASSERT(tInput >= T(0)) return (tInput && !(tInput & (tInput - T(1))));}
    template <std::integral T> static constexpr coreBool IsAligned(const T& tInput,  const coreUintW iAlign)                                  {ASSERT(tInput >= T(0)) return ((coreUintW(tInput) % iAlign) == 0u);}
    template <typename      T> static constexpr coreBool IsNear   (const T& tValue1, const T& tValue2, const T& tRange = CORE_MATH_PRECISION) {ASSERT(tRange >  T(0)) return (POW2(tValue1 - tValue2) <= POW2(tRange));}

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

    /* hash operations */
    static constexpr coreUint32 HashCombine32(const coreUint32 a, const coreUint32 b);
    static constexpr coreUint64 HashCombine64(const coreUint64 a, const coreUint64 b);

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
/* loop value within specific range */
template <std::floating_point T> inline T coreMath::FmodRange(const T& tNum, const t_ident<T>& tFrom, const t_ident<T>& tTo)
{
    ASSERT(tFrom < tTo)

    if(tNum >= tTo)   return FMOD(tNum - tFrom, tTo   - tFrom) + tFrom;
    if(tNum <  tFrom) return FMOD(tNum - tTo,   tFrom - tTo)   + tTo;

    return tNum;
}


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
#if defined(_CORE_SSE_) && defined(_CORE_FASTMATH_)

        // optimized calculation with SSE
        const coreFloat A = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(fInput)));
        return 0.5f * A * (3.0f - (fInput * A) * A);

#elif defined(_CORE_NEON_) && defined(_CORE_FASTMATH_)

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
#if defined(_CORE_SSE_) && defined(_CORE_FASTMATH_)

        // optimized calculation with SSE
        const coreFloat A = _mm_cvtss_f32(_mm_rcp_ss(_mm_set_ss(fInput)));
        return A * (2.0f - fInput * A);

#elif defined(_CORE_NEON_) && defined(_CORE_FASTMATH_)

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
constexpr coreUint32 coreMath::PopCount(const coreUint64 iInput)
{
#if defined(_CORE_SSE_) && defined(_CORE_MSVC_)

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
    coreUint64 iOutput = iInput;
    iOutput = (iOutput)                       - ((iOutput >> 1u) & 0x5555555555555555u);
    iOutput = (iOutput & 0x3333333333333333u) + ((iOutput >> 2u) & 0x3333333333333333u);
    return  (((iOutput + (iOutput >> 4u)) & 0x0F0F0F0F0F0F0F0Fu) * 0x0101010101010101u) >> 56u;
}


// ****************************************************************
/* get index of the least significant one-bit (smallest) */
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
/* get index of the most significant one-bit (biggest) */
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
    ASSERT(iShift < 32u)

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
    return iShift ? (iInput << iShift) | (iInput >> (32u - iShift)) : iInput;
}


// ****************************************************************
/* rotate bits to the left in a 64-bit sequence */
constexpr coreUint64 coreMath::RotateLeft64(const coreUint64 iInput, const coreUint8 iShift)
{
    ASSERT(iShift < 64u)

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
    return iShift ? (iInput << iShift) | (iInput >> (64u - iShift)) : iInput;
}


// ****************************************************************
/* rotate bits to the right in a 32-bit sequence */
constexpr coreUint32 coreMath::RotateRight32(const coreUint32 iInput, const coreUint8 iShift)
{
    ASSERT(iShift < 32u)

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
    return iShift ? (iInput >> iShift) | (iInput << (32u - iShift)) : iInput;
}


// ****************************************************************
/* rotate bits to the right in a 64-bit sequence */
constexpr coreUint64 coreMath::RotateRight64(const coreUint64 iInput, const coreUint8 iShift)
{
    ASSERT(iShift < 64u)

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
    return iShift ? (iInput >> iShift) | (iInput << (64u - iShift)) : iInput;
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
/* combine two 32-bit hash-values */
constexpr coreUint32 coreMath::HashCombine32(const coreUint32 a, const coreUint32 b)
{
    coreUint32 x = a + b + 0x9E3779B9u;

    x ^= x >> 16u;
    x *= 0x21F0AAADu;
    x ^= x >> 15u;
    x *= 0x735A2D97u;
    x ^= x >> 15u;

    return x;
}


// ****************************************************************
/* combine two 64-bit hash-values */
constexpr coreUint64 coreMath::HashCombine64(const coreUint64 a, const coreUint64 b)
{
    coreUint64 x = a + b + 0x9E3779B97F4A7C15u;

    x ^= x >> 32u;
    x *= 0x0E9846AF9B1A615Du;
    x ^= x >> 32u;
    x *= 0x0E9846AF9B1A615Du;
    x ^= x >> 28u;

    return x;
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
#if defined(_CORE_SSE_) && defined(_CORE_MSVC_)

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
    return [](const coreInt32 A)
    {
        coreInt32 s = ((A >> 16) & 0x00008000);
        coreInt32 e = ((A >> 23) & 0x000000FF) - (127 - 15);
        coreInt32 m = ((A)       & 0x007FFFFF);

        if(e <= 0)
        {
            if(e <= -11)
            {
                return s;
            }
            else
            {
                m = m | 0x00800000;

                const coreInt32 t = 14 - e;
                const coreInt32 a = (1 << (t - 1)) - 1;
                const coreInt32 b = (m >> t) & 1;

                return s | ((m + a + b) >> t);
            }
        }
        else if(e == 0xFF - (127 - 15))
        {
            if(m == 0)
            {
                return s | 0x7C00;
            }
            else
            {
                return s | 0x7C00 | MAX(m >> 13, 1);
            }
        }
        else
        {
            m = m + 0x00000FFF + ((m >> 13) & 1);

            if(m & 0x00800000)
            {
                m = 0;
                e = e + 1;
            }

            if(e >= 31)
            {
                return s | 0x7C00;
            }
            else
            {
                return s | (e << 10) | (m >> 13);
            }
        }
    }(coreMath::FloatToBits(fInput));
}


// ****************************************************************
/* convert half-precision float into single-precision */
constexpr coreFloat coreMath::Float16To32(const coreUint16 iInput)
{
#if defined(_CORE_SSE_) && defined(_CORE_MSVC_)

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
    return coreMath::BitsToFloat([](const coreInt32 A)
    {
        coreInt32 s = (A << 16) & 0x80000000;
        coreInt32 e = (A >> 10) & 0x0000001F;
        coreInt32 m = (A)       & 0x000003FF;

        if(e == 0)
        {
            if(m == 0)
            {
                return s;
            }
            else
            {
                while(!(m & 0x00000400))
                {
                    e = e -  1;
                    m = m << 1;
                }

                e = e +  1;
                m = m & ~0x00000400;
            }
        }
        else if(e == 31)
        {
            return s | 0x7F800000 | (m << 13);
        }

        e = e + (127 - 15);

        return s | (e << 23) | (m << 13);
    }(iInput));
}


// ****************************************************************
/* enable relevant floating-point exceptions (per thread) */
inline void coreMath::EnableExceptions()
{
#if defined(_CORE_DEBUG_) && !defined(_CORE_MACOS_)

    #if defined(_CORE_SSE_)

        // enable in the MXCSR control register (for SSE)
        _MM_SET_EXCEPTION_MASK(~(_MM_MASK_OVERFLOW | _MM_MASK_DIV_ZERO | _MM_MASK_INVALID) & _MM_MASK_MASK);

    #elif defined(_CORE_NEON_)

        // enable in the FPCR control register (for NEON, on 64-bit only)
        coreUint64 iValue;
        asm volatile("mrs %0, fpcr" : "=r" (iValue));

        ADD_BIT(iValue, 10u)   // OFE (overflow)
        ADD_BIT(iValue, 9u)    // DZE (divide by zero)
        ADD_BIT(iValue, 8u)    // IOE (invalid operation)

        asm volatile("msr fpcr, %0" :: "r" (iValue));

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

#elif defined(_CORE_NEON_)

    // enable in the FPCR control register (for NEON, on 64-bit only)
    coreUint64 iValue;
    asm volatile("mrs %0, fpcr" : "=r" (iValue));

    SET_BITVALUE(iValue, 2u, 22u, 0u)   // RMode (bits [23:22], 0 = round to nearest)

    asm volatile("msr fpcr, %0" :: "r" (iValue));

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

#elif defined(_CORE_NEON_)

    // disable in the FPCR control register (for NEON, on 64-bit only)
    coreUint64 iValue;
    asm volatile("mrs %0, fpcr" : "=r" (iValue));

    ADD_BIT(iValue, 24u)   // FZ   (single and double)
    ADD_BIT(iValue, 19u)   // FZ16 (half)

    asm volatile("msr fpcr, %0" :: "r" (iValue));

#endif

#if defined(_CORE_WINDOWS_)

    // disable with Windows function (for x87 and SSE)
    _controlfp(_DN_FLUSH, _MCW_DN);

#endif
}


#endif /* _CORE_GUARD_MATH_H_ */