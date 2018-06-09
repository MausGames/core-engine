////////////////////////////////////////////////////////////////////////////////////
//*------------------------------------------------------------------------------*//
//|      _____ ____  _____  ______    ______ _   _  _____ _____ _   _ ______     |//
//|     / ____/ __ \|  __ \|  ____|  |  ____| \ | |/ ____|_   _| \ | |  ____|    |//
//|    | |   | |  | | |__) | |__     | |__  |  \| | |  __  | | |  \| | |__       |//
//|    | |   | |  | |  _  /|  __|    |  __| |     | | |_ | | | |     |  __|      |//
//|    | |___| |__| | | \ \| |____   | |____| |\  | |__| |_| |_| |\  | |____     |//
//|     \_____\____/|_|  \_\______|  |______|_| \_|\_____|_____|_| \_|______|    |//
//|                                                                              |//
//*------------------------------------------------------------------------------*//
////////////////////////////////////////////////////////////////////////////////////
//*------------------------------------------------------------------------------*//
//| Core Engine v0.0.9a (http://www.maus-games.at)                               |//
//*------------------------------------------------------------------------------*//
//| Copyright (c) 2013-2018 Martin Mauersics                                     |//
//|                                                                              |//
//| This software is provided 'as-is', without any express or implied            |//
//| warranty. In no event will the authors be held liable for any damages        |//
//| arising from the use of this software.                                       |//
//|                                                                              |//
//| Permission is granted to anyone to use this software for any purpose,        |//
//| including commercial applications, and to alter it and redistribute it       |//
//| freely, subject to the following restrictions:                               |//
//|                                                                              |//
//|   1. The origin of this software must not be misrepresented; you must not    |//
//|   claim that you wrote the original software. If you use this software       |//
//|   in a product, an acknowledgment in the product documentation would be      |//
//|   appreciated but is not required.                                           |//
//|                                                                              |//
//|   2. Altered source versions must be plainly marked as such, and must not be |//
//|   misrepresented as being the original software.                             |//
//|                                                                              |//
//|   3. This notice may not be removed or altered from any source               |//
//|   distribution.                                                              |//
//*------------------------------------------------------------------------------*//
////////////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_H_
#define _CORE_GUARD_H_

// TODO: improve sort and structure under all class access modifiers
// TODO: check for template parameters <42>
// TODO: put everything in a namespace ? split up coreData and coreMath
// TODO: remove this whole static pointer stuff, namespace for main-classes together with math and data ?
// TODO: define standard-path (data/) were everything is loaded from
// TODO: implement GL_KHR_robustness (performance penalties ?)
// TODO: check for performance penalties and alternatives for thread_local
// TODO: extend assertion-macro and add message to all assertions (warn_if ?)
// TODO: reduce core-prefix ? (e.g. cr)
// TODO: unique pointers and move semantics for functions taking ownership of a pointer
// TODO: video class, string class, path/url class, pool+stack-allocator
// TODO: fix constexpr msvc bug (not compiler-setting?) (constexpr-results not saved after compilation, functions often re-evaluated at run-time)
// TODO: disallow two instances of the same application (also in launcher)
// TODO: #define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
// TODO: check out NVAPI and ADL/AGS
// TODO: cast unused returns to (void)?
// TODO: remove SSE from 32-bit binaries ? (compatibility binaries) _USING_V110_SDK71_
// TODO: enable MSVC warnings 4244, 4267 again
// TODO: #pragma warning(default : 4242 4244 4365) // loss of precision
// TODO: #pragma warning(default : 4820)           // byte padding
// TODO: unify const void* and const coreByte*
// TODO: "WARN_IF" where applicable, "if" where not (check between user-caused errors, system-caused errors, developer errors)

// NOTE: always compile Win32 libraries/executables for WinXP


// compiler
#if defined(_MSC_VER)
    #define _CORE_MSVC_  (_MSC_VER)
#endif
#if defined(__GNUC__)
    #define _CORE_GCC_   (__GNUC__*10000 + __GNUC_MINOR__*100 + __GNUC_PATCHLEVEL__*1)
