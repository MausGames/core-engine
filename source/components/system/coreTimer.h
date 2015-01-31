//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_TIMER_H_
#define _CORE_GUARD_TIMER_H_


// ****************************************************************
/* timer definitions */
enum coreTimerPlay : bool
{
    CORE_TIMER_PLAY_CURRENT = false,   //!< play from current position
    CORE_TIMER_PLAY_RESET   = true     //!< play from the beginning
};

enum coreTimerGet : bool
{
    CORE_TIMER_GET_NORMAL   = false,   //!< get current value
    CORE_TIMER_GET_REVERSED = true     //!< get reversed current value (end - value)
};


// ****************************************************************
/* timer class */
class coreTimer final
{
private:
    float m_fValue;         //!< current value
    float m_fEnd;           //!< target value
    float m_fSpeed;         //!< speed factor of the timer

    coreUint m_iMaxLoops;   //!< max number of loops (0 = infinite)
    coreUint m_iCurLoops;   //!< current number of loops

    int  m_iTimeID;         //!< ID of the used frame time
    bool m_bStatus;         //!< current play status


public:
    constexpr_func coreTimer()noexcept;
    constexpr_func coreTimer(const float& fEnd, const float& fSpeed, const coreUint& iLoops)noexcept;

    ENABLE_COPY(coreTimer)

    /*! update the timer */
    //! @{
    bool Update(const float& fSpeedModifier);
    //! @}

    /*! control the timer */
    //! @{
    inline void Play (const coreTimerPlay& bReset) {m_bStatus = true; if(bReset) this->Reset();}
    inline void Pause()                            {m_bStatus = false;}
    inline void Reset()                            {m_fValue  = 0.0f; m_iCurLoops = 0;}
    inline void Stop ()                            {this->Pause(); this->Reset();}
    //! @}

    /*! set all basic object properties */
    //! @{
    inline void Set(const float& fEnd, const float& fSpeed, const coreUint& iLoops) {m_fEnd = fEnd; m_fSpeed = fSpeed; m_iMaxLoops = iLoops;}
    //! @}

    /*! set object properties */
    //! @{
    inline void SetValue   (const float&    fValue)    {m_fValue    = fValue;}
    inline void SetEnd     (const float&    fEnd)      {m_fEnd      = fEnd;}
    inline void SetSpeed   (const float&    fSpeed)    {m_fSpeed    = fSpeed;}
    inline void SetMaxLoops(const coreUint& iMaxLoops) {m_iMaxLoops = iMaxLoops;}
    inline void SetCurLoops(const coreUint& iCurLoops) {m_iCurLoops = iCurLoops;}
    inline void SetTimeID  (const int&      iTimeID)   {m_iTimeID   = iTimeID;}
    //! @}

    /*! get object properties */
    //! @{
    inline float           GetValue   (const coreTimerGet& bReversed)const {return bReversed ? (m_fEnd - m_fValue) : m_fValue;}
    inline const float&    GetEnd     ()const                              {return m_fEnd;}
    inline const float&    GetSpeed   ()const                              {return m_fSpeed;}
    inline const coreUint& GetMaxLoops()const                              {return m_iMaxLoops;}
    inline const coreUint& GetCurLoops()const                              {return m_iCurLoops;}
    inline const int&      GetTimeID  ()const                              {return m_iTimeID;}
    inline const bool&     GetStatus  ()const                              {return m_bStatus;}
    //! @}
};


// ****************************************************************
/* flowing value class */
class coreFlow final
{
private:
    float m_fValue;   //!< current value


public:
    constexpr_func coreFlow()noexcept                    : m_fValue (0.0f)   {}
    constexpr_func coreFlow(const float& fValue)noexcept : m_fValue (fValue) {}

    ENABLE_COPY(coreFlow)

    /*! access current value */
    //! @{
    inline operator       float& ()           {return  m_fValue;}
    inline operator const float& ()const      {return  m_fValue;}
    inline float& operator = (const float& f) {return (m_fValue = f);}
    //! @}

    /*! update the flowing value */
    //! @{
    inline void Update(const float& fSpeed)                     {m_fValue += fSpeed * Core::System->GetTime();}
    inline void Update(const float& fSpeed, const int& iTimeID) {m_fValue += fSpeed * Core::System->GetTime(iTimeID);}
    //! @}
};


// ****************************************************************
/* constructor */
constexpr_func coreTimer::coreTimer()noexcept
: coreTimer (1.0f, 1.0f, 0)
{
}

constexpr_func coreTimer::coreTimer(const float& fEnd, const float& fSpeed, const coreUint& iLoops)noexcept
: m_fValue    (0.0f)
, m_fEnd      (fEnd)
, m_fSpeed    (fSpeed)
, m_iMaxLoops (iLoops)
, m_iCurLoops (0)
, m_iTimeID   (-1)
, m_bStatus   (false)
{
}


#endif /* _CORE_GUARD_TIMER_H_ */