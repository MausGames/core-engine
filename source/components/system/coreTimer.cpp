//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
coreTimer::coreTimer()noexcept
: m_fCurrent (0.0f)
, m_fEnd     (1.0f)
, m_fSpeed   (1.0f)
, m_iMaxLoop (0)
, m_iCurLoop (0)
, m_iTimeID  (-1)
, m_bStatus  (false)
{
}

coreTimer::coreTimer(const float& fEnd, const float& fSpeed, const coreUint& iLoops)noexcept
: m_fCurrent (0.0f)
, m_fEnd     (fEnd)
, m_fSpeed   (fSpeed)
, m_iMaxLoop (iLoops)
, m_iCurLoop (0)
, m_iTimeID  (-1)
, m_bStatus  (false)
{
}


// ****************************************************************
// update the timer
bool coreTimer::Update(const float fModifier)
{
    if(m_bStatus)
    {
        // increase and check current value
        m_fCurrent += m_fSpeed*fModifier*Core::System->GetTime(m_iTimeID);
        if(m_fCurrent >= m_fEnd)
        {
            // target value reached
            if((m_iMaxLoop-1 > m_iCurLoop) || !m_iMaxLoop)
            {
                // invoke next loop
                m_fCurrent -= m_fEnd;
                ++m_iCurLoop;
            }
            else
            {
                // stop the timer
                m_fCurrent = m_fEnd;
                m_bStatus  = false;
            }

            return true;
        }
    }

    return false;
}


// ****************************************************************
// update the flowing value
float& coreFlow::Update(const float& fSpeed)
{
    m_fCurrent += fSpeed*Core::System->GetTime();
    return m_fCurrent;
}

float& coreFlow::Update(const float& fSpeed, const int& iTimeID)
{
    m_fCurrent += fSpeed*Core::System->GetTime(iTimeID);
    return m_fCurrent;
}