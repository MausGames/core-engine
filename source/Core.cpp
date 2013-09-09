//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

#if defined(_WIN32)
    #include <direct.h>
#endif

coreLog*             Core::Log               = NULL;
coreConfig*          Core::Config            = NULL;

coreMath*            Core::Math              = NULL;
coreUtils*           Core::Utils             = NULL;
coreRand*            Core::Rand              = NULL;

CoreSystem*          Core::System            = NULL;
CoreGraphics*        Core::Graphics          = NULL;
CoreAudio*           Core::Audio             = NULL;
CoreInput*           Core::Input             = NULL;

coreMemoryManager*   Core::Manager::Memory   = NULL;
coreResourceManager* Core::Manager::Resource = NULL;


// ****************************************************************
// GLEW multi-context definition
__thread GLEWContext g_GlewContext;


// ****************************************************************
// constructor
Core::Core()
{
    // init log and configuration file
    Log    = new coreLog("logfile.html");
    Config = new coreConfig("config.ini");

    // init utilities
    Math  = new coreMath();
    Utils = new coreUtils();
    Rand  = new coreRand(2048);

    // init main interfaces
    System   = new CoreSystem();
    Graphics = new CoreGraphics();
    Audio    = new CoreAudio();
    Input    = new CoreInput();

    // init manager
    Log->Header("Manager");
    Manager::Memory   = new coreMemoryManager();
    Manager::Resource = new coreResourceManager();
}


// ****************************************************************
// destructor
Core::~Core()
{
    // delete manager
    SAFE_DELETE(Manager::Resource)
    SAFE_DELETE(Manager::Memory)

    // delete main interfaces
    SAFE_DELETE(Input)
    SAFE_DELETE(Audio)
    SAFE_DELETE(Graphics)
    SAFE_DELETE(System)

    // delete utilities
    SAFE_DELETE(Rand)
    SAFE_DELETE(Utils)
    SAFE_DELETE(Math)

    // delete log and configuration file
    SAFE_DELETE(Config)
    SAFE_DELETE(Log)
}


// ****************************************************************
// run the application
void Core::Run()
{
    if(System) return;

    // init engine
    Core* pEngine = new Core();

    // init application
    pEngine->Log->Header("Application Init");
    CoreApp* pApplication = new CoreApp();
    pEngine->Log->Header("Application Run");

#if !defined(_DEBUG)
    // set logging level
    const int iLevel = Core::Config->GetInt(CORE_CONFIG_SYSTEM_LOG, -1);
    pEngine->Log->SetLevel(iLevel);
    if(iLevel < 0) pEngine->Log->Error(0, "Logging level reduced");
#endif

    // update the window event system (main loop)
    while(pEngine->System->__UpdateEvents())
    {
        // pre-update engine
        pEngine->Input->__UpdateInput();

        // move and render application
        pApplication->Move();
        pApplication->Render();

        // post-update engine
        pEngine->Input->__UpdateCursor();
        pEngine->Graphics->__UpdateScene();
        pEngine->System->__UpdateTime();
    }

    // reset logging level
    pEngine->Log->SetLevel(0);

    // delete engine and application
    pEngine->Log->Header("Shut Down");
    SAFE_DELETE(pApplication)
    SAFE_DELETE(pEngine)
}


// ****************************************************************
// reset engine
void Core::Reset()
{
    Log->Header("Engine Reset");

    // shut down manager
    Manager::Resource->Reset(false);

    // delete main interfaces
    SAFE_DELETE(Input)
    SAFE_DELETE(Audio)
    SAFE_DELETE(Graphics)
    SAFE_DELETE(System)

    // re-init main interfaces
    System   = new CoreSystem();
    Graphics = new CoreGraphics();
    Audio    = new CoreAudio();
    Input    = new CoreInput();

    // re-init manager
    Log->Header("Manager");
    Manager::Resource->Reset(true);

    Log->Header("Application Run");
}


// ******************************************************************
// quit the application
void Core::Quit()
{
    // send quit event
    SDL_Event QuitEvent;
    QuitEvent.type = SDL_QUIT;
    SDL_PushEvent(&QuitEvent);
}


// ****************************************************************
// main function
int main(int argc, char* argv[])
{
#if defined(_MSC_VER) && defined(_DEBUG)
    _crtBreakAlloc = 0;
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    // set new working directory (cd ../..)
    char acPath[256];
    sprintf(acPath, "%s", coreUtils::AppPath());
    for(int i = 0; i < 3; ++i)
        (*strrchr(acPath, CORE_UTILS_SLASH[0])) = '\0';
    _chdir(acPath);

    // run the application
    Core::Run();
    return 0;
}