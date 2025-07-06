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
/* default character animation function */
static coreRichText::coreAnim DefaultAnimateFunc(const coreRichText::coreCharacter& oCharacter, void* pData)
{
    coreRichText::coreAnim oAnim;

    oAnim.vPosition  = coreVector2(0.0f,0.0f);
    oAnim.vDirection = coreVector2(0.0f,1.0f);
    oAnim.vColor     = coreVector4(1.0f,1.0f,1.0f,1.0f);

    return oAnim;
}


// ****************************************************************
/* constructor */
coreRichText::coreRichText()noexcept
: coreObject2D         ()
, coreTranslate        ()
, coreResourceRelation ()
, m_aStyle             {}
, m_avColor            {}
, m_sText              ("")
, m_iRectify           (0x03u)
, m_iMinLines          (0u)
, m_iMaxOrder          (CORE_RICHTEXT_MAX_ORDER)
, m_fMaxWidth          (CORE_RICHTEXT_MAX_WIDTH)
, m_fLineSkip          (0.0f)
, m_fTopHeight         (0.0f)
, m_iNumLines          (0u)
, m_iNumOrders         (0u)
, m_pAnimateData       (NULL)
, m_nAnimateFunc       (DefaultAnimateFunc)
, m_eRefresh           (CORE_RICHTEXT_REFRESH_NOTHING)
{
    // create default definitions
    m_aStyle .emplace(CORE_RICHTEXT_DEFAULT);
    m_avColor.emplace(CORE_RICHTEXT_DEFAULT, coreVector4(1.0f,1.0f,1.0f,1.0f));
}


// ****************************************************************
/* destructor */
coreRichText::~coreRichText()
{
    // exit all render passes
    this->__Reset(CORE_RESOURCE_RESET_EXIT);

    // free render pass textures
    FOR_EACH(it, m_aStyle) FOR_EACH(et, it->aPass) Core::Manager::Resource->Free(&et->pTexture);
}


