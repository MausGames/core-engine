///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
CoreSystem::CoreSystem()noexcept
: m_pWindow          (NULL)
, m_aDisplayData     {}
, m_iDisplayIndex    (Core::Config->GetInt(CORE_CONFIG_SYSTEM_DISPLAY))
, m_vResolution      (coreVector2(I_TO_F(Core::Config->GetInt(CORE_CONFIG_SYSTEM_WIDTH)), I_TO_F(Core::Config->GetInt(CORE_CONFIG_SYSTEM_HEIGHT))))
, m_eMode            (coreSystemMode(Core::Config->GetInt(CORE_CONFIG_SYSTEM_FULLSCREEN)))
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
    SDL_SetHint(SDL_HINT_APP_NAME,                           CoreApp::Settings::Name);
    SDL_SetHint(SDL_HINT_EVENT_LOGGING,                      DEFINED(_CORE_DEBUG_) ? "1" : "0");
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,   "1");
    SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE,                 "0");
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_CENTER,         "0");
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
    SDL_SetHint(SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING,      DEFINED(_CORE_DEBUG_) ? "0" : "1");

    // load SDL only once (to improve reset performance, and prevent crashes)
    UNUSED static const coreBool s_bOnce = []()
    {
        // enable SDL debug messages
        SDL_LogSetAllPriority((Core::Config->GetBool(CORE_CONFIG_BASE_DEBUGMODE) || DEFINED(_CORE_DEBUG_)) ? SDL_LOG_PRIORITY_INFO : SDL_LOG_PRIORITY_WARN);

        // get default log callback (standard output)
        SDL_LogOutputFunction nOldFunc;
        SDL_LogGetOutputFunction(&nOldFunc, NULL);

        // register new log callback
        SDL_LogSetOutputFunction([](void* pUserData, const coreInt32 iCategory, const SDL_LogPriority ePriority, const coreChar* pcMessage)
        {
            if((iCategory == SDL_LOG_CATEGORY_APPLICATION) || (iCategory == SDL_LOG_CATEGORY_ASSERT))
            {
                // forward text to the standard output
                r_cast<SDL_LogOutputFunction>(pUserData)(NULL, iCategory, ePriority, pcMessage);
            }
            else
            {
                // forward text to the log file
                Core::Log->Warning(CORE_LOG_BOLD("SDL:") " %s (category %d, priority %d)", pcMessage, iCategory, ePriority);
            }
        },
        r_cast<void*>(nOldFunc));

        // get SDL version
        SDL_version oVersionSDL; SDL_GetVersion(&oVersionSDL);
        const SDL_version* pVersionTTF = TTF_Linked_Version();
        const SDL_version* pVersionIMG = IMG_Linked_Version();

        // init SDL libraries
        if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) || TTF_Init() || !IMG_Init(IMG_INIT_PNG) || SDL_GL_LoadLibrary(NULL))
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
        WARN_IF(std::atexit([]() {SDL_GL_UnloadLibrary(); IMG_Quit(); TTF_Quit(); SDL_Quit();})) {}
        return true;
    }();

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
                coreFloat fDDPI = 0.0f, fHDPI = 0.0f, fVDPI = 0.0f;
                SDL_GetDisplayDPI(i, &fDDPI, &fHDPI, &fVDPI);

                // load all available screen resolutions
                const coreUintW iNumModes = SDL_GetNumDisplayModes(i);
                if(iNumModes)
                {
                    Core::Log->ListDeeper(CORE_LOG_BOLD("Display %u:") " %s (%d Hz, %.1f/%.1f/%.1f DPI, %d/%d offset)", i, SDL_GetDisplayName(i), oDesktop.refresh_rate, fDDPI, fHDPI, fVDPI, oWorkArea.x, oWorkArea.y);
                    {
                        // reserve some memory
                        oDisplayData.avAvailableRes.reserve(iNumModes);

                        for(coreUintW j = 0u; j < iNumModes; ++j)
                        {
                            // retrieve resolution
                            SDL_DisplayMode oMode = {};
                            SDL_GetDisplayMode(i, j, &oMode);

                            // add new resolution
                            const coreVector2 vModeRes = coreVector2(I_TO_F(oMode.w), I_TO_F(oMode.h));
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
        if(oDisplayData.vDesktopRes   .IsNull()) oDisplayData.vDesktopRes  = oDisplayData.avAvailableRes.front();
        if(oDisplayData.vWorkAreaRes  .IsNull()) oDisplayData.vWorkAreaRes = oDisplayData.avAvailableRes.front();
        if(oDisplayData.vMaximumRes   .IsNull()) oDisplayData.vMaximumRes  = oDisplayData.avAvailableRes.front();
    }

    // sanitize display index
    if(m_iDisplayIndex >= m_aDisplayData.size()) m_iDisplayIndex = 0u;
    const coreDisplay& oPrimary = m_aDisplayData[m_iDisplayIndex];

    // sanitize screen resolution
    if(oPrimary.avAvailableRes.size() == 1u) m_vResolution = oPrimary.vWorkAreaRes;
    this->SetWindowResolution(m_vResolution);

    // sanitize fullscreen mode
    if(oPrimary.avAvailableRes.size() == 1u) m_eMode = CORE_SYSTEM_MODE_WINDOWED;
    m_eMode = CLAMP(m_eMode, CORE_SYSTEM_MODE_WINDOWED, CORE_SYSTEM_MODE_FULLSCREEN);

    // configure the OpenGL context
    SDL_GL_ResetAttributes();
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,           8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,         8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,          8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,         CoreApp::Settings::Graphics::AlphaChannel               ? 8 : 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,         CoreApp::Settings::Graphics::DepthSize);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,       CoreApp::Settings::Graphics::StencilSize);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,       CoreApp::Settings::Graphics::DoubleBuffer               ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_STEREO,             CoreApp::Settings::Graphics::StereoRender               ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, Core::Config->GetInt(CORE_CONFIG_GRAPHICS_ANTIALIASING) ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, Core::Config->GetInt(CORE_CONFIG_GRAPHICS_ANTIALIASING));

