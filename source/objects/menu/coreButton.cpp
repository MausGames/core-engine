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
/* constructor */
coreButton::coreButton()noexcept
: coreObject2D   ()
, m_apBackground {NULL, NULL}
, m_pCaption     (NULL)
, m_bBusy        (false)
, m_iOverride    (0)
{
    // enable interaction handling
    m_bFocusable = true;
}

coreButton::coreButton(const coreHashString& sIdle, const coreHashString& sBusy, const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline)noexcept
: coreButton ()
{
    // construct on creation
    this->Construct(sIdle, sBusy, sFont, iHeight, iOutline);
}

coreButton::coreButton(const coreHashString& sIdle, const coreHashString& sBusy)noexcept
: coreButton ()
{
    // construct on creation
    this->Construct(sIdle, sBusy);
}


// ****************************************************************
/* destructor */
coreButton::~coreButton()
{
    // delete the label
    MANAGED_DELETE(m_pCaption)
}


// ****************************************************************
/* construct the button */
void coreButton::Construct(const coreHashString& sIdle, const coreHashString& sBusy, const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline)
{
    // create the label
    if(!m_pCaption) m_pCaption = MANAGED_NEW(coreLabel);
    m_pCaption->Construct(sFont, iHeight, iOutline);

    // construct remaining object
    this->Construct(sIdle, sBusy);
}

void coreButton::Construct(const coreHashString& sIdle, const coreHashString& sBusy)
{
    // load background textures
    if(sIdle) m_apBackground[0] = Core::Manager::Resource->Get<coreTexture>(sIdle);
    if(sBusy) m_apBackground[1] = Core::Manager::Resource->Get<coreTexture>(sBusy);
    m_apTexture[0] = m_apBackground[0];

    // load shader-program
    this->DefineProgram("default_2d_program");
}


// ****************************************************************
/* render the button */
void coreButton::Render()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_RENDER)) return;

    // render the 2d-object
    if(m_apTexture[0]) this->coreObject2D::Render();

    // render the label
    if(m_pCaption)
    {
        m_pCaption->SetAlpha(this->GetAlpha());
        m_pCaption->Render();
    }
}


// ****************************************************************
/* move the button */
void coreButton::Move()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_MOVE)) return;

    // override focus status
    if(m_iOverride < 0) this->SetFocused(false);

    // set current background texture
    const coreBool bStatus = (m_iOverride > 0) ? true : ((m_iOverride < 0) ? false : this->IsFocused());
    if(m_bBusy != bStatus)
    {
        m_bBusy = bStatus;
        m_apTexture[0] = m_apBackground[m_bBusy ? 1u : 0u];
    }

    // update the label
    if(m_pCaption)
    {
        if(HAS_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_TRANSFORM))
        {
            m_pCaption->SetPosition (this->GetPosition() + 0.5f*this->GetSize()*this->GetAlignment());
            m_pCaption->SetDirection(this->GetDirection());
            m_pCaption->SetCenter   (this->GetCenter());
            m_pCaption->SetStyle    (this->GetStyle());
            m_pCaption->Move();
        }
    }

    // move the 2d-object
    this->coreObject2D::Move();
}