// ****************************************************************
/* render the rich-text */
void coreRichText::Render()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_RENDER)) return;

    if(m_sText.empty() || !m_iMaxOrder) return;

    if(HAS_FLAG(m_eRefresh, CORE_RICHTEXT_REFRESH_TEXTURE))
    {
        // check if requested fonts are loaded
        FOR_EACH(it, m_aStyle) if(!it->pFont.IsUsable()) return;

        // generate the texture
        this->__GenerateTexture();

        // reset the refresh status
        REMOVE_FLAG(m_eRefresh, CORE_RICHTEXT_REFRESH_TEXTURE)
    }

    // center characters around object position
    const coreFloat   fCanonBase = Core::System->GetCanonBase();
    const coreVector2 vRelOffset = m_vScreenSize * coreVector2(-0.5f,0.5f) - coreVector2(0.0f, m_fTopHeight);

    const auto nCharDataFunc = [&](const coreCharacter& oCharacter, const coreAnim& oAnim, const corePass& oPass)
    {
        // calculate base parameters
        const coreVector2 vPosition  = oAnim.vPosition + coreVector2::UnpackFloat2x16(oCharacter.iPosition);
        const coreVector2 vSize      = oPass.avTexParams[oCharacter.iTexParamsIndex].xy() * CORE_RICHTEXT_TEXTURE_SIZE * CORE_LABEL_SIZE_FACTOR;
        const coreVector2 vDirection = oAnim.vDirection;

        // calculate resolution-modified transformation parameters
        coreVector2 vCharPosition  = vPosition * fCanonBase;
        coreVector2 vCharSize      = vSize     * fCanonBase;
        coreVector2 vCharDirection = vDirection.InvertedX();

        // add offset to position
        vCharPosition += vRelOffset + (vCharDirection.x ? coreVector2::Bound(vCharSize, vCharDirection) : vCharSize) * 0.5f;

        // apply other object properties
        vCharPosition  = vCharPosition .MapToAxis(m_vScreenDirection) + m_vScreenPosition;
        vCharDirection = vCharDirection.MapToAxis(m_vScreenDirection);

        // return everything together
        return coreMatrix3x2(vCharPosition.x, vCharPosition.y, vCharSize.x, vCharSize.y, vCharDirection.x, vCharDirection.y);
    };

    FOR_EACH(it, m_aStyle)
    {
        FOR_EACH(et, it->aPass)
        {
            corePass& oPass = (*et);

            if(oPass.aCharacter.empty()) continue;

            // enable render pass texture
            oPass.pTexture->Enable(CORE_LABEL_TEXTURE);

            if(oPass.aInstanceBuffer[0].IsValid())
            {
                // enable the shader-program (instancing)
                ASSERT(it->pProgramInst)
                if(!it->pProgramInst.IsUsable()) return;
                if(!it->pProgramInst->Enable())  return;

                if(HAS_FLAG(m_eRefresh, CORE_RICHTEXT_REFRESH_BUFFER))
                {
                    // invalidate and synchronize previous buffer
                    oPass.aInstanceBuffer.current().Invalidate();
                    oPass.aInstanceBuffer.current().Synchronize(CORE_DATABUFFER_MAP_INVALIDATE_ALL);

                    // switch to next available array and buffer
                    oPass.aiVertexArray  .next();
                    oPass.aInstanceBuffer.next();

                    // determine render-count
                    oPass.iNumEnabled = std::find_if(oPass.aCharacter.begin(), oPass.aCharacter.end(), [this](const coreCharacter& A) {return (A.iOrder >= m_iMaxOrder);}) - oPass.aCharacter.begin();

                    if(CORE_GL_SUPPORT(ARB_half_float_vertex))
                    {
                        // map required area of the instance data buffer
                        coreByte* pRange  = oPass.aInstanceBuffer.current().MapWrite(0u, oPass.iNumEnabled * CORE_RICHTEXT_INSTANCE_SIZE_HIGH, CORE_DATABUFFER_MAP_INVALIDATE_ALL);
                        coreByte* pCursor = pRange;

                        FOR_EACH(ut, oPass.aCharacter)
                        {
                            // get current character state
                            const coreCharacter& oCharacter = (*ut);
                            if(oCharacter.iOrder >= m_iMaxOrder) break;

                            // animate character
                            const coreAnim      oAnim     = m_nAnimateFunc(oCharacter, m_pAnimateData);
                            const coreMatrix3x2 mCharData = nCharDataFunc (oCharacter, oAnim, oPass);

                            // compress data
                            const coreVector3 vPosition  = coreVector3(mCharData._11, mCharData._12, I_TO_F(m_iDepth) / I_TO_F(CORE_OBJECT2D_DEPTH_MAX));
                            const coreUint32  vSize      = coreVector2(mCharData._21, mCharData._22)                             .PackFloat2x16();
                            const coreUint32  vDirection = coreVector2(mCharData._31, mCharData._32)                             .PackSnorm2x16();
                            const coreUint32  iColor     = (m_avColor[oCharacter.iColorIndex] * oAnim.vColor * this->GetColor4()).PackUnorm4x8 ();
                            const coreUint64  iTexParams = oPass.avTexParams[oCharacter.iTexParamsIndex]                         .PackUnorm4x16();
                            ASSERT((oAnim.vColor     .Min() >= 0.0f) && (oAnim.vColor     .Max() <= 1.0f))
                            ASSERT((this->GetColor4().Min() >= 0.0f) && (this->GetColor4().Max() <= 1.0f))

                            // write data to the buffer
                            std::memcpy(pCursor,       &vPosition,  sizeof(coreVector3));
                            std::memcpy(pCursor + 12u, &vSize,      sizeof(coreUint32));
                            std::memcpy(pCursor + 16u, &vDirection, sizeof(coreUint32));
                            std::memcpy(pCursor + 20u, &iColor,     sizeof(coreUint32));
                            std::memcpy(pCursor + 24u, &iTexParams, sizeof(coreUint64));
                            pCursor += CORE_RICHTEXT_INSTANCE_SIZE_HIGH;
                        }

                        ASSERT(coreUint32(pCursor - pRange) == oPass.iNumEnabled * CORE_RICHTEXT_INSTANCE_SIZE_HIGH)
                    }
                    else
                    {
                        // map required area of the instance data buffer
                        coreByte* pRange  = oPass.aInstanceBuffer.current().MapWrite(0u, oPass.iNumEnabled * CORE_RICHTEXT_INSTANCE_SIZE_LOW, CORE_DATABUFFER_MAP_INVALIDATE_ALL);
                        coreByte* pCursor = pRange;

                        FOR_EACH(ut, oPass.aCharacter)
                        {
                            // get current character state
                            const coreCharacter& oCharacter = (*ut);
                            if(oCharacter.iOrder >= m_iMaxOrder) break;

                            // animate character
                            const coreAnim      oAnim     = m_nAnimateFunc(oCharacter, m_pAnimateData);
                            const coreMatrix3x2 mCharData = nCharDataFunc (oCharacter, oAnim, oPass);

                            // compress data
                            const coreVector3 vPosition  = coreVector3(mCharData._11, mCharData._12, I_TO_F(m_iDepth) / I_TO_F(CORE_OBJECT2D_DEPTH_MAX));
                            const coreVector2 vSize      = coreVector2(mCharData._21, mCharData._22);
                            const coreUint32  vDirection = coreVector2(mCharData._31, mCharData._32)                             .PackSnorm2x16();
                            const coreUint32  iColor     = (m_avColor[oCharacter.iColorIndex] * oAnim.vColor * this->GetColor4()).PackUnorm4x8 ();
                            const coreUint64  iTexParams = oPass.avTexParams[oCharacter.iTexParamsIndex]                         .PackUnorm4x16();
                            ASSERT((oAnim.vColor     .Min() >= 0.0f) && (oAnim.vColor     .Max() <= 1.0f))
                            ASSERT((this->GetColor4().Min() >= 0.0f) && (this->GetColor4().Max() <= 1.0f))

                            // write data to the buffer
                            std::memcpy(pCursor,       &vPosition,  sizeof(coreVector3));
                            std::memcpy(pCursor + 12u, &vSize,      sizeof(coreVector2));
                            std::memcpy(pCursor + 20u, &vDirection, sizeof(coreUint32));
                            std::memcpy(pCursor + 24u, &iColor,     sizeof(coreUint32));
                            std::memcpy(pCursor + 28u, &iTexParams, sizeof(coreUint64));
                            pCursor += CORE_RICHTEXT_INSTANCE_SIZE_LOW;
                        }

                        ASSERT(coreUint32(pCursor - pRange) == oPass.iNumEnabled * CORE_RICHTEXT_INSTANCE_SIZE_LOW)
                    }

                    // unmap buffer
                    oPass.aInstanceBuffer.current().Unmap();
                }

                // disable current model object (because of direct VAO use)
                coreModel::Disable(false);

                // draw the model instanced
                glBindVertexArray(oPass.aiVertexArray.current());
                Core::Manager::Object->GetLowQuad()->DrawArraysInstanced(oPass.iNumEnabled);
            }
            else
            {
                // enable the shader-program (regular)
                ASSERT(it->pProgram)
                if(!it->pProgram.IsUsable()) return;
                if(!it->pProgram->Enable())  return;

                coreProgram* pProgram = it->pProgram.GetResource();
                coreModel*   pModel   = Core::Manager::Object->GetLowQuad().GetResource();

                // draw without instancing
                FOR_EACH(ut, oPass.aCharacter)
                {
                    // get current character state
                    const coreCharacter& oCharacter = (*ut);
                    if(oCharacter.iOrder >= m_iMaxOrder) break;

                    // animate character
                    const coreAnim      oAnim     = m_nAnimateFunc(oCharacter, m_pAnimateData);
                    const coreMatrix3x2 mCharData = nCharDataFunc (oCharacter, oAnim, oPass);

                    // prepare data
                    const coreVector3 vPosition  = coreVector3(mCharData._11, mCharData._12, I_TO_F(m_iDepth) / I_TO_F(CORE_OBJECT2D_DEPTH_MAX));
                    const coreVector2 vSize      = coreVector2(mCharData._21, mCharData._22);
                    const coreVector2 vDirection = coreVector2(mCharData._31, mCharData._32);
                    const coreVector4 vColor     = m_avColor[oCharacter.iColorIndex] * oAnim.vColor * this->GetColor4();
                    const coreVector4 vTexParams = oPass.avTexParams[oCharacter.iTexParamsIndex];

                    // update all character uniforms
                    pProgram->SendUniform(CORE_SHADER_UNIFORM_2D_POSITION, vPosition);
                    pProgram->SendUniform(CORE_SHADER_UNIFORM_2D_SIZE,     vSize);
                    pProgram->SendUniform(CORE_SHADER_UNIFORM_2D_ROTATION, vDirection);
                    pProgram->SendUniform(CORE_SHADER_UNIFORM_COLOR,       vColor);
                    pProgram->SendUniform(CORE_SHADER_UNIFORM_TEXPARAM,    vTexParams);

                    // draw the model
                    pModel->Enable();
                    pModel->DrawArrays();
                }
            }
        }
    }

    // reset the refresh status
    REMOVE_FLAG(m_eRefresh, CORE_RICHTEXT_REFRESH_BUFFER)
}


