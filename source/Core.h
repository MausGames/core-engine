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
//| Core Engine v0.0.8a (http://www.maus-games.at)                               |//
//*------------------------------------------------------------------------------*//
//| Copyright (c) 2013-2016 Martin Mauersics                                     |//
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
#endif
#if ((_CORE_MSVC_) < 1800) && ((_CORE_GCC_) < 40800) && ((_CORE_MINGW_) < 40800) && ((_CORE_CLANG_) < 30300)
    #warning "Compiler not supported!"
#endif

// operating system
#if defined(_WIN32)
    #define _CORE_WINDOWS_ (1)
#endif
#if defined(__linux__)
    #define _CORE_LINUX_   (1)
#endif
#if defined(__APPLE__)
    #define _CORE_OSX_     (1)
#endif
#if defined(__ANDROID__)
    #define _CORE_ANDROID_ (1)
    #undef  _CORE_LINUX_
#endif
#if !defined(_CORE_WINDOWS_) && !defined(_CORE_LINUX_) /*&& !defined(_CORE_OSX_)*/ && !defined(_CORE_ANDROID_)
    #warning "Operating System not supported!"
#endif

// debug mode
#if defined(_DEBUG) || defined(DEBUG) || ((defined(_CORE_GCC_) || defined(_CORE_MINGW_)) && !defined(__OPTIMIZE__))
    #define _CORE_DEBUG_    (1)
#endif

// parallel mode
#if !defined(_CORE_ANDROID_)
    #define _CORE_PARALLEL_ (1)
#endif

// x64 instruction set
#if defined(_M_X64) || defined(__x86_64__)
    #define _CORE_X64_      (1)
#endif

// SSE2 instruction set
#if (defined(_M_IX86) || defined(__i386__) || defined(_CORE_X64_)) && !defined(_CORE_ANDROID_)
    #define _CORE_SSE_      (1)
#endif

// OpenGL ES
#if defined(_CORE_ANDROID_)
    #define _CORE_GLES_     (1)
#endif


// ****************************************************************
/* base libraries */
#define _HAS_EXCEPTIONS (0)
#define _CRT_SECURE_NO_WARNINGS
#define _ALLOW_KEYWORD_MACROS
#define _ALLOW_RTCc_IN_STL
#define  WIN32_LEAN_AND_MEAN
#if defined(_CORE_MINGW_)
    #define  WINVER (0x0500)
    #undef __STRICT_ANSI__
#endif

#if defined(_CORE_WINDOWS_)
    #include <windows.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif
#if defined(_CORE_SSE_)
    #include <emmintrin.h>
#endif
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cfloat>
#include <cmath>
#include <ctime>
#include <memory>
#include <functional>
#include <random>
#include <array>
#include <vector>
#include <list>


// ****************************************************************
/* specific libraries */
#define HAVE_LIBC (1)
#define GLEW_MX
#define GLEW_NO_GLU
#define OV_EXCLUDE_STATIC_CALLBACKS
#define ZLIB_CONST
#define ZLIB_DLL

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#if defined(_CORE_GLES_)
    #include <GLES2/gl2.h>
#else
    #include <GL/glew.h>
    #include <GL/gl.h>
#endif
#include <AL/al.h>
#include <AL/alc.h>
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>
#include <zlib/zlib.h>
#include <SI/SimpleIni.h>


// ****************************************************************
/* compiler definitions */
#if defined(_CORE_MSVC_)
    #define UNUSED          __pragma(warning(suppress : 4100 4101 4189))
    #define OUTPUT          __restrict             //!< output parameter without aliasing (never)
    #define INTERFACE       __declspec(novtable)   //!< pure interface class without direct instantiation
    #define FORCE_INLINE    __forceinline          //!< force function to be inlined (when optimizations are enabled)
    #define RETURN_RESTRICT __declspec(restrict)   //!< returned object will not be aliased with another pointer
    #define RETURN_NONNULL                         //!< returned pointer will not be null
    #define FUNC_PURE                              //!< function reads only parameters and non-volatile globals and returns a single value
    #define FUNC_CONST      __declspec(noalias)    //!< function reads only parameters without indirections and returns a single value
    #define FUNC_NOALIAS    __declspec(noalias)    //!< function does not access global state directly and may only use (read and write) first-level indirections
    #define FUNC_NORETURN   __declspec(noreturn)   //!< function terminates (e.g. with exit(3) or abort(3))
