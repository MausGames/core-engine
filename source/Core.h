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
//| Core Engine v0.0.2a (http://www.maus-games.at)                               |//
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
#pragma once
#ifndef CORE_H
#define CORE_H
// TODO: remove deprecated stuff
// TODO: add explicit keyword
// TODO: boolean traps


// ****************************************************************
// compiler and library specific definitions
#if !defined(_MSC_VER) && !defined(__GNUC__) && !defined(__MINGW32__)
    #warning "Compiler not supported!"
#endif

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define GLEW_MX

#undef __STRICT_ANSI__

#if defined(_MSC_VER)
    #define align16(v) __declspec(align(16)) v
    #define deletefunc
#else
    #define align16(v) v __attribute__((aligned(16)))
    #define deletefunc = delete
#endif

#if defined(_MSC_VER)
    #define __thread __declspec(thread)
    #define constexpr
    #if (_MSC_VER) < 1700
        #define final
    #endif
#endif


// ****************************************************************
// general definitions
#define SAFE_DELETE(p)       {if(p) {delete   (p); (p)=NULL;}}
#define SAFE_DELETE_ARRAY(p) {if(p) {delete[] (p); (p)=NULL;}}

#define u_map unordered_map

typedef unsigned char coreByte;
typedef unsigned int  coreUint;

class coreVector2;
class coreVector3;
class coreVector4;
class coreMatrix;
class coreFile;
class coreArchive;
class coreObject2D; //!


// ****************************************************************
// general enumerations
enum coreError
{
    CORE_OK            =   0,   //!< everything is fine
    CORE_BUSY          =  10,   //!< currently waiting for an event

    CORE_FILE_ERROR    = -10,   //!< error on opening, writing or finding a file
    CORE_INVALID_CALL  = -20,   //!< function cannot be called
    CORE_INVALID_INPUT = -30,   //!< function parameters are invalid
};


// ****************************************************************
// base libraries
#if defined(_WIN32)
    #include <windows.h>
    #undef DeleteFile
#else
    #include <sys/stat.h>
    #include <unistd.h>
    #include <dirent.h>
    #include <string.h>
#endif
#include <smmintrin.h>
#include <time.h>
#include <math.h>
#include <cstdio>
#include <vector>
#include <set>
#include <unordered_map>


// ****************************************************************
// specific libraries
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>
#include <GL/gl.h>
#if !defined(_WIN32)
    #include <GL/glext.h>
#endif
#include <AL/al.h>
#include <AL/alc.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>
#include <SI/SimpleIni.h>


// ****************************************************************
// GLEW multi-context declaration
extern __thread GLEWContext g_GlewContext;
#define glewGetContext() (&g_GlewContext)


// ****************************************************************
// file classes
#include "coreLog.h"
#include "coreConfig.h"
#include "coreArchive.h"


// ****************************************************************
// utility classes
#include "coreMath.h"
#include "coreUtils.h"
#include "coreRand.h"
#include "coreVector.h"
#include "coreMatrix.h"
#include "coreSpline.h"


// ****************************************************************
// interface classes
#include "CoreSystem.h"
#include "CoreGraphics.h"
#include "CoreAudio.h"
#include "CoreInput.h"


// ****************************************************************
// system component classes
#include "coreTimer.h"
#include "coreThread.h"


// ****************************************************************
// manager classes
#include "coreResourceManager.h"


// ****************************************************************
// other component classes
#include "coreTexture.h"
#include "coreShader.h"
#include "coreSound.h"


// ****************************************************************
// main application interface
class CoreApp final
{
private:
    CoreApp()  {this->Init();}
    ~CoreApp() {this->Exit();}
    friend class Core;


public:
    // undefined init and exit function
    void Init();
    void Exit();

    // undefined render and move function
    void Render();
    void Move();
};


// ****************************************************************
// engine framework
class Core final
{
public:
    static coreLog* Log;             //!< log file
    static coreConfig* Config;       //!< configuration file

    static coreMath* Math;           //!< math collection access
    static coreUtils* Utils;         //!< utility collection access
    static coreRand* Rand;           //!< global random number generator

    static CoreSystem* System;       //!< main system interface
    static CoreGraphics* Graphics;   //!< main graphics interface
    static CoreAudio* Audio;         //!< main audio interface
    static CoreInput* Input;         //!< main input interface

    class Manager final
    {
    public:
        // memory manager
        static coreResourceManager* Resource;   //!< resource manager
        // object manager
    };


private:
    Core();
    ~Core();


public:
    // run the application
    static void Run();

    // reset engine
    static void Reset();

    // quit the application
    static void Quit();
};


#endif // CORE_H