#if defined(_CORE_GLES_)

    // always request ES profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    // check for highest OpenGL ES version
    if(!Core::Config->GetBool(CORE_CONFIG_BASE_FALLBACKMODE))
    {
        // create quick test-window
        m_pWindow = SDL_CreateWindow(NULL, 0, 0, 32, 32, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        if(m_pWindow)
        {
            // test all available versions (descending)
            constexpr coreInt32 aaiVersion[][2] = {{3, 2}, {3, 1}, {3, 0}, {2, 0}};

            for(coreUintW i = 0u; i < ARRAY_SIZE(aaiVersion); ++i)
            {
                // try to use current version
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, aaiVersion[i][0]);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, aaiVersion[i][1]);

                // create quick test-context
                const SDL_GLContext pContext = SDL_GL_CreateContext(m_pWindow);
                if(pContext)
                {
                    // select current version
                    SDL_GL_DeleteContext(pContext);
                    break;
                }
            }
            SDL_DestroyWindow(m_pWindow);
        }
    }

#else

    // check for core profile
    if(!Core::Config->GetBool(CORE_CONFIG_BASE_FALLBACKMODE))
    {
        // create quick test-window and test-context
        m_pWindow = SDL_CreateWindow(NULL, 0, 0, 32, 32, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
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
    if(Core::Config->GetBool(CORE_CONFIG_BASE_ASYNCMODE) && !DEFINED(_CORE_EMSCRIPTEN_))
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

#endif

    // define window properties
    const coreBool   bDesktop = (m_vResolution == m_aDisplayData[m_iDisplayIndex].vDesktopRes);
    const coreInt32  iPos     = SDL_WINDOWPOS_CENTERED_DISPLAY(m_iDisplayIndex);
    const coreInt32  iSizeX   = F_TO_SI(m_vResolution.x);
    const coreInt32  iSizeY   = F_TO_SI(m_vResolution.y);
    const coreUint32 iFlags   = SDL_WINDOW_OPENGL | ((m_eMode == CORE_SYSTEM_MODE_FULLSCREEN) ? (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_MOUSE_GRABBED) : ((m_eMode == CORE_SYSTEM_MODE_BORDERLESS) ? (bDesktop ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_BORDERLESS) : SDL_WINDOW_RESIZABLE));

    // create main window object
    m_pWindow = SDL_CreateWindow(NULL, iPos, iPos, iSizeX, iSizeY, iFlags);
    if(!m_pWindow)
    {
        Core::Log->Warning("Problems creating main window, trying different settings (SDL: %s)", SDL_GetError());

        // try compatible configuration
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,         16);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

        // create another main window object
        m_pWindow = SDL_CreateWindow(NULL, iPos, iPos, iSizeX, iSizeY, iFlags);
        if(!m_pWindow) Core::Log->Error("Main window could not be created (SDL: %s)", SDL_GetError());
    }
    Core::Log->Info("Main window created (%s, %s, %.0f x %.0f, display %u, mode %u)", SDL_GetCurrentVideoDriver(), SDL_GetPixelFormatName(SDL_GetWindowPixelFormat(m_pWindow)), m_vResolution.x, m_vResolution.y, m_iDisplayIndex, m_eMode);

    // restrict window size
    SDL_SetWindowMinimumSize(m_pWindow, CORE_SYSTEM_WINDOW_MINIMUM, CORE_SYSTEM_WINDOW_MINIMUM);

    // execute main-thread with higher priority
    SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH);

    // disable screen saver
    SDL_DisableScreenSaver();

    // disable unwanted events
    constexpr coreUint32 aiDisable[] = {SDL_DROPFILE, SDL_DROPTEXT, SDL_DROPBEGIN, SDL_DROPCOMPLETE, SDL_KEYMAPCHANGED};
    for(coreUintW i = 0u; i < ARRAY_SIZE(aiDisable); ++i) SDL_EventState(aiDisable[i], SDL_DISABLE);

    // remove all events created during initialization
    SDL_PumpEvents();
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

    // log platform information
    Core::Log->ListStartInfo("Platform Information");
    {
        coreUint64 iMemoryAvailable, iMemoryTotal, iSpaceAvailable;
        coreData::SystemMemory(&iMemoryAvailable, &iMemoryTotal);   // after window and context creation
        coreData::SystemSpace (&iSpaceAvailable,  NULL);

        const coreUint64 iMemoryUsed = iMemoryTotal - iMemoryAvailable;
        const coreDouble dMemoryPct  = 100.0 * (coreDouble(iMemoryUsed) / coreDouble(MAX(iMemoryTotal, 1u)));

        coreString  sLocaleStr  = "";
        SDL_Locale* pLocaleList = SDL_GetPreferredLocales();
        if(pLocaleList)
        {
            for(SDL_Locale* pCurrent = pLocaleList; pCurrent->language; ++pCurrent)
            {
                sLocaleStr += pCurrent->country ? PRINT("%s_%s", pCurrent->language, pCurrent->country) : pCurrent->language;
                sLocaleStr += ' ';
            }
            SDL_free(pLocaleList);
        }

        Core::Log->ListAdd(CORE_LOG_BOLD("Operating System:")  " %s",                                             coreData::SystemOsName());
        Core::Log->ListAdd(CORE_LOG_BOLD("Processor:")         " %s (%s, %d logical cores, %d bytes cache line)", coreData::SystemCpuBrand(), coreData::SystemCpuVendor(), SDL_GetCPUCount(), SDL_GetCPUCacheLineSize());
        Core::Log->ListAdd(CORE_LOG_BOLD("System Memory:")     " %llu/%llu MB (%.1f%%)",                          iMemoryUsed / (1024u * 1024u), iMemoryTotal / (1024u * 1024u), dMemoryPct);
        Core::Log->ListAdd(CORE_LOG_BOLD("Disk Space:")        " %llu MB available",                              iSpaceAvailable / (1024u * 1024u));
        Core::Log->ListAdd(CORE_LOG_BOLD("Preferred Locales:") " %s",                                             sLocaleStr.c_str());
    }
    Core::Log->ListEnd();

    // log FreeType library version
    coreInt32 iMajorFT, iMinorFT, iPatchFT; TTF_GetFreeTypeVersion(&iMajorFT, &iMinorFT, &iPatchFT);
    if(iMajorFT) Core::Log->Info("FreeType initialized (%d.%d.%d)", iMajorFT, iMinorFT, iPatchFT);

    // log HarfBuzz library version
    coreInt32 iMajorHB, iMinorHB, iPatchHB; TTF_GetHarfBuzzVersion(&iMajorHB, &iMinorHB, &iPatchHB);
    if(iMajorHB) Core::Log->Info("HarfBuzz initialized (%d.%d.%d)", iMajorHB, iMinorHB, iPatchHB);

