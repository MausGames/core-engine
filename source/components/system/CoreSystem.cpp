///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
CoreSystem::CoreSystem()noexcept
: m_pWindow          (NULL)
, m_aDisplayData     {}
, m_iDisplayIndex    (Core::Config->GetInt(CORE_CONFIG_SYSTEM_DISPLAY))
, m_vResolution      (coreVector2(I_TO_F(Core::Config->GetInt(CORE_CONFIG_SYSTEM_WIDTH)), I_TO_F(Core::Config->GetInt(CORE_CONFIG_SYSTEM_HEIGHT))))
, m_iFullscreen      (Core::Config->GetInt(CORE_CONFIG_SYSTEM_FULLSCREEN))
, m_dTotalTime       (0.0)
, m_dTotalTimeBefore (0.0)
, m_fLastTime        (0.0f)
, m_afTime           {}
, m_afTimeSpeed      {}
, m_iCurFrame        (0u)
, m_iSkipFrame       (1u)
, m_dPerfFrequency   (0.0)
, m_iPerfTime        (0u)
, m_bWinFocusLost    (false)
, m_bWinPosChanged   (false)
, m_bWinSizeChanged  (false)
, m_bTerminated      (false)
{
    Core::Log->Header("System Interface");

    // set SDL behavior hints
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS,            "0");
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS,            "0");
    SDL_SetHint(SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING, "0");
    SDL_SetHint(SDL_HINT_VIDEO_DOUBLE_BUFFER,           "1");

    // get SDL version
    SDL_version oVersionSDL; SDL_GetVersion(&oVersionSDL);
    const SDL_version* pVersionTTF = TTF_Linked_Version();
    const SDL_version* pVersionIMG = IMG_Linked_Version();

    // init SDL libraries
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | (Core::Config->GetBool(CORE_CONFIG_INPUT_HAPTIC) ? SDL_INIT_HAPTIC : 0u)) || TTF_Init() || !IMG_Init(IMG_INIT_PNG))
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
            for(coreUintW i = 0u; i < iNumDisplays; ++i)
            {
                m_aDisplayData.emplace_back();
                coreDisplay& oDisplayData = m_aDisplayData.back();

                // retrieve desktop resolution
                SDL_DisplayMode oDesktop = {};
                SDL_GetDesktopDisplayMode(i, &oDesktop);
                oDisplayData.vDesktopRes = coreVector2(I_TO_F(oDesktop.w), I_TO_F(oDesktop.h));

                // retrieve work area resolution
                SDL_Rect oWorkArea = {};
                SDL_GetDisplayUsableBounds(i, &oWorkArea);
                oDisplayData.vWorkAreaRes = coreVector2(I_TO_F(oWorkArea.w), I_TO_F(oWorkArea.h));

                // retrieve display DDPI
                coreFloat fDDPI = 0.0f;
                SDL_GetDisplayDPI(i, &fDDPI, NULL, NULL);

                // load all available screen resolutions
                const coreUintW iNumModes = SDL_GetNumDisplayModes(i);
                if(iNumModes)
                {
                    Core::Log->ListDeeper(CORE_LOG_BOLD("Display %u:") " %s (%d Hz, %.1f DDPI)", i+1u, SDL_GetDisplayName(i), oDesktop.refresh_rate, fDDPI);
                    {
                        for(coreUintW j = 0u; j < iNumModes; ++j)
                        {
                            // retrieve resolution
                            SDL_DisplayMode oMode = {};
                            SDL_GetDisplayMode(i, j, &oMode);
                            const coreVector2 vModeRes = coreVector2(I_TO_F(oMode.w), I_TO_F(oMode.h));

                            // add new resolution
                            if(!oDisplayData.avAvailableRes.count(vModeRes))
                            {
                                oDisplayData.avAvailableRes.push_back(vModeRes);
                                Core::Log->ListAdd("%4d x %4d%s", oMode.w, oMode.h, (vModeRes == oDisplayData.vDesktopRes) ? " (Desktop)" : "");
                            }
                        }

                        // copy highest resolution
                        oDisplayData.vMaximumRes = oDisplayData.avAvailableRes.front();

                        // reduce memory consumption
                        oDisplayData.avAvailableRes.shrink_to_fit();
                    }
                    Core::Log->ListEnd();
                }
                else Core::Log->Warning("Could not get available screen resolutions (SDL: %s)", SDL_GetError());
            }
        }
        Core::Log->ListEnd();
    }
    else Core::Log->Warning("Could not get available displays (SDL: %s)", SDL_GetError());

    // sanitize display data
    if(m_aDisplayData.empty()) m_aDisplayData.emplace_back();
    FOR_EACH(it, m_aDisplayData)
    {
        coreDisplay& oDisplayData = (*it);

        if(oDisplayData.avAvailableRes.empty ()) oDisplayData.avAvailableRes.push_back(m_vResolution);
        if(oDisplayData.vDesktopRes   .IsNull()) oDisplayData.vDesktopRes  = m_vResolution;
        if(oDisplayData.vWorkAreaRes  .IsNull()) oDisplayData.vWorkAreaRes = m_vResolution;
        if(oDisplayData.vMaximumRes   .IsNull()) oDisplayData.vMaximumRes  = m_vResolution;
    }

    // sanitize display index and screen resolution
    if(m_iDisplayIndex >= m_aDisplayData.size()) m_iDisplayIndex = 0u;
    this->SetWindowResolution(m_vResolution);

    // configure the OpenGL context
    SDL_GL_ResetAttributes();
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,           8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,         8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,          8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,         Core::Application->Settings.RenderBuffer.AlphaChannel   ? 8 : 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,         Core::Application->Settings.RenderBuffer.DepthSize);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,       Core::Application->Settings.RenderBuffer.StencilSize);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,       Core::Application->Settings.RenderBuffer.DoubleBuffer   ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_STEREO,             Core::Application->Settings.RenderBuffer.StereoRender   ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, Core::Config->GetInt(CORE_CONFIG_GRAPHICS_ANTIALIASING) ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, Core::Config->GetInt(CORE_CONFIG_GRAPHICS_ANTIALIASING));

    // check for core profile
    if(!Core::Config->GetBool(CORE_CONFIG_BASE_FALLBACKMODE) && !DEFINED(_CORE_GLES_))
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
    if(Core::Config->GetBool(CORE_CONFIG_BASE_ASYNCMODE) && DEFINED(_CORE_ASYNC_))
    {
        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_RELEASE_BEHAVIOR,   SDL_GL_CONTEXT_RELEASE_BEHAVIOR_NONE);
    }

    // check for debug context
    if(Core::Config->GetBool(CORE_CONFIG_BASE_DEBUGMODE) || DEFINED(_CORE_DEBUG_))
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    }
    else
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_NO_ERROR, 1);
    }

    // define window properties
    const coreInt32  iPos   = SDL_WINDOWPOS_CENTERED_DISPLAY(m_iDisplayIndex);
    const coreInt32  iSizeX = F_TO_SI(m_vResolution.x);
    const coreInt32  iSizeY = F_TO_SI(m_vResolution.y);
    const coreUint32 iFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | (m_iFullscreen == 2u ? (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_INPUT_GRABBED) : (m_iFullscreen == 1u ? SDL_WINDOW_BORDERLESS : SDL_WINDOW_RESIZABLE));

    // create main window object
    m_pWindow = SDL_CreateWindow(coreData::AppName(), iPos, iPos, iSizeX, iSizeY, iFlags);
    if(!m_pWindow)
    {
        Core::Log->Warning("Problems creating main window, trying different settings (SDL: %s)", SDL_GetError());

        // try compatible configuration
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,         16);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,      0);

        // create another main window object
        m_pWindow = SDL_CreateWindow(coreData::AppName(), iPos, iPos, iSizeX, iSizeY, iFlags);
        if(!m_pWindow) Core::Log->Error("Main window could not be created (SDL: %s)", SDL_GetError());
    }
    Core::Log->Info("Main window created (%.0f x %.0f, %d)", m_vResolution.x, m_vResolution.y, m_iFullscreen);

    // restrict window size
    SDL_SetWindowMinimumSize(m_pWindow, CORE_SYSTEM_WINDOW_MINIMUM, CORE_SYSTEM_WINDOW_MINIMUM);

    // execute main-thread with higher priority
    SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);

    // disable screen saver
    SDL_DisableScreenSaver();

    // ignore all events created during initialization
    SDL_FlushEvents(SDL_FIRSTEVENT, SDL_LASTEVENT);

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
        Core::Log->ListAdd(CORE_LOG_BOLD("Processor:")        " %s (%s, %d logical cores, %d bytes cache line)", coreCPUID::Brand(), coreCPUID::Vendor(), SDL_GetCPUCount(), SDL_GetCPUCacheLineSize());
        Core::Log->ListAdd(CORE_LOG_BOLD("System Memory:")    " %d MB",                                          SDL_GetSystemRAM());
    }
    Core::Log->ListEnd();

    // log Zstandard library version
    Core::Log->Info("Zstandard initialized (%s)", ZSTD_versionString());
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
// change the title of the window
void CoreSystem::SetWindowTitle(const coreChar* pcTitle)
{
    if(Core::Config->GetBool(CORE_CONFIG_BASE_DEBUGMODE) || DEFINED(_CORE_DEBUG_))
    {
        // set new title (with additional info)
        SDL_SetWindowTitle(m_pWindow, PRINT("%s (%.0f x %.0f)", pcTitle, m_vResolution.x, m_vResolution.y));
    }
    else
    {
        // set new title
        SDL_SetWindowTitle(m_pWindow, pcTitle);
    }
}


