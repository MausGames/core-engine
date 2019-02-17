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
// constructor
coreLabel::coreLabel()noexcept
: coreObject2D         ()
, coreTranslate        ()
, coreResourceRelation ()
, m_pFont              (NULL)
, m_iHeight            (0u)
, m_iOutline           (0u)
, m_vResolution        (coreVector2(0.0f,0.0f))
, m_sText              ("")
, m_fScale             (1.0f)
, m_eUpdate            (CORE_LABEL_UPDATE_NOTHING)
{
}

coreLabel::coreLabel(const coreHashString& sFont, const coreUint8 iHeight, const coreUint8 iOutline)noexcept
: coreLabel ()
{
    // construct on creation
    this->Construct(sFont, iHeight, iOutline);
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
void coreLabel::Construct(const coreHashString& sFont, const coreUint8 iHeight, const coreUint8 iOutline)
{
    // save properties
    m_iHeight  = iHeight;
    m_iOutline = iOutline;

    // set font object
    m_pFont = Core::Manager::Resource->Get<coreFont>(sFont);

    // allocate own texture to display text
    if(!m_apTexture[1]) m_apTexture[1] = Core::Manager::Resource->LoadNew<coreTexture>();

    // load shader-program
    this->DefineProgram(iOutline ? "default_label_sharp_program" : "default_label_smooth_program");
}


// ****************************************************************
// render the label
void coreLabel::Render()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_RENDER)) return;

    ASSERT(m_pProgram)
    if(m_sText.empty()) return;

    if(m_eUpdate)
    {
        // check if requested font is loaded
        if(!m_pFont.IsUsable()) return;

        if(CONTAINS_FLAG(m_eUpdate, CORE_LABEL_UPDATE_TEXTURE))
        {
            // generate the texture
            this->__GenerateTexture(m_sText.c_str());
        }
        if(CONTAINS_FLAG(m_eUpdate, CORE_LABEL_UPDATE_SIZE))
        {
            // update the object size
            this->SetSize(this->GetTexSize() * m_vResolution * (CORE_LABEL_SIZE_FACTOR * m_fScale));
            this->coreObject2D::Move();
        }

        // reset the update status
        m_eUpdate = CORE_LABEL_UPDATE_NOTHING;
    }

    // render the 2d-object
    this->coreObject2D::Render();
}


// ****************************************************************
// move the label
void coreLabel::Move()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_MOVE)) return;

    ASSERT(m_pProgram)
    if(m_sText.empty()) return;

    // move the 2d-object
    if(!CONTAINS_FLAG(m_eUpdate, CORE_LABEL_UPDATE_SIZE)) this->coreObject2D::Move();
}


// ****************************************************************
// change the current text
coreBool coreLabel::SetText(const coreChar* pcText)
{
    // check for new text
    if(std::strcmp(m_sText.c_str(), pcText))
    {
        ADD_FLAG(m_eUpdate, CORE_LABEL_UPDATE_ALL)

        // change the current text
        m_sText.assign(pcText);
        return true;
    }
    return false;
}

coreBool coreLabel::SetText(const coreChar* pcText, const coreUint8 iNum)
{
    // check for new text
    if((iNum != m_sText.length()) || std::strcmp(m_sText.c_str(), pcText))
    {
        ADD_FLAG(m_eUpdate, CORE_LABEL_UPDATE_ALL)

        // change the current text
        m_sText.assign(pcText, MIN(iNum, std::strlen(pcText)));
        return true;
    }
    return false;
}


// ****************************************************************
// reset with the resource manager
void coreLabel::__Reset(const coreResourceReset eInit)
{
    if(!m_pFont) return;

    if(eInit)
    {
        // invoke texture generation
        this->RegenerateTexture();
    }
    else
    {
        // unload texture resource data
        m_apTexture[1]->Unload();
    }
}


// ****************************************************************
// generate the texture
void coreLabel::__GenerateTexture(const coreChar* pcText)
{
    coreSurfaceScope pSolid   = NULL;
    coreSurfaceScope pOutline = NULL;
    coreByte*        pData    = NULL;

    // get relative font height
    const coreUint8 iRelHeight = CORE_LABEL_HEIGHT_RELATIVE(m_iHeight);

    // create solid text surface data
    pSolid = m_pFont->CreateText(pcText, iRelHeight);
    ASSERT(pSolid->format->BitsPerPixel == 8u)

    if(m_iOutline)
    {
        // create outlined text surface data
        pOutline = m_pFont->CreateTextOutline(pcText, iRelHeight, m_iOutline);
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
        pData = ZERO_NEW(coreByte, iSize);

        // insert solid pixels
        const coreUintW iOffset = (pOutline->pitch + 1u) * iComponents * m_iOutline;
        for(coreUintW j = 0u, je = pSolid->h; j < je; ++j)
        {
            const coreUintW b = j * pSolid  ->pitch;
            const coreUintW a = j * pOutline->pitch * iComponents + iOffset;

            for(coreUintW i = 0u, ie = pSolid->pitch; i < ie; ++i)
            {
                const coreUintW iIndex = a + i * iComponents;

                ASSERT(iIndex < iSize)
                pData[iIndex] = pInput1[b + i];
            }
        }

        // insert outlined pixels
        for(coreUintW j = 0u, je = pOutline->h; j < je; ++j)
        {
            const coreUintW b = j * pOutline->pitch;
            const coreUintW a = b * iComponents + 1u;

            for(coreUintW i = 0u, ie = pOutline->pitch; i < ie; ++i)
            {
                const coreUintW iIndex = a + i * iComponents;

                ASSERT(iIndex < iSize)
                pData[iIndex] = pInput2[b + i];
            }
        }
    }
    else pData = s_cast<coreByte*>(pSolid->pixels);

    // check if new text fits into current texture
    if((iPitch  > F_TO_UI(m_vResolution.x)) ||
       (iHeight > F_TO_UI(m_vResolution.y)))
    {
        const coreUint32 iNewPitch  = MAX(iPitch,  F_TO_UI(m_vResolution.x));
        const coreUint32 iNewHeight = MAX(iHeight, F_TO_UI(m_vResolution.y));

        // create new texture
        m_apTexture[1]->Unload();
        m_apTexture[1]->Create(iNewPitch, iNewHeight, CORE_TEXTURE_SPEC_COMPONENTS(iComponents), CORE_TEXTURE_MODE_DEFAULT);

        // save new texture resolution
        m_vResolution = coreVector2(I_TO_F(iNewPitch), I_TO_F(iNewHeight));
    }

    // update only required texture area
    m_apTexture[1]->Invalidate(0u);
    m_apTexture[1]->Modify(0u, 0u, iPitch, iHeight, iSize, pData);

    // display only visible texture area
    this->SetTexSize(coreVector2(I_TO_F(iWidth) - 0.5f, I_TO_F(iHeight)) / m_vResolution);
    ASSERT((this->GetTexSize().x <= 1.0f) && (this->GetTexSize().y <= 1.0f))

    // delete merge buffer
    if(pOutline) ZERO_DELETE(pData)
}