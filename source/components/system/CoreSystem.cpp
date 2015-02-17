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
, m_iCurFrame   (0)
, m_iSkipFrame  (2)
{
    Core::Log->Header("System Interface");

    // get SDL version
    SDL_version oVersion;
    SDL_GetVersion(&oVersion);

    // init SDL libraries
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) || TTF_Init() || !IMG_Init(IMG_INIT_PNG))
        Core::Log->Error("SDL could not be initialized (SDL: %s)", SDL_GetError());
    else Core::Log->Info("SDL initialized (%d.%d.%d %s)", oVersion.major, oVersion.minor, oVersion.patch, SDL_GetRevision());

    // get number of logical processor cores
    m_iNumCores = MAX(SDL_GetCPUCount(), 1);

    // retrieve desktop resolution
    SDL_DisplayMode oDesktop;
    SDL_GetDesktopDisplayMode(0, &oDesktop);
    const coreVector2 vDesktop = coreVector2(I_TO_F(oDesktop.w), I_TO_F(oDesktop.h));

    // load all available screen resolutions
    const int iNumModes = SDL_GetNumDisplayModes(0);
    if(iNumModes)
    {
        Core::Log->ListStartInfo("Available Screen Resolutions");
        {
            for(int i = 0; i < iNumModes; ++i)
            {
                // retrieve resolution
                SDL_DisplayMode oMode;
                SDL_GetDisplayMode(0, i, &oMode);
                const coreVector2 vMode = coreVector2(I_TO_F(oMode.w), I_TO_F(oMode.h));

                coreUint j = 0, k = coreUint(m_avAvailable.size());
                for(; j < k; ++j)
                {
                    // check for already added resolutions
                    if(m_avAvailable[j] == vMode)
                        break;
                }
                if(j == k)
                {
                    // add new resolution
                    m_avAvailable.push_back(vMode);
                    Core::Log->ListAdd("%4d x %4d%s", oMode.w, oMode.h, (vMode == vDesktop) ? " (Desktop)" : "");
                }
            }
        }
        Core::Log->ListEnd();

        // override screen resolution
        if(m_avAvailable.size() == 1) m_vResolution = m_avAvailable.back();
        if(!m_vResolution.x) m_vResolution.x = vDesktop.x;
        if(!m_vResolution.y) m_vResolution.y = vDesktop.y;
    }
    else Core::Log->Warning("Could not get available screen resolutions (SDL: %s)", SDL_GetError());

    // configure the SDL window
    const coreUint iCenter = (Core::Config->GetBool(CORE_CONFIG_SYSTEM_DEBUGMODE) || DEFINED(_CORE_DEBUG_)) ? 0 : SDL_WINDOWPOS_CENTERED;
    const coreUint iFlags  = SDL_WINDOW_OPENGL | (m_iFullscreen == 2 ? SDL_WINDOW_FULLSCREEN : (m_iFullscreen == 1 ? SDL_WINDOW_BORDERLESS : 0));

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

    // create optional debug context
    if(Core::Config->GetBool(CORE_CONFIG_SYSTEM_DEBUGMODE) || DEFINED(_CORE_DEBUG_))
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    // create main window object
    m_pWindow = SDL_CreateWindow(coreData::AppName(), iCenter, iCenter, F_TO_SI(m_vResolution.x), F_TO_SI(m_vResolution.y), iFlags);
    if(!m_pWindow)
    {
        Core::Log->Warning("Problems creating main window, trying different settings (SDL: %s)", SDL_GetError());

        // override configuration
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,         16);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

        // create another main window object
        m_pWindow = SDL_CreateWindow(coreData::AppName(), iCenter, iCenter, F_TO_SI(m_vResolution.x), F_TO_SI(m_vResolution.y), iFlags);
        if(!m_pWindow) Core::Log->Error("Main window could not be created (SDL: %s)", SDL_GetError());
    }
    Core::Log->Info("Main window created (%.0f x %.0f / %d)", m_vResolution.x, m_vResolution.y, m_iFullscreen);

    // disable screen saver (automatically re-enabled)
    SDL_DisableScreenSaver();

    // init high precision time
    m_dPerfFrequency = 1.0 / double(SDL_GetPerformanceFrequency());
    m_iPerfTime      = SDL_GetPerformanceCounter();

    // reset dynamic frame times
    for(int i = 0; i < CORE_SYSTEM_TIMES; ++i)
    {
        m_afTime[i]      = 0.0f;
        m_afTimeSpeed[i] = 1.0f;
    }

    // retrieve features of the processor