#endif
#if defined(__MINGW32__)
    #define _CORE_MINGW_ (__GNUC__*10000 + __GNUC_MINOR__*100 + __GNUC_PATCHLEVEL__*1)
    #undef  _CORE_GCC_
#endif
#if defined(__clang__)
    #define _CORE_CLANG_ (__clang_major__*10000 + __clang_minor__*100 + __clang_patchlevel__*1)
    #undef  _CORE_GCC_
#endif
#if ((_CORE_MSVC_) < 1911) && ((_CORE_GCC_) < 60200) && ((_CORE_MINGW_) < 60200) && ((_CORE_CLANG_) < 40000)
    #warning "Compiler not supported!"
#endif

// operating system
#if defined(__APPLE__)
    #include <targetconditionals.h>
#endif
#if defined(_WIN32)
    #define _CORE_WINDOWS_ (1)
#endif
#if defined(__linux__)
    #define _CORE_LINUX_   (1)
#endif
#if defined(__APPLE__) && TARGET_OS_MAC
    #define _CORE_OSX_     (1)
#endif
#if defined(__ANDROID__)
    #define _CORE_ANDROID_ (1)
    #undef  _CORE_LINUX_
#endif
#if defined(__APPLE__) && TARGET_OS_IPHONE
    #define _CORE_IOS_     (1)
    #undef  _CORE_OSX_
#endif
#if !defined(_CORE_WINDOWS_) && !defined(_CORE_LINUX_) /*&& !defined(_CORE_OSX_)*/ && !defined(_CORE_ANDROID_) /*&& defined(_CORE_IOS_)*/
    #warning "Operating System not supported!"
#endif

// mobile mode
#if defined(_CORE_ANDROID_) || defined(_CORE_IOS_)
    #define _CORE_MOBILE_ (1)
#endif

// debug mode
#if defined(_DEBUG) || defined(DEBUG) || ((defined(_CORE_GCC_) || defined(_CORE_MINGW_) || defined(_CORE_CLANG_)) && !defined(__OPTIMIZE__))
    #define _CORE_DEBUG_  (1)
#endif

// async mode
#if !defined(_CORE_MOBILE_)
    #define _CORE_ASYNC_  (1)
#endif

// OpenGL ES mode
#if defined(_CORE_MOBILE_)
    #define _CORE_GLES_   (1)
#endif

// Windows XP mode
#if (defined(_USING_V110_SDK71_) || defined(_CORE_MINGW_)) && defined(_CORE_WINDOWS_)
    #define _CORE_WINXP_  (1)
#endif

// x64 instruction set
#if defined(_M_X64) || defined(__x86_64__)
    #define _CORE_X64_    (1)
#endif

// SSE2 instruction set
#if (defined(_M_IX86) || defined(__i386__) || defined(_CORE_X64_)) && !defined(_CORE_MOBILE_) && !defined(_CORE_WINXP_)
    #define _CORE_SSE_    (1)
#endif


// ****************************************************************
/* compiler definitions */
#if defined(_CORE_MSVC_)
    #define UNUSED           [[maybe_unused]]       //!< possibly unused variable (warnings 4100, 4101, 4189)
    #define OUTPUT           __restrict             //!< output parameter without aliasing
    #define INTERFACE        __declspec(novtable)   //!< pure interface class without direct instantiation
    #define FORCE_INLINE     __forceinline          //!< always inline the function
    #define DONT_INLINE      __declspec(noinline)   //!< never inline the function
    #define RETURN_RESTRICT  __declspec(restrict)   //!< returned object will not be aliased with another pointer
    #define RETURN_NONNULL   _Ret_notnull_          //!< returned pointer will not be null
    #define RETURN_NODISCARD [[nodiscard]]          //!< returned value should not be discarded (but can be cast to void)
    #define FUNC_PURE                               //!< function does not modify anything (or reads volatile global state), and returns a value
    #define FUNC_CONST       __declspec(noalias)    //!< function only reads parameters (without indirections), and returns a value
    #define FUNC_LOCAL       __declspec(noalias)    //!< function only reads parameters, reads first-level indirections (e.g. this), and returns a value
    #define FUNC_NOALIAS     __declspec(noalias)    //!< function only reads parameters, reads and writes first-level indirections, and may return a value
    #define FUNC_TERMINATE   __declspec(noreturn)   //!< function does not return (e.g. by calling exit(3) or abort(3))
