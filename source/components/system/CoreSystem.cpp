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
, m_fRefreshRate     (Core::Config->GetFloat(CORE_CONFIG_SYSTEM_REFRESHRATE))
, m_eMode            (coreSystemMode(Core::Config->GetInt(CORE_CONFIG_SYSTEM_FULLSCREEN)))
, m_dTotalTime       (0.0)
, m_dTotalTimeBefore (0.0)
, m_dLastTimeFull    (0.0)
, m_fLastTime        (0.0f)
, m_afTime           {}
, m_afTimeSpeed      {}
, m_iCurFrame        (0u)
, m_iSkipFrame       (1u)
, m_dPerfFrequency   (0.0)
, m_iPerfTime        (0u)
, m_iMainThread      (0u)
, m_bWinFocusLost    (false)
, m_bWinPosChanged   (false)
, m_bWinSizeChanged  (false)
, m_bTerminated      (false)
{
    Core::Log->Header("System Interface");

    // load SDL only once (to improve reset performance, and prevent crashes)
    UNUSED static const coreBool s_bOnce = []()
    {
        // set SDL application metadata
        SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_NAME_STRING,       CoreApp::Settings::Name);
        SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_VERSION_STRING,    CoreApp::Settings::Version);
        SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_IDENTIFIER_STRING, CoreApp::Settings::Identifier);
        SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_CREATOR_STRING,    CoreApp::Settings::Creator);
        SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_COPYRIGHT_STRING,  PRINT("Copyright (c) %s %s", CoreApp::Settings::Year, CoreApp::Settings::Creator));
        SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_URL_STRING,        CoreApp::Settings::Website);
        SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_TYPE_STRING,       "game");

        // set SDL behavior hints
        SDL_SetHint(SDL_HINT_EVENT_LOGGING,                      DEFINED(_CORE_DEBUG_) ? "1" : "0");
        SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,   "1");
        SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE,                 "0");
        SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_MODE_CENTER,         "0");
        SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER,                   "1");
        SDL_SetHint(SDL_HINT_VIDEO_DOUBLE_BUFFER,                "1");
        SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, DEFINED(_CORE_DEBUG_) ? "0" : "1");

        // enable SDL debug messages
        SDL_SetLogPriorities(Core::Debug->IsEnabled() ? SDL_LOG_PRIORITY_INFO : SDL_LOG_PRIORITY_WARN);

        // get default log callback (standard output)
        SDL_LogOutputFunction nOldFunc;
        SDL_GetLogOutputFunction(&nOldFunc, NULL);

        // register new log callback
        SDL_SetLogOutputFunction([](void* pUserData, const coreInt32 iCategory, const SDL_LogPriority ePriority, const coreChar* pcMessage)
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
        const coreInt32 iVersionSDL = SDL_GetVersion();
        const coreInt32 iVersionTTF = TTF_Version();
        const coreInt32 iVersionIMG = IMG_Version();

        // init SDL libraries
        if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD) || !TTF_Init() || !SDL_GL_LoadLibrary(NULL))
        {
            Core::Log->Error("SDL could not be initialized (SDL: %s)", SDL_GetError());
        }
        else
        {
            Core::Log->Info("SDL initialized (%d.%d.%d %s, TTF %d.%d.%d, IMG %d.%d.%d, %s, sandbox %d)",
                            SDL_VERSIONNUM_MAJOR(iVersionSDL), SDL_VERSIONNUM_MINOR(iVersionSDL), SDL_VERSIONNUM_MICRO(iVersionSDL), SDL_GetRevision(),
                            SDL_VERSIONNUM_MAJOR(iVersionTTF), SDL_VERSIONNUM_MINOR(iVersionTTF), SDL_VERSIONNUM_MICRO(iVersionTTF),
                            SDL_VERSIONNUM_MAJOR(iVersionIMG), SDL_VERSIONNUM_MINOR(iVersionIMG), SDL_VERSIONNUM_MICRO(iVersionIMG), SDL_GetPlatform(), SDL_GetSandbox());
        }

        // execute main-thread with higher priority
        SDL_SetCurrentThreadPriority(SDL_THREAD_PRIORITY_HIGH);

        // disable screen saver
        SDL_DisableScreenSaver();

        // disable unwanted events
        constexpr coreUint32 aiDisable[] = {SDL_EVENT_DROP_FILE, SDL_EVENT_DROP_TEXT, SDL_EVENT_DROP_BEGIN, SDL_EVENT_DROP_COMPLETE, SDL_EVENT_DROP_POSITION, SDL_EVENT_KEYMAP_CHANGED, SDL_EVENT_CLIPBOARD_UPDATE};
        for(coreUintW i = 0u; i < ARRAY_SIZE(aiDisable); ++i) SDL_SetEventEnabled(aiDisable[i], false);

        // automatically shut down SDL libraries on exit
        WARN_IF(std::atexit([]() {SDL_GL_UnloadLibrary(); TTF_Quit(); SDL_Quit();})) {}
        return true;
    }();

    // load all available displays
    coreInt32      iDisplayCount = 0u;
    coreAllocScope piDisplayList = SDL_GetDisplays(&iDisplayCount);
    if(iDisplayCount)
    {
        Core::Log->ListStartInfo("Available Displays");
        {
            // reserve some memory
            m_aDisplayData.reserve(iDisplayCount);

            for(coreUintW i = 0u, ie = iDisplayCount; i < ie; ++i)
            {
                coreDisplay& oDisplayData = m_aDisplayData.emplace_back();

                // store display instance ID
                const SDL_DisplayID iDisplayID = piDisplayList[i];
                oDisplayData.iDisplayID = iDisplayID;

                // retrieve desktop resolution
                const SDL_DisplayMode* pDesktop = SDL_GetDesktopDisplayMode(iDisplayID);
                oDisplayData.vDesktopRes  = coreVector2(I_TO_F(pDesktop->w), I_TO_F(pDesktop->h));
                oDisplayData.fDesktopRate = pDesktop->refresh_rate;

                // retrieve work area resolution
                SDL_Rect oWorkArea = {};
                SDL_GetDisplayUsableBounds(iDisplayID, &oWorkArea);
                oDisplayData.vWorkAreaRes = coreVector2(I_TO_F(oWorkArea.w), I_TO_F(oWorkArea.h));

                // retrieve display scale
                const coreFloat fScale = SDL_GetDisplayContentScale(iDisplayID);

                // retrieve HDR support
                const coreBool bHdr = SDL_GetBooleanProperty(SDL_GetDisplayProperties(iDisplayID), SDL_PROP_DISPLAY_HDR_ENABLED_BOOLEAN, false);

                // load all available screen resolutions
                coreInt32      iModeCount = 0u;
                coreAllocScope ppModeList = SDL_GetFullscreenDisplayModes(iDisplayID, &iModeCount);
                if(iModeCount)
                {
                    Core::Log->ListDeeper(CORE_LOG_BOLD("Display %u:") " %s (%d x %d @ %.2f Hz, %.0f%% scale, %d/%d offset, %s)", i, SDL_GetDisplayName(iDisplayID), pDesktop->w, pDesktop->h, pDesktop->refresh_rate, fScale * 100.0f, oWorkArea.x, oWorkArea.y, bHdr ? "HDR" : "no HDR");
                    {
                        // reserve some memory
                        oDisplayData.avAvailableRes  .reserve(iModeCount);
                        oDisplayData.aafAvailableRate.reserve(iModeCount);

                        for(coreUintW j = 0u, je = iModeCount; j < je; ++j)
                        {
                            const SDL_DisplayMode* pMode = ppModeList[j];

                            // add new resolution
                            const coreVector2 vModeRes = coreVector2(I_TO_F(pMode->w), I_TO_F(pMode->h));
                            if(!oDisplayData.avAvailableRes.count(vModeRes))
                            {
                                oDisplayData.avAvailableRes  .insert(vModeRes);
                                oDisplayData.aafAvailableRate.emplace_back();
                            }

                            // add new refresh rate
                            ASSERT(oDisplayData.avAvailableRes.back() == vModeRes)
                            oDisplayData.aafAvailableRate.back().insert_once(pMode->refresh_rate);

                            Core::Log->ListAdd("%4d x %4d @ %.2f Hz", pMode->w, pMode->h, pMode->refresh_rate);
                        }

                        // copy highest resolution
                        oDisplayData.vMaximumRes = oDisplayData.avAvailableRes.front();

                        // reduce memory consumption
                        oDisplayData.avAvailableRes  .shrink_to_fit();
                        oDisplayData.aafAvailableRate.shrink_to_fit();
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

        if(oDisplayData.avAvailableRes  .empty ()) oDisplayData.avAvailableRes  .insert(m_vResolution);
        if(oDisplayData.aafAvailableRate.empty ()) oDisplayData.aafAvailableRate.emplace_back().insert(0.0f);
        if(oDisplayData.vDesktopRes     .IsNull()) oDisplayData.vDesktopRes  = oDisplayData.avAvailableRes.front();
        if(oDisplayData.vWorkAreaRes    .IsNull()) oDisplayData.vWorkAreaRes = oDisplayData.avAvailableRes.front();
        if(oDisplayData.vMaximumRes     .IsNull()) oDisplayData.vMaximumRes  = oDisplayData.avAvailableRes.front();

        ASSERT(oDisplayData.avAvailableRes.size() == oDisplayData.aafAvailableRate.size())
    }
    ASSERT(m_aDisplayData[0].iDisplayID == SDL_GetPrimaryDisplay())

    // sanitize display index
    if(m_iDisplayIndex >= m_aDisplayData.size()) m_iDisplayIndex = 0u;
    const coreDisplay& oTarget = m_aDisplayData[m_iDisplayIndex];

    // sanitize screen resolution
    if(oTarget.avAvailableRes.size() == 1u) m_vResolution = oTarget.vWorkAreaRes;
    this->SetWindowResolution(m_vResolution);

    // sanitize fullscreen mode
    if(oTarget.avAvailableRes.size() == 1u) m_eMode = CORE_SYSTEM_MODE_WINDOWED;
    m_eMode = CLAMP(m_eMode, CORE_SYSTEM_MODE_WINDOWED, CORE_SYSTEM_MODE_FULLSCREEN);

    // configure the OpenGL context
    SDL_GL_ResetAttributes();
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,                 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,               8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,                8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,               CoreApp::Settings::Graphics::AlphaChannel               ? 8 : 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,               CoreApp::Settings::Graphics::DepthSize);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,             CoreApp::Settings::Graphics::StencilSize);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,             CoreApp::Settings::Graphics::DoubleBuffer               ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_STEREO,                   CoreApp::Settings::Graphics::StereoRender               ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,       Core::Config->GetInt(CORE_CONFIG_GRAPHICS_ANTIALIASING) ? 1 : 0);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,       Core::Config->GetInt(CORE_CONFIG_GRAPHICS_ANTIALIASING));
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_RELEASE_BEHAVIOR, SDL_GL_CONTEXT_RELEASE_BEHAVIOR_NONE);

