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
coreLabel::coreLabel()noexcept
: m_iHeight     (0u)
, m_iOutline    (0u)
, m_iLength     (0u)
, m_vResolution (coreVector2(0.0f,0.0f))
, m_sText       ("")
, m_fScale      (1.0f)
, m_iUpdate     (CORE_LABEL_UPDATE_NOTHING)
{
}

coreLabel::coreLabel(const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iOutline, const coreUint8& iLength)noexcept
: coreLabel ()
{
    // construct on creation
    this->Construct(pcFont, iHeight, iOutline, iLength);
}


// ****************************************************************
// destructor
coreLabel::~coreLabel()
{
    // free own texture
    Core::Manager::Resource->Free(&m_apTexture[1]);
}


// ****************************************************************
// construct the label
void coreLabel::Construct(const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iOutline, const coreUint8& iLength)
{
    // save properties
    m_iHeight  = F_TO_UI(I_TO_F(iHeight) * (Core::System->GetResolution().y / 800.0f) * CORE_LABEL_DETAIL);
    m_iOutline = iOutline;
    m_iLength  = iLength;

    // set font object
    m_pFont = Core::Manager::Resource->Get<coreFont>(pcFont);

    // allocate own texture to display text
    if(m_apTexture[1]) Core::Manager::Resource->Free(&m_apTexture[1]);
    m_apTexture[1] = Core::Manager::Resource->LoadNew<coreTexture>();

    // load shader-program
    this->DefineProgram(iOutline ? "default_label_sharp_program" : "default_label_smooth_program");

    // reserve memory for text
    if(iLength) m_sText.reserve(iLength + 1u);
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
            this->SetSize(m_vTexSize * m_vResolution * (m_fScale * RCP(Core::System->GetResolution().y) / CORE_LABEL_DETAIL));
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
    else m_apTexture[1]->Unload();
}


// ****************************************************************
// generate the texture
void coreLabel::__Generate(const coreChar* pcText, const coreBool& bSub)
{
    SDL_Surface* pSolid   = NULL;
    SDL_Surface* pOutline = NULL;
    coreByte*    pData    = NULL;

    // create solid text surface data
    pSolid = m_pFont->CreateText(pcText, m_iHeight);
    ASSERT(pSolid->format->BitsPerPixel == 8u)

    if(m_iOutline)
    {
        // create outlined text surface data
        pOutline = m_pFont->CreateTextOutline(pcText, m_iHeight, m_iOutline);
        ASSERT(pOutline->format->BitsPerPixel == 8u)
    }

    // set texture properties
    const coreUintW  iComponents = pOutline ? (CORE_GL_SUPPORT(ARB_texture_rg) ? 2u : 3u) : 1u;
    const coreUint32 iWidth      = pOutline ?  pOutline->w                                : pSolid->w;
    const coreUint32 iHeight     = pOutline ? (pSolid->h + 2u * m_iOutline)               : pSolid->h;
    const coreUint32 iPitch      = pOutline ?  pOutline->pitch                            : pSolid->pitch;
    const coreUintW  iSize       = iPitch * iHeight * iComponents;
    ASSERT(!(iPitch % 4u))

    if(pOutline)
    {
        coreByte* pInput1 = s_cast<coreByte*>(pSolid  ->pixels);
        coreByte* pInput2 = s_cast<coreByte*>(pOutline->pixels);

        // allocate buffer to merge solid and outlined pixels
        pData = new coreByte[iSize];
        std::memset(pData, 0, iSize);

        // insert solid pixels
        const coreUintW iOffset = (pOutline->pitch + 1u) * iComponents * m_iOutline;
        for(coreUintW j = 0u, je = pSolid->h; j < je; ++j)
        {
            const coreUintW b = j * pSolid  ->pitch;
            const coreUintW a = j * pOutline->pitch * iComponents + iOffset;

            for(coreUintW i = 0u, ie = pSolid->pitch; i < ie; ++i)
                pData[a + i * iComponents] = pInput1[b + i];
        }

        // insert outlined pixels
        for(coreUintW j = 0u, je = pOutline->h; j < je; ++j)
        {
            const coreUintW b = j * pOutline->pitch;
            const coreUintW a = b * iComponents + 1u;

            for(coreUintW i = 0u, ie = pOutline->pitch; i < ie; ++i)
                pData[a + i * iComponents] = pInput2[b + i];
        }
    }
    else pData = s_cast<coreByte*>(pSolid->pixels);

    if(bSub)
    {
        // create static texture
        if(!m_vResolution.x) this->__Generate((std::string(m_iLength, 'W') + "gjy])").c_str(), false);

        // update only a specific area of the texture
        m_apTexture[1]->Modify(0u, 0u, iPitch, iHeight, iSize, pData);
    }
    else
    {
        // delete old texture
        m_apTexture[1]->Unload();

        // create new texture
        m_apTexture[1]->Create(iPitch, iHeight, CORE_TEXTURE_SPEC_COMPONENTS(iComponents), CORE_TEXTURE_MODE_DEFAULT);
        m_apTexture[1]->Modify(0u, 0u, iPitch, iHeight, iSize, pData);

        // save new texture resolution
        m_vResolution = coreVector2(I_TO_F(iPitch), I_TO_F(iHeight));
    }

    // display only visible texture area
    this->SetTexSize(coreVector2(I_TO_F(iWidth), I_TO_F(iHeight)) / m_vResolution);
    ASSERT((this->GetTexSize().x <= 1.0f) && (this->GetTexSize().y <= 1.0f))

    // delete text surface data
    SDL_FreeSurface(pSolid);
    SDL_FreeSurface(pOutline);
    if(pOutline) SAFE_DELETE_ARRAY(pData)
}