#else
    #define UNUSED          __attribute__((unused))
    #define OUTPUT          __restrict__
    #define INTERFACE
    #define FORCE_INLINE    __attribute__((always_inline))
    #define RETURN_RESTRICT __attribute__((malloc))
    #define RETURN_NONNULL  __attribute__((returns_nonnull))
    #define FUNC_PURE       __attribute__((pure))
    #define FUNC_CONST      __attribute__((const))
    #define FUNC_NOALIAS
    #define FUNC_NORETURN   __attribute__((noreturn, cold))
#endif

#if defined(_CORE_MINGW_) && defined(_CORE_SSE_)
    #define ENTRY_POINT     __attribute__((force_align_arg_pointer))   //!< realign run-time stack (and fix SSE)
#else
    #define ENTRY_POINT
#endif

#if defined(_CORE_MSVC_)
    #define noexcept  throw()   //!< exceptions disabled
    #define CONSTEXPR inline
#else
    #define CONSTEXPR constexpr
#endif

#if !defined(_CORE_PARALLEL_)
    #define thread_local
    #define SDL_AtomicLock(x)
    #define SDL_AtomicUnlock(x)
#endif

#if defined(_CORE_MSVC_)

    // disable unwanted compiler warnings (with /W4)
    #pragma warning(disable : 4100)   //!< unreferenced formal parameter
    #pragma warning(disable : 4127)   //!< constant conditional expression
    #pragma warning(disable : 4201)   //!< nameless struct or union
    #pragma warning(disable : 4244)   //!< implicit conversion to smaller integer precision
    #pragma warning(disable : 4267)   //!< implicit conversion of std::size_t

    // enable additional compiler warnings (https://msdn.microsoft.com/library/23k5d385)
    #pragma warning(default : 4191 4264 4265 4287 4289 4296 4302 4311 4355 4388 4548 4555 4557 4738 4826 4837 4928 4946)

    // #pragma warning(default : 4242 4244 4365)   //!< loss of precision
    // #pragma warning(default : 4820)             //!< byte padding

#endif


// ****************************************************************
/* general definitions */
#undef  NULL
#define NULL nullptr

#undef  __STRING
#define __STRING(a)          #a
#define STRING(a)            __STRING(a)
#define __DEFINED(a,b)       (!coreData::StrCmpConst(#a, b))
#define DEFINED(a)           (__DEFINED(a, #a))

#define SAFE_DELETE(p)       {delete   (p); (p) = NULL;}
#define SAFE_DELETE_ARRAY(p) {delete[] (p); (p) = NULL;}

#define BIT(n)               (1u << (n))   // starts with 0
#define BITLINE(n)           (BIT(n) - 1u)
#define ADD_BIT(o,n)         { (o) |=  BIT(n);}
#define ADD_FLAG(o,n)        { (o) |=     (n);}
#define REMOVE_BIT(o,n)      { (o) &= ~BIT(n);}
#define REMOVE_FLAG(o,n)     { (o) &=    ~(n);}
#define TOGGLE_BIT(o,n)      { (o) ^=  BIT(n);}
#define TOGGLE_FLAG(o,n)     { (o) ^=     (n);}
#define SET_BIT(o,n,t)       { (o) ^=  BIT(n) & ((o) ^ -int(!!(t)));}
#define SET_FLAG(o,n,t)      { (o) ^=     (n) & ((o) ^ -int(!!(t)));}
#define CONTAINS_BIT(o,n)    (((o) &   BIT(n)) ? true : false)
#define CONTAINS_FLAG(o,n)   (((o) &      (n)) == (n))

