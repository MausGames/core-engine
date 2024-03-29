///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
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
            if(++m_iCurLoops == m_iMaxLoops)   // (0 = infinite)
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