// ****************************************************************
// change the icon of the window
void CoreSystem::SetWindowIcon(const coreChar* pcPath)
{
    // retrieve texture file
    coreFileScope pFile = Core::Manager::Resource->RetrieveFile(pcPath);

    // decompress file to plain pixel data
    coreSurfaceScope pData = IMG_LoadTyped_RW(SDL_RWFromConstMem(pFile->GetData(), pFile->GetSize()), true, coreData::StrExtension(pcPath));
    if(!pData)
    {
        Core::Log->Warning("Icon (%s) could not be loaded (SDL: %s)", pcPath, SDL_GetError());
        return;
    }

    // set new window icon
    SDL_SetWindowIcon(m_pWindow, pData);

    Core::Log->Info("Icon (%s) loaded", pcPath);
}


// ****************************************************************
// change the resolution of the window (safely)
void CoreSystem::SetWindowResolution(const coreVector2& vResolution)
{
    const coreDisplay& oCurrent = m_aDisplayData[m_iDisplayIndex];

    // save new resolution
    m_vResolution = vResolution;

    // override in specific situations
    if(oCurrent.avAvailableRes.size() == 1u) m_vResolution = oCurrent.avAvailableRes.front();
    if(m_vResolution.x <= 0.0f) m_vResolution.x = oCurrent.vDesktopRes.x;
    if(m_vResolution.y <= 0.0f) m_vResolution.y = oCurrent.vDesktopRes.y;

    // clamp to bounds
    const coreVector2 vMaximum = (m_iFullscreen == 2u ? oCurrent.vMaximumRes : (m_iFullscreen == 1u ? oCurrent.vDesktopRes : (oCurrent.vWorkAreaRes - I_TO_F(CORE_SYSTEM_WINDOW_BORDER))));
    m_vResolution.x = CLAMP(m_vResolution.x, I_TO_F(CORE_SYSTEM_WINDOW_MINIMUM), vMaximum.x);
    m_vResolution.y = CLAMP(m_vResolution.y, I_TO_F(CORE_SYSTEM_WINDOW_MINIMUM), vMaximum.y);

    if(m_pWindow)
    {
        // define window properties
        const coreInt32 iPos   = SDL_WINDOWPOS_CENTERED_DISPLAY(m_iDisplayIndex);
        const coreInt32 iSizeX = F_TO_SI(m_vResolution.x);
        const coreInt32 iSizeY = F_TO_SI(m_vResolution.y);

        // set new size and position
        SDL_SetWindowSize    (m_pWindow, iSizeX, iSizeY);
        SDL_SetWindowPosition(m_pWindow, iPos,   iPos);
    }
}