#if defined(_CORE_GLES_)

    // always request ES profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

    // check for highest OpenGL ES version
    if(!Core::Config->GetBool(CORE_CONFIG_BASE_FALLBACKMODE))
    {
        // create quick test-window
        m_pWindow = SDL_CreateWindow(NULL, 32, 32, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
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
                    SDL_GL_DestroyContext(pContext);
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
        m_pWindow = SDL_CreateWindow(NULL, 32, 32, SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
        if(m_pWindow)
        {
            const SDL_GLContext pContext = SDL_GL_CreateContext(m_pWindow);
            if(pContext)
            {
                // get highest OpenGL version
                const corePoint2U8 oVersion = coreData::StrVersion(r_cast<const coreChar*>(glGetString(GL_VERSION)));
                SDL_GL_DestroyContext(pContext);

                // set version and request core profile
                if((oVersion >= corePoint2U8(3u, 0u)) || DEFINED(_CORE_MACOS_))
                {
                    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, oVersion[0]);
                    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, oVersion[1]);
                    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,  SDL_GL_CONTEXT_PROFILE_CORE);
                    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,         SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);   // overwritten by debug context
                }
            }
            SDL_DestroyWindow(m_pWindow);
        }
    }

    // check for debug context
    if(Core::Debug->IsEnabled())
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    }

