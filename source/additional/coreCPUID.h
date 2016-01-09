//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_CPUID_H_
#define	_CORE_CPUID_H_


// ****************************************************************
/* CPUID instruction definition */
#if defined(_CORE_SSE_)
    #if defined(_CORE_MSVC_)
        #define CORE_CPUID_EX(x,a,c) {__cpuidex(x, a, c);}
    #else
        #define CORE_CPUID_EX(x,a,c) {asm volatile("cpuid" : "=a" (x[0]), "=b" (x[1]), "=c" (x[2]), "=d" (x[3]) : "a" (a), "c" (c));}
    #endif
#else
    #define CORE_CPUID_EX(x,a,c) {std::memset(x, 0, 4u*sizeof(coreInt32));}
#endif


// ****************************************************************
/* CPUID access class */
class INTERFACE coreCPUID final
{
private:
    /*! CPUID data structure */
    struct __coreCPUID
    {
        coreChar  acVendor[13];
        coreChar  acBrand [49];
        coreBool  bIsIntel;
        coreBool  bIsAMD;
        coreInt32 i01ECX;
        coreInt32 i01EDX;
        coreInt32 i07EBX;
        coreInt32 i07ECX;
        coreInt32 i81ECX;
        coreInt32 i81EDX;

        __coreCPUID()noexcept;
    };


private:
    static const __coreCPUID s_CPUID;   //!< singleton object with pre-loaded processor information


public:
    DISABLE_CONSTRUCTION(coreCPUID)

    /*! get processor strings */
    static const coreChar* Vendor() {return s_CPUID.acVendor;}
    static const coreChar* Brand () {return s_CPUID.acBrand;}

    /*! get processor feature bits */
    static coreBool SSE3       () {return CONTAINS_BIT(s_CPUID.i01ECX,  0);}
    static coreBool PCLMULQDQ  () {return CONTAINS_BIT(s_CPUID.i01ECX,  1);}
    static coreBool MONITOR    () {return CONTAINS_BIT(s_CPUID.i01ECX,  3);}
    static coreBool SSSE3      () {return CONTAINS_BIT(s_CPUID.i01ECX,  9);}
    static coreBool FMA        () {return CONTAINS_BIT(s_CPUID.i01ECX, 12);}
    static coreBool CMPXCHG16B () {return CONTAINS_BIT(s_CPUID.i01ECX, 13);}
    static coreBool SSE41      () {return CONTAINS_BIT(s_CPUID.i01ECX, 19);}
    static coreBool SSE42      () {return CONTAINS_BIT(s_CPUID.i01ECX, 20);}
    static coreBool MOVBE      () {return CONTAINS_BIT(s_CPUID.i01ECX, 22);}
    static coreBool POPCNT     () {return CONTAINS_BIT(s_CPUID.i01ECX, 23);}
    static coreBool AES        () {return CONTAINS_BIT(s_CPUID.i01ECX, 25);}
    static coreBool XSAVE      () {return CONTAINS_BIT(s_CPUID.i01ECX, 26);}
    static coreBool OSXSAVE    () {return CONTAINS_BIT(s_CPUID.i01ECX, 27);}
    static coreBool AVX        () {return CONTAINS_BIT(s_CPUID.i01ECX, 28);}
    static coreBool F16C       () {return CONTAINS_BIT(s_CPUID.i01ECX, 29);}
    static coreBool RDRAND     () {return CONTAINS_BIT(s_CPUID.i01ECX, 30);}

    static coreBool MSR        () {return CONTAINS_BIT(s_CPUID.i01EDX,  5);}
    static coreBool CX8        () {return CONTAINS_BIT(s_CPUID.i01EDX,  8);}
    static coreBool SEP        () {return CONTAINS_BIT(s_CPUID.i01EDX, 11);}
    static coreBool CMOV       () {return CONTAINS_BIT(s_CPUID.i01EDX, 15);}
    static coreBool CLFSH      () {return CONTAINS_BIT(s_CPUID.i01EDX, 19);}
    static coreBool MMX        () {return CONTAINS_BIT(s_CPUID.i01EDX, 23);}
    static coreBool FXSR       () {return CONTAINS_BIT(s_CPUID.i01EDX, 24);}
    static coreBool SSE        () {return CONTAINS_BIT(s_CPUID.i01EDX, 25);}
    static coreBool SSE2       () {return CONTAINS_BIT(s_CPUID.i01EDX, 26);}

