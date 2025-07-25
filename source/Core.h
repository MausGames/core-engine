//////////////////////////////////////////////////////////////////////////////////
//*----------------------------------------------------------------------------*//
//| Core Engine v1.0.0 (https://www.maus-games.at)                             |//
//*----------------------------------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                                        |//
//|                                                                            |//
//| This software is provided 'as-is', without any express or implied          |//
//| warranty. In no event will the authors be held liable for any damages      |//
//| arising from the use of this software.                                     |//
//|                                                                            |//
//| Permission is granted to anyone to use this software for any purpose,      |//
//| including commercial applications, and to alter it and redistribute it     |//
//| freely, subject to the following restrictions:                             |//
//|                                                                            |//
//| 1. The origin of this software must not be misrepresented; you must not    |//
//|    claim that you wrote the original software. If you use this software    |//
//|    in a product, an acknowledgment in the product documentation would be   |//
//|    appreciated but is not required.                                        |//
//|                                                                            |//
//| 2. Altered source versions must be plainly marked as such, and must not be |//
//|    misrepresented as being the original software.                          |//
//|                                                                            |//
//| 3. This notice may not be removed or altered from any source distribution. |//
//*----------------------------------------------------------------------------*//
//////////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_H_
#define _CORE_GUARD_H_

// TODO 3: define standard-path (data/) were everything is loaded from
// TODO 3: unique pointers and move semantics for functions taking ownership of a pointer
// TODO 3: video class
// TODO 5: path/url class
// TODO 5: stack-allocator
// TODO 3: fix constexpr msvc bug (or confirm already fixed) (constexpr-results are not saved after compilation, functions often re-evaluated at run-time)
// TODO 3: disallow two instances (or create yes/no message box) of the same application (not for commands/headless)
// TODO 3: improve MSVC debug allocator usage, #define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
// TODO 5: check out NVAPI and ADLX/AGS
// TODO 4: handle implicit conversions, (enable MSVC warnings 4242, 4244, 4267 4365 again)
// TODO 4: unify/clarify const void* and const coreByte*
// TODO 4: "WARN_IF" where applicable, "if" where not (check between user-caused errors, system-caused errors, developer errors)
// TODO 3: automatic Core::Reshape() if not handled, currently it's explicit in every application, CoreApp callback ?
// TODO 3: [CORE2] with OpenGL 4.2 or higher, normalized floating-point data is actually mapped to [-MAX,MAX] instead of [MIN,MAX], and the current conversion doesn't fit any of those, what about ES and WebGL ? is there an extension to check ? (https://www.khronos.org/opengl/wiki/Normalized_Integer)
// TODO 4: noexcept = default, ~T()noexcept
// TODO 5: re-introduce __declspec(noalias), __attribute__((pure)) and __attribute__((const)) if it ever makes sense again, also __attribute__((noescape)), e.g. for dynamic functions


// ****************************************************************
/* target configuration */
#if defined(__APPLE__)
    #include <TargetConditionals.h>
#endif
#if !defined(__has_feature)
    #define __has_feature(...) 0
#endif

// compiler
#if defined(_MSC_VER)
    #define _CORE_MSVC_
#endif
#if defined(__GNUC__)
    #define _CORE_GCC_
#endif
#if defined(__clang__)
    #define _CORE_CLANG_
    #undef  _CORE_MSVC_
    #undef  _CORE_GCC_
#endif

// standard library
#if defined(_CPPLIB_VER) || __has_include(<yvals.h>)
    #define _CORE_STL_
#endif
#if defined(__GLIBCXX__) || __has_include(<bits/c++config.h>)
    #define _CORE_GLIBCXX_
#endif
#if defined(_LIBCPP_VERSION) || __has_include(<__config>)
    #define _CORE_LIBCPP_
#endif

// architecture
#if defined(_M_IX86) || defined(__i386__) || defined(_M_X64) || defined(__x86_64__)
    #define _CORE_X86_
#endif
#if defined(_M_ARM) || defined(__arm__) || defined(_M_ARM64) || defined(__aarch64__)
    #define _CORE_ARM_
#endif
#if defined(__wasm__)
    #define _CORE_WASM_
#endif

// operating system
#if defined(_WIN32)
    #define _CORE_WINDOWS_
#endif
#if defined(__linux__)
    #define _CORE_LINUX_
#endif
#if defined(__APPLE__) && TARGET_OS_MAC
    #define _CORE_MACOS_
#endif
#if defined(__ANDROID__)
    #define _CORE_ANDROID_
    #undef  _CORE_LINUX_
#endif
#if defined(__APPLE__) && TARGET_OS_IPHONE
    #define _CORE_IOS_
    #undef  _CORE_MACOS_
#endif
#if defined(__EMSCRIPTEN__)
    #define _CORE_EMSCRIPTEN_
#endif

// IDE code parsing
#if defined(__INTELLISENSE__) || defined(__JETBRAINS_IDE__) || defined(__clang_analyzer__)
    #define _CORE_IDE_
#endif

// run-time type information
#if defined(_CPPRTTI) || defined(__GXX_RTTI)
    #define _CORE_RTTI_
#endif

// exception handling
#if defined(_CPPUNWIND) || defined(__EXCEPTIONS)
    #define _CORE_EXCEPTIONS_
#endif