#define FOR_EACH(i,c)        for(auto i = (c).begin(),  i ## __e = (c).end();  i != i ## __e; ++i)
#define FOR_EACH_REV(i,c)    for(auto i = (c).rbegin(), i ## __e = (c).rend(); i != i ## __e; ++i)
#define FOR_EACH_SET(i,s,c)  for(auto i = (s),          i ## __e = (c).end();  i != i ## __e; ++i)
#define FOR_EACH_DYN(i,c)    for(auto i = (c).begin(),  i ## __e = (c).end();  i != i ## __e; )
#define DYN_KEEP(i)          {++i;}
#define DYN_REMOVE(i,c)      {i = (c).erase(i); i ## __e = (c).end();}

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
#define WARN_IF(c)           {SDL_assert(!(c));} if(c)
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
#define ENABLE_BITWISE(e)                                                                                                                                     \
    constexpr e  operator ~  (const e a)            {return s_cast<e>(~s_cast<std::underlying_type<e>::type>(a));}                                            \
    constexpr e  operator |  (const e a, const e b) {return s_cast<e>( s_cast<std::underlying_type<e>::type>(a) | s_cast<std::underlying_type<e>::type>(b));} \
    constexpr e  operator &  (const e a, const e b) {return s_cast<e>( s_cast<std::underlying_type<e>::type>(a) & s_cast<std::underlying_type<e>::type>(b));} \
    constexpr e  operator ^  (const e a, const e b) {return s_cast<e>( s_cast<std::underlying_type<e>::type>(a) ^ s_cast<std::underlying_type<e>::type>(b));} \
    inline    e& operator |= (e&      a, const e b) {return (a = a | b);}                                                                                     \
    inline    e& operator &= (e&      a, const e b) {return (a = a & b);}                                                                                     \
    inline    e& operator ^= (e&      a, const e b) {return (a = a ^ b);}

// shorter common keywords
#define f_list forward_list
#define u_map  unordered_map
#define u_set  unordered_set
#define s_cast static_cast
#define d_cast dynamic_cast
#define r_cast reinterpret_cast
#define c_cast const_cast

// type conversion macros
#define F_TO_SI(x) ((int)                  (x))   //!< float to signed int
#define F_TO_UI(x) ((unsigned)(int)        (x))   //!< float to unsigned int (force [_mm_cvtt_ss2si])
#define I_TO_F(x)  ((float)(int)           (x))   //!< int to float          (force [_mm_cvtepi32_ps])
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

// override string comparison operator (faster but insecure)
inline coreBool operator == (const std::string& a, const coreChar*    b) {return !std::strcmp(a.c_str(), b);}
inline coreBool operator == (const coreChar*    a, const std::string& b) {return !std::strcmp(a,         b.c_str());}
inline coreBool operator == (const std::string& a, const std::string& b) {return !std::strcmp(a.c_str(), b.c_str());}

// override integer swap function (without temporary variable)
#define __SWAP(t) namespace std{inline void swap(t& a, t& b)noexcept {(a ^ b) && (b ^= a ^= b, a ^= b);}}
    __SWAP(coreInt8)  __SWAP(coreInt16)  __SWAP(coreInt32)  __SWAP(coreInt64)
    __SWAP(coreUint8) __SWAP(coreUint16) __SWAP(coreUint32) __SWAP(coreUint64)
    __SWAP(coreBool)  __SWAP(coreChar)
#undef __SWAP

// retrieve compile-time pointer-safe array size
template <typename T, coreUintW iSize> coreChar (&__ARRAY_SIZE(T (&)[iSize]))[iSize];
#define ARRAY_SIZE(a) (sizeof(__ARRAY_SIZE(a)))

