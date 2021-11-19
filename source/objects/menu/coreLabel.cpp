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
, m_bRectify           (true)
, m_eRefresh           (CORE_LABEL_REFRESH_NOTHING)
{
}

coreLabel::coreLabel(const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline)noexcept
: coreLabel ()
{
    // construct on creation
    this->Construct(sFont, iHeight, iOutline);
}


// ****************************************************************
/* destructor */
coreLabel::~coreLabel()
{
    // free own texture
    Core::Manager::Resource->Free(&m_apTexture[CORE_LABEL_TEXTURE]);
}


// ****************************************************************
/* construct the label */
void coreLabel::Construct(const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline)
{
    // save properties
    m_iHeight  = iHeight;
    m_iOutline = iOutline;

    // set font object
    m_pFont = Core::Manager::Resource->Get<coreFont>(sFont);

    // allocate own texture to display text
    if(!m_apTexture[CORE_LABEL_TEXTURE]) m_apTexture[CORE_LABEL_TEXTURE] = Core::Manager::Resource->LoadNew<coreTexture>();

    // load shader-program
    this->DefineProgram(iOutline ? "default_label_sharp_program" : "default_label_smooth_program");
}


// ****************************************************************
/* render the label */
void coreLabel::Render()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_RENDER)) return;

    ASSERT(m_pProgram)
    if(m_sText.empty()) return;

    if(m_eRefresh)
    {
        // check if requested font is loaded
        if(!m_pFont.IsUsable()) return;

        if(HAS_FLAG(m_eRefresh, CORE_LABEL_REFRESH_TEXTURE))
        {
            // generate the texture
            this->__GenerateTexture(m_sText.c_str());
        }
        if(HAS_FLAG(m_eRefresh, CORE_LABEL_REFRESH_SIZE))
        {
            // refresh the object size
            this->SetSize(this->GetTexSize() * m_vResolution * (CORE_LABEL_SIZE_FACTOR * m_fScale));
            this->__MoveRectified();
        }

        // reset the refresh status
        m_eRefresh = CORE_LABEL_REFRESH_NOTHING;
    }

    // render the 2d-object
    this->coreObject2D::Render();
}


// ****************************************************************
/* move the label */
void coreLabel::Move()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_MOVE)) return;

    ASSERT(m_pProgram)
    if(m_sText.empty()) return;

    // move and adjust the label
    if(!HAS_FLAG(m_eRefresh, CORE_LABEL_REFRESH_SIZE)) this->__MoveRectified();
}


// ****************************************************************
/* change the current text */
coreBool coreLabel::SetText(const coreChar* pcText)
{
    ASSERT(pcText)

    // unbind from language
    this->_UnbindString(&m_sText);

    // check for new text
    if(std::strcmp(m_sText.c_str(), pcText))
    {
        ADD_FLAG(m_eRefresh, CORE_LABEL_REFRESH_ALL)

        // change the current text
        m_sText.assign(pcText);
        return true;
    }
    return false;
}

coreBool coreLabel::SetText(const coreChar* pcText, const coreUint16 iNum)
{
    ASSERT(pcText)

    // unbind from language
    this->_UnbindString(&m_sText);

    // check for new text
    if((iNum != m_sText.length()) || std::strcmp(m_sText.c_str(), pcText))
    {
        ADD_FLAG(m_eRefresh, CORE_LABEL_REFRESH_ALL)

        // change the current text
        m_sText.assign(pcText, MIN(iNum, std::strlen(pcText)));
        return true;
    }
    return false;
}


// ****************************************************************
/* reset with the resource manager */
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
        m_apTexture[CORE_LABEL_TEXTURE]->Unload();
    }
}


// ****************************************************************
/* generate the texture */
void coreLabel::__GenerateTexture(const coreChar* pcText)
{
    coreSurfaceScope pSolid   = NULL;
    coreSurfaceScope pOutline = NULL;
    coreByte*        pData    = NULL;

    // get relative font height and outline
    const coreUint16 iRelHeight  = CORE_LABEL_HEIGHT_RELATIVE(m_iHeight);
    const coreUint8  iRelOutline = CORE_LABEL_HEIGHT_RELATIVE(m_iOutline);

    // create solid text surface data
    pSolid = m_pFont->CreateText(pcText, iRelHeight);
    ASSERT(pSolid->format->BitsPerPixel == 8u)

    if(iRelOutline)
    {
        // create outlined text surface data
        pOutline = m_pFont->CreateTextOutline(pcText, iRelHeight, iRelOutline);
        ASSERT(pOutline->format->BitsPerPixel == 8u)
    }

    // set texture properties
    const coreUintW  iComponents = pOutline ? (CORE_GL_SUPPORT(ARB_texture_rg) ? 2u : 3u) : 1u;
    const coreUint32 iWidth      = pOutline ? pOutline->w     : pSolid->w;
    const coreUint32 iHeight     = pOutline ? pOutline->h     : pSolid->h;
    const coreUint32 iPitch      = pOutline ? pOutline->pitch : pSolid->pitch;
    const coreUintW  iSize       = iPitch * iHeight * iComponents;
    ASSERT(!(iPitch % 4u))

    if(pOutline)
    {
        coreByte* pInput1 = s_cast<coreByte*>(pSolid  ->pixels);
        coreByte* pInput2 = s_cast<coreByte*>(pOutline->pixels);

        // allocate buffer to merge solid and outlined pixels
        pData = ZERO_NEW(coreByte, iSize);

        // insert solid pixels
        const coreUintW iOffset = (pOutline->pitch + 1u) * iComponents * iRelOutline;
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
        m_apTexture[CORE_LABEL_TEXTURE]->Unload();
        m_apTexture[CORE_LABEL_TEXTURE]->Create(iNewPitch, iNewHeight, CORE_TEXTURE_SPEC_COMPONENTS(iComponents), CORE_TEXTURE_MODE_DEFAULT);

        // save new texture resolution
        m_vResolution = coreVector2(I_TO_F(iNewPitch), I_TO_F(iNewHeight));
    }

    // update only required texture area
    m_apTexture[CORE_LABEL_TEXTURE]->Invalidate(0u);
    m_apTexture[CORE_LABEL_TEXTURE]->Modify(0u, 0u, iPitch, iHeight, iSize, pData);

    // display only visible texture area
    this->SetTexSize(coreVector2(I_TO_F(iWidth), I_TO_F(iHeight)) / m_vResolution);
    ASSERT((this->GetTexSize().x <= 1.0f) && (this->GetTexSize().y <= 1.0f))

    // delete merge buffer
    if(pOutline) ZERO_DELETE(pData)
}


// ****************************************************************
/* move and adjust the label */
void coreLabel::__MoveRectified()
{
    if(HAS_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_TRANSFORM))
    {
        // move the 2d-object
        this->coreObject2D::Move();

        // align texture with screen pixels
        if(m_bRectify)
        {
            m_mTransform._11 = ROUND(m_mTransform._11);
            m_mTransform._12 = ROUND(m_mTransform._12);
            m_mTransform._21 = ROUND(m_mTransform._21);
            m_mTransform._22 = ROUND(m_mTransform._22);
            m_mTransform._31 = FLOOR(m_mTransform._31) + FRACT(0.5f * ABS(m_mTransform._11 + m_mTransform._21));
            m_mTransform._32 = CEIL (m_mTransform._32) - FRACT(0.5f * ABS(m_mTransform._12 + m_mTransform._22));
        }
    }
}