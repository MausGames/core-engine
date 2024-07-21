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
coreLabel::coreLabel()noexcept
: coreObject2D         ()
, coreTranslate        ()
, coreResourceRelation ()
, m_pFont              (NULL)
, m_iHeight            (0u)
, m_iOutline           (0u)
, m_vResolution        (coreVector2(0.0f,0.0f))
, m_sText              ("")
, m_vScale             (coreVector2(1.0f,1.0f))
, m_iRectify           (0x03u)
, m_iShift             (0)
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
    // make sure to change texture format
    if(coreBool(m_iOutline) != coreBool(iOutline))
    {
        m_vResolution = coreVector2(0.0f,0.0f);
    }

    // save properties
    m_iHeight  = iHeight;
    m_iOutline = iOutline;

    // load font
    m_pFont = Core::Manager::Resource->Get<coreFont>(sFont);

    // load shader-program
    this->DefineProgram(iOutline ? "default_label_sharp_program" : "default_label_smooth_program");

    // allocate own texture to display text
    if(!m_apTexture[CORE_LABEL_TEXTURE]) m_apTexture[CORE_LABEL_TEXTURE] = Core::Manager::Resource->LoadNew<coreTexture>();

    // invoke texture generation
    this->RegenerateTexture();
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
            this->__RefreshSize();
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

    // check if requested font is loaded
    if(!m_pFont.IsUsable()) return;

    if(HAS_FLAG(m_eRefresh, CORE_LABEL_REFRESH_SIZE))
    {
        // refresh the object size
        this->__RefreshSize();

        // reset the refresh status
        REMOVE_FLAG(m_eRefresh, CORE_LABEL_REFRESH_SIZE)
    }

    // move and adjust the label
    this->__MoveRectified();
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
        m_vResolution = coreVector2(0.0f,0.0f);
    }
}


// ****************************************************************
/* reshape with the resource manager */
void coreLabel::__Reshape()
{
    if(!m_pFont) return;

    // invoke texture generation
    this->RegenerateTexture();
}


