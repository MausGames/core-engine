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
coreButton::coreButton(const char* pcIdle, const char* pcBusy, const char* pcFont, const int& iHeight, const coreUint& iLength)
: coreButton (pcIdle, pcBusy)
{
    // create the caption
    m_pCaption = new coreLabel(pcFont, iHeight, iLength);
}

coreButton::coreButton(const char* pcIdle, const char* pcBusy)
: m_pCaption  (NULL)
, m_bBusy     (false)
, m_iOverride (0)
{
    // load background textures
    if(pcIdle) m_apBackground[0] = Core::Manager::Resource->LoadFile<coreTexture>(pcIdle);
    if(pcBusy) m_apBackground[1] = Core::Manager::Resource->LoadFile<coreTexture>(pcBusy);
    m_apTexture[0] = m_apBackground[0];

    // load shaders
    this->DefineProgramShare(CORE_MEMORY_SHARED)
          ->AttachShaderFile("data/shaders/default_2d.vs")
          ->AttachShaderFile("data/shaders/default.fs")
          ->Finish();
}


// ****************************************************************
// destructor
coreButton::~coreButton()
{
    // delete the caption
    SAFE_DELETE(m_pCaption)
}


// ****************************************************************
// render the button
void coreButton::Render()
{
    // render the 2d-object
    coreObject2D::Render();

    // render the caption
    if(m_pCaption) m_pCaption->Render();
}


// ****************************************************************    
// move the button
void coreButton::Move()
{
    // set current background texture
    const bool bStatus = (m_iOverride > 0) ? true : ((m_iOverride < 0) ? false : this->IsFocused());
    if(m_bBusy != bStatus)
    {
        m_bBusy = bStatus;
        m_apTexture[0] = m_apBackground[m_bBusy ? 1 : 0];
    }

    // update the caption
    if(m_pCaption) 
    {
        m_pCaption->SetAlpha(this->GetAlpha());
        if(m_iUpdate)
        {
            m_pCaption->SetPosition(this->GetPosition() + 0.5f*this->GetSize()*this->GetAlignment());
            m_pCaption->SetDirection(this->GetDirection());
            m_pCaption->SetCenter(this->GetCenter());
            m_pCaption->Move();
        }
    }

    // move the 2d-object
    coreObject2D::Move();
}