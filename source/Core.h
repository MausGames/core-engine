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
//| Core Engine v0.0.3a (http://www.maus-games.at)                               |//
//*------------------------------------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                                          |//
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
//! \file
//! \defgroup component Components
#pragma once
#ifndef _CORE_GUARD_H_
#define _CORE_GUARD_H_

/*
coreMenu:
von oben nach unten kollision testen
eine seite in eine andere einfuegen als subseite
virtuelle funktion oder funktionszeiger fuer dynamische uebergaenge
Framebuffer nutzen

Framebuffer:
Framebuffer Objekte zum Zeichnen in Texturen,
Depth Map, Multisample Buffer, Post Processing (Distortion Mapping, Bloom, Glow, HDR, Blur) -> Shader Stages laden als Pfad
Deferred Shading, Menu Objekte buendeln


Instancing:
Selbe Objekte zeichenbar, Sprites

Sprite:
Daten pro Sprite moeglichst klein halten
Instancing nutzen, Ueber-Objekte definieren
Sammelbecken im Manager?


Kollision:
Kollisionserkennung in weiterem Thread (Zahl wird in einem Thread erhoeht, im anderen gesenkt, Query),
Durchfuehrung nach Status im Hauptthread


coreLanguage:
Verknuepfung aller Label samt (Text-)ID mit Klasse,
SetText und erneute Generierung bei Sprachwechsel


Performance/Benchmark:
neues Fenster mit Status-Werten, Log, Speicher, Fehler, FPS

*/

// compiler
#if defined(_MSC_VER)
    #define _CORE_MSVC_ (_MSC_VER)
#endif
#if defined(__GNUC__)
    #define _CORE_GCC_ (__GNUC__*10000 + __GNUC_MINOR__*100 + __GNUC_PATCHLEVEL__*1)
#endif
#if defined(__MINGW32__)
    #include <_mingw.h>
    #define _CORE_MINGW_ (__MINGW_MAJOR_VERSION*10000 + __MINGW_MINOR_VERSION*100 + __MINGW_PATCHLEVEL*1)
#endif
#if defined(__clang__)
    #define _CORE_CLANG_ (__clang_major__*10000 + __clang_minor__*100 + __clang_patchlevel__*1)
#endif

// platform
#if defined(_WIN32)
    #define _CORE_WINDOWS_
#endif
#if defined(__linux__)
    #define _CORE_LINUX_
#endif
#if defined(__APPLE__)
    #define _CORE_OSX_
#endif
#if defined(__ANDROID__)
    #define _CORE_ANDROID_
#endif

// debug mode
#if defined(_DEBUG) || defined(DEBUG) || (defined(_CORE_GCC_) && !defined(__OPTIMIZE__))
    #define _CORE_DEBUG_
    const bool g_bDebug = true;
#else
    const bool g_bDebug = false;
#endif

// SIMD support
#if (defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__x86_64__)) && !defined(_CORE_ANDROID_) && !defined(_CORE_DEBUG_)
    #define _CORE_SSE_
#endif

// OpenGL ES support
#if defined(_CORE_ANDROID_)
    #define _CORE_GLES_
#endif


// ****************************************************************
// compiler, platform and library specific definitions
#if !defined(_CORE_MSVC_) && !defined(_CORE_GCC_) && !defined(_CORE_MINGW_) && !defined(_CORE_CLANG_)
    #warning "Compiler not supported!"
#endif

#define _HAS_EXCEPTIONS 0
#define _CRT_SECURE_NO_WARNINGS
#define _ALLOW_KEYWORD_MACROS
#define WIN32_LEAN_AND_MEAN

#define GLEW_MX
#define GLEW_NO_GLU
#define OV_EXCLUDE_STATIC_CALLBACKS

#if defined(_CORE_MSVC_)
    #if (_CORE_MSVC_) < 1800
        #define delete_func
    #else
        #define delete_func = delete
    #endif
    #if (_CORE_MSVC_) < 1700
        #define final
    #endif
    #define noexcept       throw()
    #define __thread       __declspec(thread)
    #define align_16(x)    __declspec(align(16)) x
    #define sse_save(x)    align_16(x)
    #define constexpr_func inline
    #define constexpr_var  const
#else
    #define delete_func    = delete
    #define align_16(x)    x __attribute__((aligned(16)))
    #define sse_save(x)    static __thread align_16(x)
    #define constexpr_func constexpr
    #define constexpr_var  constexpr
#endif

#if defined(_CORE_GCC_)
    #if (_CORE_GCC_) < 40700
        #define override
        #define final
    #endif
#endif

#if defined(_CORE_MINGW_)
    #define align_func __attribute__((force_align_arg_pointer))
    #undef __STRICT_ANSI__
#else
    #define align_func
#endif


// ****************************************************************
// general definitions
#define SAFE_DELETE(p)       {if(p) {delete   (p); (p)=NULL;}}
#define SAFE_DELETE_ARRAY(p) {if(p) {delete[] (p); (p)=NULL;}}

