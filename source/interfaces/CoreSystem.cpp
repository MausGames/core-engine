//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ******************************************************************
// constructor
CoreSystem::CoreSystem()
: m_vResolution (coreVector2((float)Core::Config->GetInt(CORE_CONFIG_SYSTEM_WIDTH), (float)Core::Config->GetInt(CORE_CONFIG_SYSTEM_HEIGHT)))
, m_iFullscreen ((coreByte)Core::Config->GetInt(CORE_CONFIG_SYSTEM_FULLSCREEN))
, m_bMinimized  (false)
, m_dTotalTime  (0.0f)
, m_fLastTime   (0.0f)
, m_iCurFrame   (0)
, m_iSkipFrame  (0)
{
    Core::Log->Header("System Interface");

    // init SDL libraries
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) || TTF_Init())
        Core::Log->Error(1, coreUtils::Print("SDL could not be initialized (SDL: %s)", SDL_GetError()));
    else Core::Log->Info("SDL initialized");

    // load all available screen resolutions
    const int iNumModes = SDL_GetNumDisplayModes(0);
    if(iNumModes)
    {
        m_avAvailable.reserve(32);

        Core::Log->ListStart("Available Screen Resolutions");
        for(int i = 0; i < iNumModes; ++i)
        {
            SDL_DisplayMode Mode;
            SDL_GetDisplayMode(0, i, &Mode);

            coreUint j = 0;
            for(; j < m_avAvailable.size(); ++j)
            {
                // check already added resolutions
                if(m_avAvailable[j].x == (float)Mode.w && m_avAvailable[j].y == (float)Mode.h)
                    break;
            }
            if(j == m_avAvailable.size())
            {
                // add new resolution
                m_avAvailable.push_back(coreVector2((float)Mode.w, (float)Mode.h));
                Core::Log->ListEntry(coreUtils::Print("%4d x %4d", Mode.w, Mode.h));
            }
        }
        Core::Log->ListEnd();

        // override screen resolution
        if(m_avAvailable.size() == 1) m_vResolution = m_avAvailable.back();
    }
    else Core::Log->Error(0, coreUtils::Print("Could not get available screen resolutions (SDL: %s)", SDL_GetError()));

    // configure the SDL window parameters
    const coreUint iFlags = SDL_WINDOW_OPENGL | (m_iFullscreen == 2 ? SDL_WINDOW_FULLSCREEN : (m_iFullscreen == 1 ? SDL_WINDOW_BORDERLESS : 0));

    // configure the OpenGL context parameters
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,                   8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,                 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,                  8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,                 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,                 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,               1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,         1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,         Core::Config->GetInt(CORE_CONFIG_GRAPHICS_ANTIALIASING));
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, Core::Config->GetBool(CORE_CONFIG_GRAPHICS_DUALCONTEXT));

    // try to force OpenGL context version
#if defined(_CORE_GLES_)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    const float fForceOpenGL = Core::Config->GetFloat(CORE_CONFIG_GRAPHICS_FORCEOPENGL);
    if(fForceOpenGL)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, int(floorf(fForceOpenGL)));
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, int(floorf(fForceOpenGL*10.0f))%10);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    }
#endif

    // create main window object
    m_pWindow = SDL_CreateWindow(coreUtils::AppName(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)m_vResolution.x, (int)m_vResolution.y, iFlags);
    if(!m_pWindow)
    {
        Core::Log->Error(0, coreUtils::Print("Problems creating main window, trying different settings (SDL: %s)", SDL_GetError()));

        // change configuration
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);

        // create compatible main window object
        m_pWindow = SDL_CreateWindow(coreUtils::AppName(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, (int)m_vResolution.x, (int)m_vResolution.y, iFlags);
        if(!m_pWindow) Core::Log->Error(1, coreUtils::Print("Main window could not be created (SDL: %s)", SDL_GetError()));
    }
    Core::Log->Info("Main window created");

    // init high precision time
    m_dPerfFrequency = 1.0/double(SDL_GetPerformanceFrequency());
    m_iPerfTime      = SDL_GetPerformanceCounter();

    // reset adjusted frame times
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
    memset(m_aaiCPUID, 0, sizeof(int)*2*4);
