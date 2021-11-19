///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
coreCheckBox::coreCheckBox()noexcept
: coreButton    ()
, m_apUnchecked {NULL, NULL}
, m_apChecked   {NULL, NULL}
, m_bCheck      (false)
{
}

coreCheckBox::coreCheckBox(const coreHashString& sIdleUnchecked, const coreHashString& sBusyUnchecked, const coreHashString& sIdleChecked, const coreHashString& sBusyChecked)noexcept
: coreCheckBox ()
{
    // construct on creation
    this->Construct(sIdleUnchecked, sBusyUnchecked, sIdleChecked, sBusyChecked);
}

coreCheckBox::coreCheckBox(const coreHashString& sIdleUnchecked, const coreHashString& sBusyUnchecked, const coreHashString& sIdleChecked, const coreHashString& sBusyChecked, const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline)noexcept
: coreCheckBox ()
{
    // construct on creation
    this->Construct(sIdleUnchecked, sBusyUnchecked, sIdleChecked, sBusyChecked, sFont, iHeight, iOutline);
}


// ****************************************************************
/* construct the check-box */
void coreCheckBox::Construct(const coreHashString& sIdleUnchecked, const coreHashString& sBusyUnchecked, const coreHashString& sIdleChecked, const coreHashString& sBusyChecked)
{
    // construct the button
    this->coreButton::Construct(sIdleUnchecked, sBusyUnchecked);

    // construct remaining object
    this->__Construct(sIdleChecked, sBusyChecked);
}

void coreCheckBox::Construct(const coreHashString& sIdleUnchecked, const coreHashString& sBusyUnchecked, const coreHashString& sIdleChecked, const coreHashString& sBusyChecked, const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline)
{
    // construct the button with label
    this->coreButton::Construct(sIdleUnchecked, sBusyUnchecked, sFont, iHeight, iOutline);
    this->GetCaption()->SetText   ("X");
    this->GetCaption()->SetEnabled(CORE_OBJECT_ENABLE_NOTHING);

    // construct remaining object
    this->__Construct(sIdleChecked, sBusyChecked);
}


// ****************************************************************
/* move the check-box */
void coreCheckBox::Move()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_MOVE)) return;

    // move the button
    this->coreButton::Move();

    // check for interaction
    if(this->IsClicked()) this->SetCheck(!m_bCheck);
}


// ****************************************************************
/* set check status */
void coreCheckBox::SetCheck(const coreBool bCheck)
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
/* construct the check-box */
void coreCheckBox::__Construct(const coreHashString& sIdleChecked, const coreHashString& sBusyChecked)
{
    // load background textures
    m_apUnchecked[0] = m_apBackground[0];
    m_apUnchecked[1] = m_apBackground[1];
    if(sIdleChecked) m_apChecked[0] = Core::Manager::Resource->Get<coreTexture>(sIdleChecked);
    if(sBusyChecked) m_apChecked[1] = Core::Manager::Resource->Get<coreTexture>(sBusyChecked);
}