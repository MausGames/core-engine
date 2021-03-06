///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SYSTEM_H_
#define _CORE_GUARD_SYSTEM_H_

// TODO 4: fullscreen enumeration
// TODO 3: some drivers may "merge" displays
// TODO 5: handle display hot-plugging
// TODO 3: improve delta-time precision, double, similar to coreThread
// TODO 3: toggling out of m_iFullscreen == 2u might cause issues with some options (e.g. disabled composition)
// TODO 5: <old comment style>


// ****************************************************************
/* system definitions */
#define CORE_SYSTEM_TIMES          (4u)     // number of dynamic frame times
#define CORE_SYSTEM_WINDOW_BORDER  (20u)    // border width used for restricting window size
#define CORE_SYSTEM_WINDOW_MINIMUM (128u)   // minimum size of the main window

#define TIME (Core::System->GetTime())


// ****************************************************************
/* main system component */
class CoreSystem final
{
private:
    /* display structure */
    struct coreDisplay final
    {
        coreSet<coreVector2> avAvailableRes;   // all available screen resolutions (highest to lowest, primary on width)
        coreVector2          vDesktopRes;      // desktop resolution
        coreVector2          vWorkAreaRes;     // work area resolution (e.g. without task bar)
        coreVector2          vMaximumRes;      // highest available resolution (primary on width)
    };


private:
    SDL_Window* m_pWindow;                         // SDL main window object

    coreList<coreDisplay> m_aDisplayData;          // all available displays
    coreUint8             m_iDisplayIndex;         // current display index

    coreVector2 m_vResolution;                     // width and height of the window
    coreUint8   m_iFullscreen;                     // fullscreen status (0 = window | 1 = borderless | 2 = fullscreen)

    coreDouble m_dTotalTime;                       // total time since start of the application
    coreDouble m_dTotalTimeBefore;                 // total time of the previous frame
    coreFloat  m_fLastTime;                        // smoothed last frame time
    coreFloat  m_afTime     [CORE_SYSTEM_TIMES];   // dynamic frame times
    coreFloat  m_afTimeSpeed[CORE_SYSTEM_TIMES];   // speed factor for the dynamic frame times

    coreUint32 m_iCurFrame;                        // current frame number since start of the application
    coreUint8  m_iSkipFrame;                       // skip frame status

    coreDouble m_dPerfFrequency;                   // high-precision time coefficient
    coreUint64 m_iPerfTime;                        // high-precision time value

    coreBool m_bWinFocusLost;                      // window/application lost focus
    coreBool m_bWinPosChanged;                     // window position changed
    coreBool m_bWinSizeChanged;                    // window size changed
    coreBool m_bTerminated;                        // application will be terminated


private:
    CoreSystem()noexcept;
    ~CoreSystem();


public:
    FRIEND_CLASS(Core)
    DISABLE_COPY(CoreSystem)

    /* control window */
    void SetWindowTitle     (const coreChar*   pcTitle);
    void SetWindowIcon      (const coreChar*   pcPath);
    void SetWindowResolution(const coreVector2 vResolution);

    /* control time */
    inline void SetTimeSpeed(const coreUintW iID, const coreFloat fTimeSpeed) {ASSERT(iID < CORE_SYSTEM_TIMES) m_afTimeSpeed[iID] = fTimeSpeed;}
    inline void SkipFrame()                                                   {m_iSkipFrame = 1u;}

    /* terminate the application */
    inline void Quit() {m_bTerminated = true;}

    /* get component properties */
    inline       SDL_Window*  GetWindow         ()const                    {return m_pWindow;}
    inline const coreDisplay& GetDisplayData    ()const                    {return m_aDisplayData[m_iDisplayIndex];}
    inline const coreDisplay& GetDisplayData    (const coreUintW iID)const {ASSERT(iID < m_aDisplayData.size()) return m_aDisplayData[iID];}
    inline       coreUintW    GetDisplayCount   ()const                    {return m_aDisplayData.size();}
    inline const coreUint8&   GetDisplayIndex   ()const                    {return m_iDisplayIndex;}
    inline const coreVector2& GetResolution     ()const                    {return m_vResolution;}
    inline const coreUint8&   GetFullscreen     ()const                    {return m_iFullscreen;}
    inline const coreDouble&  GetTotalTime      ()const                    {return m_dTotalTime;}
    inline const coreDouble&  GetTotalTimeBefore()const                    {return m_dTotalTimeBefore;}
    inline const coreFloat&   GetTime           ()const                    {return m_fLastTime;}
    inline const coreFloat&   GetTime           (const coreInt8  iID)const {ASSERT(iID < coreInt8(CORE_SYSTEM_TIMES)) return (iID >= 0) ? m_afTime[iID] : m_fLastTime;}
    inline const coreFloat&   GetTimeSpeed      (const coreUintW iID)const {ASSERT(iID <          CORE_SYSTEM_TIMES)  return m_afTimeSpeed[iID];}
    inline const coreUint32&  GetCurFrame       ()const                    {return m_iCurFrame;}
    inline const coreDouble&  GetPerfFrequency  ()const                    {return m_dPerfFrequency;}
    inline const coreUint64&  GetPerfTime       ()const                    {return m_iPerfTime;}
    inline const coreBool&    GetWinFocusLost   ()const                    {return m_bWinFocusLost;}
    inline const coreBool&    GetWinPosChanged  ()const                    {return m_bWinPosChanged;}
    inline const coreBool&    GetWinSizeChanged ()const                    {return m_bWinSizeChanged;}


private:
    /* update the event system */
    coreBool __UpdateEvents();

    /* update the main window */
    void __UpdateWindow();

    /* update the high-precision time */
    void __UpdateTime();
};


#endif /* _CORE_GUARD_SYSTEM_H_ */