#define FOR_EACH(i,c)     for(auto i = c.begin();  i != c.end();  ++i)
#define FOR_EACH_REV(i,c) for(auto i = c.rbegin(); i != c.rend(); ++i)
#define ASSERT_IF(c)      SDL_assert(!(c)); if(c)

#define DISABLE_COPY(c)      \
    c(const c&) delete_func; \
    c& operator = (const c&) delete_func;

#define DISABLE_HEAP                                \
    void* operator new (size_t) delete_func;        \
    void* operator new (size_t, void*) delete_func; \
    void* operator new[] (size_t) delete_func;      \
    void* operator new[] (size_t, void*) delete_func;

#define u_map  unordered_map
#define u_set  unordered_set
#define s_cast static_cast
#define r_cast reinterpret_cast

typedef unsigned char  coreByte;
typedef unsigned short coreWord;
typedef unsigned int   coreUint;

class coreVector2;
class coreVector3;
class coreVector4;
class coreMatrix3;
class coreMatrix4;
class coreFile;
class coreArchive;

enum coreError
{
    CORE_OK            =   0,    //!< everything is fine
    CORE_BUSY          =  10,    //!< currently waiting for an event

    CORE_FILE_ERROR    = -10,    //!< error on opening, writing or finding a file
    CORE_SYSTEM_ERROR  = -20,    //!< invalid application behavior (should never happen)

    CORE_INVALID_CALL  = -110,   //!< object has wrong status
    CORE_INVALID_INPUT = -120,   //!< function parameters are invalid
    CORE_INVALID_DATA  = -130,   //!< depending objects contain wrong data
};


// ****************************************************************
// base libraries
#if defined(_CORE_WINDOWS_)
    #include <windows.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif
#if defined(_CORE_SSE_)
    #include <smmintrin.h>
#endif
#include <cstdio>
#include <cstring>
#include <cmath>
#include <ctime>
#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>


// ****************************************************************
// specific libraries
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#if defined(_CORE_GLES_)
    #include <GLES2/gl2.h>
    #include "additional/android/glewES.h"
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
// GLEW multi-context declaration
#if !defined(_CORE_GLES_)
    extern __thread GLEWContext g_GlewContext;
    #define glewGetContext() (&g_GlewContext)
#endif


// ****************************************************************
// engine header files
#include "utilities/data/coreLookup.h"

#include "utilities/file/coreLog.h"
#include "utilities/file/coreConfig.h"
#include "utilities/file/coreArchive.h"

#include "utilities/math/coreMath.h"
#include "utilities/math/coreVector.h"
#include "utilities/math/coreMatrix.h"
#include "utilities/math/coreSpline.h"

#include "utilities/data/coreData.h"
#include "utilities/data/coreRand.h"

#include "components/system/CoreSystem.h"
#include "components/system/coreTimer.h"
#include "components/system/coreThread.h"

#include "manager/coreMemory.h"
#include "manager/coreResource.h"

#include "components/graphics/CoreGraphics.h"
#include "components/graphics/coreSync.h"
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

#include "objects/menu/coreLabel.h"
#include "objects/menu/coreButton.h"
#include "objects/menu/coreCheckBox.h"
#include "objects/menu/coreTextBox.h"
#include "objects/menu/coreSwitchBox.h"
#include "objects/menu/coreMenu.h"


// ****************************************************************
// application framework
class CoreApp final
{
private:
    CoreApp()noexcept {this->Init();}
    ~CoreApp()        {this->Exit();}
    friend class Core;


public:
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
// TODO: don't lose engine attributes after reset
// TODO: SDL_GetPowerInfo
// TODO: check GCC function attributes (pure, hot, cold)
// TODO: improve sort and structure under all class access modifiers
// TODO: don't forward/return trivial types as reference ? (address > value)
class Core final
{
public:
    static coreLog* Log;             //!< log file
    static coreConfig* Config;       //!< configuration file

    static coreMath* Math;           //!< math collection access
    static coreData* Data;           //!< data collection access
    static coreRand* Rand;           //!< global random number generator

    static CoreSystem* System;       //!< main system component
    static CoreGraphics* Graphics;   //!< main graphics component
    static CoreAudio* Audio;         //!< main audio component
    static CoreInput* Input;         //!< main input component

    class Manager final
    {
    public:
        static coreMemoryManager* Memory;       //!< memory manager
        static coreResourceManager* Resource;   //!< resource manager
        static coreObjectManager* Object;       //!< object manager
    };


private:
    Core()noexcept;
    ~Core();

    //! run the engine
    //! @{
    static void __Run();
    //! @}


public:
    //! control the engine
    //! @{
    static void Reset();
    static void Quit();
    //! @}

    //! main function
    //! @{
    friend int main(int argc, char* argv[]) align_func;
    //! @}
};


#endif // _CORE_GUARD_H_