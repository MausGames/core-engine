///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

STATIC_MEMORY(coreLog,             Core::Log)
STATIC_MEMORY(coreConfig,          Core::Config)
STATIC_MEMORY(coreLanguage,        Core::Language)
STATIC_MEMORY(coreRand,            Core::Rand)
STATIC_MEMORY(CoreSystem,          Core::System)
STATIC_MEMORY(CoreGraphics,        Core::Graphics)
STATIC_MEMORY(CoreAudio,           Core::Audio)
STATIC_MEMORY(CoreInput,           Core::Input)
STATIC_MEMORY(CorePlatform,        Core::Platform)
STATIC_MEMORY(CoreDebug,           Core::Debug)
STATIC_MEMORY(coreMemoryManager,   Core::Manager::Memory)
STATIC_MEMORY(coreResourceManager, Core::Manager::Resource)
STATIC_MEMORY(coreObjectManager,   Core::Manager::Object)
STATIC_MEMORY(CoreApp,             Core::Application)

static coreBool s_bRestart = false;   // requested application restart


// ****************************************************************
/* constructor */
Core::Core()noexcept
{
    // init log file
    STATIC_NEW(Log, coreData::UserFolderShared("log.html"))

    // init platform component
    STATIC_NEW(Platform)

    // init utilities
    Log->Header("Configuration");
    STATIC_NEW(Config, coreData::UserFolderPrivate("config.ini"))
    STATIC_NEW(Language)
    STATIC_NEW(Rand)
    coreData::LogCommandLine();
    coreData::LogEnvironment();

    // init main components
    STATIC_NEW(System)
    STATIC_NEW(Graphics)
    STATIC_NEW(Audio)
    STATIC_NEW(Input)

    // init managers
    Log->Header("Other");
    STATIC_NEW(Manager::Memory)
    STATIC_NEW(Manager::Resource)
    STATIC_NEW(Manager::Object)

    // init debug component
    STATIC_NEW(Debug)

    // load language file
    Language->Load(Config->GetString(CORE_CONFIG_BASE_LANGUAGE));

    // apply project settings
    System->SetWindowTitle(CoreApp::Settings::Name);
    System->SetWindowIcon (CoreApp::Settings::IconPath);
    Input ->SetCursor     (CoreApp::Settings::CursorPath);

    // init application
    Log->Header("Application Setup");
    STATIC_NEW(Application)
    Manager::Resource->UpdateWait(CORE_RESOURCE_WAIT_STARTUP);
    Log->Header("Application Run");
}


// ****************************************************************
/* destructor */
Core::~Core()
{
    Log->Header("Shut Down");

    // finish all remaining OpenGL operations
    coreSync::Finish();

    // delete application
    STATIC_DELETE(Application)

    // delete debug component
    STATIC_DELETE(Debug)

    // delete managers
    STATIC_DELETE(Manager::Object)
    STATIC_DELETE(Manager::Resource)
    STATIC_DELETE(Manager::Memory)

    // delete main components
    STATIC_DELETE(Input)
    STATIC_DELETE(Audio)
    STATIC_DELETE(Graphics)
    STATIC_DELETE(System)

    // delete platform component
    STATIC_DELETE(Platform)

    // delete utilities
    STATIC_DELETE(Rand)
    STATIC_DELETE(Language)
    STATIC_DELETE(Config)
    STATIC_DELETE(Log)
}


// ****************************************************************
/* reset engine */
void Core::Reset()
{
    Log->Warning("Reset started");

    // finish all remaining OpenGL operations
    coreSync::Finish();

    // save current state
    const coreDouble  dTotalTime       = System  ->m_dTotalTime;
    const coreDouble  dTotalTimeBefore = System  ->m_dTotalTimeBefore;
    const coreUint32  iCurFrame        = System  ->m_iCurFrame;
    const coreFloat   fFOV             = Graphics->m_fFOV;
    const coreFloat   fNearClip        = Graphics->m_fNearClip;
    const coreFloat   fFarClip         = Graphics->m_fFarClip;
    const coreVector3 vCamPosition     = Graphics->m_vCamPosition;
    const coreVector3 vCamDirection    = Graphics->m_vCamDirection;
    const coreVector3 vCamOrientation  = Graphics->m_vCamOrientation;
    coreFloat               afTimeSpeed[CORE_SYSTEM_TIMES];    std::memcpy(afTimeSpeed, System  ->m_afTimeSpeed, sizeof(afTimeSpeed));
    CoreGraphics::coreLight aLight     [CORE_GRAPHICS_LIGHTS]; std::memcpy(aLight,      Graphics->m_aLight,      sizeof(aLight));

    // shut down managers
    Manager::Resource->Reset(CORE_RESOURCE_RESET_EXIT);
    Manager::Object->__Reset(CORE_RESOURCE_RESET_EXIT);

    // shut down main components
    STATIC_DELETE(Debug)
    STATIC_DELETE(Graphics)
    STATIC_DELETE(System)

    // start up main components
    STATIC_NEW(System)
    STATIC_NEW(Graphics)
    STATIC_NEW(Debug)

    // load former state
    System  ->m_dTotalTime       = dTotalTime;
    System  ->m_dTotalTimeBefore = dTotalTimeBefore;
    System  ->m_iCurFrame        = iCurFrame;
    Graphics->SetView  (System->m_vResolution, fFOV, fNearClip, fFarClip);
    Graphics->SetCamera(vCamPosition, vCamDirection, vCamOrientation);
    for(coreUintW i = 0u; i < CORE_SYSTEM_TIMES;    ++i) System  ->SetTimeSpeed(i, afTimeSpeed[i]);
    for(coreUintW i = 0u; i < CORE_GRAPHICS_LIGHTS; ++i) Graphics->SetLight    (i, aLight[i].vPosition, aLight[i].vDirection, aLight[i].vValue);

    // start up managers
    Manager::Object->__Reset(CORE_RESOURCE_RESET_INIT);
    Manager::Resource->Reset(CORE_RESOURCE_RESET_INIT);

    // apply project settings
    System->SetWindowTitle(CoreApp::Settings::Name);
    System->SetWindowIcon (CoreApp::Settings::IconPath);
    Input ->SetCursor     (CoreApp::Settings::CursorPath);

    Log->Header("Reset finished");
}