#else
    #define UNUSED           __attribute__((unused))
    #define OUTPUT           __restrict__
    #define INTERFACE
    #define FORCE_INLINE     __attribute__((always_inline)) inline
    #define DONT_INLINE      __attribute__((noinline))
    #define RETURN_RESTRICT  __attribute__((malloc))
    #define RETURN_NONNULL   __attribute__((returns_nonnull))
    #define RETURN_NODISCARD __attribute__((warn_unused_result))
    #define FUNC_PURE        __attribute__((pure))
    #define FUNC_CONST       __attribute__((const))
    #define FUNC_LOCAL       __attribute__((pure))
    #define FUNC_NOALIAS
    #define FUNC_TERMINATE   __attribute__((noreturn, cold))
#endif

#if defined(_CORE_MINGW_) && defined(_CORE_SSE_)
    #define ENTRY_POINT      __attribute__((force_align_arg_pointer))   //!< realign run-time stack (to fix SSE)
#else
    #define ENTRY_POINT
#endif

#if defined(_CORE_MSVC_)

    // disable unwanted compiler warnings (with /Wall)
    #pragma warning(disable : 4061)   //!< enumerator not handled in switch
    #pragma warning(disable : 4100)   //!< unreferenced formal parameter
    #pragma warning(disable : 4201)   //!< nameless struct or union
    #pragma warning(disable : 4242)   //!< implicit conversion to different precision #1
    #pragma warning(disable : 4244)   //!< implicit conversion to different precision #2
    #pragma warning(disable : 4266)   //!< virtual function not overridden
    #pragma warning(disable : 4267)   //!< implicit conversion of std::size_t
    #pragma warning(disable : 4365)   //!< implicit conversion between signed and unsigned
    #pragma warning(disable : 4557)   //!< __assume contains effect (# only false-positives)
    #pragma warning(disable : 4577)   //!< noexcept used without exception handling
    #pragma warning(disable : 4623)   //!< default constructor implicitly deleted
    #pragma warning(disable : 4625)   //!< copy constructor implicitly deleted
    #pragma warning(disable : 4626)   //!< copy assignment operator implicitly deleted
    #pragma warning(disable : 4647)   //!< behavior change of std::is_pod
    #pragma warning(disable : 4668)   //!< preprocessor macro not defined
    #pragma warning(disable : 4710)   //!< function not inlined
    #pragma warning(disable : 4711)   //!< function automatically inlined
    #pragma warning(disable : 4774)   //!< format string not a string literal
    #pragma warning(disable : 4820)   //!< padding after data member
    #pragma warning(disable : 5026)   //!< move constructor implicitly deleted
    #pragma warning(disable : 5027)   //!< move assignment operator implicitly deleted
    #pragma warning(disable : 5039)   //!< potentially throwing function passed to extern C function
    #pragma warning(disable : 5045)   //!< possible Spectre vulnerability

    // check for floating-point results stored in memory, causing performance loss
    #if defined(_CORE_X64_)
        #pragma warning(error : 4738)
    #else
        #pragma warning(disable : 4738)
    #endif

#elif defined(_CORE_GCC_) || defined(_CORE_MINGW_)

    // disable unwanted compiler warnings (with -Wall)
    #pragma GCC diagnostic ignored "-Wmisleading-indentation"

#endif

#if !defined(_CORE_DEBUG_)
    #if defined(_CORE_MSVC_)
        #pragma fenv_access (off)   //!< ignore access to the floating-point environment
        #pragma fp_contract (on)    //!< allow contracting of floating-point expressions
    #else
        #pragma STDC FENV_ACCESS OFF
        #pragma STDC FP_CONTRACT ON
    #endif
#endif


// ****************************************************************
/* standard libraries */
#define _GNU_SOURCE     (1)
#define _HAS_EXCEPTIONS (0)
#define _ALLOW_KEYWORD_MACROS
#define _ALLOW_RTCc_IN_STL
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#if defined(_CORE_MINGW_)
    #undef __STRICT_ANSI__
