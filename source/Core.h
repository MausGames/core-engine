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
//| Core Engine v0.0.6a (http://www.maus-games.at)                               |//
//*------------------------------------------------------------------------------*//
//| Copyright (c) 2013-2014 Martin Mauersics                                     |//
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
// TODO: don't forward/return trivial types as reference ? (address > value)
// TODO: check for template parameters <42>
// TODO: put everything in a namespace ? split up coreData and coreMath
// TODO: remove this whole static pointer stuff, namespace for main-classes together with math and data ?
// TODO: define standard-path (data/) were everything is loaded from
// TODO: implement GL_KHR_robustness (performance penalties ?)
// TODO: explicit flush control for context change (SDL?)
// TODO: check for performance penalties and alternatives for thread_local
// TODO: setup 64-bit Windows build (libraries!)
// TODO: WinXP requires MSVC redist 12 (MinGW) or XP compiler


// compiler
#if defined(_MSC_VER)
    #define _CORE_MSVC_  (_MSC_VER)
#endif
#if defined(__GNUC__)
    #define _CORE_GCC_   (__GNUC__*10000 + __GNUC_MINOR__*100 + __GNUC_PATCHLEVEL__*1)
#endif
#if defined(__MINGW32__)
    #include <_mingw.h>
    #define _CORE_MINGW_ (__MINGW_MAJOR_VERSION*10000 + __MINGW_MINOR_VERSION*100 + __MINGW_PATCHLEVEL*1)
#endif
#if defined(__clang__)
    #define _CORE_CLANG_ (__clang_major__*10000 + __clang_minor__*100 + __clang_patchlevel__*1)
#endif
#if (!defined(_CORE_MSVC_)  || (_CORE_MSVC_)  <  1800) && \
    (!defined(_CORE_GCC_)   || (_CORE_GCC_)   < 40800) && \
    (!defined(_CORE_MINGW_) || (_CORE_MINGW_) < 40800) && \
    (!defined(_CORE_CLANG_) || (_CORE_CLANG_) < 30300)
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
#endif
#if !defined(_CORE_WINDOWS_) && !defined(_CORE_LINUX_) /*&& !defined(_CORE_OSX_)*/ && !defined(_CORE_ANDROID_)
    #warning "Operating System not supported!"
#endif

// debug mode
#if defined(_DEBUG) || defined(DEBUG) || (defined(_CORE_GCC_) && !defined(__OPTIMIZE__))
    #define _CORE_DEBUG_ (1)
#endif

// OpenGL ES
#if defined(_CORE_ANDROID_)
    #define _CORE_GLES_  (1)
#endif

// x86 SIMD
#if (defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__x86_64__)) && !defined(_CORE_ANDROID_)
    #define _CORE_SSE_   (1)
#endif


// ****************************************************************
// basic libraries
#define _HAS_EXCEPTIONS (0)
#define _CRT_SECURE_NO_WARNINGS
#define _ALLOW_KEYWORD_MACROS
#define  WIN32_LEAN_AND_MEAN
#if defined(_CORE_MINGW_)
    #undef __STRICT_ANSI__
#endif

#if defined(_CORE_WINDOWS_)
    #include <windows.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif
#if defined(_CORE_SSE_)
    #include <smmintrin.h>
#endif
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <ctime>
#include <memory>
#include <functional>
#include <random>
#include <array>
#include <vector>
#include <list>
#include <unordered_set>
#include <unordered_map>


// ****************************************************************
// specific libraries
#define HAVE_LIBC
#define GLEW_MX
#define GLEW_NO_GLU
#define OV_EXCLUDE_STATIC_CALLBACKS

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
#include <SI/SimpleIni.h>


// ****************************************************************
// compiler definitions
#if defined(_CORE_MSVC_)
    #define hot_func
    #define cold_func
    #define constexpr_func inline
    #define constexpr_var  const
    #define noexcept       throw()              // keyword
    #define thread_local   __declspec(thread)   // keyword