#if !defined(_CORE_EMSCRIPTEN_)

    // log Zstandard library version
    Core::Log->Info("Zstandard initialized (%s, %s-threaded)", ZSTD_versionString(), ZSTD_cParam_getBounds(ZSTD_c_nbWorkers).upperBound ? "multi" : "single");

#endif

    // log xxHash library version
    Core::Log->Info("xxHash initialized (%s)", XXH_versionString());
}


// ****************************************************************
/* destructor */
CoreSystem::~CoreSystem()
{
    // clear memory
    m_aDisplayData.clear();

    // delete main window object
    SDL_DestroyWindow(m_pWindow);

    Core::Log->Info(CORE_LOG_BOLD("System Interface shut down"));
}


// ****************************************************************
/* change the title of the window */
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
/* change the icon of the window */
void CoreSystem::SetWindowIcon(const coreChar* pcPath)
{
    if(pcPath)
    {
        // retrieve texture file
        coreFileScope pFile = Core::Manager::Resource->RetrieveFile(pcPath);

        // decompress file to plain pixel data
        coreSurfaceScope pData = IMG_LoadTyped_RW(pFile->CreateReadStream(), 1, coreData::StrExtension(pcPath));
        if(!pData)
        {
            Core::Log->Warning("Icon (%s) could not be loaded (SDL: %s)", pcPath, SDL_GetError());
            return;
        }

        // set new window icon
        SDL_SetWindowIcon(m_pWindow, pData);
    }

    Core::Log->Info("Icon (%s) loaded", pcPath);
}


