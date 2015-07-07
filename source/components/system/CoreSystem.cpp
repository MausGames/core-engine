//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
CoreSystem::CoreSystem()noexcept
: m_vResolution (coreVector2(I_TO_F(Core::Config->GetInt(CORE_CONFIG_SYSTEM_WIDTH)), I_TO_F(Core::Config->GetInt(CORE_CONFIG_SYSTEM_HEIGHT))))
, m_iFullscreen (Core::Config->GetInt(CORE_CONFIG_SYSTEM_FULLSCREEN))
, m_bMinimized  (false)
, m_bTerminated (false)
, m_dTotalTime  (0.0f)
, m_fLastTime   (0.0f)
, m_iCurFrame   (0u)
, m_iSkipFrame  (2u)
{
    Core::Log->Header("System Interface");

    // get SDL version
    SDL_version oVersion;
    SDL_GetVersion(&oVersion);

    // init SDL libraries
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) || TTF_Init() || !IMG_Init(IMG_INIT_PNG))
        Core::Log->Error("SDL could not be initialized (SDL: %s)", SDL_GetError());
    else Core::Log->Info("SDL initialized (%d.%d.%d %s)", oVersion.major, oVersion.minor, oVersion.patch, SDL_GetRevision());

    // automatically shut down SDL libraries on exit
    std::atexit([]() {IMG_Quit(); TTF_Quit(); SDL_Quit();});

    // load all available displays
    const coreUintW iNumDisplays = SDL_GetNumVideoDisplays();
    if(iNumDisplays)
    {
        Core::Log->ListStartInfo("Available Displays");
        {
            // retrieve display
            for(coreUintW i = 0u; i < iNumDisplays; ++i)
                Core::Log->ListAdd(CORE_LOG_BOLD("%u:") " %s", i+1u, SDL_GetDisplayName(i));
        }
        Core::Log->ListEnd();
    }
    else Core::Log->Warning("Could not get available displays (SDL: %s)", SDL_GetError());

    // retrieve desktop resolution
    SDL_DisplayMode oDesktop;
    SDL_GetDesktopDisplayMode(0, &oDesktop);
    const coreVector2 vDesktop = coreVector2(I_TO_F(oDesktop.w), I_TO_F(oDesktop.h));

    // load all available screen resolutions
    const coreUintW iNumModes = SDL_GetNumDisplayModes(0);
    if(iNumModes)
    {
        Core::Log->ListStartInfo("Available Screen Resolutions");
        {
            for(coreUintW i = 0u; i < iNumModes; ++i)
            {
                // retrieve resolution
                SDL_DisplayMode oMode;
                SDL_GetDisplayMode(0, i, &oMode);
                const coreVector2 vMode = coreVector2(I_TO_F(oMode.w), I_TO_F(oMode.h));

                coreUintW j = 0u, je = m_avAvailable.size();
                for(; j < je; ++j)
                {
                    // check for already added resolutions
                    if(m_avAvailable[j] == vMode)
                        break;
                }
                if(j == je)
                {
                    // add new resolution
                    m_avAvailable.push_back(vMode);
                    Core::Log->ListAdd("%4d x %4d%s", oMode.w, oMode.h, (vMode == vDesktop) ? " (Desktop)" : "");
                }
            }
        }
        Core::Log->ListEnd();

        // override screen resolution
        if(m_avAvailable.size() == 1u) m_vResolution = m_avAvailable.back();
        if(!m_vResolution.x) m_vResolution.x = vDesktop.x;
        if(!m_vResolution.y) m_vResolution.y = vDesktop.y;
    }
    else Core::Log->Warning("Could not get available screen resolutions (SDL: %s)", SDL_GetError());

    // configure the SDL window
    const coreUint32 iCenter = (Core::Config->GetBool(CORE_CONFIG_SYSTEM_DEBUGMODE) || DEFINED(_CORE_DEBUG_)) ? 0u : SDL_WINDOWPOS_CENTERED;
    const coreUint32 iFlags  = SDL_WINDOW_OPENGL | (m_iFullscreen == 2u ? SDL_WINDOW_FULLSCREEN : (m_iFullscreen == 1u ? SDL_WINDOW_BORDERLESS : 0u));

    // configure the OpenGL context
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,                   8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,                 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,                  8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,                 Core::Config->GetBool(CORE_CONFIG_GRAPHICS_ALPHACHANNEL)    ? 8 : 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,                 Core::Config->GetInt (CORE_CONFIG_GRAPHICS_DEPTHSIZE));
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,               Core::Config->GetInt (CORE_CONFIG_GRAPHICS_STENCILSIZE));
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,               Core::Config->GetBool(CORE_CONFIG_GRAPHICS_DOUBLEBUFFER)    ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,         Core::Config->GetInt (CORE_CONFIG_GRAPHICS_ANTIALIASING)    ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,         Core::Config->GetInt (CORE_CONFIG_GRAPHICS_ANTIALIASING));
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, Core::Config->GetBool(CORE_CONFIG_GRAPHICS_RESOURCECONTEXT) ? 1 : 0);

    // check for core profile
    if(!Core::Config->GetBool(CORE_CONFIG_GRAPHICS_FALLBACKMODE) && !DEFINED(_CORE_GLES_))
    {
        // create quick test-window and -context
        m_pWindow = SDL_CreateWindow("OpenGL Test", -32, -32, 32, 32, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        if(m_pWindow)
        {
            SDL_GLContext pContext = SDL_GL_CreateContext(m_pWindow);
            if(pContext)
            {
                // get highest OpenGL version
                const coreFloat fVersion = coreData::StrVersion(r_cast<const coreChar*>(glGetString(GL_VERSION)));
                SDL_GL_DeleteContext(pContext);

                // set version and request core profile
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, F_TO_SI(fVersion));
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, F_TO_SI(fVersion*10.0f) % 10);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,  SDL_GL_CONTEXT_PROFILE_CORE);
            }
            SDL_DestroyWindow(m_pWindow);
        }
    }

    // check for debug context
    if(Core::Config->GetBool(CORE_CONFIG_SYSTEM_DEBUGMODE) || DEFINED(_CORE_DEBUG_))
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    // create main window object
    m_pWindow = SDL_CreateWindow(coreData::AppName(), iCenter, iCenter, F_TO_SI(m_vResolution.x), F_TO_SI(m_vResolution.y), iFlags);
    if(!m_pWindow)
    {
        Core::Log->Warning("Problems creating main window, trying different settings (SDL: %s)", SDL_GetError());

        // try compatible configuration
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,         16);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

        // create another main window object
        m_pWindow = SDL_CreateWindow(coreData::AppName(), iCenter, iCenter, F_TO_SI(m_vResolution.x), F_TO_SI(m_vResolution.y), iFlags);
        if(!m_pWindow) Core::Log->Error("Main window could not be created (SDL: %s)", SDL_GetError());
    }
    Core::Log->Info("Main window created (%.0f x %.0f / %d)", m_vResolution.x, m_vResolution.y, m_iFullscreen);

    // disable screen saver
    SDL_DisableScreenSaver();

    // grab mouse on fullscreen
    if(m_iFullscreen == 2u) SDL_SetWindowGrab(m_pWindow, SDL_TRUE);

    // init high-precision time
    m_dPerfFrequency = 1.0 / coreDouble(SDL_GetPerformanceFrequency());
    m_iPerfTime      = SDL_GetPerformanceCounter();

    // reset dynamic frame times
    for(coreUintW i = 0u; i < CORE_SYSTEM_TIMES; ++i)
    {
        m_afTime     [i] = 0.0f;
        m_afTimeSpeed[i] = 1.0f;
    }

    // log processor information
    Core::Log->ListStartInfo("Platform Information");
    {
        Core::Log->ListAdd(CORE_LOG_BOLD("Operating System:") " %s",                                             coreData::SystemName());
        Core::Log->ListAdd(CORE_LOG_BOLD("Processor:")        " %s (%s, %d Logical Cores, %d Bytes Cache Line)", coreCPUID::Brand(), coreCPUID::Vendor(), SDL_GetCPUCount(), SDL_GetCPUCacheLineSize());
        Core::Log->ListAdd(CORE_LOG_BOLD("System Memory:")    " %d MiB",                                         SDL_GetSystemRAM());
    }
    Core::Log->ListEnd();
}