// debug mode
#if defined(_DEBUG) || ((defined(_CORE_GCC_) || defined(_CORE_CLANG_)) && !defined(__OPTIMIZE__))
    #define _CORE_DEBUG_
#endif

// test mode
#if defined(__SANITIZE_ADDRESS__) || __has_feature(address_sanitizer)
    #define _CORE_TEST_
#endif

// mobile mode
#if defined(_CORE_ANDROID_) || defined(_CORE_IOS_)
    #define _CORE_MOBILE_
#endif

// ANGLE mode
#if defined(__ANGLE__)
    #define _CORE_ANGLE_
#endif

// OpenGL ES mode
#if defined(_CORE_ANDROID_) || defined(_CORE_IOS_) || defined(_CORE_EMSCRIPTEN_) || defined(_CORE_ANGLE_)
    #define _CORE_GLES_
#endif

// 64-bit instruction set
#if defined(_M_X64) || defined(__x86_64__) || defined(_M_ARM64) || defined(__aarch64__)
    #define _CORE_64BIT_
#endif

// SSE instruction set
#if defined(_CORE_X86_)
    #define _CORE_SSE_
#endif

// NEON instruction set
#if defined(_CORE_ARM_) && defined(_CORE_64BIT_)
    #define _CORE_NEON_
#endif

// target configuration checks
#if !defined(_CORE_MSVC_) && !defined(_CORE_GCC_) && !defined(_CORE_CLANG_)
    #error Compiler not supported!
#endif
#if !defined(_CORE_STL_) && !defined(_CORE_GLIBCXX_) && !defined(_CORE_LIBCPP_)
    #error Standard Library not supported!
#endif
#if !defined(_CORE_X86_) && !defined(_CORE_ARM_) && !defined(_CORE_WASM_)
    #error Architecture not supported!
#endif
#if !defined(_CORE_WINDOWS_) && !defined(_CORE_LINUX_) && !defined(_CORE_MACOS_) && !defined(_CORE_ANDROID_) && !defined(_CORE_IOS_) && !defined(_CORE_EMSCRIPTEN_) && !defined(_CORE_SWITCH_)
    #error Operating System not supported!
#endif
#if !defined(_CORE_DEBUG_) && (defined(_CORE_RTTI_) || defined(_CORE_EXCEPTIONS_))
    #error Run-Time Type Information or Exception Handling still enabled!
#endif

// additional configuration checks
#if defined(__SSSE3__) || defined(__SSE4_1__) || defined(__SSE4_2__) || defined(__AVX__) || defined(__AVX2__) || defined(__AVX512F__)
    #warning SSSE3 or higher should not be enabled!
#endif
#if defined(_M_FP_FAST) || defined(__FAST_MATH__)
    #warning Fast math should not be enabled!
#endif


// ****************************************************************
/* compiler definitions */
#if defined(_CORE_MSVC_)
    #define OUTPUT           __restrict             // output pointer or reference parameter without aliasing
    #define INTERFACE        __declspec(novtable)   // pure interface class without direct instantiation
    #define UNUSED           [[maybe_unused]]       // possibly unused variable (warnings 4100, 4101, 4189)
    #define THREAD_LOCAL     __declspec(thread)     // thread-local storage without dynamic construction and destruction
    #define FORCE_INLINE     __forceinline          // always inline the function
    #define DONT_INLINE      __declspec(noinline)   // never inline the function
    #define WITHIN_INLINE                           // inline everything within the function
    #define FALLTHROUGH      [[fallthrough]];       // intentionally fall through to the next switch-label
    #define RETURN_RESTRICT  __declspec(restrict)   // returned object will not be aliased with another pointer
    #define RETURN_NONNULL   _Ret_notnull_          // returned pointer will not be null
    #define RETURN_NODISCARD [[nodiscard]]          // returned value should not be discarded (but can be cast to void)
    #define FUNC_TERMINATE   [[noreturn]]           // function does not return (e.g. by calling exit(3) or abort(3))
#else
    #define OUTPUT           __restrict__
    #define INTERFACE
    #define UNUSED           [[maybe_unused]]
    #define THREAD_LOCAL     __thread
    #define FORCE_INLINE     __attribute__((always_inline)) inline
    #define DONT_INLINE      __attribute__((noinline))
    #define WITHIN_INLINE    __attribute__((flatten))
    #define FALLTHROUGH      [[fallthrough]];
    #define RETURN_RESTRICT  __attribute__((returns_nonnull))
    #define RETURN_NONNULL   __attribute__((returns_nonnull))
    #define RETURN_NODISCARD [[nodiscard]]
    #define FUNC_TERMINATE   [[noreturn]]
#endif