// ****************************************************************
/* move the rich-text */
void coreRichText::Move()
{
    if(!this->IsEnabled(CORE_OBJECT_ENABLE_MOVE)) return;

    if(m_sText.empty()) return;

    if(HAS_FLAG(m_eRefresh, CORE_RICHTEXT_REFRESH_LAYOUT))
    {
        // check if requested fonts are loaded
        FOR_EACH(it, m_aStyle) if(!it->pFont.IsUsable()) return;

        // parse and arrange characters
        this->__ParseText();

        // reset the refresh status
        REMOVE_FLAG(m_eRefresh, CORE_RICHTEXT_REFRESH_LAYOUT)
    }

    // move and adjust the text
    this->__MoveRectified();

    // invoke buffer update
    ADD_FLAG(m_eRefresh, CORE_RICHTEXT_REFRESH_BUFFER)
}


// ****************************************************************
/* create or update font style definitions */
void coreRichText::AssignStyle(const coreHashString& sName, const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline)
{
    coreStyle& oStyle = m_aStyle[sName];

    // make sure to change texture format
    if(coreBool(oStyle.iOutline) != coreBool(iOutline))
    {
        FOR_EACH(et, oStyle.aPass) et->pTexture->Unload();
    }

    // save font style definition
    oStyle.pFont        = Core::Manager::Resource->Get<coreFont>(sFont);
    oStyle.iHeight      = iHeight;
    oStyle.iOutline     = iOutline;
    oStyle.pProgram     = Core::Manager::Resource->Get<coreProgram>(iOutline ? "default_label_sharp_program"      : "default_label_smooth_program");
    oStyle.pProgramInst = Core::Manager::Resource->Get<coreProgram>(iOutline ? "default_label_sharp_inst_program" : "default_label_smooth_inst_program");

    // invoke texture generation
    this->RegenerateTexture(sName);
}


// ****************************************************************
/* create or update color definition */
void coreRichText::AssignColor4(const coreHashString& sName, const coreVector4 vColor)
{
    // save color definition
    ASSERT((vColor.Min() >= 0.0f) && (vColor.Max() <= 1.0f))
    m_avColor[sName] = vColor;

    // invoke buffer update
    ADD_FLAG(m_eRefresh, CORE_RICHTEXT_REFRESH_BUFFER)
}

