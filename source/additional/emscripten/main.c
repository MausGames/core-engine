///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#if defined(__EMSCRIPTEN__)

#define SDL_MAIN_HANDLED
#include <SDL3/SDL_main.h>

extern int coreMain(int argc, char** argv);


// ****************************************************************
/* start up the application */
int main(int argc, char** argv)
{
    // run the application (with SDL hint parsing)
    return SDL_RunApp(argc, argv, coreMain, NULL);
}


#endif /* __EMSCRIPTEN__ */