#if defined(_CORE_MSVC_)

    // disable unwanted compiler warnings (with /Wall)
    #pragma warning(disable : 4061)   // enumerator not handled in switch
    #pragma warning(disable : 4100)   // unreferenced formal parameter
    #pragma warning(disable : 4127)   // constant conditional expression
    #pragma warning(disable : 4191)   // unsafe conversion between function types
    #pragma warning(disable : 4201)   // nameless struct or union
    #pragma warning(disable : 4242)   // implicit conversion to different precision #1
    #pragma warning(disable : 4244)   // implicit conversion to different precision #2
    #pragma warning(disable : 4266)   // virtual function not overridden
    #pragma warning(disable : 4267)   // implicit conversion of std::size_t
    #pragma warning(disable : 4296)   // expression is always true
    #pragma warning(disable : 4324)   // padding added due to alignment attribute
    #pragma warning(disable : 4365)   // implicit conversion between signed and unsigned
    #pragma warning(disable : 4514)   // unreferenced inline function removed
    #pragma warning(disable : 4557)   // __assume contains side-effect (# only false-positives)
    #pragma warning(disable : 4571)   // semantic change of catch(...)
    #pragma warning(disable : 4577)   // noexcept used without exception handling (# also disabled with compiler-flag)
    #pragma warning(disable : 4623)   // default constructor implicitly deleted
    #pragma warning(disable : 4625)   // copy constructor implicitly deleted
    #pragma warning(disable : 4626)   // copy assignment operator implicitly deleted
    #pragma warning(disable : 4668)   // preprocessor macro not defined
    #pragma warning(disable : 4706)   // assignment within conditional expression
    #pragma warning(disable : 4710)   // function not inlined
    #pragma warning(disable : 4711)   // function automatically inlined
    #pragma warning(disable : 4738)   // floating-point results stored in memory (# possible compiler bug)
    #pragma warning(disable : 4774)   // format string not a string literal
    #pragma warning(disable : 4800)   // implicit conversion to bool
    #pragma warning(disable : 4820)   // padding after data member
    #pragma warning(disable : 5026)   // move constructor implicitly deleted
    #pragma warning(disable : 5027)   // move assignment operator implicitly deleted
    #pragma warning(disable : 5039)   // potentially throwing function passed to extern C function
    #pragma warning(disable : 5045)   // possible Spectre vulnerability
    #pragma warning(disable : 5262)   // implicit fall-through
    #pragma warning(disable : 5264)   // const variable not used

#else

    // disable unwanted compiler warnings (with -Wall -Wextra -Wpedantic)
    #pragma clang diagnostic ignored "-Wassume"
    #pragma clang diagnostic ignored "-Wdefaulted-function-deleted"
    #pragma clang diagnostic ignored "-Wformat-security"
    #pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
    #pragma clang diagnostic ignored "-Wmisleading-indentation"
    #pragma clang diagnostic ignored "-Wunused-parameter"

#endif

#if !defined(_CORE_DEBUG_) && !defined(_CORE_EMSCRIPTEN_) && !defined(_CORE_SWITCH_)
    #if defined(_CORE_MSVC_)
        #pragma fenv_access (off)   // ignore access to the floating-point environment (on purpose)
        #pragma fp_contract (off)   // disallow contracting of floating-point expressions
    #elif defined(_CORE_CLANG_)
        #pragma STDC FENV_ACCESS OFF
        #pragma STDC FP_CONTRACT OFF
    #endif
#endif

#if defined(_CORE_EMSCRIPTEN_)
    #undef  THREAD_LOCAL
    #define THREAD_LOCAL   // multi-threading is not supported or required
#endif


// ****************************************************************
/* standard libraries */
#define _CRT_SECURE_NO_WARNINGS
#define _LIBCPP_REMOVE_TRANSITIVE_INCLUDES
#if !defined(_CORE_EXCEPTIONS_)
    #define _HAS_EXCEPTIONS 0
#endif
#if defined(_CORE_DEBUG_)
    #define _CRTDBG_MAP_ALLOC
    #define _GLIBCXX_ASSERTIONS
    #define _LIBCPP_HARDENING_MODE _LIBCPP_HARDENING_MODE_FAST
#endif
#if defined(_CORE_TEST_)
    #define _GLIBCXX_DEBUG
    #define _GLIBCXX_SANITIZE_VECTOR 1
    #define _LIBCPP_HARDENING_MODE _LIBCPP_HARDENING_MODE_EXTENSIVE
#endif

#if defined(_CORE_WINDOWS_)
    #include "additional/windows/header.h"
#elif defined(_CORE_EMSCRIPTEN_)
    #include <emscripten.h>
#endif
#if defined(_CORE_SSE_)
    #include <immintrin.h>
#elif defined(_CORE_NEON_)
    #include <arm_neon.h>
#endif
#include <cassert>
#include <cctype>
#include <cfenv>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <charconv>
#include <concepts>
#include <deque>
#include <functional>
#include <ios>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>


// ****************************************************************
/* external libraries */
#define GLEW_NO_GLU
#define GLEW_STATIC
#define AL_DISABLE_NOEXCEPT
#define STB_VORBIS_HEADER_ONLY
#define STB_VORBIS_NO_PUSHDATA_API
#define STB_VORBIS_MAX_CHANNELS 2
#if defined(_CORE_MSVC_)
    #define ZSTD_DLL_IMPORT 1
#endif
#if defined(_CORE_TEST_)
    #define STB_SPRINTF_NOUNALIGNED
#endif

#include <SDL3/SDL.h>
#include <SDL3/SDL_ttf.h>
#include <SDL3/SDL_image.h>
#if defined(_CORE_GLES_)
    #include <GLES3/gl32.h>
#else
    #include <GL/glew.h>
#endif
#include <AL/alext.h>
#include <zstd/zstd.h>
#include <opus/opusfile.h>
#include <stb_vorbis.h>


