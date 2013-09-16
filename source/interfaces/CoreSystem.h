//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SYSTEM_H_
#define _CORE_GUARD_SYSTEM_H_


// ****************************************************************
// system definitions
#define CORE_SYSTEM_TIMES 8


// ****************************************************************
// main system interface
//! \ingroup interface
class CoreSystem final
{
private:
    SDL_Window* m_pWindow;                    //!< SDL main window object

    coreVector2 m_vResolution;                //!< width and height of the window
    coreByte m_iFullscreen;                   //!< fullscreen status (0 = window | 1 = borderless | 2 = fullscreen)

    std::vector<coreVector2> m_avAvailable;   //!< all available screen resolutions
    bool m_bMinimized;                        //!< window was minimized

    double m_dTotalTime;                      //!< total time since start of the application
    float m_fLastTime;                        //!< smoothed last frame time
    float m_afTime[CORE_SYSTEM_TIMES];        //!< adjusted frame times
    float m_afTimeSpeed[CORE_SYSTEM_TIMES];   //!< speed factor for the adjusted frame times

    coreUint m_iCurFrame;                     //!< current frame number since start of the application
    coreByte m_iSkipFrame;                    //!< skip frame status

#if defined(_CORE_WINDOWS_)
    float m_fPerfFrequency;                   //!< high precision time coefficient (Windows)
    LARGE_INTEGER m_iPerfTime;                //!< high precision time value (Windows)
#else
    timespec m_iPerfTime;                     //!< high precision time value (Linux)
#endif

    int m_aaiCPUID[2][4];                     //!< features of the processor
    bool m_abSSE[5];                          //!< available SSE versions (1, 2, 3, 4.1, 4.2)


private:
    CoreSystem();
    ~CoreSystem();
    friend class Core;

    //! update the window event system
    //! @{
    bool __UpdateEvents();
    //! @}

    //! update the high precision time calculation
    //! @{
    void __UpdateTime();
    //! @}


public:
    //! control window
    //! @{
    inline void SetTitle(const char* pcTitle) {SDL_SetWindowTitle(m_pWindow, pcTitle);}
    void MsgBox(const char* pcMessage, const char* pcTitle, const int& iType);
    //! @}

    //! control time
    //! @{
    inline void SetTimeSpeed(const int& iID, const float& fTimeSpeed) {SDL_assert(iID < CORE_SYSTEM_TIMES); m_afTimeSpeed[iID] = fTimeSpeed;}
    inline void SkipFrame()                                           {m_iSkipFrame = 2;}
    //! @}

    //! get attributes
    //! @{
    inline SDL_Window* GetWindow()const                        {return m_pWindow;}
    inline const coreVector2& GetResolution()const             {return m_vResolution;}
    inline const coreByte& GetFullscreen()const                {return m_iFullscreen;}
    inline const std::vector<coreVector2>& GetAvailable()const {return m_avAvailable;}
    inline const bool& GetMinimized()const                     {return m_bMinimized;}
    inline const double& GetTotalTime()const                   {return m_dTotalTime;}
    inline const float& GetTime()const                         {return m_fLastTime;}
    inline const float& GetTime(const int& iID)const           {SDL_assert(iID < CORE_SYSTEM_TIMES); return (m_fLastTime >= 0) ? m_afTime[iID] : m_fLastTime;}
    inline const float& GetTimeSpeed(const int& iID)const      {SDL_assert(iID < CORE_SYSTEM_TIMES); return m_afTimeSpeed[iID];}
    inline const coreUint& GetCurFrame()const                  {return m_iCurFrame;}
    //! @}

    //! check hardware support
    //! @{
    inline const bool& SupportSSE()const   {return m_abSSE[0];}
    inline const bool& SupportSSE2()const  {return m_abSSE[1];}
    inline const bool& SupportSSE3()const  {return m_abSSE[2];}
    inline const bool& SupportSSE41()const {return m_abSSE[3];}
    inline const bool& SupportSSE42()const {return m_abSSE[4];}
    //! @}
};


#endif // _CORE_GUARD_SYSTEM_H_