// ****************************************************************
/* change the resolution of the window (clamped) */
void CoreSystem::SetWindowResolution(const coreVector2 vResolution)
{
    const coreDisplay& oCurrent = m_aDisplayData[m_iDisplayIndex];

    // skip on restricted target systems
    if(oCurrent.avAvailableRes.size() != 1u)
    {
        // save new resolution
        m_vResolution = vResolution;
        if(m_vResolution.x <= 0.0f) m_vResolution.x = oCurrent.vDesktopRes.x;
        if(m_vResolution.y <= 0.0f) m_vResolution.y = oCurrent.vDesktopRes.y;

        // clamp to bounds
        const coreVector2 vMaximum = (m_eMode == CORE_SYSTEM_MODE_FULLSCREEN) ? oCurrent.vMaximumRes : ((m_eMode == CORE_SYSTEM_MODE_BORDERLESS) ? oCurrent.vDesktopRes : (((oCurrent.vWorkAreaRes - I_TO_F(CORE_SYSTEM_WINDOW_BORDER)) / 2.0f).Processed(ROUND) * 2.0f));
        m_vResolution.x = CLAMP(m_vResolution.x, I_TO_F(CORE_SYSTEM_WINDOW_MINIMUM), vMaximum.x);
        m_vResolution.y = CLAMP(m_vResolution.y, I_TO_F(CORE_SYSTEM_WINDOW_MINIMUM), vMaximum.y);

        if(m_pWindow)
        {
            // fit to desktop if required
            if(m_eMode == CORE_SYSTEM_MODE_BORDERLESS) SDL_SetWindowFullscreen(m_pWindow, (m_vResolution == oCurrent.vDesktopRes) ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0u);

            // define window properties
            const coreInt32 iPos   = SDL_WINDOWPOS_CENTERED_DISPLAY(m_iDisplayIndex);
            const coreInt32 iSizeX = F_TO_SI(m_vResolution.x);
            const coreInt32 iSizeY = F_TO_SI(m_vResolution.y);

            // set new size and position
            SDL_SetWindowSize    (m_pWindow, iSizeX, iSizeY);
            SDL_SetWindowPosition(m_pWindow, iPos,   iPos);
        }
    }
}