// ****************************************************************
/* general definitions */
#undef NULL
#undef __STRING
#undef __CONCAT

#define NULL                        nullptr
#define __STRING(a)                 #a
#define STRING(a)                   __STRING(a)
#define __CONCAT(a,b)               a ## b
#define CONCAT(a,b)                 __CONCAT(a, b)
#define __DEFINED(a,b)              !!FORCE_COMPILE_TIME(coreStrCmpConst(#a, b))
#define DEFINED(a)                  __DEFINED(a, #a)

#define SAFE_DELETE(p)              {delete   (p); (p) = NULL;}
#define SAFE_DELETE_ARRAY(p)        {delete[] (p); (p) = NULL;}

#define UINT_LITERAL(x)             ((coreUint32(x[3]) << 24u) | (coreUint32(x[2]) << 16u) | (coreUint32(x[1]) << 8u) | (coreUint32(x[0])))

#define BIT(n)                      (1ull << (n))
#define BITLINE(n)                  (BIT((n) - 1ull) * 2ull - 1ull)
#define BITSOF(o)                   (sizeof(o) * 8uz)

#define ADD_BIT(o,n)                {__CHECK_BIT (o, n);  (o) |=  BIT(n);}
#define ADD_FLAG(o,n)               {__CHECK_FLAG(o, n);  (o) |=     (n);}
#define REMOVE_BIT(o,n)             {__CHECK_BIT (o, n);  (o) &= ~BIT(n);}
#define REMOVE_FLAG(o,n)            {__CHECK_FLAG(o, n);  (o) &=    ~(n);}
#define TOGGLE_BIT(o,n)             {__CHECK_BIT (o, n);  (o) ^=  BIT(n);}
#define TOGGLE_FLAG(o,n)            {__CHECK_FLAG(o, n);  (o) ^=     (n);}
#define SET_BIT(o,n,t)              {__CHECK_BIT (o, n);  (o) ^=  BIT(n) & ((o) ^ ((t) ? ~0ull : 0ull));}
#define SET_FLAG(o,n,t)             {__CHECK_FLAG(o, n);  (o) ^=     (n) & ((o) ^ ((t) ? ~0ull : 0ull));}
#define HAS_BIT(o,n)                (__CHECK_BIT (o, n), ((o) &   BIT(n)) != 0ull)
#define HAS_FLAG(o,n)               (__CHECK_FLAG(o, n), ((o) &      (n)) == (n))

#define ADD_BIT_EX(o,n)             {__CHECK_BIT_EX(o, n); ADD_BIT   ((o)[(n) / BITSOF((o)[0])], (n) % BITSOF((o)[0]))}
#define REMOVE_BIT_EX(o,n)          {__CHECK_BIT_EX(o, n); REMOVE_BIT((o)[(n) / BITSOF((o)[0])], (n) % BITSOF((o)[0]))}
#define TOGGLE_BIT_EX(o,n)          {__CHECK_BIT_EX(o, n); TOGGLE_BIT((o)[(n) / BITSOF((o)[0])], (n) % BITSOF((o)[0]))}
#define SET_BIT_EX(o,n,t)           {__CHECK_BIT_EX(o, n); SET_BIT   ((o)[(n) / BITSOF((o)[0])], (n) % BITSOF((o)[0]), (t))}
#define HAS_BIT_EX(o,n)             (__CHECK_BIT_EX(o, n), HAS_BIT   ((o)[(n) / BITSOF((o)[0])], (n) % BITSOF((o)[0])))

#define BITVALUE(n,s,v)             (__CHECK_BITVALUE(n, v),              ((v) & BITLINE(n)) << (s))
#define SET_BITVALUE(o,n,s,v)       {__CHECK_FLAG(o, BITLINE((n) + (s)));  (o) = BITVALUE(n, s, v) | ((o) & ~(BITLINE(n) << (s)));}
#define GET_BITVALUE(o,n,s)         (__CHECK_FLAG(o, BITLINE((n) + (s))), ((o) >> (s)) & BITLINE(n))

#define FOR_EACH(i,c)               for(auto i = (c).begin(),  i ## __e = (c).end();  i != i ## __e; __CHECK_ITERATOR    (i ## __e, c), ++i)
#define FOR_EACH_REV(i,c)           for(auto i = (c).rbegin(), i ## __e = (c).rend(); i != i ## __e; __CHECK_ITERATOR_REV(i ## __e, c), ++i)
#define FOR_EACH_SET(i,s,c)         for(auto i = (s),          i ## __e = (c).end();  i != i ## __e; __CHECK_ITERATOR    (i ## __e, c), ++i)
#define FOR_EACH_DYN(i,c)           for(auto i = (c).begin(),  i ## __e = (c).end();  i != i ## __e; )
#define DYN_KEEP(i,c)               {__CHECK_ITERATOR(i ## __e, c); ++i;}
#define DYN_REMOVE(i,c)             {__CHECK_ITERATOR(i ## __e, c); i = (c).erase(i); i ## __e = (c).end();}

