#include "Core.h"


// ****************************************************************    
// init resource thread
int coreResourceManager::__Init()
{
    // assign secondary OpenGL context to resource thread
    if(SDL_GL_MakeCurrent(Core::System->GetWindow(), Core::Graphic->GetResourceContext()))
        Core::Log->Error(1, coreUtils::Print("Secondary OpenGL context could not be assigned to resource thread (SDL: %s)", SDL_GetError()));
    else Core::Log->Info("Secondary OpenGL context assigned to resource thread");

    return 0;
}


// ****************************************************************    
// run resource thread
int coreResourceManager::__Run()
{


    return 0;
}