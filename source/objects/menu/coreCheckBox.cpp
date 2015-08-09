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
// constructor
coreCheckBox::coreCheckBox()noexcept
: coreButton    ()
, m_apUnchecked {NULL, NULL}
, m_apChecked   {NULL, NULL}
, m_bCheck      (false)
{
}

coreCheckBox::coreCheckBox(const coreChar* pcIdleUnchecked, const coreChar* pcBusyUnchecked, const coreChar* pcIdleChecked, const coreChar* pcBusyChecked)noexcept
: coreCheckBox ()
{
    // construct on creation
    this->Construct(pcIdleUnchecked, pcBusyUnchecked, pcIdleChecked, pcBusyChecked);
}

coreCheckBox::coreCheckBox(const coreChar* pcIdleUnchecked, const coreChar* pcBusyUnchecked, const coreChar* pcIdleChecked, const coreChar* pcBusyChecked, const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iOutline)noexcept
: coreCheckBox ()
{
    // construct on creation
    this->Construct(pcIdleUnchecked, pcBusyUnchecked, pcIdleChecked, pcBusyChecked, pcFont, iHeight, iOutline);
}


// ****************************************************************
// construct the check-box
void coreCheckBox::Construct(const coreChar* pcIdleUnchecked, const coreChar* pcBusyUnchecked, const coreChar* pcIdleChecked, const coreChar* pcBusyChecked)
{
    // construct the button
    coreButton::Construct(pcIdleUnchecked, pcBusyUnchecked);

    // construct remaining object
    this->__Construct(pcIdleChecked, pcBusyChecked);
}

void coreCheckBox::Construct(const coreChar* pcIdleUnchecked, const coreChar* pcBusyUnchecked, const coreChar* pcIdleChecked, const coreChar* pcBusyChecked, const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iOutline)
{
    // construct the button with label
    coreButton::Construct(pcIdleUnchecked, pcBusyUnchecked, pcFont, iHeight, iOutline, 0u);
    this->GetCaption()->SetText   ("X");
    this->GetCaption()->SetEnabled(CORE_OBJECT_ENABLE_NOTHING);

    // construct remaining object
    this->__Construct(pcIdleChecked, pcBusyChecked);
}


// ****************************************************************
// move the check-box
void coreCheckBox::Move()
{
    // move the button
    coreButton::Move();

    // check for interaction
    if(this->IsClicked()) this->SetCheck(!m_bCheck);
}


// ****************************************************************
// set check status
void coreCheckBox::SetCheck(const coreBool& bCheck)
{
    if(m_bCheck == bCheck) return;

    // save new check status
    m_bCheck = bCheck;

    // change background textures
    m_apBackground[0] = m_bCheck ? m_apChecked[0] : m_apUnchecked[0];
    m_apBackground[1] = m_bCheck ? m_apChecked[1] : m_apUnchecked[1];

    // change label visibility
    if(this->GetCaption())
    {
        this->GetCaption()->SetEnabled(m_bCheck ? CORE_OBJECT_ENABLE_ALL : CORE_OBJECT_ENABLE_NOTHING);
        this->GetCaption()->Move();
    }

    // invoke background update
    m_bBusy = !m_bBusy;
}


// ****************************************************************
// construct the check-box
void coreCheckBox::__Construct(const coreChar* pcIdleChecked, const coreChar* pcBusyChecked)
{
    // load background textures
    m_apUnchecked[0] = m_apBackground[0];
    m_apUnchecked[1] = m_apBackground[1];
    if(pcIdleChecked) m_apChecked[0] = Core::Manager::Resource->Get<coreTexture>(pcIdleChecked);
    if(pcBusyChecked) m_apChecked[1] = Core::Manager::Resource->Get<coreTexture>(pcBusyChecked);
}