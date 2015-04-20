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

// TODO: implement flow-interface to reduce clamped to zero


// ****************************************************************
/* timer definitions */
enum coreTimerPlay : coreBool
{
    CORE_TIMER_PLAY_CURRENT = false,   //!< play from current position
    CORE_TIMER_PLAY_RESET   = true     //!< play from the beginning
};

enum coreTimerGet : coreBool
{
    CORE_TIMER_GET_NORMAL   = false,   //!< get current value
    CORE_TIMER_GET_REVERSED = true     //!< get reversed current value (end - value)
};


// ****************************************************************
/* timer class */
class coreTimer final
{
private:
    coreFloat m_fValue;       //!< current value
    coreFloat m_fEnd;         //!< target value
    coreFloat m_fSpeed;       //!< speed factor of the timer

    coreUint16 m_iMaxLoops;   //!< max number of loops (0 = infinite)
    coreUint16 m_iCurLoops;   //!< current number of loops

    coreInt8 m_iTimeID;       //!< ID of the used frame time
    coreBool m_bStatus;       //!< current play status


public:
    constexpr_func coreTimer()noexcept;
    constexpr_func coreTimer(const coreFloat& fEnd, const coreFloat& fSpeed, const coreUint16& iLoops)noexcept;

    ENABLE_COPY(coreTimer)

    /*! update the timer */
    //! @{
    coreBool Update(const coreFloat& fSpeedModifier);
    //! @}

    /*! control the timer */
    //! @{
    inline void Play (const coreTimerPlay& bReset) {m_bStatus = true; if(bReset) this->Reset();}
    inline void Pause()                            {m_bStatus = false;}
    inline void Reset()                            {m_fValue  = 0.0f; m_iCurLoops = 0u;}
    inline void Stop ()                            {this->Pause(); this->Reset();}
    //! @}

    /*! set all basic object properties */
    //! @{
    inline void Set(const coreFloat& fEnd, const coreFloat& fSpeed, const coreUint16& iLoops) {m_fEnd = fEnd; m_fSpeed = fSpeed; m_iMaxLoops = iLoops;}
    //! @}

    /*! set object properties */
    //! @{
    inline void SetValue   (const coreFloat&  fValue)    {m_fValue    = fValue;}
    inline void SetEnd     (const coreFloat&  fEnd)      {m_fEnd      = fEnd;}
    inline void SetSpeed   (const coreFloat&  fSpeed)    {m_fSpeed    = fSpeed;}
    inline void SetMaxLoops(const coreUint16& iMaxLoops) {m_iMaxLoops = iMaxLoops;}
    inline void SetCurLoops(const coreUint16& iCurLoops) {m_iCurLoops = iCurLoops;}
    inline void SetTimeID  (const coreInt8&   iTimeID)   {m_iTimeID   = iTimeID;}
    //! @}

    /*! get object properties */
    //! @{
    inline       coreFloat   GetValue   (const coreTimerGet& bReversed)const {return bReversed ? (m_fEnd - m_fValue) : m_fValue;}
    inline const coreFloat&  GetEnd     ()const                              {return m_fEnd;}
    inline const coreFloat&  GetSpeed   ()const                              {return m_fSpeed;}
    inline const coreUint16& GetMaxLoops()const                              {return m_iMaxLoops;}
    inline const coreUint16& GetCurLoops()const                              {return m_iCurLoops;}
    inline const coreInt8&   GetTimeID  ()const                              {return m_iTimeID;}
    inline const coreBool&   GetStatus  ()const                              {return m_bStatus;}
    //! @}
};


// ****************************************************************
/* flowing value class */
class coreFlow final
{
private:
    coreFloat m_fValue;   //!< current value


public:
    constexpr_func coreFlow() = default;
    constexpr_func coreFlow(const coreFloat& fValue)noexcept : m_fValue (fValue) {}

    ENABLE_COPY(coreFlow)

    /*! access current value */
    //! @{
    inline operator       coreFloat& ()               {return  m_fValue;}
    inline operator const coreFloat& ()const          {return  m_fValue;}
    inline coreFloat& operator = (const coreFloat& f) {return (m_fValue = f);}
    //! @}

    /*! update the flowing value */
    //! @{
    inline void Update(const coreFloat& fSpeed)                          {m_fValue += fSpeed * Core::System->GetTime();}
    inline void Update(const coreFloat& fSpeed, const coreInt8& iTimeID) {m_fValue += fSpeed * Core::System->GetTime(iTimeID);}
    //! @}
};


// ****************************************************************
/* constructor */
constexpr_func coreTimer::coreTimer()noexcept
: coreTimer (1.0f, 1.0f, 0u)
{
}

constexpr_func coreTimer::coreTimer(const coreFloat& fEnd, const coreFloat& fSpeed, const coreUint16& iLoops)noexcept
: m_fValue    (0.0f)
, m_fEnd      (fEnd)
, m_fSpeed    (fSpeed)
, m_iMaxLoops (iLoops)
, m_iCurLoops (0u)
, m_iTimeID   (-1)
, m_bStatus   (false)
{
}


#endif /* _CORE_GUARD_TIMER_H_ */