// ****************************************************************
// update the event system
coreBool CoreSystem::__UpdateEvents()
{
    // reset window states
    m_bWinFocusLost   = false;
    m_bWinPosChanged  = false;
    m_bWinSizeChanged = false;

    // process events
    SDL_Event oEvent;
    while(SDL_PollEvent(&oEvent))
    {
        switch(oEvent.type)
        {
        // process window events
        case SDL_WINDOWEVENT:
            switch(oEvent.window.event)
            {
            // window moved
            case SDL_WINDOWEVENT_MOVED:
                m_bWinPosChanged = true;
                m_iDisplayIndex  = CLAMP(SDL_GetWindowDisplayIndex(m_pWindow), 0, coreInt32(m_aDisplayData.size()) - 1);
                Core::Config->SetInt(CORE_CONFIG_SYSTEM_DISPLAY, m_iDisplayIndex);
                break;

            // window size changed
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                m_bWinSizeChanged = true;
                m_vResolution     = coreVector2(I_TO_F(oEvent.window.data1), I_TO_F(oEvent.window.data2));
                Core::Config->SetInt(CORE_CONFIG_SYSTEM_WIDTH,  F_TO_SI(m_vResolution.x));
                Core::Config->SetInt(CORE_CONFIG_SYSTEM_HEIGHT, F_TO_SI(m_vResolution.y));
                break;

            // window focus lost
            case SDL_WINDOWEVENT_HIDDEN:
            case SDL_WINDOWEVENT_MINIMIZED:
            case SDL_WINDOWEVENT_MAXIMIZED:
            case SDL_WINDOWEVENT_RESTORED:
            case SDL_WINDOWEVENT_FOCUS_LOST:
                m_bWinFocusLost = true;
                break;

            // window closed
            case SDL_WINDOWEVENT_CLOSE:
                if(oEvent.window.windowID == SDL_GetWindowID(m_pWindow)) this->Quit();
                else SDL_DestroyWindow(SDL_GetWindowFromID(oEvent.window.windowID));
                break;
            }
            break;

        // application focus lost
        case SDL_APP_WILLENTERBACKGROUND:
        case SDL_APP_DIDENTERFOREGROUND:
            m_bWinFocusLost = true;
            break;

        // application closed
        case SDL_QUIT:
        case SDL_APP_TERMINATING:
            this->Quit();
            break;

        // joystick detached
        case SDL_JOYDEVICEREMOVED:
            m_bWinFocusLost = true;
            FALLTHROUGH

        // forward event to input component
        default: if(!Core::Input->ProcessEvent(oEvent)) return true;
        }
    }

    return !m_bTerminated;
}