#else
    #define hot_func       __attribute__((hot))
    #define cold_func      __attribute__((cold))
    #define constexpr_func constexpr
    #define constexpr_var  constexpr
#endif

#if defined(_CORE_MINGW_)
    #define align_func __attribute__((force_align_arg_pointer))
#else
    #define align_func
#endif

#if defined(_CORE_CLANG_)
    #define constexpr_obj inline
#else
    #define constexpr_obj constexpr_func
#endif


// ****************************************************************
// general definitions
#undef  NULL
#define NULL nullptr

#define SAFE_DELETE(p)       {if(p) {delete   (p); (p) = NULL;}}
#define SAFE_DELETE_ARRAY(p) {if(p) {delete[] (p); (p) = NULL;}}
#define ARRAY_SIZE(a)        (sizeof(a) / sizeof((a)[0]))

#define ASSERT(c)            {SDL_assert( (c));}
#define WARN_IF(c)           {SDL_assert(!(c));} if(c)
#define STATIC_ASSERT(c)     static_assert(c, "[" #c "]");

#define __DEFINED(a,b)       (!coreData::StrCmpConst(#a, b))
#define DEFINED(a)           (__DEFINED(a, #a))
#define __STRING(a)          (#a)
#define STRING(a)            (__STRING(a))
                             
#define BIT(n)               (1 << (n))
#define BIT_SET(o,n)         {(o) |=  BIT(n);}
#define BIT_RESET(o,n)       {(o) &= ~BIT(n);}
                             
#define FOR_EACH(i,c)        for(auto i = (c).begin(),  i ## __e = (c).end();  i != i ## __e; ++i)
#define FOR_EACH_REV(i,c)    for(auto i = (c).rbegin(), i ## __e = (c).rend(); i != i ## __e; ++i)
#define FOR_EACH_SET(i,s,c)  for(auto i = (s),          i ## __e = (c).end();  i != i ## __e; ++i)
#define FOR_EACH_DYN(i,c)    for(auto i = (c).begin(),  i ## __e = (c).end();  i != i ## __e; )
#define DYN_KEEP(i)          {++i;}
#define DYN_REMOVE(i,c)      {i = (c).erase(i); i ## __e = (c).end();}