#endif
#if defined(_CORE_WINXP_)
    #define _WIN32_WINNT _WIN32_WINNT_WINXP
    #define WINVER       _WIN32_WINNT_WINXP
#else
    #define _WIN32_WINNT _WIN32_WINNT_WIN7
    #define WINVER       _WIN32_WINNT_WIN7
#endif

#if defined(_CORE_WINDOWS_)
    #include <windows.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif
#if defined(_CORE_SSE_)
    #include <immintrin.h>
#endif
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cfloat>
#include <cfenv>
#include <cmath>
#include <ctime>
#include <type_traits>
#include <functional>
#include <memory>
#include <random>
#include <string>
#include <array>
#include <vector>
#include <list>


// ****************************************************************
/* external libraries */
#define HAVE_LIBC (1)
#define GLEW_NO_GLU
#define OV_EXCLUDE_STATIC_CALLBACKS
#define ZLIB_CONST
#define ZLIB_DLL

#include <sdl2/sdl.h>
#include <sdl2/sdl_ttf.h>
#include <sdl2/sdl_image.h>
#if defined(_CORE_GLES_)
    #include <gles2/gl2.h>
#else
    #include <gl/glew.h>
#endif
#include <al/al.h>
#include <al/alc.h>
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>
#include <zlib/zlib.h>
#include <si/simpleini.h>


// ****************************************************************
/* general definitions */
#undef NULL
#undef __STRING
#undef __CONCAT

#define NULL                 nullptr
#define __STRING(a)          #a
#define STRING(a)            __STRING(a)
#define __CONCAT(a,b)        a ## b
#define CONCAT(a,b)          __CONCAT(a, b)
#define DEFINED(a)           (FORCE_COMPILE_TIME(!coreData::StrCmpConst(STRING(a), #a)))

#define SAFE_DELETE(p)       {delete   (p); (p) = NULL;}
#define SAFE_DELETE_ARRAY(p) {delete[] (p); (p) = NULL;}

#define BIT(n)               (1ull << (n))   // starts with 0
#define BITLINE(n)           (BIT(n) - 1ull)
#define ADD_BIT(o,n)         { (o) |=  BIT(n);}
#define ADD_FLAG(o,n)        { (o) |=     (n);}
#define REMOVE_BIT(o,n)      { (o) &= ~BIT(n);}
#define REMOVE_FLAG(o,n)     { (o) &=    ~(n);}
#define TOGGLE_BIT(o,n)      { (o) ^=  BIT(n);}
#define TOGGLE_FLAG(o,n)     { (o) ^=     (n);}
#define SET_BIT(o,n,t)       { (o) ^=  BIT(n) & ((o) ^ ((t) ? ~0ull : 0ull));}
#define SET_FLAG(o,n,t)      { (o) ^=     (n) & ((o) ^ ((t) ? ~0ull : 0ull));}
#define CONTAINS_BIT(o,n)    (((o) &   BIT(n)) ? true : false)
#define CONTAINS_FLAG(o,n)   (((o) &      (n)) == (n))

#define FOR_EACH(i,c)        for(auto i = (c).begin(),  i ## __e = (c).end();  i != i ## __e; ++i)
#define FOR_EACH_REV(i,c)    for(auto i = (c).rbegin(), i ## __e = (c).rend(); i != i ## __e; ++i)
#define FOR_EACH_SET(i,s,c)  for(auto i = (s),          i ## __e = (c).end();  i != i ## __e; ++i)
#define FOR_EACH_DYN(i,c)    for(auto i = (c).begin(),  i ## __e = (c).end();  i != i ## __e; )
#define DYN_KEEP(i)          {++i;}
#define DYN_REMOVE(i,c)      {i = (c).erase(i); i ## __e = (c).end();}

#define BIG_STATIC           static
#define FRIEND_CLASS(c)      friend class c;

#if defined(_CORE_DEBUG_)
    #define ASSERT(c)        {SDL_assert((c));}