void coreRichText::AssignColor3(const coreHashString& sName, const coreVector3 vColor)
{
    // set with default alpha
    this->AssignColor4(sName, coreVector4(vColor, 1.0f));
}


// ****************************************************************
/* invoke texture generation */
void coreRichText::RegenerateTexture()
{
    FOR_EACH(it, m_aStyle)
    {
        // clear texture entries
        it->aEntry.clear();
        it->iPassIndex  = 0u;
        it->iEntryCount = 0u;

        FOR_EACH(et, it->aPass)
        {
            // clear render pass properties
            et->aCharacter .clear();
            et->avTexParams.clear();
            et->avTexShift .clear();
            et->fHeightFrom = 0.0f;
            et->fHeightTo   = 0.0f;
        }
    }

    // update everything
    ADD_FLAG(m_eRefresh, CORE_RICHTEXT_REFRESH_ALL)
}

void coreRichText::RegenerateTexture(const coreHashString& sStyleName)
{
    ASSERT(m_aStyle.count(sStyleName))
    coreStyle& oStyle = m_aStyle.at(sStyleName);

    // clear texture entries
    oStyle.aEntry.clear();
    oStyle.iPassIndex  = 0u;
    oStyle.iEntryCount = 0u;

    FOR_EACH(et, oStyle.aPass)
    {
        // clear render pass properties
        et->aCharacter .clear();
        et->avTexParams.clear();
        et->avTexShift .clear();
        et->fHeightFrom = 0.0f;
        et->fHeightTo   = 0.0f;
    }

    // update everything
    ADD_FLAG(m_eRefresh, CORE_RICHTEXT_REFRESH_ALL)
}


// ****************************************************************
/* change the current text */
coreBool coreRichText::SetText(const coreChar* pcText)
{
    ASSERT(pcText)

    // unbind from language
    this->_UnbindString(&m_sText);

    // check for new text
    if(std::strcmp(m_sText.c_str(), pcText))
    {
        ADD_FLAG(m_eRefresh, CORE_RICHTEXT_REFRESH_LAYOUT)

        // change the current text
        m_sText.assign(pcText);
        return true;
    }
    return false;
}


// ****************************************************************
/* reset with the resource manager */
void coreRichText::__Reset(const coreResourceReset eInit)
{
    if(eInit)
    {
        // init all render passes
        FOR_EACH(it, m_aStyle) FOR_EACH(et, it->aPass) this->__InitPass(&(*et));

        // invoke texture generation
        this->RegenerateTexture();
    }
    else
    {
        // exit all render passes
        FOR_EACH(it, m_aStyle) FOR_EACH(et, it->aPass) this->__ExitPass(&(*et));
        FOR_EACH(it, m_aStyle) FOR_EACH(et, it->aPass) et->pTexture->Unload();
    }
}


// ****************************************************************
/* reshape with the resource manager */
void coreRichText::__Reshape()
{
    // invoke texture generation
    this->RegenerateTexture();
}


