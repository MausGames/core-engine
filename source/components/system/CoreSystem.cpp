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
: m_pWindow          (NULL)
, m_aDisplayData     {}
, m_iDisplayIndex    (Core::Config->GetInt(CORE_CONFIG_SYSTEM_DISPLAY))
, m_vResolution      (coreVector2(I_TO_F(Core::Config->GetInt(CORE_CONFIG_SYSTEM_WIDTH)), I_TO_F(Core::Config->GetInt(CORE_CONFIG_SYSTEM_HEIGHT))))
, m_iFullscreen      (Core::Config->GetInt(CORE_CONFIG_SYSTEM_FULLSCREEN))
, m_bMinimized       (false)
, m_bTerminated      (false)
, m_dTotalTime       (0.0)
, m_dTotalTimeBefore (0.0)
, m_fLastTime        (0.0f)
, m_afTime           {}
, m_afTimeSpeed      {}
, m_iCurFrame        (0u)
, m_iSkipFrame       (1u)
, m_dPerfFrequency   (0.0)
, m_iPerfTime        (0u)
{
    Core::Log->Header("System Interface");

    // get SDL version
    SDL_version oVersionSDL; SDL_GetVersion(&oVersionSDL);
    const SDL_version* pVersionTTF = TTF_Linked_Version();
    const SDL_version* pVersionIMG = IMG_Linked_Version();

    // init SDL libraries
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC) || TTF_Init() || !IMG_Init(IMG_INIT_PNG))
    {
        Core::Log->Error("SDL could not be initialized (SDL: %s)", SDL_GetError());
    }
    else
    {
        Core::Log->Info("SDL initialized (%d.%d.%d %s, TTF %d.%d.%d, IMG %d.%d.%d)",
                        oVersionSDL .major, oVersionSDL .minor, oVersionSDL .patch, SDL_GetRevision(),
                        pVersionTTF->major, pVersionTTF->minor, pVersionTTF->patch,
                        pVersionIMG->major, pVersionIMG->minor, pVersionIMG->patch);
    }

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
            {
                m_aDisplayData.emplace_back();
                coreDisplay& oDisplayData = m_aDisplayData.back();

                // retrieve display DDPI
                coreFloat fDDPI;
                SDL_GetDisplayDPI(i, &fDDPI, NULL, NULL);

                // retrieve desktop resolution
                SDL_DisplayMode oDesktop;
                SDL_GetDesktopDisplayMode(i, &oDesktop);
                oDisplayData.vDesktopRes = coreVector2(I_TO_F(oDesktop.w), I_TO_F(oDesktop.h));

                // load all available screen resolutions
                const coreUintW iNumModes = SDL_GetNumDisplayModes(i);
                if(iNumModes)
                {
                    Core::Log->ListDeeper(CORE_LOG_BOLD("Display %u:") " %s (%d Hz, %.1f DDPI)", i+1u, SDL_GetDisplayName(i), oDesktop.refresh_rate, fDDPI);
                    {
                        for(coreUintW j = 0u; j < iNumModes; ++j)
                        {
                            // retrieve resolution
                            SDL_DisplayMode oMode;
                            SDL_GetDisplayMode(i, j, &oMode);
                            const coreVector2 vMode = coreVector2(I_TO_F(oMode.w), I_TO_F(oMode.h));

                            coreUintW k = 0u, ke = oDisplayData.avAvailableRes.size();
                            for(; k < ke; ++k)
                            {
                                // check for already added resolutions
                                if(oDisplayData.avAvailableRes[k] == vMode)
                                    break;
                            }
                            if(k == ke)
                            {
                                // add new resolution
                                oDisplayData.avAvailableRes.push_back(vMode);
                                Core::Log->ListAdd("%4d x %4d%s", oMode.w, oMode.h, (vMode == oDisplayData.vDesktopRes) ? " (Desktop)" : "");
                            }
                        }
                    }
                    Core::Log->ListEnd();
                }
                else Core::Log->Warning("Could not get available screen resolutions (SDL: %s)", SDL_GetError());
            }
        }
        Core::Log->ListEnd();

        // clamp display index and resolution
        if(m_iDisplayIndex >= m_aDisplayData.size()) m_iDisplayIndex = 0u;
        const coreDisplay& oPrimary = m_aDisplayData[m_iDisplayIndex];

        if(!Core::Config->GetBool(CORE_CONFIG_SYSTEM_DEBUGMODE) && !DEFINED(_CORE_DEBUG_))
        {
            m_vResolution.x = CLAMP(m_vResolution.x, 0.0f, oPrimary.vDesktopRes.x);
            m_vResolution.y = CLAMP(m_vResolution.y, 0.0f, oPrimary.vDesktopRes.y);
        }

        // override screen resolution
        if(oPrimary.avAvailableRes.size() == 1u) m_vResolution = oPrimary.avAvailableRes.back();
        if(!m_vResolution.x) m_vResolution.x = oPrimary.vDesktopRes.x;
        if(!m_vResolution.y) m_vResolution.y = oPrimary.vDesktopRes.y;
    }
    else Core::Log->Warning("Could not get available displays (SDL: %s)", SDL_GetError());

    // configure the OpenGL context
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,           8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,         8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,          8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,         Core::Config->GetBool(CORE_CONFIG_GRAPHICS_ALPHACHANNEL) ? 8 : 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,         Core::Config->GetInt (CORE_CONFIG_GRAPHICS_DEPTHSIZE));
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,       Core::Config->GetInt (CORE_CONFIG_GRAPHICS_STENCILSIZE));
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,       Core::Config->GetBool(CORE_CONFIG_GRAPHICS_DOUBLEBUFFER) ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, Core::Config->GetInt (CORE_CONFIG_GRAPHICS_ANTIALIASING) ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, Core::Config->GetInt (CORE_CONFIG_GRAPHICS_ANTIALIASING));

    // check for core profile
    if(!Core::Config->GetBool(CORE_CONFIG_GRAPHICS_FALLBACKMODE) && !DEFINED(_CORE_GLES_))
    {
        // create quick test-window and -context
        m_pWindow = SDL_CreateWindow("OpenGL Test", 0, 0, 32, 32, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        if(m_pWindow)
        {
            const SDL_GLContext pContext = SDL_GL_CreateContext(m_pWindow);
            if(pContext)
            {
                // get highest OpenGL version
                const coreFloat fVersion = coreData::StrVersion(r_cast<const coreChar*>(glGetString(GL_VERSION)));
                SDL_GL_DeleteContext(pContext);

                // set version and request core profile
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, F_TO_SI(fVersion));
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, F_TO_SI(fVersion*10.0f) % 10);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,  SDL_GL_CONTEXT_PROFILE_CORE);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,         SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);   // overwritten by debug context
            }
            SDL_DestroyWindow(m_pWindow);
        }
    }

    // check for shared context
    if(Core::Config->GetBool(CORE_CONFIG_GRAPHICS_RESOURCECONTEXT) && DEFINED(_CORE_PARALLEL_))
    {
        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_RELEASE_BEHAVIOR,   SDL_GL_CONTEXT_RELEASE_BEHAVIOR_NONE);
    }

    // check for debug context
    if(Core::Config->GetBool(CORE_CONFIG_SYSTEM_DEBUGMODE) || DEFINED(_CORE_DEBUG_))
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    // configure the SDL window
    const coreInt32  iPos   = (Core::Config->GetBool(CORE_CONFIG_SYSTEM_DEBUGMODE) || DEFINED(_CORE_DEBUG_)) ? 0 : SDL_WINDOWPOS_CENTERED_DISPLAY(m_iDisplayIndex);
    const coreInt32  iSizeX = F_TO_SI(m_vResolution.x);
    const coreInt32  iSizeY = F_TO_SI(m_vResolution.y);
    const coreUint32 iFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | (m_iFullscreen == 2u ? (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_INPUT_GRABBED) : (m_iFullscreen == 1u ? SDL_WINDOW_BORDERLESS : 0u));

    // create main window object
    m_pWindow = SDL_CreateWindow(coreData::AppName(), iPos, iPos, iSizeX, iSizeY, iFlags);
    if(!m_pWindow)
    {
        Core::Log->Warning("Problems creating main window, trying different settings (SDL: %s)", SDL_GetError());

        // try compatible configuration
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,         16);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

        // create another main window object
        m_pWindow = SDL_CreateWindow(coreData::AppName(), iPos, iPos, iSizeX, iSizeY, iFlags);
        if(!m_pWindow) Core::Log->Error("Main window could not be created (SDL: %s)", SDL_GetError());
    }
    Core::Log->Info("Main window created (%.0f x %.0f / %d)", m_vResolution.x, m_vResolution.y, m_iFullscreen);

    // disable screen saver
    SDL_DisableScreenSaver();

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

    // log zlib library version
    Core::Log->Info("zlib initialized (%s 0x%08X)", zlibVersion(), zlibCompileFlags());
}


// ****************************************************************
// destructor
CoreSystem::~CoreSystem()
{
    // clear memory
    m_aDisplayData.clear();

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
        case SDL_JOYDEVICEREMOVED:
        case SDL_CONTROLLERDEVICEREMOVED:
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

    if(m_iSkipFrame || (fNewLastTime >= 0.25f))
    {
        Core::Log->Warning("Skipped Frame (%u:%f:%f)", m_iCurFrame, m_dTotalTime, fNewLastTime);

        // skip frames
        m_fLastTime = 0.0f;
        if(m_iSkipFrame) --m_iSkipFrame;
    }
    else
    {
        // save total time of the previous frame
        m_dTotalTimeBefore = m_dTotalTime;

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