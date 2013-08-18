#include "Core.h"

coreLog*             Core::Log               = NULL;
coreConfig*          Core::Config            = NULL;
                                             
coreMath*            Core::Math              = NULL;
coreUtils*           Core::Utils             = NULL;
coreRand*            Core::Rand              = NULL;
                                             
CoreSystem*          Core::System            = NULL;
CoreGraphic*         Core::Graphic           = NULL;
CoreSound*           Core::Sound             = NULL;
CoreInput*           Core::Input             = NULL;

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
    System  = new CoreSystem();
    Graphic = new CoreGraphic();
    Sound   = new CoreSound();
    Input   = new CoreInput();

    // init manager
    Log->Header("Manager");
    Manager::Resource = new coreResourceManager();
}


// ****************************************************************    
// destructor
Core::~Core()
{
    // delete manager
    SAFE_DELETE(Manager::Resource)

    // delete main interfaces
    SAFE_DELETE(Input)
    SAFE_DELETE(Sound)
    SAFE_DELETE(Graphic)
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

#if !defined (_DEBUG)
    // set logging level
    const int iLevel = Core::Config->GetInt(CORE_CONFIG_SYSTEM_LOG, -1);
    pEngine->Log->SetLevel(iLevel);
    if(iLevel < 0) pEngine->Log->Error(0, "Logging level reduced, show only warnings and errors");
#endif

    // update the window event system
    while(pEngine->System->__UpdateEvents())
    {    
        // update the input interface
        pEngine->Input->__UpdateInput();

        // move and render the application
        pApplication->Move();
        pApplication->Render();

        // update the graphical mouse cursor
        pEngine->Input->__UpdateCursor();

        // update the graphic scene
        pEngine->Graphic->__UpdateScene();

        // update the high precission time calculation
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
//  reset engine
void Core::Reset()
{
    Log->Header("Reset Engine");

    // shut down manager
    Manager::Resource->__Reset(false);

    // delete main interfaces
    SAFE_DELETE(Input)
    SAFE_DELETE(Sound)
    SAFE_DELETE(Graphic)
    SAFE_DELETE(System)

    // re-init main interfaces
    System  = new CoreSystem();
    Graphic = new CoreGraphic();
    Sound   = new CoreSound();
    Input   = new CoreInput();

    // re-init manager
    Log->Header("Manager");
    Manager::Resource->__Reset(true);

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
#if defined (_WIN32) && defined (_DEBUG)
    //_crtBreakAlloc = 0;
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); 
#endif

    Core::Run();
    return 0;
}