#endif

    // check for no-error context
    if(!Core::Debug->IsEnabled())
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_NO_ERROR, 1);
    }

    // check for shared context
    if(Core::Config->GetBool(CORE_CONFIG_BASE_ASYNCMODE) && !DEFINED(_CORE_EMSCRIPTEN_) && (SDL_GetNumLogicalCPUCores() > 1))
    {
        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    }

    // define window appearance
    const coreInt32 iPos   = SDL_WINDOWPOS_CENTERED_DISPLAY(oTarget.iDisplayID);
    const coreInt32 iSizeX = F_TO_SI(m_vResolution.x);
    const coreInt32 iSizeY = F_TO_SI(m_vResolution.y);

    // set function properties
    coreProperties oProps;
    SDL_SetNumberProperty (oProps, SDL_PROP_WINDOW_CREATE_X_NUMBER,              iPos);
    SDL_SetNumberProperty (oProps, SDL_PROP_WINDOW_CREATE_Y_NUMBER,              iPos);
    SDL_SetNumberProperty (oProps, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER,          iSizeX);
    SDL_SetNumberProperty (oProps, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER,         iSizeY);
    SDL_SetBooleanProperty(oProps, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN,        true);
    SDL_SetBooleanProperty(oProps, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN,    (m_eMode == CORE_SYSTEM_MODE_FULLSCREEN) || ((m_eMode == CORE_SYSTEM_MODE_BORDERLESS) && (m_vResolution == oTarget.vDesktopRes)));
    SDL_SetBooleanProperty(oProps, SDL_PROP_WINDOW_CREATE_MOUSE_GRABBED_BOOLEAN, (m_eMode == CORE_SYSTEM_MODE_FULLSCREEN) && !DEFINED(_CORE_DEBUG_));
    SDL_SetBooleanProperty(oProps, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN,    (m_eMode != CORE_SYSTEM_MODE_WINDOWED));
    SDL_SetBooleanProperty(oProps, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN,     (m_eMode == CORE_SYSTEM_MODE_WINDOWED));

    // create main window object
    m_pWindow = SDL_CreateWindowWithProperties(oProps);
    if(!m_pWindow)
    {
        Core::Log->Warning("Problems creating main window, trying different settings (SDL: %s)", SDL_GetError());

        // try compatible configuration
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,         16);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

        // create another main window object
        m_pWindow = SDL_CreateWindowWithProperties(oProps);
        if(!m_pWindow) Core::Log->Error("Main window could not be created (SDL: %s)", SDL_GetError());
    }
    Core::Log->Info("Main window created (%s, %s, %.0f x %.0f, display %u, mode %u)", SDL_GetCurrentVideoDriver(), SDL_GetPixelFormatName(SDL_GetWindowPixelFormat(m_pWindow)), m_vResolution.x, m_vResolution.y, m_iDisplayIndex, m_eMode);

    // set fullscreen mode
    if(m_eMode == CORE_SYSTEM_MODE_FULLSCREEN)
    {
        SDL_DisplayMode oMode;
        SDL_SetWindowFullscreenMode(m_pWindow, SDL_GetClosestFullscreenDisplayMode(oTarget.iDisplayID, iSizeX, iSizeY, m_fRefreshRate, true, &oMode) ? &oMode : NULL);
    }

    // restrict window size
    SDL_SetWindowMinimumSize(m_pWindow, CORE_SYSTEM_WINDOW_MINIMUM, CORE_SYSTEM_WINDOW_MINIMUM);

    // remove all events created during initialization
    SDL_PumpEvents();
    SDL_FlushEvents(SDL_EVENT_FIRST, SDL_EVENT_LAST);

    // init high-precision time
    m_dPerfFrequency = 1.0 / coreDouble(SDL_GetPerformanceFrequency());
    m_iPerfTime      = SDL_GetPerformanceCounter();

    // reset dynamic frame times
    for(coreUintW i = 0u; i < CORE_SYSTEM_TIMES; ++i)
    {
        m_afTime     [i] = 0.0f;
        m_afTimeSpeed[i] = 1.0f;
    }

    // save thread-ID from the main-thread
    m_iMainThread = SDL_GetCurrentThreadID();

    // log platform information
    Core::Log->ListStartInfo("Platform Information");
    {
        coreUint64 iMemoryAvailable, iMemoryTotal, iSpaceAvailable;
        coreData::SystemMemory(&iMemoryAvailable, &iMemoryTotal);   // after window and context creation
        coreData::SystemSpace (&iSpaceAvailable,  NULL);

        const coreUint64 iMemoryUsed = iMemoryTotal - iMemoryAvailable;
        const coreDouble dMemoryPct  = 100.0 * (coreDouble(iMemoryUsed) / coreDouble(MAX(iMemoryTotal, 1u)));

        coreString     sLocaleStr   = "";
        coreAllocScope ppLocaleList = SDL_GetPreferredLocales(NULL);
        if(ppLocaleList)
        {
            for(SDL_Locale** ppCurrent = ppLocaleList.Get(); (*ppCurrent); ++ppCurrent)
            {
                sLocaleStr += (*ppCurrent)->country ? PRINT("%s_%s", (*ppCurrent)->language, (*ppCurrent)->country) : (*ppCurrent)->language;
                sLocaleStr += ' ';
            }
        }

        coreInt32 iSeconds, iPercent;
        const SDL_PowerState ePowerState = SDL_GetPowerInfo(&iSeconds, &iPercent);

        Core::Log->ListAdd(CORE_LOG_BOLD("Operating System:")  " %s",                                             coreData::SystemOsName());
        Core::Log->ListAdd(CORE_LOG_BOLD("Processor:")         " %s (%s, %d logical cores, %d bytes cache line)", coreData::SystemCpuBrand(), coreData::SystemCpuVendor(), SDL_GetNumLogicalCPUCores(), SDL_GetCPUCacheLineSize());
        Core::Log->ListAdd(CORE_LOG_BOLD("System Memory:")     " %llu/%llu MB (%.1f%%)",                          iMemoryUsed / (1024u * 1024u), iMemoryTotal / (1024u * 1024u), dMemoryPct);
        Core::Log->ListAdd(CORE_LOG_BOLD("Disk Space:")        " %llu MB available",                              iSpaceAvailable / (1024u * 1024u));
        Core::Log->ListAdd(CORE_LOG_BOLD("Preferred Locales:") " %s",                                             sLocaleStr.c_str());
        Core::Log->ListAdd(CORE_LOG_BOLD("Battery Status:")    " %d (%d%%, %d minutes)",                          ePowerState, MAX0(iPercent), MAX0(iSeconds) / 60);
    }
    Core::Log->ListEnd();

    // log FreeType library version
    coreInt32 iMajorFT, iMinorFT, iPatchFT; TTF_GetFreeTypeVersion(&iMajorFT, &iMinorFT, &iPatchFT);
    if(iMajorFT) Core::Log->Info("FreeType initialized (%d.%d.%d)", iMajorFT, iMinorFT, iPatchFT);

    // log HarfBuzz library version
    coreInt32 iMajorHB, iMinorHB, iPatchHB; TTF_GetHarfBuzzVersion(&iMajorHB, &iMinorHB, &iPatchHB);
    if(iMajorHB) Core::Log->Info("HarfBuzz initialized (%d.%d.%d)", iMajorHB, iMinorHB, iPatchHB);

    // log WebP library version
    const coreInt32 iDecoderVersion = WebPGetDecoderVersion();
    const coreInt32 iDemuxVersion   = WebPGetDemuxVersion();
    Core::Log->Info("WebP initialized (decode %d.%d.%d, demux %d.%d.%d)", (iDecoderVersion >> 16) & 0xFF, (iDecoderVersion >> 8) & 0xFF, iDecoderVersion & 0xFF, (iDemuxVersion >> 16) & 0xFF, (iDemuxVersion >> 8) & 0xFF, iDemuxVersion & 0xFF);

    // log Zstandard library version
    Core::Log->Info("Zstandard initialized (%s, %s-threaded)", ZSTD_versionString(), ZSTD_cParam_getBounds(ZSTD_c_nbWorkers).upperBound ? "multi" : "single");

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
    if(Core::Debug->IsEnabled())
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
        coreSurfaceScope pData = IMG_LoadTyped_IO(pFile->CreateReadStream(), true, coreData::StrExtension(pcPath));
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
        const coreVector2 vMaximum = (m_eMode == CORE_SYSTEM_MODE_FULLSCREEN) ? oCurrent.vMaximumRes : ((m_eMode == CORE_SYSTEM_MODE_BORDERLESS) ? oCurrent.vDesktopRes : (oCurrent.vWorkAreaRes - I_TO_F(CORE_SYSTEM_WINDOW_BORDER)).Processed(coreMath::RoundFactor, 2.0f));
        m_vResolution.x = CLAMP(m_vResolution.x, I_TO_F(CORE_SYSTEM_WINDOW_MINIMUM), vMaximum.x);
        m_vResolution.y = CLAMP(m_vResolution.y, I_TO_F(CORE_SYSTEM_WINDOW_MINIMUM), vMaximum.y);

        if(m_pWindow)
        {
            // always leave fullscreen (otherwise many following changes will be ignored)
            SDL_SetWindowFullscreen(m_pWindow, false);

            // define window appearance
            const coreInt32 iPos   = SDL_WINDOWPOS_CENTERED_DISPLAY(oCurrent.iDisplayID);
            const coreInt32 iSizeX = F_TO_SI(m_vResolution.x);
            const coreInt32 iSizeY = F_TO_SI(m_vResolution.y);

            // set new size and position
            SDL_SetWindowSize    (m_pWindow, iSizeX, iSizeY);
            SDL_SetWindowPosition(m_pWindow, iPos,   iPos);

            // set new fullscreen mode
            if(m_eMode == CORE_SYSTEM_MODE_FULLSCREEN)
            {
                SDL_DisplayMode oMode;
                SDL_SetWindowFullscreenMode(m_pWindow, SDL_GetClosestFullscreenDisplayMode(oCurrent.iDisplayID, iSizeX, iSizeY, m_fRefreshRate, true, &oMode) ? &oMode : NULL);
                SDL_SetWindowFullscreen    (m_pWindow, true);
            }
            else if((m_eMode == CORE_SYSTEM_MODE_BORDERLESS) && (m_vResolution == oCurrent.vDesktopRes))
            {
                SDL_SetWindowFullscreenMode(m_pWindow, NULL);
                SDL_SetWindowFullscreen    (m_pWindow, true);
            }

            // sync window (# and pump events)
            SDL_SyncWindow(m_pWindow);

            // ignore code-generated events
            SDL_FlushEvent(SDL_EVENT_WINDOW_MOVED);
            SDL_FlushEvent(SDL_EVENT_WINDOW_RESIZED);
        }
    }
}