#define __CHECK_BIT(o,n)            ([&]() {ASSERT(coreUint64(n) <          BITSOF((o)))}())
#define __CHECK_BIT_EX(o,n)         ([&]() {ASSERT(coreUint64(n) <          BITSOF((o)[0]) * ARRAY_SIZE(o))}())
#define __CHECK_FLAG(o,n)           ([&]() {ASSERT(coreUint64(n) <= BITLINE(BITSOF((o))) && ((n) != 0u))}())
#define __CHECK_BITVALUE(n,v)       ([&]() {ASSERT(coreUint64(v) <= BITLINE(n)           && ((n) != 0u))}())
#define __CHECK_ITERATOR(e,c)       ([&]() {ASSERT((e) == (c).end ())}())
#define __CHECK_ITERATOR_REV(e,c)   ([&]() {ASSERT((e) == (c).rend())}())

#define BIG_STATIC                  static THREAD_LOCAL
#define FRIEND_CLASS(c)             friend class c;
#define STATIC_ASSERT(c)            static_assert(c, "STATIC ASSERT [" #c "]");

#if defined(_CORE_DEBUG_) && !defined(_CORE_IDE_)
    #define ASSERT(c)               {if(false) assert(c); SDL_assert_core(c);}   // strong
#else
    #if defined(_CORE_MSVC_)
        #define ASSERT(c)           {__assume(!!(c));}
    #elif defined(_CORE_GCC_)
        #define ASSERT(c)           {[[assume(!!(c))]];}
    #elif defined(_CORE_CLANG_)
        #define ASSERT(c)           {__builtin_assume(!!(c));}
    #endif
#endif

#if defined(_CORE_DEBUG_) && !defined(_CORE_IDE_)
    #define WARN_IF(c)              if([](const coreBool bCondition) {ASSERT(!bCondition) return bCondition;}(!!(c)))   // weak
#else
    #if defined(_CORE_MSVC_)
        #define WARN_IF(c)          if(!!(c)) [[unlikely]]
    #else
        #define WARN_IF(c)          if(__builtin_expect(!!(c), 0))
    #endif
#endif

#if defined(_CORE_DEBUG_)
    #define UNREACHABLE             {ASSERT(false)}
#else
    #if defined(_CORE_MSVC_)
        #define UNREACHABLE         {__assume(false);}
    #else
        #define UNREACHABLE         {__builtin_unreachable();}
    #endif
#endif

#if defined(_CORE_DEBUG_)
    #define ASSUME_ALIGNED(p,a)     ([](auto* pPointer, const coreUintW iAlign) {ASSERT(coreMath::IsAligned(P_TO_UI(pPointer), iAlign)) return pPointer;}(p, a))
#else
    #if defined(_CORE_MSVC_)
        #define ASSUME_ALIGNED(p,a) (std::assume_aligned<a>(p))
    #else
        #define ASSUME_ALIGNED(p,a) (s_cast<decltype(p)>(__builtin_assume_aligned(p, a)))
    #endif
#endif

#if defined(_CORE_DEBUG_)
    #define LOOP_NONZERO(c)         ([](const auto iNum) {ASSERT(iNum > 0) return iNum;}(c))
#else
    #if defined(_CORE_MSVC_)
        #define LOOP_NONZERO(c)     (__assume((c) > 0), (c))
    #elif defined(_CORE_GCC_)
        #define LOOP_NONZERO(c)     ([[assume((c) > 0)]], (c))
    #elif defined(_CORE_CLANG_)
        #define LOOP_NONZERO(c)     (__builtin_assume((c) > 0), (c))
    #endif
#endif

// disable constructor and destructor of the defined class
#define DISABLE_CONSTRUCTION(c) \
     c() = delete;              \
    ~c() = delete;

// enable (explicitly) or disable copy- and move-operations with the defined class
#define ENABLE_COPY(c)                          \
    c             (const c&)noexcept = default; \
    c& operator = (const c&)noexcept = default; \
    c             (c&&)     noexcept = default; \
    c& operator = (c&&)     noexcept = default;
#define DISABLE_COPY(c)                         \
    c             (const c&) = delete;          \
    c& operator = (const c&) = delete;          \
    c             (c&&)      = delete;          \
    c& operator = (c&&)      = delete;

// enable per-member comparison-operations with the defined class
#define ENABLE_COMPARISON(c) \
    RETURN_NODISCARD constexpr auto operator <=> (const c&)const = default;

// disable heap-operations with the defined class
#define DISABLE_HEAP                                \
    void* operator new      (std::size_t) = delete; \
    void* operator new[]    (std::size_t) = delete; \
    void  operator delete   (void*)       = delete; \
    void  operator delete[] (void*)       = delete;

// enable bitwise-operations with the defined enumeration
#define ENABLE_BITWISE(e)                                                                                                                                                \
    RETURN_NODISCARD constexpr e  operator ~  (const e   a)            {return s_cast<e>(~s_cast<std::underlying_type_t<e>>(a));}                                        \
    RETURN_NODISCARD constexpr e  operator &  (const e   a, const e b) {return s_cast<e>( s_cast<std::underlying_type_t<e>>(a) & s_cast<std::underlying_type_t<e>>(b));} \
    RETURN_NODISCARD constexpr e  operator |  (const e   a, const e b) {return s_cast<e>( s_cast<std::underlying_type_t<e>>(a) | s_cast<std::underlying_type_t<e>>(b));} \
    RETURN_NODISCARD constexpr e  operator ^  (const e   a, const e b) {return s_cast<e>( s_cast<std::underlying_type_t<e>>(a) ^ s_cast<std::underlying_type_t<e>>(b));} \
    inline                     e& operator &= (e& OUTPUT a, const e b) {return (a = a & b);}                                                                             \
    inline                     e& operator |= (e& OUTPUT a, const e b) {return (a = a | b);}                                                                             \
    inline                     e& operator ^= (e& OUTPUT a, const e b) {return (a = a ^ b);}

