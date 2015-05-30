//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SYSTEM_H_
#define _CORE_GUARD_SYSTEM_H_

// TODO: fullscreen enumeration


// ****************************************************************
// system definitions
#define CORE_SYSTEM_TIMES (4u)   //!< number of dynamic frame times


// ****************************************************************
// main system component
class CoreSystem final
{
private:
    SDL_Window* m_pWindow;                         //!< SDL main window object

    coreVector2 m_vResolution;                     //!< width and height of the window
    coreUint8   m_iFullscreen;                     //!< fullscreen status (0 = window | 1 = borderless | 2 = fullscreen)

    std::vector<coreVector2> m_avAvailable;        //!< all available screen resolutions
    coreBool m_bMinimized;                         //!< window/application was minimized
    coreBool m_bTerminated;                        //!< application will be terminated

    coreDouble m_dTotalTime;                       //!< total time since start of the application
    coreFloat  m_fLastTime;                        //!< smoothed last frame time
    coreFloat  m_afTime     [CORE_SYSTEM_TIMES];   //!< dynamic frame times
    coreFloat  m_afTimeSpeed[CORE_SYSTEM_TIMES];   //!< speed factor for the dynamic frame times

    coreUint32 m_iCurFrame;                        //!< current frame number since start of the application
    coreUint8  m_iSkipFrame;                       //!< skip frame status

    coreDouble m_dPerfFrequency;                   //!< high-precision time coefficient
    coreUint64 m_iPerfTime;                        //!< high-precision time value


private:
    CoreSystem()noexcept;
    ~CoreSystem();


public:
    FRIEND_CLASS(Core)
    DISABLE_COPY(CoreSystem)

    //! control window
    //! @{
    inline void SetWindowTitle(const coreChar* pcTitle) {SDL_SetWindowTitle(m_pWindow, pcTitle);}
    void        SetWindowIcon (const coreChar* pcPath);
    //! @}

    //! control time
    //! @{
    inline void SetTimeSpeed(const coreUintW& iID, const coreFloat& fTimeSpeed) {ASSERT(iID < CORE_SYSTEM_TIMES) m_afTimeSpeed[iID] = fTimeSpeed;}
    inline void SkipFrame()                                                     {m_iSkipFrame = 2u;}
    //! @}

    //! terminate the application
    //! @{
    inline void Quit() {m_bTerminated = true;}
    //! @}

    //! get component properties
    //! @{
    inline       SDL_Window*               GetWindow       ()const                     {return m_pWindow;}
    inline const coreVector2&              GetResolution   ()const                     {return m_vResolution;}
    inline const coreUint8&                GetFullscreen   ()const                     {return m_iFullscreen;}
    inline const std::vector<coreVector2>& GetAvailable    ()const                     {return m_avAvailable;}
    inline const coreBool&                 GetMinimized    ()const                     {return m_bMinimized;}
    inline const coreDouble&               GetTotalTime    ()const                     {return m_dTotalTime;}
    inline const coreFloat&                GetTime         ()const                     {return m_fLastTime;}
    inline const coreFloat&                GetTime         (const coreInt8&  iID)const {ASSERT(iID < coreInt8(CORE_SYSTEM_TIMES)) return (iID >= 0) ? m_afTime[iID] : m_fLastTime;}
    inline const coreFloat&                GetTimeSpeed    (const coreUintW& iID)const {ASSERT(iID <          CORE_SYSTEM_TIMES)  return m_afTimeSpeed[iID];}
    inline const coreUint32&               GetCurFrame     ()const                     {return m_iCurFrame;}
    inline const coreDouble&               GetPerfFrequency()const                     {return m_dPerfFrequency;}
    inline const coreUint64&               GetPerfTime     ()const                     {return m_iPerfTime;}
    //! @}


private:
    //! update the window event system
    //! @{
    coreBool __UpdateEvents();
    //! @}

    //! update the high-precision time
    //! @{
    void __UpdateTime();
    //! @}
};


#endif // _CORE_GUARD_SYSTEM_H_