#if defined(_CORE_SSE_)
    #if defined(_CORE_MSVC_)
        __cpuid(m_aaiCPUID[0], 0);
        __cpuid(m_aaiCPUID[1], 1);
    #else
        asm volatile("cpuid" : "=a" (m_aaiCPUID[0][0]), "=b" (m_aaiCPUID[0][1]), "=c" (m_aaiCPUID[0][2]), "=d" (m_aaiCPUID[0][3]) : "a" (0), "c" (0));
        asm volatile("cpuid" : "=a" (m_aaiCPUID[1][0]), "=b" (m_aaiCPUID[1][1]), "=c" (m_aaiCPUID[1][2]), "=d" (m_aaiCPUID[1][3]) : "a" (1), "c" (0));
    #endif
#else
    std::memset(m_aaiCPUID, 0, sizeof(m_aaiCPUID));
#endif

    // check for SIMD support
         if(m_aaiCPUID[1][2] & BIT(20)) m_fSSE = 4.2f;
    else if(m_aaiCPUID[1][2] & BIT(19)) m_fSSE = 4.1f;
    else if(m_aaiCPUID[1][2] & BIT( 0)) m_fSSE = 3.0f;
    else if(m_aaiCPUID[1][3] & BIT(26)) m_fSSE = 2.0f;
    else if(m_aaiCPUID[1][3] & BIT(25)) m_fSSE = 1.0f;
                                   else m_fSSE = 0.0f;
         if(m_aaiCPUID[1][1] & BIT( 5)) m_fAVX = 2.0f;
    else if(m_aaiCPUID[1][2] & BIT(28)) m_fAVX = 1.0f;
                                   else m_fAVX = 0.0f;

    // log processor information
    Core::Log->ListStartInfo("Platform Information");
    {
        Core::Log->ListAdd(CORE_LOG_BOLD("Operating System:") " %s",                              coreData::SystemName());
        Core::Log->ListAdd(CORE_LOG_BOLD("Processor:")        " %.4s%.4s%.4s (%d Logical Cores)", r_cast<char*>(&m_aaiCPUID[0][1]), r_cast<char*>(&m_aaiCPUID[0][3]), r_cast<char*>(&m_aaiCPUID[0][2]), m_iNumCores);
        Core::Log->ListAdd(CORE_LOG_BOLD("System Memory:")    " %d MB",                           SDL_GetSystemRAM());
        Core::Log->ListAdd(CORE_LOG_BOLD("SIMD Support:")     " SSE %s, AVX %s",                  m_fSSE ? PRINT("%.1f", m_fSSE) : "-", m_fAVX ? PRINT("%.1f", m_fAVX) : "-");
        Core::Log->ListAdd(CORE_LOG_BOLD("CPUID[0]:")         " %08X %08X %08X %08X",             m_aaiCPUID[0][0], m_aaiCPUID[0][1], m_aaiCPUID[0][2], m_aaiCPUID[0][3]);
        Core::Log->ListAdd(CORE_LOG_BOLD("CPUID[1]:")         " %08X %08X %08X %08X",             m_aaiCPUID[1][0], m_aaiCPUID[1][1], m_aaiCPUID[1][2], m_aaiCPUID[1][3]);
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

    // shut down SDL libraries
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    Core::Log->Info(CORE_LOG_BOLD("System Interface shut down"));
}


// ****************************************************************
// change the icon of the window
void CoreSystem::SetWindowIcon(const char* pcPath)
{
    coreFile* pFile = Core::Manager::Resource->RetrieveFile(pcPath);

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
bool CoreSystem::__UpdateEvents()
{
    // reset minimize status
    m_bMinimized = false;

    // process events
    SDL_Event Event;
    while(SDL_PollEvent(&Event))
    {
        switch(Event.type)
        {
        // control window
        case SDL_WINDOWEVENT:
            switch(Event.window.event)
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
                if(Event.window.windowID == SDL_GetWindowID(m_pWindow)) this->Quit();
                else SDL_DestroyWindow(SDL_GetWindowFromID(Event.window.windowID));
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
        default: if(!Core::Input->ProcessEvent(Event)) return true;
        }
    }

    return !m_bTerminated;
}


// ****************************************************************
// update the high precision time
void CoreSystem::__UpdateTime()
{
    // measure and calculate last frame time
    const uint64_t iNewPerfTime = SDL_GetPerformanceCounter();
    const float    fNewLastTime = float(double(iNewPerfTime - m_iPerfTime) * m_dPerfFrequency);
    m_iPerfTime                 = iNewPerfTime;

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
        m_dTotalTime += (double)m_fLastTime;
    }

    // update dynamic frame times
    for(int i = 0; i < CORE_SYSTEM_TIMES; ++i)
        m_afTime[i] = m_fLastTime*m_afTimeSpeed[i];

    // increase current frame number
    ++m_iCurFrame;
}