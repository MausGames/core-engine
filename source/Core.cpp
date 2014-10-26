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
CoreApp*             Core::Application       = NULL;


// ****************************************************************
// constructor
Core::Core()noexcept
{
    // init utilities
    Log    = new coreLog("logfile.html");
    Log->Header("Utilities");
    Config = new coreConfig("config.ini");
    Rand   = new coreRand(2048);
    
    // init main components
    System   = new CoreSystem();
    Graphics = new CoreGraphics();
    Audio    = new CoreAudio();
    Input    = new CoreInput();

    // init managers
    Log->Header("Managers");
    Manager::Memory   = new coreMemoryManager();
    Manager::Resource = new coreResourceManager();
    Manager::Object   = new coreObjectManager();

    // load language file
    Language = new coreLanguage(Config->GetString(CORE_CONFIG_SYSTEM_LANGUAGE));

    // init application
    Log->Header("Application Init");
    Application = new CoreApp();
    Manager::Resource->UpdateResources();
    Log->Header("Application Run");
}


// ****************************************************************
// destructor
Core::~Core()
{
    Core::Log->Header("Shut Down");

    // delete application
    SAFE_DELETE(Application)

    // delete managers
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
// reset engine
void Core::Reset()
{
    Log->Warning("Reset started");

    // save current state
    const double      dTotalTime      = System->m_dTotalTime;
    const coreUint    iCurFrame       = System->m_iCurFrame;
    const float       fFOV            = Graphics->m_fFOV;
    const float       fNearClip       = Graphics->m_fNearClip;
    const float       fFarClip        = Graphics->m_fFarClip;
    const coreVector3 vCamPosition    = Graphics->m_vCamPosition;
    const coreVector3 vCamDirection   = Graphics->m_vCamDirection;
    const coreVector3 vCamOrientation = Graphics->m_vCamOrientation;
    float                   afTimeSpeed[CORE_SYSTEM_TIMES];    std::memcpy(afTimeSpeed, System->m_afTimeSpeed, sizeof(afTimeSpeed));
    CoreGraphics::coreLight aLight     [CORE_GRAPHICS_LIGHTS]; std::memcpy(aLight,      Graphics->m_aLight,    sizeof(aLight));

    // shut down resource manager
    Manager::Resource->Reset(CORE_RESOURCE_RESET_EXIT);

    // shut down main components
    SAFE_DELETE(Input)
    SAFE_DELETE(Graphics)
    SAFE_DELETE(System)

    // start up main components
    System   = new CoreSystem();
    Graphics = new CoreGraphics();
    Input    = new CoreInput();

    // setup the application
    Application->Setup();

    // start up resource manager
    Manager::Resource->Reset(CORE_RESOURCE_RESET_INIT);

    // load former state
    System->m_dTotalTime = dTotalTime;
    System->m_iCurFrame  = iCurFrame;
    Graphics->SetView  (System->GetResolution(), fFOV, fNearClip, fFarClip);
    Graphics->SetCamera(vCamPosition, vCamDirection, vCamOrientation);
    for(int i = 0; i < CORE_SYSTEM_TIMES;    ++i) System->SetTimeSpeed(i, afTimeSpeed[i]);
    for(int i = 0; i < CORE_GRAPHICS_LIGHTS; ++i) Graphics->SetLight(i, aLight[i].vPosition, aLight[i].vDirection, aLight[i].vValue);

    Log->Warning("Reset finished");
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

    // run engine
    return Core::Run();
}


// ****************************************************************
// run engine
int Core::Run()
{
    // create engine instance
    Core Engine;

    // set logging level
    if(!Core::Config->GetBool(CORE_CONFIG_SYSTEM_DEBUGMODE) && !DEFINED(_CORE_DEBUG_))
    {
        Core::Log->SetLevel(CORE_LOG_LEVEL_WARNING | CORE_LOG_LEVEL_ERROR);
        Core::Log->Warning("Logging level reduced");
    }

    // update the window event system (main loop)
    while(Core::System->__UpdateEvents())
    {
        // update the input button interface
        Core::Input->__UpdateButtons();

        // move and render the application
        Core::Application->Move();
        Core::Application->Render();

        // update all remaining components
        Core::Graphics->__UpdateScene();
        Core::System->__UpdateTime();
        Core::Input->__ClearButtons();
    
        // update the resource manager with only one context
        if(!Core::Graphics->GetResourceContext())
        {
            Core::Manager::Resource->UpdateFunctions();
            Core::Manager::Resource->UpdateResources();
        }
    }

    // reset logging level
    Core::Log->SetLevel(CORE_LOG_LEVEL_ALL);
   
    return 0;
}