// ****************************************************************
/* change everything of the window (in correct order) */
void CoreSystem::SetWindowAll(const coreUint8 iDisplayIndex, const coreVector2 vResolution, const coreSystemMode eMode)
{
    if(m_iDisplayIndex != iDisplayIndex)
    {
        // change the display
        m_iDisplayIndex = CLAMP(iDisplayIndex, 0u, m_aDisplayData.size() - 1u);
        Core::Config->SetInt(CORE_CONFIG_SYSTEM_DISPLAY, m_iDisplayIndex);
    }

    if(m_eMode != eMode)
    {
        // change the fullscreen mode
        m_eMode = eMode;
        Core::Config->SetInt(CORE_CONFIG_SYSTEM_FULLSCREEN, m_eMode);

        // set new window appearance
        SDL_SetWindowFullscreen(m_pWindow, (m_eMode == CORE_SYSTEM_MODE_FULLSCREEN) ? SDL_WINDOW_FULLSCREEN : 0u);
        SDL_SetWindowMouseGrab (m_pWindow, (m_eMode == CORE_SYSTEM_MODE_FULLSCREEN) ? SDL_TRUE : SDL_FALSE);
        SDL_SetWindowBordered  (m_pWindow, (m_eMode == CORE_SYSTEM_MODE_WINDOWED)   ? SDL_TRUE : SDL_FALSE);
        SDL_SetWindowResizable (m_pWindow, (m_eMode == CORE_SYSTEM_MODE_WINDOWED)   ? SDL_TRUE : SDL_FALSE);
    }

    // change the resolution
    this->SetWindowResolution(vResolution);
}


// ****************************************************************
/* update the event system */
void CoreSystem::__UpdateEvents()
{
    // check for system errors
    coreData::CheckLastError();

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

        // process display events
        case SDL_DISPLAYEVENT:
            switch(oEvent.display.event)
            {
            // display focus lost
            case SDL_DISPLAYEVENT_DISCONNECTED:
                m_bWinFocusLost = true;
                break;
            }
            break;

        // application focus lost
        case SDL_JOYDEVICEREMOVED:
        case SDL_USEREVENT:
        case SDL_APP_WILLENTERBACKGROUND:
        case SDL_APP_DIDENTERFOREGROUND:
            m_bWinFocusLost = true;
            break;

        // application closed
        case SDL_QUIT:
        case SDL_APP_TERMINATING:
            this->Quit();
            break;
        }

        // forward event to input component
        if(!Core::Input->ProcessEvent(oEvent)) return;
    }
}


// ****************************************************************
/* update the main window */
void CoreSystem::__UpdateWindow()
{
    // reduce overhead if window is not visible
    if(SDL_GetWindowFlags(m_pWindow) & (SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED))
        SDL_Delay(10u);

    // toggle between borderless and windowed mode
    if(Core::Input->GetKeyboardButton(CORE_INPUT_KEY(LALT),   CORE_INPUT_HOLD) &&
       Core::Input->GetKeyboardButton(CORE_INPUT_KEY(RETURN), CORE_INPUT_PRESS))
    {
        // change fullscreen mode and resolution to fit desktop (always)
        this->SetWindowAll(m_iDisplayIndex, m_aDisplayData[m_iDisplayIndex].vDesktopRes, m_eMode ? CORE_SYSTEM_MODE_WINDOWED : CORE_SYSTEM_MODE_BORDERLESS);
    }
}


// ****************************************************************
/* update the high-precision time */
void CoreSystem::__UpdateTime()
{
    // measure and calculate last frame time
    const coreUint64 iNewPerfTime = SDL_GetPerformanceCounter();
    const coreFloat  fNewLastTime = coreFloat(coreDouble(iNewPerfTime - m_iPerfTime) * m_dPerfFrequency);
    m_iPerfTime                   = iNewPerfTime;

    if(m_iSkipFrame)
    {
        // skip frame
        m_fLastTime   = 0.0f;
        m_iSkipFrame -= 1u;
    }
    else
    {
        // save total time of the previous frame
        m_dTotalTimeBefore = m_dTotalTime;

        // smooth last frame time and increase total time
        m_fLastTime   = (fNewLastTime > 0.1f) ? 0.001f : LERP(m_fLastTime, fNewLastTime, 0.15f);
        m_dTotalTime += coreDouble(m_fLastTime);
    }

    // update dynamic frame times
    for(coreUintW i = 0u; i < CORE_SYSTEM_TIMES; ++i)
        m_afTime[i] = m_fLastTime * m_afTimeSpeed[i];

    // increase current frame number
    ++m_iCurFrame;
}