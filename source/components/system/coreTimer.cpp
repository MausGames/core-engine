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
coreUint16 coreTimer::Update(const coreFloat fSpeedModifier)
{
    if(m_bStatus)
    {
        // increase and check current value
        m_fValue += m_fSpeed * fSpeedModifier * Core::System->GetTime(m_iTimeID);
        if(m_fValue >= m_fEnd)
        {
            coreUint32 iCount = F_TO_UI(m_fValue / m_fEnd);
            ASSERT(m_iCurLoops + iCount <= UINT16_MAX)

            // target value reached
            if(m_iMaxLoops && ((m_iCurLoops += iCount) >= m_iMaxLoops))
            {
                iCount -= m_iCurLoops - m_iMaxLoops;

                // halt the timer
                m_fValue    = m_fEnd;
                m_iCurLoops = m_iMaxLoops;
                this->Pause();
            }
            else
            {
                // invoke next loop
                m_fValue -= m_fEnd * I_TO_F(iCount);
            }
            return iCount;
        }
    }
    return 0u;
}