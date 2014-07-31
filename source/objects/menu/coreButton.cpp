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
coreButton::coreButton(const char* pcIdle, const char* pcBusy, const char* pcFont, const int& iHeight, const coreUint& iLength)noexcept
: coreButton ()
{
    // construct on creation
    this->Construct(pcIdle, pcBusy, pcFont, iHeight, iLength);
}

coreButton::coreButton(const char* pcIdle, const char* pcBusy)noexcept
: coreButton ()
{
    // construct on creation
    this->Construct(pcIdle, pcBusy);
}


// ****************************************************************
// destructor
coreButton::~coreButton()
{
    // delete the label
    SAFE_DELETE(m_pCaption)
}


// ****************************************************************
// construct the button
void coreButton::Construct(const char* pcIdle, const char* pcBusy, const char* pcFont, const int& iHeight, const coreUint& iLength)
{
    // create the label
    if(m_pCaption) SAFE_DELETE(m_pCaption)
    m_pCaption = new coreLabel(pcFont, iHeight, iLength);

    // construct remaining object 
    this->Construct(pcIdle, pcBusy);
}

void coreButton::Construct(const char* pcIdle, const char* pcBusy)
{
    // load background textures
    if(pcIdle) m_apBackground[0] = Core::Manager::Resource->Get<coreTexture>(pcIdle);
    if(pcBusy) m_apBackground[1] = Core::Manager::Resource->Get<coreTexture>(pcBusy);
    m_apTexture[0] = m_apBackground[0];

    // load shaders
    this->DefineProgram(Core::Manager::Resource->Load<coreProgram>("default_2d",      CORE_RESOURCE_UPDATE_AUTO,   NULL))
        ->AttachShader (Core::Manager::Resource->Load<coreShader> ("default_2d.vert", CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_2d.vert"))
        ->AttachShader (Core::Manager::Resource->Load<coreShader> ("default_2d.frag", CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_2d.frag"))
        ->Finish();
}


// ****************************************************************
// render the button
void coreButton::Render()
{
    ASSERT(m_pProgram)

    // render the 2d-object
    coreObject2D::Render();

    // render the label
    if(m_pCaption)
    {
        m_pCaption->SetAlpha(this->GetAlpha());
        m_pCaption->Render();
    }
}


// ****************************************************************    
// move the button
void coreButton::Move()
{
    ASSERT(m_pProgram)

    // set current background texture
    const bool bStatus = (m_iOverride > 0) ? true : ((m_iOverride < 0) ? false : this->IsFocused());
    if(m_bBusy != bStatus)
    {
        m_bBusy = bStatus;
        m_apTexture[0] = m_apBackground[m_bBusy ? 1 : 0];
    }

    // update the label
    if(m_pCaption) 
    {
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