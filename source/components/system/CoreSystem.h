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


// ****************************************************************
// system definitions
#define CORE_SYSTEM_TIMES (4)   //!< number of dynamic frame times (signed)


// ****************************************************************
// main system component
class CoreSystem final
{
private:
    SDL_Window* m_pWindow;                     //!< SDL main window object

    coreVector2 m_vResolution;                 //!< width and height of the window
    coreByte    m_iFullscreen;                 //!< fullscreen status (0 = window | 1 = borderless | 2 = fullscreen)

    std::vector<coreVector2> m_avAvailable;    //!< all available screen resolutions
    bool m_bMinimized;                         //!< window/application was minimized
    bool m_bTerminated;                        //!< application will be terminated

    double m_dTotalTime;                       //!< total time since start of the application
    float  m_fLastTime;                        //!< smoothed last frame time
    float  m_afTime[CORE_SYSTEM_TIMES];        //!< dynamic frame times
    float  m_afTimeSpeed[CORE_SYSTEM_TIMES];   //!< speed factor for the dynamic frame times

    coreUint m_iCurFrame;                      //!< current frame number since start of the application
    coreByte m_iSkipFrame;                     //!< skip frame status

    double     m_dPerfFrequency;               //!< high precision time coefficient
    coreUint64 m_iPerfTime;                    //!< high precision time value

    coreByte m_iNumCores;                      //!< number of logical processor cores
    float m_fSSE;                              //!< available SSE version
    float m_fAVX;                              //!< available AVX version
    int   m_aaiCPUID[2][4];                    //!< features of the processor


private:
    CoreSystem()noexcept;
    ~CoreSystem();
    friend class Core;


public:
    //! control window
    //! @{
    inline void SetWindowTitle(const char* pcTitle) {SDL_SetWindowTitle(m_pWindow, pcTitle);}
    void        SetWindowIcon (const char* pcPath);
    //! @}

    //! control time
    //! @{
    inline void SetTimeSpeed(const int& iID, const float& fTimeSpeed) {ASSERT(iID < CORE_SYSTEM_TIMES) m_afTimeSpeed[iID] = fTimeSpeed;}
    inline void SkipFrame()                                           {m_iSkipFrame = 2;}
    //! @}

    //! terminate the application
    //! @{
    inline void Quit() {m_bTerminated = true;}
    //! @}

    //! get component properties
    //! @{
    inline SDL_Window*                     GetWindow    ()const               {return m_pWindow;}
    inline const coreVector2&              GetResolution()const               {return m_vResolution;}
    inline const coreByte&                 GetFullscreen()const               {return m_iFullscreen;}
    inline const std::vector<coreVector2>& GetAvailable ()const               {return m_avAvailable;}
    inline const bool&                     GetMinimized ()const               {return m_bMinimized;}
    inline const double&                   GetTotalTime ()const               {return m_dTotalTime;}
    inline const float&                    GetTime      ()const               {return m_fLastTime;}
    inline const float&                    GetTime      (const int& iID)const {ASSERT(iID < CORE_SYSTEM_TIMES) return (m_fLastTime >= 0) ? m_afTime[iID] : m_fLastTime;}
    inline const float&                    GetTimeSpeed (const int& iID)const {ASSERT(iID < CORE_SYSTEM_TIMES) return m_afTimeSpeed[iID];}
    inline const coreUint&                 GetCurFrame  ()const               {return m_iCurFrame;}
    //! @}

    //! check hardware support
    //! @{
    inline const coreByte& SupportNumCores()const {return m_iNumCores;}
    inline const float&    SupportSSE     ()const {return m_fSSE;}
    inline const float&    SupportAVX     ()const {return m_fAVX;}
    //! @}


private:
    //! update the window event system
    //! @{
    bool __UpdateEvents();
    //! @}

    //! update the high precision time
    //! @{
    void __UpdateTime();
    //! @}
};


#endif // _CORE_GUARD_SYSTEM_H_