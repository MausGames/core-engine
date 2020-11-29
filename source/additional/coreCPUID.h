///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_CPUID_H_
#define _CORE_GUARD_CPUID_H_


// ****************************************************************
/* CPUID instruction definition */
#if defined(_CORE_SSE_)
    #if defined(_CORE_MSVC_)
        #define CORE_CPUID_FUNC(x,a,c) {__cpuidex(x, a, c);}
        #define CORE_XGETBV_FUNC(x)    {(*(x)) = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);}
    #else
        #define CORE_CPUID_FUNC(x,a,c) {asm volatile("cpuid" : "=a" (x[0]), "=b" (x[1]), "=c" (x[2]), "=d" (x[3]) : "a" (a), "c" (c));}
        #define CORE_XGETBV_FUNC(x)    {coreInt32 v[2]; asm volatile("xgetbv" : "=a" (v[0]), "=d" (v[1]) : "c" (0)); std::memcpy(x, v, sizeof(coreUint64));}
    #endif
#else
    #define CORE_CPUID_FUNC(x,a,c) {std::memset(x, 0, sizeof(coreInt32) * 4u);}
    #define CORE_XGETBV_FUNC(x)    {std::memset(x, 0, sizeof(coreUint64));}
#endif

#define CORE_CPUID_BASIC    (0x00000000u)
#define CORE_CPUID_EXTENDED (0x80000000u)


// ****************************************************************
/* CPUID access class */
class INTERFACE coreCPUID final
{
private:
    /* CPUID data structure */
    struct __coreCPUID final
    {
        coreChar   acVendor[13];
        coreChar   acBrand [49];
        coreBool   bIsIntel;
        coreBool   bIsAMD;
        coreInt32  i01ECX;
        coreInt32  i01EDX;
        coreInt32  i07EBX;
        coreInt32  i07ECX;
        coreInt32  i81ECX;
        coreInt32  i81EDX;
        coreUint64 iXCR;

        __coreCPUID()noexcept;
    };


private:
    alignas(ALIGNMENT_CACHE) static const __coreCPUID s_CPUID;   // singleton object with pre-loaded processor information


public:
    DISABLE_CONSTRUCTION(coreCPUID)

    /* get processor strings */
    static const coreChar* Vendor() {return s_CPUID.acVendor;}
    static const coreChar* Brand () {return s_CPUID.acBrand;}

    /* get processor feature bits */
    static coreBool SSE3        () {return CONTAINS_BIT(s_CPUID.i01ECX,  0);}
    static coreBool PCLMULQDQ   () {return CONTAINS_BIT(s_CPUID.i01ECX,  1);}
    static coreBool MONITOR     () {return CONTAINS_BIT(s_CPUID.i01ECX,  3);}
    static coreBool SSSE3       () {return CONTAINS_BIT(s_CPUID.i01ECX,  9);}
    static coreBool FMA         () {return CONTAINS_BIT(s_CPUID.i01ECX, 12);}
    static coreBool CMPXCHG16B  () {return CONTAINS_BIT(s_CPUID.i01ECX, 13);}
    static coreBool SSE41       () {return CONTAINS_BIT(s_CPUID.i01ECX, 19);}
    static coreBool SSE42       () {return CONTAINS_BIT(s_CPUID.i01ECX, 20);}
    static coreBool MOVBE       () {return CONTAINS_BIT(s_CPUID.i01ECX, 22);}
    static coreBool POPCNT      () {return CONTAINS_BIT(s_CPUID.i01ECX, 23);}
    static coreBool AES         () {return CONTAINS_BIT(s_CPUID.i01ECX, 25);}
    static coreBool XSAVE       () {return CONTAINS_BIT(s_CPUID.i01ECX, 26);}
    static coreBool OSXSAVE     () {return CONTAINS_BIT(s_CPUID.i01ECX, 27);}
    static coreBool AVX         () {return CONTAINS_BIT(s_CPUID.i01ECX, 28) && CONTAINS_FLAG(s_CPUID.iXCR, 0x06u);}
    static coreBool F16C        () {return CONTAINS_BIT(s_CPUID.i01ECX, 29);}
    static coreBool RDRAND      () {return CONTAINS_BIT(s_CPUID.i01ECX, 30);}

    static coreBool MSR         () {return CONTAINS_BIT(s_CPUID.i01EDX,  5);}
    static coreBool CX8         () {return CONTAINS_BIT(s_CPUID.i01EDX,  8);}
    static coreBool SEP         () {return CONTAINS_BIT(s_CPUID.i01EDX, 11);}
    static coreBool CMOV        () {return CONTAINS_BIT(s_CPUID.i01EDX, 15);}
    static coreBool CLFSH       () {return CONTAINS_BIT(s_CPUID.i01EDX, 19);}
    static coreBool MMX         () {return CONTAINS_BIT(s_CPUID.i01EDX, 23);}
    static coreBool FXSR        () {return CONTAINS_BIT(s_CPUID.i01EDX, 24);}
    static coreBool SSE         () {return CONTAINS_BIT(s_CPUID.i01EDX, 25);}
    static coreBool SSE2        () {return CONTAINS_BIT(s_CPUID.i01EDX, 26);}