// enable additional information about the defined class
#define ENABLE_INFO(c)                                    \
    static inline const char* GetTypeName()  {return #c;} \
    static inline const char* GetClassPath() {return __FILE__;}

// disable constructor and destructor of the defined class
#define DISABLE_TORS(c) \
    c()  = delete;      \
    ~c() = delete;

// disable copy and move operations with the defined class
#define DISABLE_COPY(c)                \
    c(const c&)              = delete; \
    c& operator = (const c&) = delete;

// disable heap operations with the defined class
#define DISABLE_HEAP                               \
    void* operator new   (size_t)        = delete; \
    void* operator new   (size_t, void*) = delete; \
    void* operator new[] (size_t)        = delete; \
    void* operator new[] (size_t, void*) = delete;

// enable bit-operations with the defined enumeration
#define EXTEND_ENUM(e)                                                                                                                                               \
    constexpr_func e  operator ~  (const e& a)             {return s_cast<e>(~s_cast<std::underlying_type<e>::type>(a));}                                            \
    constexpr_func e  operator |  (const e& a, const e& b) {return s_cast<e>( s_cast<std::underlying_type<e>::type>(a) | s_cast<std::underlying_type<e>::type>(b));} \
    constexpr_func e  operator &  (const e& a, const e& b) {return s_cast<e>( s_cast<std::underlying_type<e>::type>(a) & s_cast<std::underlying_type<e>::type>(b));} \
    constexpr_func e  operator ^  (const e& a, const e& b) {return s_cast<e>( s_cast<std::underlying_type<e>::type>(a) ^ s_cast<std::underlying_type<e>::type>(b));} \
    inline         e& operator |= (e&       a, const e& b) {return (a = a | b);}                                                                                     \
    inline         e& operator &= (e&       a, const e& b) {return (a = a & b);}                                                                                     \
    inline         e& operator ^= (e&       a, const e& b) {return (a = a ^ b);}

// shorter common types and keywords
#define f_list forward_list
#define u_map  unordered_map
#define u_set  unordered_set
#define s_cast static_cast
#define d_cast dynamic_cast
#define r_cast reinterpret_cast
#define c_cast const_cast

// basic sized unsigned integer types
typedef std::uint8_t  coreByte;
typedef std::uint16_t coreUshort;
typedef std::uint32_t coreUint;
typedef std::uint64_t coreUint64;

// type conversion macros
#define F_TO_SI(x) ((int)          (x))   //!< float to signed int
#define F_TO_UI(x) ((unsigned)(int)(x))   //!< float to unsigned int (force [_mm_cvtt_ss2si] instead of [_ftoui])
#define I_TO_F(x)  ((float)(int)   (x))   //!< int to float (force [_mm_cvtepi32_ps] instead of [_mm_cvtepi32_pd] with [_mm_cvtpd_ps])
#define P_TO_I(x)  ((int)(long)    (x))   //!< pointer to int
#define I_TO_P(x)  ((void*)(long)  (x))   //!< int to pointer

enum coreError : int
{
    CORE_OK            =   0,    //!< everything is fine
    CORE_BUSY          =  10,    //!< currently waiting for an event

    CORE_ERROR_FILE    = -10,    //!< error on reading, writing or finding a file or folder
    CORE_ERROR_SUPPORT = -20,    //!< requested feature is not supported on the target system
    CORE_ERROR_SYSTEM  = -30,    //!< invalid system or application behavior

    CORE_INVALID_CALL  = -110,   //!< object has wrong status
    CORE_INVALID_INPUT = -120,   //!< function parameters are invalid
    CORE_INVALID_DATA  = -130,   //!< depending objects contain wrong data
};


// ****************************************************************
// forward declarations
class coreVector2;
class coreVector3;
class coreVector4;
class coreMatrix3;
class coreMatrix4;
class coreFile;
class coreArchive;
class coreObject2D;
class coreObject3D;
class coreParticleEffect;

class coreLog;
class coreConfig;
class coreLanguage;
class coreRand;
class CoreSystem;
class CoreGraphics;
class CoreAudio;
class CoreInput;
class coreMemoryManager;
class coreResourceManager;
class coreObjectManager;


// ****************************************************************
// application framework
class CoreApp final
{
private:
    CoreApp()noexcept {this->Setup(); this->Init();}
    ~CoreApp()        {this->Exit();}
    friend class Core;

    //! auto-generated setup function
    //! @{
    void Setup();
    //! @}

    //! undefined init and exit function
    //! @{
    void Init();
    void Exit();
    //! @}

    //! undefined render and move function
    //! @{
    void Render();
    void Move();
    //! @}
};


// ****************************************************************
// engine framework
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

    struct Manager
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
    //! reset engine
    //! @{
    static void Reset();
    //! @}


private:
    //! run engine
    //! @{
    friend int main(int argc, char* argv[])align_func;
    static int Run();
    //! @}
};


// ****************************************************************
// engine header files
#if defined(_CORE_GLES_)
    #include "additional/coreGLES.h"
#else
    #include "additional/coreGL.h"
#endif

#include "utilities/math/coreMath.h"
#include "utilities/data/coreData.h"

#include "utilities/data/coreSelect.h"
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
#include "objects/game/coreScene.h"
#include "objects/game/coreParticle.h"
#include "objects/menu/coreLabel.h"
#include "objects/menu/coreButton.h"
#include "objects/menu/coreCheckBox.h"
#include "objects/menu/coreTextBox.h"
#include "objects/menu/coreSwitchBox.h"
#include "objects/menu/coreMenu.h"


#endif // _CORE_GUARD_H_

// 0100010001101111001000000111010001101000011010010110111001100111011100110010000001
// 1100100110100101100111011010000111010000101100001000000110111101110010001000000110
// 0100011011110110111000100111011101000010000001100100011011110010000001110100011010
// 0001100101011011010010000001100001011101000010000001100001011011000110110000101110