// safely convert pointers and references along the inheritance hierarchy
#if defined(_CORE_RTTI_)
    template <typename T, typename S> RETURN_NODISCARD inline T coreDynamicCast(S a) {T b = dynamic_cast<T>(a); ASSERT(!a || b) return b;}
#else
    #define coreDynamicCast static_cast
#endif

// shorter common keywords
#define s_cast  static_cast
#define d_cast  coreDynamicCast
#define r_cast  reinterpret_cast
#define c_cast  const_cast
#define t_ident std::type_identity_t

// type conversion macros
#define F_TO_SI(x) ((coreInt32)            (x))   // float to signed int
#define F_TO_UI(x) ((coreUint32)(coreInt32)(x))   // float to unsigned int (force [_mm_cvtt_ss2si])
#define I_TO_F(x)  ((coreFloat)(coreInt32) (x))   // int to float          (force [_mm_cvtepi32_ps])
#define P_TO_SI(x) ((std::intptr_t)(void*) (x))   // pointer to signed int
#define P_TO_UI(x) ((std::uintptr_t)(void*)(x))   // pointer to unsigned int
#define I_TO_P(x)  ((void*)(std::intptr_t) (x))   // int to pointer

// type definitions
using coreInt8   = std::int8_t;
using coreInt16  = std::int16_t;
using coreInt32  = std::int32_t;
using coreInt64  = long long;
using coreIntW   = std::make_signed_t<std::size_t>;
using coreLong   = long;
using coreUint8  = std::uint8_t;
using coreUint16 = std::uint16_t;
using coreUint32 = std::uint32_t;
using coreUint64 = unsigned long long;
using coreUintW  = std::size_t;
using coreUlong  = unsigned long;
using coreBool   = bool;
using coreChar   = char;
using coreChar8  = char8_t;
using coreChar16 = char16_t;
using coreChar32 = char32_t;
using coreWchar  = wchar_t;
using coreFloat  = float;
using coreDouble = double;
using coreByte   = std::uint8_t;

STATIC_ASSERT(sizeof(coreInt8)   == 1u)
STATIC_ASSERT(sizeof(coreInt16)  == 2u)
STATIC_ASSERT(sizeof(coreInt32)  == 4u)
STATIC_ASSERT(sizeof(coreInt64)  == 8u)
STATIC_ASSERT(sizeof(coreUint8)  == 1u)
STATIC_ASSERT(sizeof(coreUint16) == 2u)
STATIC_ASSERT(sizeof(coreUint32) == 4u)
STATIC_ASSERT(sizeof(coreUint64) == 8u)
STATIC_ASSERT(sizeof(coreByte)   == 1u)

// retrieve compile-time string properties
RETURN_NODISCARD constexpr coreUintW coreStrLenConst(const coreChar* s)                    {ASSERT(s)      if(std::is_constant_evaluated()) {coreUintW i = 0u; while(s[i]) ++i; return i;}                return std::strlen(s);}
RETURN_NODISCARD constexpr coreInt32 coreStrCmpConst(const coreChar* s, const coreChar* t) {ASSERT(s && t) if(std::is_constant_evaluated()) {while((*s) == (*t) && (*s)) {++s; ++t;} return (*s) - (*t);} return std::strcmp(s, t);}

// retrieve compile-time pointer-safe array size
template <typename T, coreUintW iSize> coreByte (&__ARRAY_SIZE(T (&)[iSize]))[iSize];
#define ARRAY_SIZE(a) (sizeof(__ARRAY_SIZE(a)))

// retrieve compile-time function and lambda properties
template <typename T>                            struct INTERFACE coreFunctionTraits final                        : public coreFunctionTraits<decltype(&std::decay_t<T>::operator())> {};
template <typename R, typename C, typename... A> struct INTERFACE coreFunctionTraits<R(C::*)(A...)const noexcept> : public coreFunctionTraits<R(A...)>                                {using coreClassType = C;};
template <typename R, typename C, typename... A> struct INTERFACE coreFunctionTraits<R(C::*)(A...)const>          : public coreFunctionTraits<R(A...)>                                {using coreClassType = C;};
template <typename R, typename C, typename... A> struct INTERFACE coreFunctionTraits<R(C::*)(A...)noexcept>       : public coreFunctionTraits<R(A...)>                                {using coreClassType = C;};
template <typename R, typename C, typename... A> struct INTERFACE coreFunctionTraits<R(C::*)(A...)>               : public coreFunctionTraits<R(A...)>                                {using coreClassType = C;};
template <typename R,             typename... A> struct INTERFACE coreFunctionTraits<R   (*)(A...)noexcept>       : public coreFunctionTraits<R(A...)>                                {};
template <typename R,             typename... A> struct INTERFACE coreFunctionTraits<R   (*)(A...)>               : public coreFunctionTraits<R(A...)>                                {};
template <typename R,             typename... A> struct INTERFACE coreFunctionTraits<R      (A...)noexcept>       : public coreFunctionTraits<R(A...)>                                {};
template <typename R,             typename... A> struct INTERFACE coreFunctionTraits<R      (A...)>
{
    using coreFuncType   = R(*)(A...);                                                                // function type (without class)
    using coreClassType  = void;                                                                      // class type
    using coreReturnType = R;                                                                         // return type
    template <coreUintW iIndex> using coreArgType = std::tuple_element_t<iIndex, std::tuple<A...>>;   // argument types
    enum : coreUintW {iArity = sizeof...(A)};                                                         // number of arguments
};
#define TRAIT_FUNC_TYPE(f)   coreFunctionTraits<f>::coreFuncType
#define TRAIT_CLASS_TYPE(f)  coreFunctionTraits<f>::coreClassType
#define TRAIT_RETURN_TYPE(f) coreFunctionTraits<f>::coreReturnType
#define TRAIT_ARG_TYPE(f,i)  coreFunctionTraits<f>::template coreArgType<i>
#define TRAIT_ARITY(f)       coreFunctionTraits<f>::iArity

