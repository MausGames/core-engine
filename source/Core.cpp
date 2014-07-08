//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

coreLog*             Core::Log               = NULL;
coreConfig*          Core::Config            = NULL;
coreLanguage*        Core::Language          = NULL;
coreRand*            Core::Rand              = NULL;
CoreSystem*          Core::System            = NULL;
CoreGraphics*        Core::Graphics          = NULL;
CoreAudio*           Core::Audio             = NULL;
CoreInput*           Core::Input             = NULL;
coreMemoryManager*   Core::Manager::Memory   = NULL;
coreResourceManager* Core::Manager::Resource = NULL;
coreObjectManager*   Core::Manager::Object   = NULL;


// ****************************************************************
// context definition
#if !defined(_CORE_GLES_)
    thread_local GLEWContext g_GlewContext;
#endif


// ****************************************************************
// constructor
Core::Core()noexcept
{
    // init utilities
    Log    = new coreLog("logfile.html");
    Config = new coreConfig("config.ini");
    Rand   = new coreRand(2048);

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

    // load language file
    Language = new coreLanguage(Config->GetString(CORE_CONFIG_SYSTEM_LANGUAGE));
}


// ****************************************************************
// destructor
Core::~Core()
{
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
    SAFE_DELETE(Language)
    SAFE_DELETE(Config)
    SAFE_DELETE(Log)
}


// ****************************************************************
// reset the engine
void Core::Reset()
{
    // set logging level
    const coreLogLevel iLevel = Core::Log->GetLevel();
    Core::Log->SetLevel(CORE_LOG_LEVEL_ALL);
    
    // shut down resource manager
    Log->Header("Engine Reset");
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

    // reset logging level
    Core::Log->SetLevel(iLevel);
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

    // set new working directory
    coreData::SetCurDir("../..");

    // run the engine
    return Core::__Run();
}


// ****************************************************************
// run the engine
int Core::__Run()
{
    // init engine
    Core* pEngine = new Core();

    // init application
    Core::Log->Header("Application Init");
    CoreApp* pApplication = new CoreApp();
    Core::Log->Header("Application Run");

    // set logging level
    if(!Core::Config->GetBool(CORE_CONFIG_SYSTEM_DEBUG) && !DEFINED(_CORE_DEBUG_))
    {
        Core::Log->SetLevel(CORE_LOG_LEVEL_ONLY_ERROR);
        Core::Log->Warning("Logging level reduced");
    }

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
            Core::Manager::Resource->__RunThread();
    }

    // reset logging level
    Core::Log->SetLevel(CORE_LOG_LEVEL_ALL);

    // delete application and engine
    Core::Log->Header("Shut Down");
    SAFE_DELETE(pApplication)
    SAFE_DELETE(pEngine)

    return 0;
}