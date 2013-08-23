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
// timer class
class coreTimer
{
private:
    float m_fCurrent;      // current value
    float m_fEnd;          // target value
    float m_fSpeed;        // speed factor of the timer

    coreUint m_iMaxLoop;   // max number of loops (0 = infinite)
    coreUint m_iCurLoop;   // current number of loops

    int m_iTimeID;         // ID of the used frame time 
    bool m_bStatus;        // current status


public:
    coreTimer();
    coreTimer(const float& fEnd, const float& fSpeed, const coreUint& iLoop);

    // update the timer
    bool Update(const float fModifier = 1.0f);

    // control the timer
    inline void Play(const bool& bReset) {if(bReset) this->Reset(); m_bStatus = true;}
    inline void Pause()                  {m_bStatus = false;}
    inline void Reset()                  {m_fCurrent = 0.0f; m_iCurLoop = 0;}
    inline void Stop()                   {this->Pause(); this->Reset();}

    // set the timer basic attributes
    inline void Set(const float& fEnd, const float& fSpeed, const coreUint& iLoop) {m_fEnd = fEnd; m_fSpeed = fSpeed; m_iMaxLoop = iLoop;}

    // set attributes
    inline void SetCurrent(const float& fCurrent)    {m_fCurrent = fCurrent;}
    inline void SetEnd(const float& fEnd)            {m_fEnd     = fEnd;}
    inline void SetSpeed(const float& fSpeed)        {m_fSpeed   = fSpeed;}
    inline void SetMaxLoop(const coreUint& iMaxLoop) {m_iMaxLoop = iMaxLoop;}
    inline void SetCurLoop(const coreUint& iCurLoop) {m_iCurLoop = iCurLoop;}
    inline void SetTimeID(const int& iTimeID)        {m_iTimeID  = iTimeID;}

    // get attributes
    inline float GetCurrent(const bool& bReverse)const {return bReverse ? m_fEnd-m_fCurrent : m_fCurrent;}
    inline const float& GetEnd()const                  {return m_fEnd;}
    inline const float& GetSpeed()const                {return m_fSpeed;}
    inline const coreUint& GetMaxLoop()const           {return m_iMaxLoop;}
    inline const coreUint& GetCurLoop()const           {return m_iCurLoop;}
    inline const int& GetTimeID()const                 {return m_iTimeID;}
    inline const bool& GetStatus()const                {return m_bStatus;}
};


// ****************************************************************
// simple timer class
class coreSimpleTimer
{
private:
    float m_fCurrent;   // current value


public:
    coreSimpleTimer() : m_fCurrent(0.0f) {}

    // access current value
    inline operator float& ()                 {return m_fCurrent;}
    inline float& operator = (const float& f) {m_fCurrent = f; return m_fCurrent;}

    // update the timer
    float& Update(const float& fSpeed);
    float& Update(const float& fSpeed, const int& iTimeID);
};