// retrieve compile-time type properties
#define IS_TRIVIAL(t) (std::is_trivially_copyable_v<t> && std::is_trivially_default_constructible_v<t> && std::is_trivially_destructible_v<t>)

// safely force compile-time evaluation (without creating symbols)
template <typename T, T tExpression> struct INTERFACE coreForceCompileTime final
{
    enum : T {tResult = tExpression};
};
#if defined(_CORE_IDE_)
    #define FORCE_COMPILE_TIME(x) (x)
#else
    #define FORCE_COMPILE_TIME(x) (coreForceCompileTime<decltype(x), x>::tResult)
#endif

// directly call constructor and destructor on pointer
#define CALL_CONSTRUCTOR(p,...) {using __t = std::decay_t<decltype(*(p))>; new(p) __t(__VA_ARGS__);}
#define CALL_DESTRUCTOR(p)      {using __t = std::decay_t<decltype(*(p))>; (p)->~__t();}

// default color values
#define COLOR_WHITE   (coreVector3(1.000f, 1.000f, 1.000f))
#define COLOR_YELLOW  (coreVector3(1.000f, 0.824f, 0.292f))
#define COLOR_ORANGE  (coreVector3(1.000f, 0.543f, 0.177f))
#define COLOR_RED     (coreVector3(0.950f, 0.225f, 0.225f))
#define COLOR_MAGENTA (coreVector3(1.000f, 0.310f, 0.720f))
#define COLOR_PURPLE  (coreVector3(0.760f, 0.333f, 1.000f))
#define COLOR_BLUE    (coreVector3(0.102f, 0.602f, 1.000f))
#define COLOR_CYAN    (coreVector3(0.000f, 0.776f, 0.806f))
#define COLOR_GREEN   (coreVector3(0.253f, 0.745f, 0.253f))
#define COLOR_BRONZE  (coreVector3(0.925f, 0.663f, 0.259f))
#define COLOR_SILVER  (coreVector3(0.855f, 0.855f, 0.878f))
#define COLOR_GOLD    (coreVector3(1.000f, 0.859f, 0.000f))

// default alignment values
#define ALIGNMENT_NEW   (alignof(std::max_align_t))
#define ALIGNMENT_SIMD  (16u)
#define ALIGNMENT_CACHE (64u)
#define ALIGNMENT_PAGE  (4096u)
#define ALIGNMENT_ALLOC (65536u)

enum coreStatus : coreInt8
{
    CORE_OK            =  0,    // everything is fine
    CORE_BUSY          =  1,    // currently waiting for an event

    CORE_ERROR_FILE    = -1,    // error on reading, writing or finding a file or directory
    CORE_ERROR_SUPPORT = -2,    // requested feature is not supported on the target system
    CORE_ERROR_SYSTEM  = -3,    // invalid system or application behavior

    CORE_INVALID_CALL  = -11,   // object has invalid status
    CORE_INVALID_INPUT = -12,   // function parameters are invalid
    CORE_INVALID_DATA  = -13    // depending objects contain invalid data
};


// ****************************************************************
/* forward declarations */
class  coreVector2;
class  coreVector3;
class  coreVector4;
class  coreMatrix2;
class  coreMatrix3;
class  coreMatrix4;
class  coreString;
class  coreFile;
class  coreArchive;
class  coreObject2D;
class  coreObject3D;
class  coreParticleEffect;
struct coreTextureSpec;
template <typename T> class coreResourcePtr;

class  coreLog;
class  coreConfig;
class  coreLanguage;
class  coreRand;
class  CoreSystem;
class  CoreGraphics;
class  CoreAudio;
class  CoreInput;
class  CorePlatform;
class  CoreDebug;
class  coreMemoryManager;
class  coreResourceManager;
class  coreObjectManager;

extern "C" coreInt32 SDLCALL coreMain(coreInt32 argc, coreChar** argv);
extern "C" coreInt32 SDLCALL coreThreadMain(void* pData);


// ****************************************************************
/* application framework */
class CoreApp final
{
public:
    /* project settings */
    struct INTERFACE Settings final
    {
        static const coreChar* const Name;             // project name
        static const coreChar* const Version;          // version string
        static const coreChar* const Identifier;       // unique application identifier (e.g. at.maus-games.core-app)
        static const coreChar* const Creator;          // author/company name
        static const coreChar* const Year;             // copyright year
        static const coreChar* const Website;          // website URL
        static const coreChar* const IconPath;         // window icon file path
        static const coreChar* const CursorPath;       // mouse cursor file path
        static const coreBool        UserManagement;   // enable user-management (OS and platform)