// ****************************************************************
/* parse and arrange characters */
void coreRichText::__ParseText()
{
    ASSERT(!m_aStyle.empty() && !m_avColor.empty() && (m_fLineSkip || (m_fMaxWidth >= CORE_RICHTEXT_MAX_WIDTH)))

    coreVector2 vCurPos    = coreVector2(0.0f,0.0f);
    coreVector2 vCurSize   = coreVector2(0.0f,0.0f);
    coreChar32  cPrevGlyph = U'\0';

    // clear arranged characters
    FOR_EACH(it, m_aStyle) FOR_EACH(et, it->aPass) et->aCharacter.clear();
    m_fTopHeight = 0.0f;
    m_iNumLines  = 1u;
    m_iNumOrders = 0u;

    // calculate line parameters
    const coreFloat fFullMaxWidth =       m_fMaxWidth * CORE_LABEL_DETAIL;
    const coreFloat fFullLineSkip = ROUND(m_fLineSkip * CORE_LABEL_DETAIL);   // always align

    // start with default definitions
    coreStyle* pStyle      = &m_aStyle.front();
    coreFont*  pFont       = pStyle->pFont.GetResource();
    coreUint16 iRelHeight  = CORE_LABEL_HEIGHT_RELATIVE (pStyle->iHeight);
    coreUint8  iRelOutline = CORE_LABEL_OUTLINE_RELATIVE(pStyle->iOutline);
    coreInt32  iAscent     = pFont->RetrieveAscent (iRelHeight, iRelOutline);
    coreInt32  iDescent    = pFont->RetrieveDescent(iRelHeight, iRelOutline);
    coreUint16 iColorIndex = 0u;

    // prepare range pointers (from, to) and end pointer (out of bound)
    const coreChar* pcFrom = m_sText.data();
    const coreChar* pcEnd  = m_sText.data() + m_sText.length();

    // continue into next text line
    const auto nNewLineFunc = [&]()
    {
        vCurPos.x  = 0.0f;
        vCurPos.y -= fFullLineSkip;
        cPrevGlyph = 0u;

        m_iNumLines += 1u;
    };

    while(pcFrom < pcEnd)
    {
        if((*pcFrom) == '<')
        {
            pcFrom += 1u;

            if((*pcFrom) == '/')
            {
                // reset to default definitions
                pStyle      = &m_aStyle.front();
                pFont       = pStyle->pFont.GetResource();
                iRelHeight  = CORE_LABEL_HEIGHT_RELATIVE (pStyle->iHeight);
                iRelOutline = CORE_LABEL_OUTLINE_RELATIVE(pStyle->iOutline);
                iAscent     = pFont->RetrieveAscent (iRelHeight, iRelOutline);
                iDescent    = pFont->RetrieveDescent(iRelHeight, iRelOutline);
                iColorIndex = 0u;

                pcFrom += 1u;
            }
            else
            {
                // search for delimiters
                const coreChar* pcAssign = s_cast<const coreChar*>(std::memchr(pcFrom, '=', pcEnd - pcFrom));
                const coreChar* pcClose  = s_cast<const coreChar*>(std::memchr(pcFrom, '>', pcEnd - pcFrom));
                WARN_IF(!pcAssign || !pcClose) continue;

                // extract key and value
                coreChar acKey[64], acValue[64];
                coreData::StrCopy(acKey,   ARRAY_SIZE(acKey),   pcFrom,        pcAssign - pcFrom);
                coreData::StrCopy(acValue, ARRAY_SIZE(acValue), pcAssign + 1u, pcClose  - pcAssign - 1u);

                if(!std::strcmp(acKey, "style"))
                {
                    // select font style definition
                    WARN_IF(!m_aStyle.count(acValue)) {}
                    else
                    {
                        pStyle      = &m_aStyle.at(acValue);
                        pFont       = pStyle->pFont.GetResource();
                        iRelHeight  = CORE_LABEL_HEIGHT_RELATIVE (pStyle->iHeight);
                        iRelOutline = CORE_LABEL_OUTLINE_RELATIVE(pStyle->iOutline);
                        iAscent     = pFont->RetrieveAscent (iRelHeight, iRelOutline);
                        iDescent    = pFont->RetrieveDescent(iRelHeight, iRelOutline);
                    }
                }
                else if(!std::strcmp(acKey, "color"))
                {
                    // select color definition
                    WARN_IF(!m_avColor.count(acValue)) {}
                    else
                    {
                        iColorIndex = m_avColor.index(acValue);
                    }
                }
                else WARN_IF(true) {}

                pcFrom = pcClose;
            }

            pcFrom += 1u;
        }
        else if((*pcFrom) == ' ')
        {
            coreBool bNewLine = false;

            const coreChar* pcCursor       = pcFrom;
            coreFloat       fTestWidth     = vCurPos.x;
            coreChar32      cTestPrevGlyph = U'\0';

            do
            {
                // skip definitions
                if((*pcCursor) == '<')
                {
                    pcCursor = s_cast<const coreChar*>(std::memchr(pcCursor, '>', pcEnd - pcCursor));
                    WARN_IF(!pcCursor++) break;
                }

                // convert to glyph and move cursor
                coreChar32 cGlyph;
                pcCursor += coreFont::ConvertToGlyph(pcCursor, &cGlyph);

                // retrieve dimensions
                coreInt32 iAdvance;
                pFont->RetrieveGlyphMetrics(cGlyph, iRelHeight, iRelOutline, NULL, NULL, NULL, NULL, &iAdvance, NULL);

                // retrieve kerning
                const coreInt32 iKerning = pFont->RetrieveGlyphKerning(cTestPrevGlyph, cGlyph, iRelHeight, iRelOutline);

                // apply advance and kerning
                fTestWidth += I_TO_F(iAdvance + iKerning);

                // check limit
                if(fTestWidth >= fFullMaxWidth)
                {
                    bNewLine = true;
                    break;
                }

                // save previous glyph (for kerning)
                cTestPrevGlyph = cGlyph;
            }
            while(((*pcCursor) != ' ') && ((*pcCursor) != '\n') && ((*pcCursor) != '\0'));

            if(bNewLine)
            {
                // start new text line
                nNewLineFunc();

                pcFrom += 1u;
                continue;
            }
        }
        else if((*pcFrom) == '\n')
        {
            // start new text line
            nNewLineFunc();

            pcFrom += 1u;
            continue;
        }

        // convert to glyph and move cursor
        coreChar32 cGlyph;
        pcFrom += coreFont::ConvertToGlyph(pcFrom, &cGlyph);

        // retrieve dimensions
        coreInt32 iMinX, iMaxX, iMinY, iMaxY, iAdvance;
        pFont->RetrieveGlyphMetrics(cGlyph, iRelHeight, iRelOutline, &iMinX, &iMaxX, &iMinY, &iMaxY, &iAdvance, NULL);

        // fix negative coordinates
        coreInt32 iCorrection = 0;
        if(iMinX < 0) {iCorrection = iMinX; iMaxX -= iMinX; iMinX = 0;}

        // retrieve kerning
        const coreInt32 iKerning = pFont->RetrieveGlyphKerning(cPrevGlyph, cGlyph, iRelHeight, iRelOutline);

        // only render visible characters
        if((cGlyph != U' ') && (cGlyph != CORE_FONT_GLYPH_NBSP))
        {
            const coreFloat fBaseHeight = I_TO_F(iAscent - iDescent);

            if(!pStyle->aEntry.count_bs(cGlyph))
            {
                const coreFloat fGlyphHeight = I_TO_F(MAX(iAscent, iMaxY) - MIN(iDescent, iMinY));
                const coreFloat fGlyphPitch  = I_TO_F(coreMath::CeilAlign(MAX(iAdvance + 2u * iRelOutline - iCorrection, iMaxX), 4u));

                // init first render pass
                if(pStyle->aPass.empty())
                {
                    this->__InitPass(&pStyle->aPass.emplace_back());
                }

                do
                {
                    corePass& oPass = pStyle->aPass[pStyle->iPassIndex];

                    // calculate texture parameters
                    const coreVector4 vPrevParams = oPass.avTexParams.empty() ? coreVector4(0.0f,0.0f,0.0f,0.0f) : oPass.avTexParams.back();
                    const coreVector2 vPrevShift  = oPass.avTexShift .empty() ? coreVector2(0.0f,0.0f)           : oPass.avTexShift .back();
                    const coreVector2 vTexSize    = coreVector2(I_TO_F(iMaxX - iMinX), I_TO_F(iMaxY - iMinY)) / CORE_RICHTEXT_TEXTURE_SIZE;
                    coreFloat         fTexOffset  = oPass.avTexParams.empty() ? 0.0f : (vPrevParams.x + vPrevParams.z + vPrevShift.x + CORE_RICHTEXT_TEXTURE_MARGIN.x);

                    // check width and switch to next generation line
                    if(fTexOffset + fGlyphPitch / CORE_RICHTEXT_TEXTURE_SIZE.x >= 1.0f)
                    {
                        oPass.fHeightFrom = oPass.fHeightTo + CORE_RICHTEXT_TEXTURE_MARGIN.y;
                        fTexOffset = 0.0f;
                    }

                    // track current height
                    oPass.fHeightTo = MAX(oPass.fHeightTo, oPass.fHeightFrom + fGlyphHeight / CORE_RICHTEXT_TEXTURE_SIZE.y);

                    // check height and switch to next render pass
                    if(oPass.fHeightTo >= 1.0f)
                    {
                        if(++pStyle->iPassIndex == pStyle->aPass.size()) this->__InitPass(&pStyle->aPass.emplace_back());
                        continue;
                    }

                    // create new render pass properties
                    const coreVector4 vNewParam = coreVector4(vTexSize, fTexOffset, oPass.fHeightFrom);
                    const coreVector2 vNewShift = coreVector2(I_TO_F(iMinX), I_TO_F(MAX(iAscent - iMaxY, 0))) / CORE_RICHTEXT_TEXTURE_SIZE;

                    // add properties to list
                    oPass.avTexParams.push_back(vNewParam + coreVector4(0.0f, 0.0f, vNewShift));
                    oPass.avTexShift .push_back(vNewShift);

                    // create new texture entry
                    coreEntry oNewEntry;
                    oNewEntry.bGenerated = false;
                    oNewEntry.iPass      = pStyle->iPassIndex;
                    oNewEntry.iIndex     = oPass.avTexParams.size() - 1u;

                    // add entry to list
                    pStyle->aEntry.emplace_bs(cGlyph, oNewEntry);

                    break;
                }
                while(true);

                // invoke texture generation
                ADD_FLAG(m_eRefresh, CORE_RICHTEXT_REFRESH_TEXTURE)
            }

            // apply kerning value
            vCurPos.x += I_TO_F(iKerning);

            // check limit and start new text line
            if(vCurPos.x + I_TO_F(iAdvance) >= fFullMaxWidth)
            {
                nNewLineFunc();
            }

            const coreEntry& oEntry = pStyle->aEntry.at_bs(cGlyph);
            corePass&        oPass  = pStyle->aPass[oEntry.iPass];

            // create new arranged character
            coreCharacter oCharacter;
            oCharacter.iPosition       = ((vCurPos + coreVector2(I_TO_F(iMinX), I_TO_F(iMinY - iDescent))) * CORE_LABEL_SIZE_FACTOR).PackFloat2x16();
            oCharacter.iColorIndex     = iColorIndex;
            oCharacter.iTexParamsIndex = oEntry.iIndex;
            oCharacter.iOrder          = m_iNumOrders++;

            // add character to list
            oPass.aCharacter.push_back(oCharacter);

            // apply advance value
            vCurPos.x += I_TO_F(iAdvance);

            // track total size
            vCurSize.x = MAX(vCurSize.x, vCurPos.x + I_TO_F(iRelOutline) * 2.0f);
            vCurSize.y = MAX(vCurSize.y, fBaseHeight - vCurPos.y, fBaseHeight + I_TO_F(m_iMinLines - 1u) * fFullLineSkip);

            // track height of the first text line (as it expands top-right)
            if(m_iNumLines == 1u) m_fTopHeight = MAX(m_fTopHeight, fBaseHeight);
        }
        else
        {
            // apply advance and kerning
            vCurPos.x += I_TO_F(iAdvance + iKerning);
        }

        // save previous glyph (for kerning)
        cPrevGlyph = cGlyph;
    }

    // set object size
    this->SetSize(vCurSize * CORE_LABEL_SIZE_FACTOR);

    // adjust render pass capacities
    FOR_EACH(it, m_aStyle) FOR_EACH(et, it->aPass) this->__ReallocatePass(&(*et), et->aCharacter.size());
}