// retrieve compile-time function and lambda properties
template <typename T>                            struct INTERFACE function_traits final               : public function_traits<decltype(&T::operator())> {};
template <typename R, typename C, typename... A> struct INTERFACE function_traits<R(C::*)(A...)const> : public function_traits<R(A...)>                  {};
template <typename R, typename C, typename... A> struct INTERFACE function_traits<R(C::*)(A...)>      : public function_traits<R(A...)>                  {};
template <typename R,             typename... A> struct INTERFACE function_traits<R   (*)(A...)>      : public function_traits<R(A...)>                  {};
template <typename R,             typename... A> struct INTERFACE function_traits<R      (A...)>
{
    using return_type = R;                                                                                      //!< return type
    template <coreUintW iIndex> using arg_type = typename std::tuple_element<iIndex, std::tuple<A...>>::type;   //!< argument types
    static const coreUintW arity = sizeof...(A);                                                                //!< number of arguments
};
#define TRAIT_RETURN_TYPE(f) function_traits<f>::return_type
#define TRAIT_ARG_TYPE(f,i)  function_traits<f>::template arg_type<i>
#define TRAIT_ARITY(f)       function_traits<f>::arity

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

enum coreStatus : coreInt8
{
    CORE_OK            =  0,    //!< everything is fine
    CORE_BUSY          =  1,    //!< currently waiting for an event

    CORE_ERROR_FILE    = -1,    //!< error on reading, writing or finding a file or folder
    CORE_ERROR_SUPPORT = -2,    //!< requested feature is not supported on the target system
    CORE_ERROR_SYSTEM  = -3,    //!< invalid system or application behavior

    CORE_INVALID_CALL  = -11,   //!< object has wrong status
    CORE_INVALID_INPUT = -12,   //!< function parameters are invalid
    CORE_INVALID_DATA  = -13    //!< depending objects contain wrong data
};


// ****************************************************************
/* forward declarations */
class  coreVector2;
class  coreVector3;
class  coreVector4;
class  coreMatrix2;
class  coreMatrix3;
class  coreMatrix4;
class  coreFile;
class  coreArchive;
class  coreObject2D;
class  coreObject3D;
class  coreParticleEffect;
struct coreTextureSpec;

class coreLog;
class coreConfig;
class coreLanguage;
class coreRand;
class CoreSystem;
class CoreGraphics;
class CoreAudio;
class CoreInput;
class CoreDebug;
class coreMemoryManager;
class coreResourceManager;
class coreObjectManager;


// ****************************************************************
/* application framework */
class CoreApp final
{
private:
    CoreApp()noexcept {this->Setup(); this->Init();}
    ~CoreApp()        {this->Exit();}

    FRIEND_CLASS(Core)

    /*! auto-generated setup function */
    //! @{
    void Setup();
    //! @}

    /*! undefined init and exit function */
    //! @{
    void Init();
    void Exit();
    //! @}

    /*! undefined render and move function */
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
    static coreLog*      Log;        //!< log file
    static coreConfig*   Config;     //!< configuration file
    static coreLanguage* Language;   //!< language file
    static coreRand*     Rand;       //!< random number generator

    static CoreSystem*   System;     //!< main system component
    static CoreGraphics* Graphics;   //!< main graphics component
    static CoreAudio*    Audio;      //!< main audio component
    static CoreInput*    Input;      //!< main input component
    static CoreDebug*    Debug;      //!< main debug component

    struct INTERFACE Manager final
    {
        static coreMemoryManager*   Memory;     //!< memory manager
        static coreResourceManager* Resource;   //!< resource manager
        static coreObjectManager*   Object;     //!< object manager
    };

    static CoreApp* Application;     //!< application object


private:
    Core()noexcept;
    ~Core();


public:
    DISABLE_COPY(Core)

    /*! reset engine */
    //! @{
    static void Reset();
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
#include "utilities/data/coreData.h"
#include "utilities/data/hash/CRC32.h" // ###
#include "utilities/data/hash/FNV1.h"  // ###
#include "utilities/data/coreProtect.h"
#include "utilities/data/coreHashString.h"
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