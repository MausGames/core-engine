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
//| Core Engine v0.0.1a (http://www.maus-games.at)                               |//
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
#define _CRT_SECURE_NO_DEPRECATE 
#define _SCL_SECURE_NO_DEPRECATE
#define WIN32_LEAN_AND_MEAN
#define GLEW_MX

// TODO: remove deprecated stuff
// TODO: add explicit keyword
// TODO: boolean traps


// ****************************************************************    
// general macros and definitions
#define SAFE_DELETE(p)       {if(p) {delete   (p); (p)=NULL;}}
#define SAFE_DELETE_ARRAY(p) {if(p) {delete[] (p); (p)=NULL;}}

#if defined(_MSC_VER)
    #define __align16(v) __declspec(align(16)) v
    #define __thread     __declspec(thread)
#elif defined(__MINGW32__) || defined(__CYGWIN__)
    #define __align16(v) __declspec(aligned(16)) v
#else 
    #define __align16(v) v __attribute__(align(16))
#endif

typedef unsigned char coreByte;
typedef unsigned int  coreUint;

class coreVector2;
class coreVector3;
class coreVector4;
class coreMatrix;
class coreFile;
class coreArchive;
class coreObject2D;

enum coreError
{
    CORE_OK            =   0,
    CORE_BUSY          =  10,

    CORE_FILE_ERROR    = -10,
    CORE_INVALID_CALL  = -20,
    CORE_INVALID_INPUT = -30,
};


// ****************************************************************
// base libraries
#if defined(_MSC_VER)
    #include <windows.h>
    #undef DeleteFile
#else
    #include <sys/stat.h>
    #include <unistd.h>
    #include <dirent.h> 
    #include <string.h>
    #include <xmmintrin.h>
    #include <pmmintrin.h>
    #include <smmintrin.h>
#endif
#include <time.h>
#include <math.h>
#include <cstdio>
#include <vector>
#include <set>
#include <map>


// ****************************************************************
// specific libraries
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <GL/glew.h>
#if defined(_WIN32)
    #include <GL/glaux.h>
#else
    #include <GL/gl.h>
    #include <GL/glext.h>
#endif
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>
#include <SimpleIni.h>


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
#include "coreTimer.h"
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
// component classes
#include "coreThread.h"


// ****************************************************************
// manager classes
#include "coreResourceManager.h"


// ****************************************************************
// component classes
#include "coreTexture.h"


// ****************************************************************    
// main application interface
class CoreApp
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
class Core
{
public:
    static coreLog* Log;             // log file
    static coreConfig* Config;       // configuration file

    static coreMath* Math;           // math collection access
    static coreUtils* Utils;         // utils collection access
    static coreRand* Rand;           // global random number generator

    static CoreSystem* System;       // main system interface
    static CoreGraphics* Graphics;   // main graphics interface
    static CoreAudio* Audio;         // main audio interface
    static CoreInput* Input;         // main input interface

    class Manager
    {
    public:
        // memory manager
        static coreResourceManager* Resource;   // resource manager
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