// ****************************************************************
/* change everything of the window (in correct order) */
void CoreSystem::SetWindowAll(const coreUint8 iDisplayIndex, const coreVector2 vResolution, const coreFloat fRefreshRate, const coreSystemMode eMode)
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

        // set new window properties
        SDL_SetWindowMouseGrab(m_pWindow, (m_eMode == CORE_SYSTEM_MODE_FULLSCREEN) && !DEFINED(_CORE_DEBUG_));
        SDL_SetWindowBordered (m_pWindow, (m_eMode == CORE_SYSTEM_MODE_WINDOWED));
        SDL_SetWindowResizable(m_pWindow, (m_eMode == CORE_SYSTEM_MODE_WINDOWED));
    }

    // change the refresh rate
    m_fRefreshRate = fRefreshRate;
    Core::Config->SetFloat(CORE_CONFIG_SYSTEM_REFRESHRATE, m_fRefreshRate);

    // change the resolution
    this->SetWindowResolution(vResolution);
    Core::Config->SetInt(CORE_CONFIG_SYSTEM_WIDTH,  F_TO_SI(vResolution.x));   // # use unmodified resolution
    Core::Config->SetInt(CORE_CONFIG_SYSTEM_HEIGHT, F_TO_SI(vResolution.y));
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
        // window position changed
        case SDL_EVENT_WINDOW_MOVED:
            m_bWinPosChanged = true;
            m_iDisplayIndex  = CLAMP(this->__GetDisplayIndex(SDL_GetDisplayForWindow(m_pWindow)), 0u, m_aDisplayData.size() - 1u);
            Core::Config->SetInt(CORE_CONFIG_SYSTEM_DISPLAY, m_iDisplayIndex);
            break;

        // window size changed
        case SDL_EVENT_WINDOW_RESIZED:
            m_bWinSizeChanged = true;
            m_vResolution     = coreVector2(I_TO_F(oEvent.window.data1), I_TO_F(oEvent.window.data2));
            Core::Config->SetInt(CORE_CONFIG_SYSTEM_WIDTH,  F_TO_SI(m_vResolution.x));
            Core::Config->SetInt(CORE_CONFIG_SYSTEM_HEIGHT, F_TO_SI(m_vResolution.y));
            break;

        // window focus lost
        case SDL_EVENT_WINDOW_HIDDEN:
        case SDL_EVENT_WINDOW_MINIMIZED:
        case SDL_EVENT_WINDOW_MAXIMIZED:
        case SDL_EVENT_WINDOW_RESTORED:
        case SDL_EVENT_WINDOW_FOCUS_LOST:
            m_bWinFocusLost = true;
            break;

        // window closed
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            if(oEvent.window.windowID == SDL_GetWindowID(m_pWindow)) this->Quit();
            else SDL_DestroyWindow(SDL_GetWindowFromID(oEvent.window.windowID));
            break;

        // application focus lost
        case SDL_EVENT_DISPLAY_REMOVED:
        case SDL_EVENT_KEYBOARD_REMOVED:
        case SDL_EVENT_MOUSE_REMOVED:
        case SDL_EVENT_JOYSTICK_REMOVED:
        case SDL_EVENT_USER:
        case SDL_EVENT_WILL_ENTER_BACKGROUND:
        case SDL_EVENT_DID_ENTER_FOREGROUND:
            m_bWinFocusLost = true;
            break;

        // application closed
        case SDL_EVENT_QUIT:
        case SDL_EVENT_TERMINATING:
            this->Quit();
            break;
        }

        // forward event to other components
        const coreBool bState1 = Core::Input->ProcessEvent(oEvent);
        const coreBool bState2 = Core::Debug->ProcessEvent(oEvent);
        if(!bState1 || !bState2) return;
    }
}


