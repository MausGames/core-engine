//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#pragma warning(disable : 4100)

#include <SDL2/SDL_main.h>
#include <windows.h>
#include <stdlib.h>


// ****************************************************************
/* start up the application */
int WINAPI WinMain(HINSTANCE pInstance, HINSTANCE pPrevInstance, LPSTR pcCmdLine, int iCmdShow)
{
    // initialize the SDL library
    SDL_SetMainReady();

    // run the application
    char* argv[2] = {"CoreApp", NULL};
    SDL_main(1, argv);

    return 0;
}


#endif /* _WIN32 */