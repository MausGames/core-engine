//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* update the timer */
coreBool coreTimer::Update(const coreFloat fSpeedModifier)
{
    if(m_bStatus)
    {
        // increase and check current value
        m_fValue += m_fSpeed * fSpeedModifier * Core::System->GetTime(m_iTimeID);
        if(m_fValue >= m_fEnd)
        {
            // target value reached
            if(++m_iCurLoops == m_iMaxLoops)
            {
                // halt the timer
                m_fValue = m_fEnd;
                this->Pause();
            }
            else
            {
                // invoke next loop
                m_fValue -= m_fEnd;
            }
            return true;
        }
    }
    return false;
}