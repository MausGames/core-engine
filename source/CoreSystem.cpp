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
: m_vResolution     (coreVector2((float)Core::Config->GetInt(CORE_CONFIG_SYSTEM_WIDTH, 800), (float)Core::Config->GetInt(CORE_CONFIG_SYSTEM_HEIGHT, 600)))
, m_iFullscreen     ((coreByte)Core::Config->GetInt(CORE_CONFIG_SYSTEM_FULLSCREEN, 0))
, m_bMinimized      (false)
, m_fTime           (0.0f)
, m_fTimeConstant   (0.0f)
, m_dTimeTotal      (0.0f)
, m_iCurFrame       (0)
, m_fTimeFactor     (1.0f)
, m_iSkipFrame      (0)
{
    Core::Log->Header("System Interface");

    // init SDL libraries
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) || TTF_Init())
        Core::Log->Error(1, coreUtils::Print("SDL could not be initialized (SDL: %s)", SDL_GetError()));
    else Core::Log->Info("SDL initialized");

    // configure the SDL window parameters
    const coreUint iFlags = SDL_WINDOW_OPENGL | (m_iFullscreen == 2 ? SDL_WINDOW_FULLSCREEN : (m_iFullscreen == 1 ? SDL_WINDOW_BORDERLESS : NULL));
    
    // configure the OpenGL context parameters
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,                   8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,                 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,                  8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,                 0);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,                 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,               1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS,         1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,         Core::Config->GetInt(CORE_CONFIG_GRAPHICS_MULTISAMPLING, 0));
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
    SDL_GL_SetSwapInterval(1);

    // try to force OpenGL context version
    const float fForceOpenGL = Core::Config->GetFloat(CORE_CONFIG_GRAPHICS_FORCEOPENGL, 0.0f);
    if(fForceOpenGL)
    {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, int(floorf(fForceOpenGL))); 
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, int(floorf(fForceOpenGL*10.0f))%10); 
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
    }

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
    else Core::Log->Info("Main window created");

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
    }
    else Core::Log->Error(0, coreUtils::Print("Could not get available screen resolutions (SDL: %s)", SDL_GetError()));

    // init high precission time
#if defined(_WIN32)
    QueryPerformanceFrequency(&m_PerfEndTime);
    m_fPerfFrequency = 1.0f/float(m_PerfEndTime.QuadPart);
    QueryPerformanceCounter(&m_PerfEndTime); 
#else
    clock_gettime(CLOCK_MONOTONIC, &m_PerfEndTime);
#endif
    m_PerfStartTime = m_PerfEndTime;

    // retrieve features of the processor
#if defined(_DEBUG)
    memset(m_aaiCPUID, 0, sizeof(int)*2*4);
#else
    #if defined(_WIN32)
        __cpuid(m_aaiCPUID[0], 0);
        __cpuid(m_aaiCPUID[1], 1);
    #else
        asm volatile("cpuid" : "=a" (m_aaiCPUID[0][0]), "=b" (m_aaiCPUID[0][1]), "=c" (m_aaiCPUID[0][2]), "=d" (m_aaiCPUID[0][3]) : "a" (0), "c" (0));
        asm volatile("cpuid" : "=a" (m_aaiCPUID[1][0]), "=b" (m_aaiCPUID[1][1]), "=c" (m_aaiCPUID[1][2]), "=d" (m_aaiCPUID[1][3]) : "a" (1), "c" (0));
    #endif
#endif

    // check for SSE support
    m_abSSE[0] = (m_aaiCPUID[1][3] & 0x2000000) ? true : false;
    m_abSSE[1] = (m_aaiCPUID[1][3] & 0x4000000) ? true : false;
    m_abSSE[2] = (m_aaiCPUID[1][2] & 0x01)      ? true : false;
    m_abSSE[3] = (m_aaiCPUID[1][2] & 0x80000)   ? true : false;
    m_abSSE[4] = (m_aaiCPUID[1][2] & 0x100000)  ? true : false;

    // log processor information
    std::string sCPU;
    for(int i = 0; i < 4; ++i) sCPU += (*(((char*)&m_aaiCPUID[0][1])+i));
    for(int i = 0; i < 4; ++i) sCPU += (*(((char*)&m_aaiCPUID[0][3])+i));
    for(int i = 0; i < 4; ++i) sCPU += (*(((char*)&m_aaiCPUID[0][2])+i));
    Core::Log->ListStart("Processor Information");
    Core::Log->ListEntry(coreUtils::Print("<b>Vendor:</b> %s", sCPU.c_str()));
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
            Core::Input->SetMouseRelative(coreVector2(float(Event.motion.xrel), -float(Event.motion.yrel))/m_vResolution * m_fTimeConstant*120.0f);
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
            if(ABS(Event.jaxis.value) > 8000) Core::Input->SetJoystickRelative(Event.jbutton.which, Event.jaxis.axis, SIG(Event.jaxis.value));
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
// update the high precission time calculation
void CoreSystem::__UpdateTime()
{
    // measure and calculate constant time
#if defined(_WIN32)
    QueryPerformanceCounter(&m_PerfEndTime);
    m_fTimeConstant = float(m_PerfEndTime.QuadPart - m_PerfStartTime.QuadPart) * m_fPerfFrequency;
#else
    clock_gettime(CLOCK_MONOTONIC, &m_PerfEndTime);
    m_fTimeConstant = float(0.000000001*(double(m_PerfEndTime.tv_sec - m_PerfStartTime.tv_sec)*1000000000.0 + double(m_PerfEndTime.tv_nsec - m_PerfStartTime.tv_nsec)));
#endif
    m_PerfStartTime = m_PerfEndTime;

    // increase total time and calculate parameterized time
    m_dTimeTotal += (double)m_fTimeConstant;
    m_fTime       = m_fTime ? (0.85f*m_fTime + 0.15f*m_fTimeConstant*m_fTimeFactor) : (m_fTimeConstant*m_fTimeFactor);

    // skip frames
    if(m_iSkipFrame || m_fTimeConstant >= 1.0f) 
    {
        m_fTime         = 0.0f;
        m_fTimeConstant = 0.0f;
        if(m_iSkipFrame) --m_iSkipFrame;
    }

    // increate current frame number
    ++m_iCurFrame;
}


// ******************************************************************
// show message box
void CoreSystem::MsgBox(const char* pcMessage, const char* pcTitle, const int& iType)
{
#if defined(_WIN32)
    switch(iType)
    {
    case 0: MessageBoxA(NULL, pcMessage, pcTitle, MB_OK | MB_ICONINFORMATION); break;
    case 1: MessageBoxA(NULL, pcMessage, pcTitle, MB_OK | MB_ICONQUESTION);    break;
    case 2: MessageBoxA(NULL, pcMessage, pcTitle, MB_OK | MB_ICONEXCLAMATION); break;
    case 3: MessageBoxA(NULL, pcMessage, pcTitle, MB_OK | MB_ICONSTOP);        break;
    }
#else
    // TODO: implement function
#endif
}