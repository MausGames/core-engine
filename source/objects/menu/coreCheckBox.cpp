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
// constructor
coreCheckBox::coreCheckBox(const char* pcIdleUnchecked, const char* pcBusyUnchecked, const char* pcIdleChecked, const char* pcBusyChecked)
: coreButton (pcIdleUnchecked, pcBusyUnchecked)
, m_bCheck   (false)
{
    // load background textures
    m_apUnchecked[0] = m_apBackground[0];
    m_apUnchecked[1] = m_apBackground[1];
    if(pcIdleChecked) m_apChecked[0] = Core::Manager::Resource->LoadFile<coreTexture>(pcIdleChecked);
    if(pcBusyChecked) m_apChecked[1] = Core::Manager::Resource->LoadFile<coreTexture>(pcBusyChecked);
}


// ****************************************************************
// Auswahlkasten berechnen
void coreCheckBox::Move()
{
    // move the button
    coreButton::Move();

    // check for interaction
    if(this->IsPushed()) this->SetCheck(!m_bCheck);
}


// ****************************************************************
// set check status
void coreCheckBox::SetCheck(const bool& bCheck)
{
    if(m_bCheck == bCheck) return;

    // save new check status
    m_bCheck = bCheck;

    // change background textures
    m_apBackground[0] = m_bCheck ? m_apChecked[0] : m_apUnchecked[0];
    m_apBackground[1] = m_bCheck ? m_apChecked[1] : m_apUnchecked[1];

    // invoke background update
    m_bBusy = !m_bBusy;
}