// ****************************************************************
// update the main window
void CoreSystem::__UpdateWindow()
{
    // reduce overhead if window is not visible
    if(SDL_GetWindowFlags(m_pWindow) & (SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED))
        SDL_Delay(100u);

    // toggle between borderless and windowed mode
    if(Core::Input->GetKeyboardButton(CORE_INPUT_KEY(LALT),   CORE_INPUT_HOLD) &&
       Core::Input->GetKeyboardButton(CORE_INPUT_KEY(RETURN), CORE_INPUT_PRESS))
    {
        // set new configuration value
        m_iFullscreen = m_iFullscreen ? 0u : 1u;
        Core::Config->SetInt(CORE_CONFIG_SYSTEM_FULLSCREEN, m_iFullscreen);

        // set new window appearance
        SDL_SetWindowFullscreen(m_pWindow, 0u);
        SDL_SetWindowBordered  (m_pWindow, m_iFullscreen ? SDL_FALSE : SDL_TRUE);
        SDL_SetWindowResizable (m_pWindow, m_iFullscreen ? SDL_FALSE : SDL_TRUE);

        // change resolution to fit desktop (always)
        this->SetWindowResolution(m_aDisplayData[m_iDisplayIndex].vDesktopRes);
    }
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
        Core::Log->Warning("Skipped Frame (%u / %.5f / %.5f)", m_iCurFrame, m_dTotalTime, fNewLastTime);

        // skip frames
        m_fLastTime = 0.0f;
        if(m_iSkipFrame) --m_iSkipFrame;
    }
    else
    {
        // save total time of the previous frame
        m_dTotalTimeBefore = m_dTotalTime;

        // smooth last frame time and increase total time
        m_fLastTime   = LERP(m_fLastTime, fNewLastTime, 0.15f);
        m_dTotalTime += coreDouble(m_fLastTime);
    }

    // update dynamic frame times
    for(coreUintW i = 0u; i < CORE_SYSTEM_TIMES; ++i)
        m_afTime[i] = m_fLastTime * m_afTimeSpeed[i];

    // increase current frame number
    ++m_iCurFrame;
}