#else
    #if defined(_CORE_MSVC_)
        #define ASSERT(c)    {__assume(!!(c));}
    #else
        #define ASSERT(c)    {if(!(c)) __builtin_unreachable();}
    #endif
#endif

#if defined(_CORE_DEBUG_)
    #define WARN_IF(c)       if([](const coreBool bWarnIf) {ASSERT(!bWarnIf); return bWarnIf;}(!!(c)))
#else
    #if defined(_CORE_MSVC_)
        #define WARN_IF(c)   if(c)
    #else
        #define WARN_IF(c)   if(__builtin_expect(c, 0))
    #endif
#endif

#define STATIC_ASSERT(c)     static_assert(c, "Static Assertion [" #c "]");

// disable constructor and destructor of the defined class
#define DISABLE_CONSTRUCTION(c) \
     c() = delete;              \
    ~c() = delete;

// enable (explicitly) or disable copy- and move-operations with the defined class
#define ENABLE_COPY(c)                  \
    c             (const c&) = default; \
    c& operator = (const c&) = default; \
    c             (c&&)      = default; \
    c& operator = (c&&)      = default;
#define DISABLE_COPY(c)                 \
    c             (const c&) = delete;  \
    c& operator = (const c&) = delete;  \
    c             (c&&)      = delete;  \
    c& operator = (c&&)      = delete;

// disable heap-operations with the defined class
#define DISABLE_NEW                                        \
    void* operator new      (std::size_t)        = delete; \
    void* operator new      (std::size_t, void*) = delete; \
    void* operator new[]    (std::size_t)        = delete; \
    void* operator new[]    (std::size_t, void*) = delete;
#define DISABLE_DELETE                                     \
    void  operator delete   (void*)              = delete; \
    void  operator delete   (void*, void*)       = delete; \
    void  operator delete   (void*, std::size_t) = delete; \
    void  operator delete[] (void*)              = delete; \
    void  operator delete[] (void*, void*)       = delete; \
    void  operator delete[] (void*, std::size_t) = delete;

// enable bitwise-operations with the defined enumeration
#define ENABLE_BITWISE(e)                                                                                                                                       \
    constexpr e  operator ~  (const e   a)            {return s_cast<e>(~s_cast<std::underlying_type<e>::type>(a));}                                            \
    constexpr e  operator |  (const e   a, const e b) {return s_cast<e>( s_cast<std::underlying_type<e>::type>(a) | s_cast<std::underlying_type<e>::type>(b));} \
    constexpr e  operator &  (const e   a, const e b) {return s_cast<e>( s_cast<std::underlying_type<e>::type>(a) & s_cast<std::underlying_type<e>::type>(b));} \
    constexpr e  operator ^  (const e   a, const e b) {return s_cast<e>( s_cast<std::underlying_type<e>::type>(a) ^ s_cast<std::underlying_type<e>::type>(b));} \
    inline    e& operator |= (e& OUTPUT a, const e b) {return (a = a | b);}                                                                                     \
    inline    e& operator &= (e& OUTPUT a, const e b) {return (a = a & b);}                                                                                     \
    inline    e& operator ^= (e& OUTPUT a, const e b) {return (a = a ^ b);}

// shorter common keywords
#define f_list forward_list
#define u_map  unordered_map
#define u_set  unordered_set
#define s_cast static_cast
#define d_cast dynamic_cast
#define r_cast reinterpret_cast
#define c_cast const_cast

// type conversion macros
#define F_TO_SI(x) ((coreInt32)            (x))   //!< float to signed int
#define F_TO_UI(x) ((coreUint32)(coreInt32)(x))   //!< float to unsigned int (force [_mm_cvtt_ss2si])
#define I_TO_F(x)  ((coreFloat)(coreInt32) (x))   //!< int to float          (force [_mm_cvtepi32_ps])
#define P_TO_SI(x) ((std::intptr_t)(void*) (x))   //!< pointer to signed int
#define P_TO_UI(x) ((std::uintptr_t)(void*)(x))   //!< pointer to unsigned int
#define I_TO_P(x)  ((void*)(std::intptr_t) (x))   //!< int to pointer

