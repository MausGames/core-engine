//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

#define CORE_LABEL_DETAIL 0.95f


// ****************************************************************
// constructor
coreLabel::coreLabel()noexcept
: m_iHeight     (0)
, m_vResolution (coreVector2(0.0f,0.0f))
, m_iLength     (0)
, m_sText       ("")
, m_fScale      (1.0f)
, m_iUpdate     (CORE_LABEL_UPDATE_NOTHING)
{
}

coreLabel::coreLabel(const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iLength)noexcept
: coreLabel ()
{
    // construct on creation
    this->Construct(pcFont, iHeight, iLength);
}


// ****************************************************************
// destructor
coreLabel::~coreLabel()
{
    // free own texture
    Core::Manager::Resource->Free(&m_apTexture[0]);
}


// ****************************************************************
// construct the label
void coreLabel::Construct(const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iLength)
{
    // save properties
    m_iHeight = F_TO_UI(I_TO_F(iHeight) * (Core::System->GetResolution().y / 800.0f) * CORE_LABEL_DETAIL);
    m_iLength = iLength;

    // set font object
    m_pFont = Core::Manager::Resource->Get<coreFont>(pcFont);

    // allocate own texture to display text
    m_apTexture[0] = Core::Manager::Resource->LoadNew<coreTexture>();

    // load shader-program
    this->DefineProgram("default_label_sharp_program");

    // reserve memory for text
    if(iLength) m_sText.reserve(iLength + 1);
}


// ****************************************************************
// render the label
void coreLabel::Render()
{
    ASSERT(m_pProgram)
    if(m_sText.empty()) return;

    if(m_iUpdate)
    {
        // check if requested font is loaded
        if(!m_pFont.GetHandle()->IsLoaded()) return;

        if(CONTAINS_VALUE(m_iUpdate, CORE_LABEL_UPDATE_TEXTURE))
        {
            // generate the texture
            this->__Generate(m_sText.c_str(), m_iLength ? true : false);
        }
        if(CONTAINS_VALUE(m_iUpdate, CORE_LABEL_UPDATE_SIZE))
        {
            // update the object size
            this->SetSize(m_fScale * m_vTexSize * (m_vResolution * RCP(Core::System->GetResolution().y)) * (1.0f / CORE_LABEL_DETAIL));
            coreObject2D::Move();
        }

        // reset the update status
        m_iUpdate = CORE_LABEL_UPDATE_NOTHING;
    }

    // render the 2d-object
    coreObject2D::Render();
}


// ****************************************************************
// move the label
void coreLabel::Move()
{
    ASSERT(m_pProgram)
    if(m_sText.empty()) return;

    // move the 2d-object
    if(!CONTAINS_VALUE(m_iUpdate, CORE_LABEL_UPDATE_SIZE)) coreObject2D::Move();
}


// ****************************************************************
// change the current text
coreBool coreLabel::SetText(const coreChar* pcText)
{
    ASSERT(!m_iLength || std::strlen(pcText) <= m_iLength)

    // check for new text
    if(std::strcmp(m_sText.c_str(), pcText))
    {
        ADD_VALUE(m_iUpdate, CORE_LABEL_UPDATE_ALL)

        // change the current text
        if(m_iLength) m_sText.assign(pcText, m_iLength);
                 else m_sText.assign(pcText);
        return true;
    }
    return false;
}

coreBool coreLabel::SetText(const coreChar* pcText, const coreUint8& iNum)
{
    ASSERT(!m_iLength || iNum <= m_iLength)

    // check for new text
    if(iNum != m_sText.length() || std::strcmp(m_sText.c_str(), pcText))
    {
        ADD_VALUE(m_iUpdate, CORE_LABEL_UPDATE_ALL)

        // change the current text
        if(m_iLength) m_sText.assign(pcText, MIN(iNum, m_iLength));
                 else m_sText.assign(pcText, iNum);
        return true;
    }
    return false;
}


// ****************************************************************
// reset with the resource manager
void coreLabel::__Reset(const coreResourceReset& bInit)
{
    if(!m_pFont) return;

    if(bInit)
    {
        // invoke texture generation
        m_vResolution = coreVector2(0.0f,0.0f);
        ADD_VALUE(m_iUpdate, CORE_LABEL_UPDATE_ALL)
    }
    else m_apTexture[0]->Unload();
}


// ****************************************************************
// generate the texture
void coreLabel::__Generate(const coreChar* pcText, const coreBool& bSub)
{
    // create text surface data
    SDL_Surface* pSurface = m_pFont->CreateText(pcText, m_iHeight);
    ASSERT(pSurface->format->BitsPerPixel == 8)

    // convert text surface data
    SDL_Surface* pConvert = SDL_CreateRGBSurface(0, coreMath::CeilAlign<4>(pSurface->w), pSurface->h, 24, CORE_TEXTURE_MASK);
    SDL_BlitSurface(pSurface, NULL, pConvert, NULL);

    if(bSub)
    {
        if(!m_vResolution.x)
        {
            // create static texture
            this->__Generate((std::string(m_iLength, 'W') + "gjy").c_str(), false);
        }

        // update only a specific area of the texture
        m_apTexture[0]->Modify(0, 0, pConvert->w, pConvert->h, pConvert->w * pConvert->h * 3, pConvert->pixels);
    }
    else
    {
        // delete old texture
        m_apTexture[0]->Unload();

        // create new texture
        m_apTexture[0]->Create(pConvert->w, pConvert->h, CORE_TEXTURE_SPEC_RGB, GL_CLAMP_TO_EDGE, false);
        m_apTexture[0]->Modify(0, 0, pConvert->w, pConvert->h, pConvert->w * pConvert->h * 3, pConvert->pixels);

        // save new texture resolution
        m_vResolution = coreVector2(I_TO_F(pConvert->w), I_TO_F(pConvert->h));
    }

    // display only visible texture area
    this->SetTexSize(coreVector2(I_TO_F(pSurface->w), I_TO_F(pSurface->h)) / m_vResolution);
    ASSERT((this->GetTexSize().x <= 1.0f) && (this->GetTexSize().y <= 1.0f))

    // delete text surface data
    SDL_FreeSurface(pSurface);
    SDL_FreeSurface(pConvert);
}