//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

#if defined(_CORE_WINDOWS_)
    #include <direct.h>
    #define chdir _chdir
#endif

coreLog*             Core::Log               = NULL;
coreConfig*          Core::Config            = NULL;
coreMath*            Core::Math              = NULL;
coreData*            Core::Data              = NULL;
coreRand*            Core::Rand              = NULL;
CoreSystem*          Core::System            = NULL;
CoreGraphics*        Core::Graphics          = NULL;
CoreAudio*           Core::Audio             = NULL;
CoreInput*           Core::Input             = NULL;
coreMemoryManager*   Core::Manager::Memory   = NULL;
coreResourceManager* Core::Manager::Resource = NULL;
coreObjectManager*   Core::Manager::Object   = NULL;


// ****************************************************************
// GLEW multi-context definition
#if !defined(_CORE_GLES_)
    __thread GLEWContext g_GlewContext;
#endif


// ****************************************************************
// constructor
Core::Core()noexcept
{
    // init files
    Log    = new coreLog("logfile.html");
    Config = new coreConfig("config.ini");

    // init utilities
    Math = new coreMath();
    Data = new coreData();
    Rand = new coreRand(2048);

    // init main components
    System   = new CoreSystem();
    Graphics = new CoreGraphics();
    Audio    = new CoreAudio();
    Input    = new CoreInput();

    // init manager
    Log->Header("Manager");
    Manager::Memory   = new coreMemoryManager();
    Manager::Resource = new coreResourceManager();
    Manager::Object   = new coreObjectManager();

    // init resource classes
    coreModel::Init();
    coreShader::Init();
}


// ****************************************************************
// destructor
Core::~Core()
{
    // exit resource classes
    coreShader::Exit();
    coreModel::Exit();

    // delete manager
    SAFE_DELETE(Manager::Object)
    SAFE_DELETE(Manager::Resource)
    SAFE_DELETE(Manager::Memory)

    // delete main components
    SAFE_DELETE(Input)
    SAFE_DELETE(Audio)
    SAFE_DELETE(Graphics)
    SAFE_DELETE(System)

    // delete utilities
    SAFE_DELETE(Rand)
    SAFE_DELETE(Data)
    SAFE_DELETE(Math)

    // delete files
    SAFE_DELETE(Config)
    SAFE_DELETE(Log)
}


// ****************************************************************
// run the engine
void Core::__Run()
{
    // init engine
    Core* pEngine = new Core();

    // init application
    Core::Log->Header("Application Init");
    CoreApp* pApplication = new CoreApp();
    Core::Log->Header("Application Run");

#if !defined(_CORE_DEBUG_)

    // set logging level
    const int iLevel = Core::Config->GetInt(CORE_CONFIG_SYSTEM_LOG);
    Core::Log->SetLevel(iLevel);
    if(iLevel < 0) Core::Log->Error(0, "Logging level reduced");

#endif

    // update the window event system (main loop)
    while(Core::System->__UpdateEvents())
    {
        // update the input button interface
        Core::Input->__UpdateButtons();

        // move and render the application
        pApplication->Move();
        pApplication->Render();

        // update all remaining components
        Core::Graphics->__UpdateScene();
        Core::System->__UpdateTime();
        Core::Input->__ClearButtons();
    
        // update the resource manager with only one context
        if(!Core::Graphics->GetResourceContext())
            Core::Manager::Resource->__Run();
    }

    // reset logging level
    Core::Log->SetLevel(0);

    // delete application and engine
    Core::Log->Header("Shut Down");
    SAFE_DELETE(pApplication)
    SAFE_DELETE(pEngine)
}


// ****************************************************************
// reset the engine
void Core::Reset()
{
    Log->Header("Engine Reset");

    // shut down resource manager
    Manager::Resource->Reset(false);

    // shut down main components
    SAFE_DELETE(Input)
    SAFE_DELETE(Audio)
    SAFE_DELETE(Graphics)
    SAFE_DELETE(System)

    // re-init main components
    System   = new CoreSystem();
    Graphics = new CoreGraphics();
    Audio    = new CoreAudio();
    Input    = new CoreInput();

    // re-init resource manager
    Manager::Resource->Reset(true);

    Log->Header("Application Run");
}


// ******************************************************************
// quit the engine
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
#if defined(_CORE_MSVC_) && defined(_CORE_DEBUG_)

    // activate memory debugging on MSVC
    _crtBreakAlloc = 0;
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

#endif

#if !defined(_CORE_ANDROID_)

    // set new working directory
    chdir("../..");

#endif

    // run the engine
    Core::__Run();
    return 0;
}