#endif

    // check for SSE support
    m_abSSE[0] = (m_aaiCPUID[1][3] & 0x2000000) ? true : false;
    m_abSSE[1] = (m_aaiCPUID[1][3] & 0x4000000) ? true : false;
    m_abSSE[2] = (m_aaiCPUID[1][2] & 0x01)      ? true : false;
    m_abSSE[3] = (m_aaiCPUID[1][2] & 0x80000)   ? true : false;
    m_abSSE[4] = (m_aaiCPUID[1][2] & 0x100000)  ? true : false;

    // log processor information
    Core::Log->ListStart("Processor Information");
    Core::Log->ListEntry(coreUtils::Print("<b>Vendor:</b> %.4s%.4s%.4s", (char*)&m_aaiCPUID[0][1], (char*)&m_aaiCPUID[0][3], (char*)&m_aaiCPUID[0][2]));
    Core::Log->ListEntry(coreUtils::Print("<b>CPUID[0]:</b> %08X %08X %08X %08X", m_aaiCPUID[0][0], m_aaiCPUID[0][1], m_aaiCPUID[0][2], m_aaiCPUID[0][3]));
    Core::Log->ListEntry(coreUtils::Print("<b>CPUID[1]:</b> %08X %08X %08X %08X", m_aaiCPUID[1][0], m_aaiCPUID[1][1], m_aaiCPUID[1][2], m_aaiCPUID[1][3]));
    Core::Log->ListEntry(coreUtils::Print("<b>SSE support:</b> %s%s%s%s%s", m_abSSE[0] ? "1 " : "", m_abSSE[1] ? "2 " : "", m_abSSE[2] ? "3 " : "", m_abSSE[3] ? "4.1 " : "", m_abSSE[4] ? "4.2 " : ""));
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
// update the window event system
bool CoreSystem::__UpdateEvents()
{
    SDL_Event Event;

    // TODO MAJOR: define, where to reset relative mouse values (+joysticks?)
    //Core::Input->SetMouseRelative(coreVector2(0.0f,0.0f));
    //Core::Input->SetMouseWheel(0.0f);

    // reset minimize status
    m_bMinimized = false;

    // process events
    while(SDL_PollEvent(&Event))
    {
        switch(Event.type)
        {
        // change window
        case SDL_WINDOWEVENT:
            switch(Event.window.event)
            {
                // minimize window
                case SDL_WINDOWEVENT_HIDDEN:
                case SDL_WINDOW_MINIMIZED:
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

        // TODO: implement with coreTextBox
        //case SDL_TEXTINPUT:
        //    Core::Input->SetKeyboardChar((char)Event.text.text[0]);
        //    break;

        // press keyboard button
        case SDL_KEYDOWN:
            Core::Input->SetKeyboardButton(Event.key.keysym.scancode, true);
                 if(Event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) Core::Input->SetKeyboardChar((char)8);
            else if(Event.key.keysym.scancode == SDL_SCANCODE_RETURN)    Core::Input->SetKeyboardChar((char)13);
            break;

        // release keyboard button
        case SDL_KEYUP:
            Core::Input->SetKeyboardButton(Event.key.keysym.scancode, false);
            break;

        // press mouse button
        case SDL_MOUSEBUTTONDOWN:
            Core::Input->SetMouseButton(Event.button.button, true);
            break;

        // release mouse button
        case SDL_MOUSEBUTTONUP:
            Core::Input->SetMouseButton(Event.button.button, false);
            break;

        // move mouse position
        case SDL_MOUSEMOTION:
            Core::Input->SetMousePosition(coreVector2(float(Event.motion.x),    -float(Event.motion.y))   /m_vResolution + coreVector2(-0.5f, 0.5f));
            Core::Input->SetMouseRelative(coreVector2(float(Event.motion.xrel), -float(Event.motion.yrel))/m_vResolution * m_fLastTime*120.0f);
            break;

        // move mouse wheel
        case SDL_MOUSEWHEEL:
            Core::Input->SetMouseWheel((float)Event.wheel.y);
            break;

        // press joystick button
        case SDL_JOYBUTTONDOWN:
            Core::Input->SetJoystickButton(Event.jbutton.which, Event.jbutton.button, true);
            break;

        // release joystick button
        case SDL_JOYBUTTONUP:
            Core::Input->SetJoystickButton(Event.jbutton.which, Event.jbutton.button, false);
            break;

        // move joystick axis
        case SDL_JOYAXISMOTION:
            if(ABS(Event.jaxis.value) > 8000) Core::Input->SetJoystickRelative(Event.jbutton.which, Event.jaxis.axis, (float)SIG(Event.jaxis.value));
                                         else Core::Input->SetJoystickRelative(Event.jbutton.which, Event.jaxis.axis, 0.0f);
            break;

        // quit the application
        case SDL_QUIT:
            return false;
        }
    }

    return true;
}


// ******************************************************************
// update the high precision time calculation
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
        m_fLastTime   = m_fLastTime ? (0.85f*m_fLastTime + 0.15f*fNewLastTime) : fNewLastTime;
        m_dTotalTime += (double)m_fLastTime;
    }

    // update adjusted frame times
    for(int i = 0; i < CORE_SYSTEM_TIMES; ++i)
        m_afTime[i] = m_fLastTime*m_afTimeSpeed[i];

    // increate current frame number
    ++m_iCurFrame;
}