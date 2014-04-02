//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

#if defined(_CORE_GLES_)
    #define CORE_SYSTEM_CONTEXT SDL_GL_CONTEXT_PROFILE_ES
#else
    #define CORE_SYSTEM_CONTEXT SDL_GL_CONTEXT_PROFILE_CORE
#endif


// ******************************************************************
// constructor
CoreSystem::CoreSystem()noexcept
: m_vResolution (coreVector2((float)Core::Config->GetInt(CORE_CONFIG_SYSTEM_WIDTH), (float)Core::Config->GetInt(CORE_CONFIG_SYSTEM_HEIGHT)))
, m_iFullscreen ((coreByte)Core::Config->GetInt(CORE_CONFIG_SYSTEM_FULLSCREEN))
, m_bMinimized  (false)
, m_dTotalTime  (0.0f)
, m_fLastTime   (0.0f)
, m_iCurFrame   (0)
, m_iSkipFrame  (2)
{
    Core::Log->Header("System Interface");

    // get SDL version
    SDL_version Version;
    SDL_GetVersion(&Version);

    // init SDL libraries
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) || TTF_Init() || !IMG_Init(IMG_INIT_PNG))
        Core::Log->Error(true, "SDL could not be initialized (SDL: %s)", SDL_GetError());
    else Core::Log->Info("SDL initialized (%d.%d.%d %s)", Version.major, Version.minor, Version.patch, SDL_GetRevision());

    // get number of logical processor cores
    m_iNumCores = MAX(SDL_GetCPUCount(), 1);

    // retrieve desktop resolution
    SDL_DisplayMode Desktop;
    SDL_GetDesktopDisplayMode(0, &Desktop);
    const coreVector2 vDesktop = coreVector2((float)Desktop.w, (float)Desktop.h);

    // load all available screen resolutions
    const int iNumModes = SDL_GetNumDisplayModes(0);
    if(iNumModes)
    {
        m_avAvailable.reserve(32);

        Core::Log->ListStart("Available Screen Resolutions");
        for(int i = 0; i < iNumModes; ++i)
        {
            // retrieve resolution
            SDL_DisplayMode Mode;
            SDL_GetDisplayMode(0, i, &Mode);
            const coreVector2 vMode = coreVector2((float)Mode.w, (float)Mode.h);

            coreUint j = 0;
            for(; j < m_avAvailable.size(); ++j)
            {
                // check already added resolutions
                if(m_avAvailable[j] == vMode)
                    break;
            }
            if(j == m_avAvailable.size())
            {
                // add new resolution
                m_avAvailable.push_back(vMode);
                Core::Log->ListEntry("%4d x %4d%s", Mode.w, Mode.h, (vMode == vDesktop) ? " (Desktop)" : "");
            }
        }
        Core::Log->ListEnd();

        // override screen resolution
        if(m_avAvailable.size() == 1) m_vResolution = m_avAvailable.back();
        if(!m_vResolution.x) m_vResolution.x = vDesktop.x;
        if(!m_vResolution.y) m_vResolution.y = vDesktop.y;
    }
    else Core::Log->Error(false, "Could not get available screen resolutions (SDL: %s)", SDL_GetError());

    // configure the SDL window
    const coreUint iFlags = SDL_WINDOW_OPENGL | (m_iFullscreen == 2 ? SDL_WINDOW_FULLSCREEN : (m_iFullscreen == 1 ? SDL_WINDOW_BORDERLESS : 0));

    // configure the OpenGL context
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,                   8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,                 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,                  8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,                 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,                 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,               1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,         1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,         Core::Config->GetInt(CORE_CONFIG_GRAPHICS_ANTIALIASING));
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, Core::Config->GetBool(CORE_CONFIG_GRAPHICS_DUALCONTEXT) && (m_iNumCores >= 2));

    // try to force a different OpenGL context
    const float fForceOpenGL = Core::Config->GetFloat(CORE_CONFIG_GRAPHICS_FORCEOPENGL);
    if(fForceOpenGL)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, int(std::floor(fForceOpenGL)));
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, int(std::floor(fForceOpenGL * 10.0f)) % 10);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,  CORE_SYSTEM_CONTEXT);
    }
    if(Core::Config->GetBool(CORE_CONFIG_GRAPHICS_DEBUGCONTEXT) || g_bCoreDebug)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);

    // create main window object
    m_pWindow = SDL_CreateWindow(coreData::AppName(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)m_vResolution.x, (int)m_vResolution.y, iFlags);
    if(!m_pWindow)
    {
        Core::Log->Error(false, "Problems creating main window, trying different settings (SDL: %s)", SDL_GetError());

        // change configuration
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

        // create compatible main window object
        m_pWindow = SDL_CreateWindow(coreData::AppName(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)m_vResolution.x, (int)m_vResolution.y, iFlags);
        if(!m_pWindow) Core::Log->Error(true, "Main window could not be created (SDL: %s)", SDL_GetError());
    }
    Core::Log->Info("Main window created (%.0f x %.0f / %d)", m_vResolution.x, m_vResolution.y, m_iFullscreen);

    // init high precision time
    m_dPerfFrequency = 1.0/double(SDL_GetPerformanceFrequency());
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

    // check for SSE support
    m_abSSE[0] = (m_aaiCPUID[1][3] & 0x2000000) ? true : false;
    m_abSSE[1] = (m_aaiCPUID[1][3] & 0x4000000) ? true : false;
    m_abSSE[2] = (m_aaiCPUID[1][2] & 0x01)      ? true : false;
    m_abSSE[3] = (m_aaiCPUID[1][2] & 0x80000)   ? true : false;
    m_abSSE[4] = (m_aaiCPUID[1][2] & 0x100000)  ? true : false;

    // log processor information
    Core::Log->ListStart("Platform Information");
    Core::Log->ListEntry("<b>Processor:</b> %.4s%.4s%.4s (%d Logical Cores)", (char*)&m_aaiCPUID[0][1], (char*)&m_aaiCPUID[0][3], (char*)&m_aaiCPUID[0][2], m_iNumCores);
    Core::Log->ListEntry("<b>System Memory:</b> %d MB",          SDL_GetSystemRAM());
    Core::Log->ListEntry("<b>SSE Support:</b> %s%s%s%s%s",       m_abSSE[0] ? "1" : "", m_abSSE[1] ? " 2" : "", m_abSSE[2] ? " 3" : "", m_abSSE[3] ? " 4.1" : "", m_abSSE[4] ? " 4.2" : "");
    Core::Log->ListEntry("<b>CPUID[0]:</b> %08X %08X %08X %08X", m_aaiCPUID[0][0], m_aaiCPUID[0][1], m_aaiCPUID[0][2], m_aaiCPUID[0][3]);
    Core::Log->ListEntry("<b>CPUID[1]:</b> %08X %08X %08X %08X", m_aaiCPUID[1][0], m_aaiCPUID[1][1], m_aaiCPUID[1][2], m_aaiCPUID[1][3]);
    Core::Log->ListEnd();
}