// ****************************************************************
/* generate the texture */
void coreRichText::__GenerateTexture()
{
    FOR_EACH(it, m_aStyle)
    {
        coreStyle& oStyle = (*it);

        // determine components
        const coreUintW iComponents = oStyle.iOutline ? (CORE_GL_SUPPORT(ARB_texture_rg) ? 2u : 3u) : 1u;

        FOR_EACH(et, oStyle.aPass)
        {
            if(et->aCharacter.empty()) continue;

            if(!et->pTexture->GetIdentifier())
            {
                // create render pass texture
                et->pTexture->Create(F_TO_UI(CORE_RICHTEXT_TEXTURE_SIZE.x), F_TO_UI(CORE_RICHTEXT_TEXTURE_SIZE.y), CORE_TEXTURE_SPEC_COMPONENTS(iComponents), CORE_TEXTURE_MODE_DEFAULT);
            }

            if(!oStyle.iEntryCount)
            {
                // invalidate render pass texture
                if(HAS_FLAG(m_iRectify, 0x03u)) et->pTexture->Invalidate(0u);
                                           else et->pTexture->Clear     (0u);
            }
        }

        coreFont* pFont = oStyle.pFont.GetResource();

        coreSurfaceScope pSolid    = NULL;
        coreSurfaceScope pOutline  = NULL;
        coreByte*        pData     = NULL;
        coreUintW        iDataSize = 0u;

        // get relative font height and outline
        const coreUint16 iRelHeight  = CORE_LABEL_HEIGHT_RELATIVE (oStyle.iHeight);
        const coreUint8  iRelOutline = CORE_LABEL_OUTLINE_RELATIVE(oStyle.iOutline);

        FOR_EACH(et, oStyle.aEntry)
        {
            coreEntry&       oEntry = (*et);
            const coreChar32 cGlyph = (*oStyle.aEntry.get_key(et));

            // only generate new entries
            if(oEntry.bGenerated) continue;
            oEntry.bGenerated = true;

            // create solid text surface data
            pSolid = pFont->CreateGlyph(cGlyph, iRelHeight);
            WARN_IF(!pSolid) break;
            ASSERT((SDL_BITSPERPIXEL(pSolid->format) == 8u) && !SDL_MUSTLOCK(pSolid))

            if(iRelOutline)
            {
                // create outlined text surface data
                pOutline = pFont->CreateGlyphOutline(cGlyph, iRelHeight, iRelOutline);
                WARN_IF(!pOutline) break;
                ASSERT((SDL_BITSPERPIXEL(pOutline->format) == 8u) && !SDL_MUSTLOCK(pSolid))
            }

            // set texture properties
            const coreUint32 iWidth  = pOutline ? pOutline->w : pSolid->w;
            const coreUint32 iHeight = pOutline ? pOutline->h : pSolid->h;
            const coreUint32 iPitch  = coreMath::CeilAlign(iWidth, 4u);
            const coreUintW  iSize   = iPitch * iHeight * iComponents;

            // allocate buffer to merge or transform pixels
            if(iDataSize < iSize)
            {
                DYNAMIC_RESIZE(pData, iSize)
                iDataSize = iSize;
            }
            std::memset(pData, 0, iSize);

            if(pOutline)
            {
                const coreByte* pInput1 = ASSUME_ALIGNED(s_cast<const coreByte*>(pSolid  ->pixels), ALIGNMENT_NEW);
                const coreByte* pInput2 = ASSUME_ALIGNED(s_cast<const coreByte*>(pOutline->pixels), ALIGNMENT_NEW);

                // insert solid pixels
                const coreUintW iOffset = (iPitch + 1u) * iComponents * iRelOutline;
                for(coreUintW j = 0u, je = LOOP_NONZERO(pSolid->h); j < je; ++j)
                {
                    const coreUintW b = j * pSolid->pitch;
                    const coreUintW a = j * iPitch * iComponents + iOffset;

                    for(coreUintW i = 0u, ie = LOOP_NONZERO(pSolid->w); i < ie; ++i)
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
                    const coreUintW a = j * iPitch * iComponents + 1u;

                    for(coreUintW i = 0u, ie = LOOP_NONZERO(pOutline->w); i < ie; ++i)
                    {
                        const coreUintW iIndex = a + i * iComponents;

                        ASSERT(iIndex < iSize)
                        pData[iIndex] = pInput2[b + i];
                    }
                }
            }
            else
            {
                ASSERT(iComponents == 1u)

                const coreByte* pInput1 = ASSUME_ALIGNED(s_cast<const coreByte*>(pSolid->pixels), ALIGNMENT_NEW);

                // transform solid pixels
                for(coreUintW j = 0u, je = LOOP_NONZERO(pSolid->h); j < je; ++j)
                {
                    std::memcpy(pData + (j * iPitch), pInput1 + (j * pSolid->pitch), pSolid->w);
                }
            }

            // retrieve render pass properties
            const corePass&   oPass   = oStyle.aPass[oEntry.iPass];
            const coreVector2 vOffset = (oPass.avTexParams[oEntry.iIndex].zw() - oPass.avTexShift[oEntry.iIndex]) * CORE_RICHTEXT_TEXTURE_SIZE;

            // update only required texture area
            oPass.pTexture->Modify(F_TO_UI(vOffset.x), F_TO_UI(vOffset.y), iPitch, iHeight, iSize, pData);
        }

        // delete merge buffer
        DYNAMIC_DELETE(pData)

        // save number of generated texture entries
        oStyle.iEntryCount = oStyle.aEntry.size();
    }
}