    static coreBool FSGSBASE   () {return CONTAINS_BIT(s_CPUID.i07EBX,  0);}
    static coreBool BMI1       () {return CONTAINS_BIT(s_CPUID.i07EBX,  3);}
    static coreBool HLE        () {return CONTAINS_BIT(s_CPUID.i07EBX,  4) && s_CPUID.bIsIntel;}
    static coreBool AVX2       () {return CONTAINS_BIT(s_CPUID.i07EBX,  5);}
    static coreBool BMI2       () {return CONTAINS_BIT(s_CPUID.i07EBX,  8);}
    static coreBool ERMS       () {return CONTAINS_BIT(s_CPUID.i07EBX,  9);}
    static coreBool INVPCID    () {return CONTAINS_BIT(s_CPUID.i07EBX, 10);}
    static coreBool RTM        () {return CONTAINS_BIT(s_CPUID.i07EBX, 11) && s_CPUID.bIsIntel;}
    static coreBool AVX512F    () {return CONTAINS_BIT(s_CPUID.i07EBX, 16);}
    static coreBool RDSEED     () {return CONTAINS_BIT(s_CPUID.i07EBX, 18);}
    static coreBool ADX        () {return CONTAINS_BIT(s_CPUID.i07EBX, 19);}
    static coreBool AVX512PF   () {return CONTAINS_BIT(s_CPUID.i07EBX, 26);}
    static coreBool AVX512ER   () {return CONTAINS_BIT(s_CPUID.i07EBX, 27);}
    static coreBool AVX512CD   () {return CONTAINS_BIT(s_CPUID.i07EBX, 28);}
    static coreBool SHA        () {return CONTAINS_BIT(s_CPUID.i07EBX, 29);}

    static coreBool PREFETCHWT1() {return CONTAINS_BIT(s_CPUID.i07ECX,  0);}

    static coreBool LAHF       () {return CONTAINS_BIT(s_CPUID.i81ECX,  0);}
    static coreBool LZCNT      () {return CONTAINS_BIT(s_CPUID.i81ECX,  5) && s_CPUID.bIsIntel;}
    static coreBool ABM        () {return CONTAINS_BIT(s_CPUID.i81ECX,  5) && s_CPUID.bIsAMD;}
    static coreBool SSE4a      () {return CONTAINS_BIT(s_CPUID.i81ECX,  6) && s_CPUID.bIsAMD;}
    static coreBool XOP        () {return CONTAINS_BIT(s_CPUID.i81ECX, 11) && s_CPUID.bIsAMD;}
    static coreBool TBM        () {return CONTAINS_BIT(s_CPUID.i81ECX, 21) && s_CPUID.bIsAMD;}

    static coreBool SYSCALL    () {return CONTAINS_BIT(s_CPUID.i81EDX, 11) && s_CPUID.bIsIntel;}
    static coreBool MMXEXT     () {return CONTAINS_BIT(s_CPUID.i81EDX, 22) && s_CPUID.bIsAMD;}
    static coreBool RDTSCP     () {return CONTAINS_BIT(s_CPUID.i81EDX, 27) && s_CPUID.bIsIntel;}
    static coreBool _3DNOWEXT  () {return CONTAINS_BIT(s_CPUID.i81EDX, 30) && s_CPUID.bIsAMD;}
    static coreBool _3DNOW     () {return CONTAINS_BIT(s_CPUID.i81EDX, 31) && s_CPUID.bIsAMD;}
};


#endif /* _CORE_CPUID_H_ */