// type definitions
using coreInt8   = std::int8_t;
using coreInt16  = std::int16_t;
using coreInt32  = std::int32_t;
using coreInt64  = std::int64_t;
using coreUint8  = std::uint8_t;
using coreUint16 = std::uint16_t;
using coreUint32 = std::uint32_t;
using coreUint64 = std::uint64_t;
using coreUintW  = std::size_t;
using coreByte   = std::uint8_t;
using coreBool   = bool;
using coreChar   = char;
using coreFloat  = float;
using coreDouble = double;

// user-defined literals
constexpr coreUintW operator "" _zu(unsigned long long i) {return coreUintW(i);}

// override string comparison operator
inline coreBool operator == (const std::string& a, const coreChar*    b) {return !std::strcmp(a.c_str(), b);}
inline coreBool operator != (const std::string& a, const coreChar*    b) {return  std::strcmp(a.c_str(), b);}
inline coreBool operator == (const coreChar*    a, const std::string& b) {return !std::strcmp(a,         b.c_str());}
inline coreBool operator != (const coreChar*    a, const std::string& b) {return  std::strcmp(a,         b.c_str());}

// retrieve compile-time pointer-safe array size
template <typename T, coreUintW iSize> coreChar (&__ARRAY_SIZE(T (&)[iSize]))[iSize];
#define ARRAY_SIZE(a) (sizeof(__ARRAY_SIZE(a)))

// retrieve compile-time function and lambda properties
template <typename T>                            struct INTERFACE coreFunctionTraits final               : public coreFunctionTraits<decltype(&std::remove_reference<T>::type::operator())> {};
template <typename R, typename C, typename... A> struct INTERFACE coreFunctionTraits<R(C::*)(A...)const> : public coreFunctionTraits<R(A...)>                                               {};
template <typename R, typename C, typename... A> struct INTERFACE coreFunctionTraits<R(C::*)(A...)>      : public coreFunctionTraits<R(A...)>                                               {};
template <typename R,             typename... A> struct INTERFACE coreFunctionTraits<R   (*)(A...)>      : public coreFunctionTraits<R(A...)>                                               {};
template <typename R,             typename... A> struct INTERFACE coreFunctionTraits<R      (A...)>
{
    using uReturnType = R;                                                                                      //!< return type
    template <coreUintW iIndex> using uArgType = typename std::tuple_element<iIndex, std::tuple<A...>>::type;   //!< argument types
    enum : coreUintW {iArity = sizeof...(A)};                                                                   //!< number of arguments
};
#define TRAIT_RETURN_TYPE(f) coreFunctionTraits<f>::uReturnType
#define TRAIT_ARG_TYPE(f,i)  coreFunctionTraits<f>::template uArgType<i>
#define TRAIT_ARITY(f)       coreFunctionTraits<f>::iArity

// safely force compile-time evaluation (without creating symbols)
template <typename T, T tExpression> struct INTERFACE coreForceCompileTime final
{
    enum : T {tResult = tExpression};
};
#define FORCE_COMPILE_TIME(x) (coreForceCompileTime<decltype(x), x>::tResult)

// directly call constructor and destructor on pointer
#define CALL_CONSTRUCTOR(p,...) {using __t = std::remove_reference<decltype(*(p))>::type; new(p) __t(__VA_ARGS__);}
#define CALL_DESTRUCTOR(p)      {using __t = std::remove_reference<decltype(*(p))>::type; (p)->~__t();}

// default color values
#define COLOR_WHITE  (coreVector3(1.000f, 1.000f, 1.000f))
#define COLOR_YELLOW (coreVector3(1.000f, 0.824f, 0.392f))
#define COLOR_ORANGE (coreVector3(1.000f, 0.443f, 0.227f))
#define COLOR_RED    (coreVector3(1.000f, 0.275f, 0.275f))
#define COLOR_PURPLE (coreVector3(0.710f, 0.333f, 1.000f))
#define COLOR_BLUE   (coreVector3(0.102f, 0.702f, 1.000f))
#define COLOR_GREEN  (coreVector3(0.118f, 0.745f, 0.353f))
#define COLOR_BRONZE (coreVector3(0.925f, 0.663f, 0.259f))
#define COLOR_SILVER (coreVector3(0.855f, 0.855f, 0.878f))
#define COLOR_GOLD   (coreVector3(1.000f, 0.859f, 0.000f))