// ****************************************************************
/* move and adjust the text */
void coreRichText::__MoveRectified()
{
    if(HAS_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_TRANSFORM))
    {
        // move the 2d-object
        this->coreObject2D::Move();

        // handle global 2d-object rotation
        const coreVector2 vResolution = Core::System->GetResolution();
        const coreVector2 vViewDir    = HAS_FLAG(m_eStyle, CORE_OBJECT2D_STYLE_VIEWDIR) ? Core::Manager::Object->GetSpriteViewDir() : coreVector2(0.0f,1.0f);
        const coreVector2 vViewAlign  = this->GetAlignment().MapToAxisInv(vViewDir);

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


// ****************************************************************
/* init render pass */
void coreRichText::__InitPass(corePass* OUTPUT pPass)
{
    // allocate render pass texture
    if(!pPass->pTexture) pPass->pTexture = Core::Manager::Resource->LoadNew<coreTexture>();

    // check for OpenGL extensions
    if(!CORE_GL_SUPPORT(ARB_instanced_arrays) || !CORE_GL_SUPPORT(ARB_vertex_array_object)) return;

    WARN_IF(pPass->aInstanceBuffer[0].IsValid()) return;

    // only allocate with enough capacity
    if(pPass->iNumInstances)
    {
        FOR_EACH(it, pPass->aInstanceBuffer)
        {
            // create vertex array object
            coreGenVertexArrays(1u, &pPass->aiVertexArray.current());
            glBindVertexArray(pPass->aiVertexArray.current());
            pPass->aiVertexArray.next();

            if(CORE_GL_SUPPORT(ARB_half_float_vertex))
            {
                // create instance data buffer (high quality compression)
                it->Create(pPass->iNumInstances, CORE_RICHTEXT_INSTANCE_SIZE_HIGH, NULL, CORE_DATABUFFER_STORAGE_DYNAMIC);
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_2D_POSITION_NUM, 3u, GL_FLOAT,          12u, false, 0u, 0u);
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_2D_SIZE_NUM,     2u, GL_HALF_FLOAT,     4u,  false, 0u, 12u);
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_2D_ROTATION_NUM, 2u, GL_SHORT,          4u,  false, 0u, 16u);
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,       4u, GL_UNSIGNED_BYTE,  4u,  false, 0u, 20u);
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM_NUM,    4u, GL_UNSIGNED_SHORT, 8u,  false, 0u, 24u);
            }
            else
            {
                // create instance data buffer (low quality compression)
                it->Create(pPass->iNumInstances, CORE_RICHTEXT_INSTANCE_SIZE_LOW, NULL, CORE_DATABUFFER_STORAGE_DYNAMIC);
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_2D_POSITION_NUM, 3u, GL_FLOAT,          12u, false, 0u, 0u);
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_2D_SIZE_NUM,     2u, GL_FLOAT,          8u,  false, 0u, 12u);
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_2D_ROTATION_NUM, 2u, GL_SHORT,          4u,  false, 0u, 20u);
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM,       4u, GL_UNSIGNED_BYTE,  4u,  false, 0u, 24u);
                it->DefineAttribute(CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM_NUM,    4u, GL_UNSIGNED_SHORT, 8u,  false, 0u, 28u);
            }

            // set vertex data
            Core::Manager::Object->GetLowQuad()->GetVertexBuffer(0u)->Activate(0u);
            it->Activate(1u);
        }

        // disable current model object (to fully enable the next model)
        coreModel::Disable(false);

        // invoke buffer update
        ADD_FLAG(m_eRefresh, CORE_RICHTEXT_REFRESH_BUFFER)
    }
}


// ****************************************************************
/* exit render pass */
void coreRichText::__ExitPass(corePass* OUTPUT pPass)
{
    // check for OpenGL extensions
    if(!CORE_GL_SUPPORT(ARB_instanced_arrays) || !CORE_GL_SUPPORT(ARB_vertex_array_object)) return;

    // delete vertex array objects
    if(pPass->aiVertexArray[0]) coreDelVertexArrays(CORE_RICHTEXT_INSTANCE_BUFFERS, pPass->aiVertexArray.data());
    pPass->aiVertexArray.fill(0u);

    // delete instance data buffers
    FOR_EACH(it, pPass->aInstanceBuffer) it->Delete();

    // reset selected array and buffer (to synchronize)
    pPass->aiVertexArray  .select(0u);
    pPass->aInstanceBuffer.select(0u);
}


// ****************************************************************
/* change current render pass size */
void coreRichText::__ReallocatePass(corePass* OUTPUT pPass, const coreUint32 iSize)
{
    if(iSize <= pPass->iNumInstances) return;

    // change current size
    pPass->iNumInstances = iSize;

    // reallocate the instance data buffers
    this->__ExitPass(pPass);
    this->__InitPass(pPass);
}