// ****************************************************************
// destructor
CoreSystem::~CoreSystem()
{
    // clear memory
    m_avAvailable.clear();

    // delete main window object
    SDL_DestroyWindow(m_pWindow);

    Core::Log->Info(CORE_LOG_BOLD("System Interface shut down"));
}


// ****************************************************************
// change the icon of the window
void CoreSystem::SetWindowIcon(const coreChar* pcPath)
{
    coreFile* pFile = Core::Manager::Resource->RetrieveFile(pcPath);
    coreFileUnload oUnload(pFile);

    // load texture from file
    SDL_Surface* pData = IMG_LoadTyped_RW(SDL_RWFromConstMem(pFile->GetData(), pFile->GetSize()), true, coreData::StrExtension(pFile->GetPath()));
    if(pData)
    {
        // create icon and free the texture
        SDL_SetWindowIcon(Core::System->GetWindow(), pData);
        SDL_FreeSurface(pData);
    }
}


// ****************************************************************
// update the window event system
coreBool CoreSystem::__UpdateEvents()
{
    // reset minimize status
    m_bMinimized = false;

    // process events
    SDL_Event oEvent;
    while(SDL_PollEvent(&oEvent))
    {
        switch(oEvent.type)
        {
        // control window
        case SDL_WINDOWEVENT:
            switch(oEvent.window.event)
            {
            // minimize window
            case SDL_WINDOWEVENT_HIDDEN:
            case SDL_WINDOWEVENT_MINIMIZED:
            case SDL_WINDOWEVENT_MAXIMIZED:
            case SDL_WINDOWEVENT_RESTORED:
            case SDL_WINDOWEVENT_FOCUS_LOST:
                m_bMinimized = true;
                break;

            // close window
            case SDL_WINDOWEVENT_CLOSE:
                if(oEvent.window.windowID == SDL_GetWindowID(m_pWindow)) this->Quit();
                else SDL_DestroyWindow(SDL_GetWindowFromID(oEvent.window.windowID));
                break;
            }
            break;

        // minimize application
        case SDL_APP_WILLENTERBACKGROUND:
        case SDL_APP_DIDENTERFOREGROUND:
            m_bMinimized = true;
            break;

        // quit application
        case SDL_QUIT:
        case SDL_APP_TERMINATING:
            this->Quit();
            break;

        // forward event to input component
        default: if(!Core::Input->ProcessEvent(oEvent)) return true;
        }
    }

    return !m_bTerminated;
}


// ****************************************************************
// update the high-precision time
void CoreSystem::__UpdateTime()
{
    // measure and calculate last frame time
    const coreUint64 iNewPerfTime = SDL_GetPerformanceCounter();
    const coreFloat  fNewLastTime = coreFloat(coreDouble(iNewPerfTime - m_iPerfTime) * m_dPerfFrequency);
    m_iPerfTime                   = iNewPerfTime;

    if(m_iSkipFrame || fNewLastTime >= 0.25f)
    {
        Core::Log->Warning("Skipped Frame (%u:%f:%f)", m_iCurFrame, m_dTotalTime, fNewLastTime);

        // skip frames
        m_fLastTime = 0.0f;
        if(m_iSkipFrame) --m_iSkipFrame;
    }
    else
    {
        // smooth last frame time and increase total time
        m_fLastTime   = 0.85f * m_fLastTime + 0.15f * fNewLastTime;
        m_dTotalTime += coreDouble(m_fLastTime);
    }

    // update dynamic frame times
    for(coreUintW i = 0u; i < CORE_SYSTEM_TIMES; ++i)
        m_afTime[i] = m_fLastTime * m_afTimeSpeed[i];

    // increase current frame number
    ++m_iCurFrame;
}