// ****************************************************************
/* reshape engine */
void Core::Reshape()
{
    // reset view frustum
    Graphics->SetView(System->m_vResolution, Graphics->m_fFOV, Graphics->m_fNearClip, Graphics->m_fFarClip);

    // reshape resources
    Manager::Resource->Reshape();

    // refresh all existing 2d-objects
    Manager::Object->RefreshSprites();

    // apply project settings
    System->SetWindowTitle(CoreApp::Settings::Name);
}


// ****************************************************************
/* restart application */
void Core::Restart()
{
    // deferr actual restart
    s_bRestart = true;
}


// ****************************************************************
/* main function */
coreInt32 SDLCALL coreMain(coreInt32 argc, coreChar** argv)
{
    // disable synchronization between standard streams
    std::ios_base::sync_with_stdio(false);

    // set command line arguments
    coreData::SetCommandLine(argc, argv);
    coreData::InitDefaultFolders();

    // set floating-point behavior
    coreMath::EnableExceptions();
    coreMath::EnableRoundToNearest();
    coreMath::DisableDenormals();

    // run engine
    return Core::Run();
}


// ****************************************************************
/* run engine */
coreStatus Core::Run()
{
    // create engine instance
    Core Engine;

    // set logging level
    if(!Debug->IsEnabled())
    {
        Log->SetLevel(CORE_LOG_LEVEL_WARNING | CORE_LOG_LEVEL_ERROR);
        Log->Warning ("Logging level reduced");
    }

#if defined(_CORE_EMSCRIPTEN_)

    // execute with requestAnimationFrame (main loop)
    emscripten_set_main_loop([]()
    {
        // wait until loading is finished
        Manager::Resource->UpdateWait();
        if(Manager::Resource->IsLoading()) return;

        // switch to the inner loop
        emscripten_cancel_main_loop();
        emscripten_set_main_loop([]()
        {
            // update the Emscripten canvas
            Graphics->__UpdateEmscripten();

#else

    // execute until terminated (main loop)
    while(!System->GetTerminated())

#endif

    {
        // perform deferred application restart
        if(s_bRestart)
        {
            Core::__PerformRestart();
            s_bRestart = false;
        }

        // update the event system
        System->__UpdateEvents();

        // update the input button interface
        Input->__UpdateButtonsStart();

        // move and render the application
        Application->Move();
        Application->Render();

        // update the object manager
        Manager::Object->__UpdateObjects();

        // update all remaining components
        Audio   ->__UpdateSources();
        Platform->__UpdateBackend();
        Debug   ->__UpdateOutput();
        Graphics->__UpdateScene();   // # contains frame terminator
        System  ->__UpdateWindow();
        System  ->__UpdateTime();
        Input   ->__UpdateButtonsEnd();

        // update the resource manager on the main-thread
        if(!Manager::Resource->GetActive())
        {
            Manager::Resource->UpdateResources(0.004f);
            Manager::Resource->UpdateFunctions();
        }

        // unload all unreferenced resources
        Manager::Resource->ApplyNullify();
    }

#if defined(_CORE_EMSCRIPTEN_)

        }, 0, 1);
    }, 0, 1);

#endif

    // reset logging level
    Log->SetLevel(CORE_LOG_LEVEL_ALL);

    return CORE_OK;
}


// ****************************************************************
/* perform deferred application restart */
void Core::__PerformRestart()
{
    Log->Warning("Application Restart");

    // restart application
    Application->Exit();
    Application->Init();

    // reload all resources
    Manager::Resource->UpdateWait(CORE_RESOURCE_WAIT_STARTUP);

    Log->Header("Application Run");
}