// default alignment values
#define ALIGNMENT_SSE   (16u)
#define ALIGNMENT_CACHE (64u)
#define ALIGNMENT_PAGE  (4096u)
#define ALIGNMENT_ALLOC (65536u)

enum coreStatus : coreInt8
{
    CORE_OK            =  0,    //!< everything is fine
    CORE_BUSY          =  1,    //!< currently waiting for an event

    CORE_ERROR_FILE    = -1,    //!< error on reading, writing or finding a file or folder
    CORE_ERROR_SUPPORT = -2,    //!< requested feature is not supported on the target system
    CORE_ERROR_SYSTEM  = -3,    //!< invalid system or application behavior

    CORE_INVALID_CALL  = -11,   //!< object has invalid status
    CORE_INVALID_INPUT = -12,   //!< function parameters are invalid
    CORE_INVALID_DATA  = -13    //!< depending objects contain invalid data
};


// ****************************************************************
/* multi-threading utilities */
#if defined(_CORE_ASYNC_)

    // acquire the spinlock
    FORCE_INLINE void coreAtomicLock(SDL_SpinLock* OUTPUT piLock)
    {
    #if defined(_CORE_WINDOWS_)
        while(_InterlockedExchange(r_cast<long*>(piLock), 1))
    #else
        while(!SDL_AtomicTryLock(piLock))
    #endif
    #if defined(_CORE_SSE_)
        _mm_pause();     // processor level spinning
    #else
        SDL_Delay(0u);   // OS level spinning
    #endif
    }

    // release the spinlock
    FORCE_INLINE void coreAtomicUnlock(SDL_SpinLock* OUTPUT piLock)
    {
    #if defined(_CORE_WINDOWS_)
        _ReadWriteBarrier();
        (*piLock) = 0;
    #else
        SDL_AtomicUnlock(piLock);
    #endif
    }

    // prevent default spinlock functions
    #define SDL_AtomicLock(x)   (error)
    #define SDL_AtomicUnlock(x) (error)

#else

    // disable multi-threading utilities
    #define thread_local
    #define coreAtomicLock(x)
    #define coreAtomicUnlock(x)

#endif


// ****************************************************************
/* forward declarations */
union  coreVector2;
union  coreVector3;
union  coreVector4;
union  coreMatrix2;
union  coreMatrix3;
union  coreMatrix4;
class  coreFile;
class  coreArchive;
class  coreObject2D;
class  coreObject3D;
class  coreParticleEffect;
struct coreTextureSpec;

class  coreLog;
class  coreConfig;
class  coreLanguage;
class  coreRand;
class  CoreSystem;
class  CoreGraphics;
class  CoreAudio;
class  CoreInput;
class  CoreDebug;
class  coreMemoryManager;
class  coreResourceManager;
class  coreObjectManager;


// ****************************************************************
/* application framework */
class CoreApp final
{
public:
    /*! project settings */
    struct Settings final
    {
        static const coreChar* const Name;         //!< project name
        static const coreChar* const IconPath;     //!< window icon file path
        static const coreChar* const CursorPath;   //!< mouse cursor file path

        struct RenderBuffer final
        {
            static const coreUint8 DepthSize;      //!< depth buffer size (0, 16, 24, 32)
            static const coreUint8 StencilSize;    //!< stencil buffer size (0, 8)
            static const coreBool  AlphaChannel;   //!< enable alpha channel (RGBA8)
            static const coreBool  DoubleBuffer;   //!< enable double buffering
            static const coreBool  StereoRender;   //!< enable stereo rendering
        }
        RenderBuffer;
    }
    Settings;


private:
    CoreApp()noexcept {this->Setup(); this->Init();}
    ~CoreApp()        {this->Exit();}

    FRIEND_CLASS(Core)
    DISABLE_COPY(CoreApp)

    /*! auto-generated setup function */
    //! @{
    void Setup();
    //! @}

