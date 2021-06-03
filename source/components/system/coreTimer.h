///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_TIMER_H_
#define _CORE_GUARD_TIMER_H_


// ****************************************************************
/* timer definitions */
enum coreTimerPlay : coreBool
{
    CORE_TIMER_PLAY_CURRENT = false,   // play from current position
    CORE_TIMER_PLAY_RESET   = true     // play from the beginning
};

enum coreTimerGet : coreBool
{
    CORE_TIMER_GET_NORMAL   = false,   // get current value
    CORE_TIMER_GET_REVERSED = true     // get reversed current value (end - value)
};


// ****************************************************************
/* timer class */
class coreTimer final
{
private:
    coreFloat m_fValue;       // current value
    coreFloat m_fEnd;         // target value
    coreFloat m_fSpeed;       // speed factor of the timer

    coreUint16 m_iMaxLoops;   // max number of loops (0 = infinite)
    coreUint16 m_iCurLoops;   // current number of loops

    coreInt8 m_iTimeID;       // ID of the used frame time
    coreBool m_bStatus;       // current play status


public:
    constexpr coreTimer(const coreFloat fEnd, const coreFloat fSpeed, const coreUint16 iLoops)noexcept;
    constexpr coreTimer()noexcept;

    ENABLE_COPY(coreTimer)

    /* update the timer */
    coreBool Update(const coreFloat fSpeedModifier);

    /* control the timer */
    inline void Play (const coreTimerPlay eReset) {m_bStatus = true; if(eReset) this->Reset();}
    inline void Pause()                           {m_bStatus = false;}
    inline void Reset()                           {m_fValue  = 0.0f; m_iCurLoops = 0u;}
    inline void Stop ()                           {this->Pause(); this->Reset();}

    /* set all basic object properties */
    inline void Set(const coreFloat fEnd, const coreFloat fSpeed, const coreUint16 iLoops) {m_fEnd = fEnd; m_fSpeed = fSpeed; m_iMaxLoops = iLoops;}

    /* set object properties */
    inline void SetValue   (const coreFloat  fValue)    {m_fValue    = fValue;}
    inline void SetEnd     (const coreFloat  fEnd)      {m_fEnd      = fEnd;}
    inline void SetSpeed   (const coreFloat  fSpeed)    {m_fSpeed    = fSpeed;}
    inline void SetMaxLoops(const coreUint16 iMaxLoops) {m_iMaxLoops = iMaxLoops;}
    inline void SetCurLoops(const coreUint16 iCurLoops) {m_iCurLoops = iCurLoops;}
    inline void SetTimeID  (const coreInt8   iTimeID)   {m_iTimeID   = iTimeID;}

    /* get object properties */
    inline       coreFloat   GetValue   (const coreTimerGet eReversed)const {return eReversed ? (m_fEnd - m_fValue) : m_fValue;}
    inline const coreFloat&  GetEnd     ()const                             {return m_fEnd;}
    inline const coreFloat&  GetSpeed   ()const                             {return m_fSpeed;}
    inline const coreUint16& GetMaxLoops()const                             {return m_iMaxLoops;}
    inline const coreUint16& GetCurLoops()const                             {return m_iCurLoops;}
    inline const coreInt8&   GetTimeID  ()const                             {return m_iTimeID;}
    inline const coreBool&   GetStatus  ()const                             {return m_bStatus;}
};


// ****************************************************************
/* flowing value class */
class coreFlow final
{
private:
    coreFloat m_fValue;   // current value


public:
    coreFlow() = default;
    constexpr FORCE_INLINE coreFlow(const coreFloat fValue)noexcept : m_fValue (fValue) {}

    ENABLE_COPY(coreFlow)

    /* access the flowing value */
    inline    operator       coreFloat& ()      {return m_fValue;}
    constexpr operator const coreFloat& ()const {return m_fValue;}
    constexpr const coreFloat& ToFloat  ()const {return m_fValue;}

    /* update the flowing value */
    inline void Update(const coreFloat fSpeed)                         {m_fValue += fSpeed * Core::System->GetTime();}
    inline void Update(const coreFloat fSpeed, const coreInt8 iTimeID) {m_fValue += fSpeed * Core::System->GetTime(iTimeID);}

    /* update the flowing value with modulo operation */
    inline void UpdateMod(const coreFloat fSpeed, const coreFloat fMod)                         {m_fValue = FMOD(m_fValue + fSpeed * Core::System->GetTime(),        fMod);}
    inline void UpdateMod(const coreFloat fSpeed, const coreFloat fMod, const coreInt8 iTimeID) {m_fValue = FMOD(m_fValue + fSpeed * Core::System->GetTime(iTimeID), fMod);}

    /* update the flowing value with min and max operation */
    inline void UpdateMin(const coreFloat fSpeed, const coreFloat fMin)                         {m_fValue = MIN(m_fValue + fSpeed * Core::System->GetTime(),        fMin);}
    inline void UpdateMin(const coreFloat fSpeed, const coreFloat fMin, const coreInt8 iTimeID) {m_fValue = MIN(m_fValue + fSpeed * Core::System->GetTime(iTimeID), fMin);}
    inline void UpdateMax(const coreFloat fSpeed, const coreFloat fMax)                         {m_fValue = MAX(m_fValue + fSpeed * Core::System->GetTime(),        fMax);}
    inline void UpdateMax(const coreFloat fSpeed, const coreFloat fMax, const coreInt8 iTimeID) {m_fValue = MAX(m_fValue + fSpeed * Core::System->GetTime(iTimeID), fMax);}
};


// ****************************************************************
/* constructor */
constexpr coreTimer::coreTimer(const coreFloat fEnd, const coreFloat fSpeed, const coreUint16 iLoops)noexcept
: m_fValue    (0.0f)
, m_fEnd      (fEnd)
, m_fSpeed    (fSpeed)
, m_iMaxLoops (iLoops)
, m_iCurLoops (0u)
, m_iTimeID   (-1)
, m_bStatus   (false)
{
}

constexpr coreTimer::coreTimer()noexcept
: coreTimer (1.0f, 1.0f, 0u)
{
}


// ****************************************************************
/* additional checks */
STATIC_ASSERT(std::is_trivial<coreFlow>::value == true)


#endif /* _CORE_GUARD_TIMER_H_ */