        struct INTERFACE System final
        {
            static const coreDouble AspectRatio;   // canonical aspect ratio
        };

        struct INTERFACE Graphics final
        {
            static const coreUint8 DepthSize;      // depth buffer size (0, 16, 24, 32)
            static const coreUint8 StencilSize;    // stencil buffer size (0, 8)
            static const coreBool  AlphaChannel;   // enable alpha channel (RGBA8)
            static const coreBool  DoubleBuffer;   // enable double buffering
            static const coreBool  StereoRender;   // enable stereo rendering
        };

        struct INTERFACE Platform final
        {
            static const coreUint32      SteamAppID[2];         // Steam application ID  (0 = ignore) (full, demo)
            static const coreChar* const EpicProductID;         // Epic product ID       (NULL = ignore)
            static const coreChar* const EpicSandboxID   [3];   // Epic sandbox ID       (for store-related information) (dev, stage, live)
            static const coreChar* const EpicDeploymentID[3];   // Epic deployment ID    (for user-related information)
            static const coreChar* const EpicClientID;          // Epic client ID
            static const coreChar* const EpicClientSecret;      // Epic client secret
            static const coreUint64      DiscordClientID;       // Discord client ID
        };

        /* dynamic project settings */
        static coreBool IsDemo();
    };


private:
    CoreApp()noexcept {this->Setup(); this->Init();}
    ~CoreApp()        {this->Exit();}

    FRIEND_CLASS(Core)
    DISABLE_COPY(CoreApp)

    /* auto-generated setup function */
    void Setup();

    /* user-defined init and exit function */
    void Init();
    void Exit();

    /* user-defined render and move function */
    void Render();
    void Move();
};


// ****************************************************************
/* engine framework */
class Core final
{
public:
    static coreLog*      const Log;        // log file
    static coreConfig*   const Config;     // configuration file
    static coreLanguage* const Language;   // language file
    static coreRand*     const Rand;       // random number generator

    static CoreSystem*   const System;     // main system component
    static CoreGraphics* const Graphics;   // main graphics component
    static CoreAudio*    const Audio;      // main audio component
    static CoreInput*    const Input;      // main input component
    static CorePlatform* const Platform;   // main platform component
    static CoreDebug*    const Debug;      // main debug component

    struct INTERFACE Manager final
    {
        static coreMemoryManager*   const Memory;     // memory manager
        static coreResourceManager* const Resource;   // resource manager
        static coreObjectManager*   const Object;     // object manager
    };

    static CoreApp* const Application;     // application object


private:
    Core()noexcept;
    ~Core();


public:
    DISABLE_COPY(Core)

    /* reset engine */
    static void Reset();

    /* reshape engine */
    static void Reshape();

    /* restart application */
    static void Restart();


private:
    /* main function */
    friend coreInt32 SDLCALL coreMain(coreInt32 argc, coreChar** argv);

    /* run engine or command */
    static coreStatus RunEngine();
    static coreStatus RunCommand();

    /* perform deferred application restart */
    static void __PerformRestart();
};


// ****************************************************************
/* engine header files */
#include "utilities/math/corePoint.h"
#if defined(_CORE_GLES_)
    #include "additional/coreGLES.h"
#else
    #include "additional/coreGL.h"
#endif
#include "additional/coreCPUID.h"
#include "utilities/math/coreMath.h"
#include "utilities/data/hash/CRC32.h"
#include "utilities/data/hash/FNV1.h"
#include "utilities/data/hash/Murmur.h"
#include "utilities/data/hash/XXH.h"
#include "utilities/data/coreString.h"
#include "utilities/data/coreHashString.h"
#include "utilities/data/coreSpinLock.h"
#include "utilities/data/coreList.h"
#include "utilities/data/coreSet.h"
#include "utilities/data/coreMap.h"
#include "utilities/data/coreRing.h"
#include "utilities/data/coreRand.h"
#include "utilities/data/coreData.h"
#include "utilities/data/coreProtect.h"
#include "utilities/data/coreVariant.h"
#include "utilities/file/coreLog.h"
#include "utilities/file/coreConfig.h"
#include "utilities/file/coreLanguage.h"
#include "utilities/file/coreArchive.h"
#include "utilities/math/coreVector.h"
#include "utilities/math/coreMatrix.h"
#include "utilities/math/coreSpline.h"
#include "utilities/data/coreScope.h"
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
#include "components/graphics/coreAnimation.h"
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
#include "objects/menu/coreViewBox.h"
#include "objects/menu/coreRichText.h"
#include "objects/menu/coreMenu.h"
#include "components/platform/CorePlatform.h"
#include "components/debug/CoreDebug.h"


#endif /* _CORE_GUARD_H_ */

/*
0100010001101111001000000111010001101000011010010110111001100111011100110010000001
1100100110100101100111011010000111010000101100001000000110111101110010001000000110
0100011011110110111000100111011101000010000001100100011011110010000001110100011010
0001100101011011010010000001100001011101000010000001100001011011000110110000101110
*/