    /*! user-defined init and exit function */
    //! @{
    void Init();
    void Exit();
    //! @}

    /*! user-defined render and move function */
    //! @{
    void Render();
    void Move();
    //! @}
};


// ****************************************************************
/* engine framework */
class Core final
{
public:
    static coreLog*      const Log;        //!< log file
    static coreConfig*   const Config;     //!< configuration file
    static coreLanguage* const Language;   //!< language file
    static coreRand*     const Rand;       //!< random number generator

    static CoreSystem*   const System;     //!< main system component
    static CoreGraphics* const Graphics;   //!< main graphics component
    static CoreAudio*    const Audio;      //!< main audio component
    static CoreInput*    const Input;      //!< main input component
    static CoreDebug*    const Debug;      //!< main debug component

    struct INTERFACE Manager final
    {
        static coreMemoryManager*   const Memory;     //!< memory manager
        static coreResourceManager* const Resource;   //!< resource manager
        static coreObjectManager*   const Object;     //!< object manager
    };

    static CoreApp* const Application;     //!< application object


private:
    Core()noexcept;
    ~Core();


public:
    DISABLE_COPY(Core)

    /*! reset engine */
    //! @{
    static void Reset();
    //! @}

    /*! reshape engine */
    //! @{
    static void Reshape();
    //! @}


private:
    /*! run engine */
    //! @{
    friend ENTRY_POINT coreInt32 main(coreInt32 argc, coreChar** argv);
    static coreStatus Run();
    //! @}
};


// ****************************************************************
/* engine header files */
#if defined(_CORE_GLES_)
    #include "additional/coreGLES.h"
#else
    #include "additional/coreGL.h"
#endif
#include "additional/coreCPUID.h"
#include "utilities/math/coreMath.h"
#include "utilities/data/hash/CRC32.h"
#include "utilities/data/hash/FNV1.h"
#include "utilities/data/hash/Murmur2.h"
#include "utilities/data/coreData.h"
#include "utilities/data/coreProtect.h"
#include "utilities/data/coreHashString.h"
#include "utilities/data/coreVariant.h"
#include "utilities/data/coreArray.h"
#include "utilities/data/coreSet.h"
#include "utilities/data/coreLookup.h"
#include "utilities/data/coreRand.h"
#include "utilities/file/coreLog.h"
#include "utilities/file/coreConfig.h"
#include "utilities/file/coreLanguage.h"
#include "utilities/file/coreArchive.h"
#include "utilities/math/coreVector.h"
#include "utilities/math/coreMatrix.h"
#include "utilities/math/coreSpline.h"
#include "components/system/CoreSystem.h"
#include "components/system/coreTimer.h"
#include "components/system/coreThread.h"
#include "manager/coreMemory.h"
#include "manager/coreResource.h"
#include "components/graphics/coreSync.h"
#include "components/graphics/coreDataBuffer.h"
#include "components/graphics/CoreGraphics.h"
#include "components/graphics/coreModel.h"
#include "components/graphics/coreTexture.h"
#include "components/graphics/coreShader.h"
#include "components/graphics/coreFont.h"
#include "components/graphics/coreFrameBuffer.h"
#include "components/audio/CoreAudio.h"
#include "components/audio/coreSound.h"
#include "components/audio/coreMusic.h"
#include "components/input/CoreInput.h"
#include "manager/coreObject.h"
#include "objects/game/coreObject2D.h"
#include "objects/game/coreObject3D.h"
#include "objects/game/coreParticle.h"
#include "objects/menu/coreLabel.h"
#include "objects/menu/coreButton.h"
#include "objects/menu/coreCheckBox.h"
#include "objects/menu/coreTextBox.h"
#include "objects/menu/coreSwitchBox.h"
#include "objects/menu/coreMenu.h"
#include "components/debug/CoreDebug.h"


#endif /* _CORE_GUARD_H_ */

/*
0100010001101111001000000111010001101000011010010110111001100111011100110010000001
1100100110100101100111011010000111010000101100001000000110111101110010001000000110
0100011011110110111000100111011101000010000001100100011011110010000001110100011010
0001100101011011010010000001100001011101000010000001100001011011000110110000101110
*/