// ******************************************************************
// destructor
CoreSystem::~CoreSystem()
{
    Core::Log->Info("System Interface shut down");

    // clear memory
    m_avAvailable.clear();

    // delete SDL main window object
    SDL_DestroyWindow(m_pWindow);

    // shut down SDL libraries
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}


// ******************************************************************
// change the icon of the window
void CoreSystem::SetIcon(const char* pcPath)
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


// ******************************************************************
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
                m_bMinimized = true;
                break;

            // close window
            case SDL_WINDOWEVENT_CLOSE:
                if(Event.window.windowID == SDL_GetWindowID(m_pWindow)) Core::Quit();
                else SDL_DestroyWindow(SDL_GetWindowFromID(Event.window.windowID));
                break;
            }
            break;

        // quit the application
        case SDL_QUIT: return false;

        // forward event to input component
        default: if(!Core::Input->ProcessEvent(Event)) return true;
        }
    }

    return true;
}


// ******************************************************************
// update the high precision time
void CoreSystem::__UpdateTime()
{
    // measure and calculate last frame time
    const uint64_t iNewPerfTime = SDL_GetPerformanceCounter();
    const float fNewLastTime    = float(double(iNewPerfTime - m_iPerfTime) * m_dPerfFrequency);
    m_iPerfTime                 = iNewPerfTime;

    if(m_iSkipFrame || fNewLastTime >= 0.25f)
    {
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

    // increate current frame number
    ++m_iCurFrame;
}