// ****************************************************************
/* generate the texture */
void coreLabel::__GenerateTexture(const coreChar* pcText)
{
    ASSERT(m_pFont.IsUsable())

    coreSurfaceScope pSolid   = NULL;
    coreSurfaceScope pOutline = NULL;
    coreByte*        pData    = NULL;

    // get relative font height and outline
    const coreUint16 iRelHeight  = CORE_LABEL_HEIGHT_RELATIVE (m_iHeight);
    const coreUint8  iRelOutline = CORE_LABEL_OUTLINE_RELATIVE(m_iOutline);

    // create solid text surface data
    pSolid = m_pFont->CreateText(pcText, iRelHeight);
    WARN_IF(!pSolid) return;
    ASSERT((pSolid->format->BitsPerPixel == 8u) && !SDL_MUSTLOCK(pSolid))

    if(iRelOutline)
    {
        // create outlined text surface data
        pOutline = m_pFont->CreateTextOutline(pcText, iRelHeight, iRelOutline);
        WARN_IF(!pOutline) return;
        ASSERT((pOutline->format->BitsPerPixel == 8u) && !SDL_MUSTLOCK(pSolid))
    }

    // set texture properties
    const coreUintW  iComponents = pOutline ? (CORE_GL_SUPPORT(ARB_texture_rg) ? 2u : 3u) : 1u;
    const coreUint32 iWidth      = pOutline ? pOutline->w     : pSolid->w;
    const coreUint32 iHeight     = pOutline ? pOutline->h     : pSolid->h;
    const coreUint32 iPitch      = pOutline ? pOutline->pitch : pSolid->pitch;
    const coreUintW  iSize       = iPitch * iHeight * iComponents;
    ASSERT(coreMath::IsAligned(iPitch, 4u))

    if(pOutline)
    {
        const coreByte* pInput1 = s_cast<const coreByte*>(pSolid  ->pixels);
        const coreByte* pInput2 = s_cast<const coreByte*>(pOutline->pixels);

        // allocate buffer to merge solid and outlined pixels
        pData = ZERO_NEW(coreByte, iSize);

        // insert solid pixels
        const coreUintW iOffset = (pOutline->pitch + 1u) * iComponents * iRelOutline;
        for(coreUintW j = 0u, je = LOOP_NONZERO(pSolid->h); j < je; ++j)
        {
            const coreUintW b = j * pSolid  ->pitch;
            const coreUintW a = j * pOutline->pitch * iComponents + iOffset;

            for(coreUintW i = 0u, ie = LOOP_NONZERO(pSolid->pitch); i < ie; ++i)
            {
                const coreUintW iIndex = a + i * iComponents;

                ASSERT(iIndex < iSize)
                pData[iIndex] = pInput1[b + i];
            }
        }

        // insert outlined pixels
        for(coreUintW j = 0u, je = LOOP_NONZERO(pOutline->h); j < je; ++j)
        {
            const coreUintW b = j * pOutline->pitch;
            const coreUintW a = j * pOutline->pitch * iComponents + 1u;

            for(coreUintW i = 0u, ie = LOOP_NONZERO(pOutline->pitch); i < ie; ++i)
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
/* refresh the object size */
void coreLabel::__RefreshSize()
{
    ASSERT(m_pFont.IsUsable())

    // get relative font height and outline
    const coreUint16 iRelHeight  = CORE_LABEL_HEIGHT_RELATIVE (m_iHeight);
    const coreUint8  iRelOutline = CORE_LABEL_OUTLINE_RELATIVE(m_iOutline);

    if(HAS_FLAG(m_eRefresh, CORE_LABEL_REFRESH_TEXTURE))
    {
        // set size by text dimensions
        const coreVector2 vDimensions = m_pFont->RetrieveTextDimensions(m_sText.c_str(), iRelHeight, iRelOutline);
        this->SetSize(vDimensions * m_vScale * CORE_LABEL_SIZE_FACTOR);
    }
    else
    {
        // set size by texture coordinates
        this->SetSize(this->GetTexSize() * m_vResolution * m_vScale * CORE_LABEL_SIZE_FACTOR);
    }

    // retrieve vertical shift
    m_iShift = m_pFont->RetrieveTextShift(m_sText.c_str(), iRelHeight, iRelOutline);
}


// ****************************************************************
/* move and adjust the label */
void coreLabel::__MoveRectified()
{
    if(HAS_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_TRANSFORM))
    {
        // move the 2d-object
        this->coreObject2D::Move();

        // handle global 2d-object rotation
        const coreVector2 vResolution = Core::System->GetResolution();
        const coreVector2 vViewDir    = HAS_FLAG(m_eStyle, CORE_OBJECT2D_STYLE_VIEWDIR) ? Core::Manager::Object->GetSpriteViewDir() : coreVector2(0.0f,1.0f);
        const coreVector2 vViewAlign  = this->GetAlignment().MapToAxisInv(vViewDir);

        // apply vertical shift
        m_vScreenPosition -= m_vScreenDirection * (I_TO_F(m_iShift) * 0.5f);

        // align texture with screen pixels
        if(HAS_FLAG(m_iRectify, vViewDir.y ? 0x01u : 0x02u))
        {
            coreFloat& fSize = m_vScreenDirection.y ? m_vScreenSize.x : m_vScreenSize.y;
            fSize = ROUND(fSize);

            const coreFloat fHalf = FRACT((fSize + vResolution.x) * 0.5f) * SIGN(vViewAlign.x);
            m_vScreenPosition.x = ROUND(m_vScreenPosition.x - fHalf) + fHalf;
        }
        if(HAS_FLAG(m_iRectify, vViewDir.y ? 0x02u : 0x01u))
        {
            coreFloat& fSize = m_vScreenDirection.y ? m_vScreenSize.y : m_vScreenSize.x;
            fSize = ROUND(fSize);

            const coreFloat fHalf = FRACT((fSize + vResolution.y) * 0.5f) * SIGN(vViewAlign.y);
            m_vScreenPosition.y = ROUND(m_vScreenPosition.y - fHalf) + fHalf;
        }
    }
}