// ****************************************************************
/* update the main window */
void CoreSystem::__UpdateWindow()
{
    // reduce overhead if window is not visible
    if(SDL_GetWindowFlags(m_pWindow) & (SDL_WINDOW_HIDDEN | SDL_WINDOW_MINIMIZED))
        SDL_Delay(30u);

    // toggle between borderless and windowed mode
    if(Core::Input->GetKeyboardButton(CORE_INPUT_KEY(LALT),   CORE_INPUT_HOLD) &&
       Core::Input->GetKeyboardButton(CORE_INPUT_KEY(RETURN), CORE_INPUT_PRESS))
    {
        // change fullscreen mode and resolution to fit desktop (always)
        this->SetWindowAll(m_iDisplayIndex, coreVector2(0.0f,0.0f), m_fRefreshRate, m_eMode ? CORE_SYSTEM_MODE_WINDOWED : CORE_SYSTEM_MODE_BORDERLESS);

        // handle as user-generated events
        m_bWinPosChanged  = true;
        m_bWinSizeChanged = true;
    }
}


// ****************************************************************
/* update the high-precision time */
void CoreSystem::__UpdateTime()
{
    // measure and calculate last frame time
    const coreUint64 iNewPerfTime = SDL_GetPerformanceCounter();
    const coreDouble dNewLastTime = coreDouble(iNewPerfTime - m_iPerfTime) * m_dPerfFrequency;
    m_iPerfTime                   = iNewPerfTime;

    if(m_iSkipFrame)
    {
        // skip frame
        m_dLastTimeFull = 0.0;
        m_fLastTime     = 0.0f;
        m_iSkipFrame   -= 1u;
    }
    else
    {
        // save total time of the previous frame
        m_dTotalTimeBefore = m_dTotalTime;

        // smooth and round last frame time and increase total time
        m_dLastTimeFull = (dNewLastTime > 0.1) ? 0.001 : LERP(m_dLastTimeFull, dNewLastTime, 0.15);
        m_fLastTime     = RCP(coreMath::RoundFactor(RCP(m_dLastTimeFull), 0.1));
        m_dTotalTime   += coreDouble(m_fLastTime);
    }

    // update dynamic frame times
    for(coreUintW i = 0u; i < CORE_SYSTEM_TIMES; ++i)
        m_afTime[i] = m_fLastTime * m_afTimeSpeed[i];

    // increase current frame number
    ++m_iCurFrame;
}


// ****************************************************************
/* convert display instance ID to display index */
coreUintW CoreSystem::__GetDisplayIndex(const SDL_DisplayID iID)const
{
    ASSERT(!m_aDisplayData.empty())

    // find required display
    FOR_EACH(it, m_aDisplayData)
    {
        if(it->iDisplayID == iID) return m_aDisplayData.index(it);
    }

    // return index to primary display
    return 0u;
}