    static coreBool FSGSBASE    () {return CONTAINS_BIT(s_CPUID.i07EBX,  0);}
    static coreBool BMI1        () {return CONTAINS_BIT(s_CPUID.i07EBX,  3);}
    static coreBool HLE         () {return CONTAINS_BIT(s_CPUID.i07EBX,  4) && s_CPUID.bIsIntel;}
    static coreBool AVX2        () {return CONTAINS_BIT(s_CPUID.i07EBX,  5);}
    static coreBool BMI2        () {return CONTAINS_BIT(s_CPUID.i07EBX,  8);}
    static coreBool ERMS        () {return CONTAINS_BIT(s_CPUID.i07EBX,  9);}
    static coreBool INVPCID     () {return CONTAINS_BIT(s_CPUID.i07EBX, 10);}
    static coreBool RTM         () {return CONTAINS_BIT(s_CPUID.i07EBX, 11) && s_CPUID.bIsIntel;}
    static coreBool AVX512F     () {return CONTAINS_BIT(s_CPUID.i07EBX, 16);}
    static coreBool AVX512DQ    () {return CONTAINS_BIT(s_CPUID.i07EBX, 17);}
    static coreBool RDSEED      () {return CONTAINS_BIT(s_CPUID.i07EBX, 18);}
    static coreBool ADX         () {return CONTAINS_BIT(s_CPUID.i07EBX, 19);}
    static coreBool AVX512IFMA  () {return CONTAINS_BIT(s_CPUID.i07EBX, 21);}
    static coreBool AVX512PF    () {return CONTAINS_BIT(s_CPUID.i07EBX, 26);}
    static coreBool AVX512ER    () {return CONTAINS_BIT(s_CPUID.i07EBX, 27);}
    static coreBool AVX512CD    () {return CONTAINS_BIT(s_CPUID.i07EBX, 28);}
    static coreBool SHA         () {return CONTAINS_BIT(s_CPUID.i07EBX, 29);}
    static coreBool AVX512BW    () {return CONTAINS_BIT(s_CPUID.i07EBX, 30);}
    static coreBool AVX512VL    () {return CONTAINS_BIT(s_CPUID.i07EBX, 31);}

    static coreBool PREFETCHWT1 () {return CONTAINS_BIT(s_CPUID.i07ECX,  0);}
    static coreBool AVX512VBMI  () {return CONTAINS_BIT(s_CPUID.i07ECX,  1);}
    static coreBool AVX512VBMI2 () {return CONTAINS_BIT(s_CPUID.i07ECX,  6);}
    static coreBool AVX512VNNI  () {return CONTAINS_BIT(s_CPUID.i07ECX, 11);}
    static coreBool AVX512BITALG() {return CONTAINS_BIT(s_CPUID.i07ECX, 12);}

    static coreBool LAHF        () {return CONTAINS_BIT(s_CPUID.i81ECX,  0);}
    static coreBool LZCNT       () {return CONTAINS_BIT(s_CPUID.i81ECX,  5) && s_CPUID.bIsIntel;}
    static coreBool ABM         () {return CONTAINS_BIT(s_CPUID.i81ECX,  5) && s_CPUID.bIsAMD;}
    static coreBool SSE4A       () {return CONTAINS_BIT(s_CPUID.i81ECX,  6) && s_CPUID.bIsAMD;}
    static coreBool XOP         () {return CONTAINS_BIT(s_CPUID.i81ECX, 11) && s_CPUID.bIsAMD;}
    static coreBool TBM         () {return CONTAINS_BIT(s_CPUID.i81ECX, 21) && s_CPUID.bIsAMD;}

    static coreBool SYSCALL     () {return CONTAINS_BIT(s_CPUID.i81EDX, 11) && s_CPUID.bIsIntel;}
    static coreBool MMXEXT      () {return CONTAINS_BIT(s_CPUID.i81EDX, 22) && s_CPUID.bIsAMD;}
    static coreBool RDTSCP      () {return CONTAINS_BIT(s_CPUID.i81EDX, 27) && s_CPUID.bIsIntel;}
    static coreBool _3DNOWEXT   () {return CONTAINS_BIT(s_CPUID.i81EDX, 30) && s_CPUID.bIsAMD;}
    static coreBool _3DNOW      () {return CONTAINS_BIT(s_CPUID.i81EDX, 31) && s_CPUID.bIsAMD;}
};


#endif /* _CORE_GUARD_CPUID_H_ */