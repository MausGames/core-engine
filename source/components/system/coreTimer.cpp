//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// update the timer
bool coreTimer::Update(const float& fModifier)
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