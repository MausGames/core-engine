//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once


// ****************************************************************
// main system interface
class CoreSystem
{
private:
    SDL_Window* m_pWindow;                    // SDL main window object

    coreVector2 m_vResolution;                // width and height of the window
    coreByte m_iFullscreen;                   // fullscreen status (0 = window | 1 = borderless | 2 = fullscreen)

    std::vector<coreVector2> m_avAvailable;   // all available screen resolutions
    bool m_bMinimized;                        // window was minimized

    float m_fTime;                            // parameterized time
    float m_fTimeConstant;                    // constant time
    double m_dTimeTotal;                      // total time since start of the application
    coreUint m_iCurFrame;                     // current frame number since start of the application

    float m_fTimeFactor;                      // coefficient for the parameterized time
    coreByte m_iSkipFrame;                    // skip frames

#if defined (_WIN32)
    float m_fPerfFrequency;                   // high precission time coefficient (WIN32)
    LARGE_INTEGER m_PerfStartTime;            // high precission start time value (WIN32)
    LARGE_INTEGER m_PerfEndTime;              // high precission end time value (WIN32)
#else
    timespec m_PerfStartTime;                 // high precission start time value (Linux)
    timespec m_PerfEndTime;                   // high precission end time value (Linux)
#endif

    int m_aaiCPUID[2][4];                     // features of the processor
    bool m_abSSE[5];                          // available SSE versions (1, 2, 3, 4.1, 4.2)


private:
    CoreSystem();
    ~CoreSystem();
    friend class Core;

    // update the window event system
    bool __UpdateEvents();

    // update the high precission time calculation
    void __UpdateTime();


public:
    // control window
    inline void SetTitle(const char* pcTitle) {SDL_SetWindowTitle(m_pWindow, pcTitle);}
    void MsgBox(const char* pcMessage, const char* pcTitle, const int& iType);

    // control time
    inline void SetTimeFactor(const float& fTimeFactor) {m_fTimeFactor = fTimeFactor;}
    inline void SkipFrame()                             {m_iSkipFrame = 2;}

    // get attributes
    inline SDL_Window* GetWindow()const                        {return m_pWindow;}
    inline const coreVector2& GetResolution()const             {return m_vResolution;}
    inline const coreByte& GetFullscreen()const                {return m_iFullscreen;}
    inline const std::vector<coreVector2>& GetAvailable()const {return m_avAvailable;}
    inline const bool& GetMinimized()const                     {return m_bMinimized;}
    inline const float& GetTime()const                         {return m_fTime;}
    inline const float& GetTimeConstant()const                 {return m_fTimeConstant;}
    inline const double& GetTimeTotal()const                   {return m_dTimeTotal;}
    inline const coreUint& GetCurFrame()const                  {return m_iCurFrame;}
    inline const float& GetTimeFactor()const                   {return m_fTimeFactor;}

    // check hardware support
    inline const bool& SupportSSE()const   {return m_abSSE[0];}
    inline const bool& SupportSSE2()const  {return m_abSSE[1];}
    inline const bool& SupportSSE3()const  {return m_abSSE[2];}
    inline const bool& SupportSSE41()const {return m_abSSE[3];}
    inline const bool& SupportSSE42()const {return m_abSSE[4];}
};