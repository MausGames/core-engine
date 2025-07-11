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
coreFont::coreFont(const TTF_HintingFlags eHinting, const coreBool bKerning)noexcept
: coreResource   ()
, m_aapFont      {}
, m_pFile        (NULL)
, m_eHinting     (eHinting)
, m_bKerning     (bKerning)
, m_iLastHeight  (0u)
, m_iLastOutline (0u)
{
}


// ****************************************************************
/* destructor */
coreFont::~coreFont()
{
    this->Unload();
}


// ****************************************************************
/* load font resource data */
coreStatus coreFont::Load(coreFile* pFile)
{
    WARN_IF(m_pFile)      return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetSize()) return CORE_ERROR_FILE;   // do not load file data

    // copy file object for later sub-font creation
    coreFile::InternalNew(&m_pFile, pFile);

    // create test sub-font
    WARN_IF(!this->__InitHeight(1u, 0u))
    {
        Core::Log->Warning("Font (%s) could not be loaded", m_sName.c_str());
        coreFile::InternalDelete(&m_pFile);

        return CORE_INVALID_DATA;
    }

    Core::Log->Info("Font (%s, %s, %s %d, %d faces) loaded", m_sName.c_str(), this->RetrieveFamilyName(), this->RetrieveStyleName(), TTF_GetFontWeight(m_aapFont.front().front()), TTF_GetNumFontFaces(m_aapFont.front().front()));
    return CORE_OK;
}


// ****************************************************************
/* unload font resource data */
coreStatus coreFont::Unload()
{
    if(!m_pFile) return CORE_INVALID_CALL;

    // delete all sub-fonts
    FOR_EACH(it, m_aapFont) FOR_EACH(et, *it) TTF_CloseFont(*et);
    m_aapFont.clear();

    // delete file object
    coreFile::InternalDelete(&m_pFile);
    if(!m_sName.empty()) Core::Log->Info("Font (%s) unloaded", m_sName.c_str());

    // reset properties
    m_iLastHeight  = 0u;
    m_iLastOutline = 0u;

    return CORE_OK;
}


// ****************************************************************
/* create solid text with the font */
SDL_Surface* coreFont::CreateText(const coreChar* pcText, const coreUint16 iHeight)
{
    // render and return the text surface
    return this->CreateTextOutline(pcText, iHeight, 0u);
}

SDL_Surface* coreFont::CreateGlyph(const coreChar32 cGlyph, const coreUint16 iHeight)
{
    // render and return the text surface
    return this->CreateGlyphOutline(cGlyph, iHeight, 0u);
}


// ****************************************************************
/* create outlined text with the font */
SDL_Surface* coreFont::CreateTextOutline(const coreChar* pcText, const coreUint16 iHeight, const coreUint8 iOutline)
{
    ASSERT(pcText)

    // check for requested height and outline
    this->__EnsureHeight(iHeight, iOutline);

    // render and return the text surface
    return TTF_RenderText_Shaded(m_aapFont.at(iHeight).at(iOutline), (pcText[0] == '\0') ? " " : pcText, 0u, CORE_FONT_COLOR_FRONT, CORE_FONT_COLOR_BACK);
}

SDL_Surface* coreFont::CreateGlyphOutline(const coreChar32 cGlyph, const coreUint16 iHeight, const coreUint8 iOutline)
{
    // check for requested height and outline
    this->__EnsureHeight(iHeight, iOutline);

    // render and return the text surface
    return TTF_RenderGlyph_Shaded(m_aapFont.at(iHeight).at(iOutline), cGlyph, CORE_FONT_COLOR_FRONT, CORE_FONT_COLOR_BACK);
}


// ****************************************************************
/* check if all glyphs are provided by the font */
coreBool coreFont::AreGlyphsProvided(const coreChar* pcText)
{
    ASSERT(!m_aapFont.empty())

    // use first available sub-font
    TTF_Font* pFont = m_aapFont.front().front();

    const coreChar* pcCursor = pcText;
    while(*pcCursor)
    {
        // convert multibyte UTF-8 character to UTF-32 glyph
        coreChar32 cGlyph;
        pcCursor += coreFont::ConvertToGlyph(pcCursor, &cGlyph);

        // check for the glyph
        if(!TTF_FontHasGlyph(pFont, cGlyph))
        {
            return false;
        }
    }

    return true;
}


// ****************************************************************
/* retrieve the dimensions of a rendered string of text */
coreVector2 coreFont::RetrieveTextDimensions(const coreChar* pcText, const coreUint16 iHeight, const coreUint8 iOutline)
{
    ASSERT(pcText)

    // check for requested height and outline
    this->__EnsureHeight(iHeight, iOutline);

    // retrieve dimensions
    coreInt32 iX, iY;
    TTF_GetStringSize(m_aapFont.at(iHeight).at(iOutline), pcText, 0u, &iX, &iY);

    return coreVector2(I_TO_F(iX), I_TO_F(iY));
}


// ****************************************************************
/* retrieve the vertical shift of a rendered string of text */
coreInt8 coreFont::RetrieveTextShift(const coreChar* pcText, const coreUint16 iHeight, const coreUint8 iOutline)
{
    coreInt32 iTotalMinY = INT32_MAX, iTotalMaxY = INT32_MIN;

    // check for requested height and outline
    this->__EnsureHeight(iHeight, iOutline);
    TTF_Font* pFont = m_aapFont.at(iHeight).at(iOutline);

    const coreChar* pcCursor = pcText;
    while(*pcCursor)
    {
        // convert multibyte UTF-8 character to UTF-32 glyph
        coreChar32 cGlyph;
        pcCursor += coreFont::ConvertToGlyph(pcCursor, &cGlyph);

        // retrieve vertical bounds
        coreInt32 iMinY, iMaxY;
        TTF_GetGlyphMetrics(pFont, cGlyph, NULL, NULL, &iMinY, &iMaxY, NULL);

        iTotalMinY = MIN(iTotalMinY, iMinY);
        iTotalMaxY = MAX(iTotalMaxY, iMaxY);
    }

    // retrieve baseline offsets
    const coreInt32 iAscent  = TTF_GetFontAscent (pFont);
    const coreInt32 iDescent = TTF_GetFontDescent(pFont);

    // calculate final shift
    return coreInt8(MIN(iAscent - iTotalMaxY, 0) + MAX(iDescent - iTotalMinY, 0));
}


// ****************************************************************
/* check if a glyph if provided by the font */
coreBool coreFont::IsGlyphProvided(const coreChar32 cGlyph)
{
    ASSERT(!m_aapFont.empty())

    // check for the glyph with first available sub-font
    return TTF_FontHasGlyph(m_aapFont.front().front(), cGlyph);
}

coreBool coreFont::IsGlyphProvided(const coreChar* pcMultiByte)
{
    // convert multibyte UTF-8 character to UTF-32 glyph
    coreChar32 cGlyph;
    coreFont::ConvertToGlyph(pcMultiByte, &cGlyph);

    // check for the glyph
    return this->IsGlyphProvided(cGlyph);
}


// ****************************************************************
/* retrieve dimensions of a glyph */
void coreFont::RetrieveGlyphMetrics(const coreChar32 cGlyph, const coreUint16 iHeight, const coreUint8 iOutline, coreInt32* OUTPUT piMinX, coreInt32* OUTPUT piMaxX, coreInt32* OUTPUT piMinY, coreInt32* OUTPUT piMaxY, coreInt32* OUTPUT piAdvance, coreInt32* OUTPUT piShift)
{
    // check for requested height and outline
    this->__EnsureHeight(iHeight, iOutline);
    TTF_Font* pFont = m_aapFont.at(iHeight).at(iOutline);

    // retrieve dimensions
    coreInt32 iMinY, iMaxY;
    TTF_GetGlyphMetrics(pFont, cGlyph, piMinX, piMaxX, &iMinY, &iMaxY, piAdvance);

    // return vertical bounds
    if(piMinY) (*piMinY) = iMinY;
    if(piMaxY) (*piMaxY) = iMaxY;

    if(piShift)
    {
        // retrieve baseline offsets
        const coreInt32 iAscent  = TTF_GetFontAscent (pFont);
        const coreInt32 iDescent = TTF_GetFontDescent(pFont);

        // calculate final shift
        (*piShift) = MIN(iAscent - iMaxY, 0) + MAX(iDescent - iMinY, 0);
    }
}

coreUint8 coreFont::RetrieveGlyphMetrics(const coreChar* pcMultiByte, const coreUint16 iHeight, const coreUint8 iOutline, coreInt32* OUTPUT piMinX, coreInt32* OUTPUT piMaxX, coreInt32* OUTPUT piMinY, coreInt32* OUTPUT piMaxY, coreInt32* OUTPUT piAdvance, coreInt32* OUTPUT piShift)
{
    // convert multibyte UTF-8 character to UTF-32 glyph
    coreChar32 cGlyph;
    const coreUint8 iBytes = coreFont::ConvertToGlyph(pcMultiByte, &cGlyph);

    // retrieve dimensions and return number of bytes
    this->RetrieveGlyphMetrics(cGlyph, iHeight, iOutline, piMinX, piMaxX, piMinY, piMaxY, piAdvance, piShift);
    return iBytes;
}


// ****************************************************************
/* retrieve kerning between two glyphs */
coreInt32 coreFont::RetrieveGlyphKerning(const coreChar32 cGlyph1, const coreChar32 cGlyph2, const coreUint16 iHeight, const coreUint8 iOutline)
{
    // check for requested height and outline
    this->__EnsureHeight(iHeight, iOutline);

    // retrieve kerning
    coreInt32 iKerning;
    return TTF_GetGlyphKerning(m_aapFont.at(iHeight).at(iOutline), cGlyph1, cGlyph2, &iKerning) ? iKerning : 0;
}

coreInt32 coreFont::RetrieveGlyphKerning(const coreChar* pcMultiByte1, const coreChar* pcMultiByte2, const coreUint16 iHeight, const coreUint8 iOutline)
{
    // convert multibyte UTF-8 character to UTF-32 glyph
    coreChar32 cGlyph1, cGlyph2;
    coreFont::ConvertToGlyph(pcMultiByte1, &cGlyph1);
    coreFont::ConvertToGlyph(pcMultiByte2, &cGlyph2);

    // retrieve kerning
    return this->RetrieveGlyphKerning(cGlyph1, cGlyph2, iHeight, iOutline);
}


// ****************************************************************
/* convert multibyte UTF-8 character to UTF-32 glyph */
coreUint8 coreFont::ConvertToGlyph(const coreChar* pcMultiByte, coreChar32* OUTPUT pcGlyph)
{
    ASSERT(pcMultiByte && pcGlyph)

    // handle UTF-8 encoding
    if(HAS_FLAG((*pcMultiByte), 0x80u))
    {
        // count number of bytes
        const coreUint8 iBytes = 2u + HAS_FLAG((*pcMultiByte), 0xE0u) + HAS_FLAG((*pcMultiByte), 0xF0u);
        ASSERT(iBytes <= 4u)

        // convert character
        (*pcGlyph) = SDL_StepUTF8(&pcMultiByte, NULL);
        ASSERT((*pcGlyph) != SDL_INVALID_UNICODE_CODEPOINT)

        return iBytes;
    }

    // just forward the character
    (*pcGlyph) = (*pcMultiByte);

    return 1u;
}


// ****************************************************************
/* init font with specific properties */
coreBool coreFont::__InitHeight(const coreUint16 iHeight, const coreUint8 iOutline)
{
    ASSERT(!m_aapFont.count(iHeight) || !m_aapFont.at(iHeight).count(iOutline))

    // set function properties
    coreProperties oProps;
    if(m_aapFont.empty())
    {
        SDL_SetPointerProperty(oProps, TTF_PROP_FONT_CREATE_IOSTREAM_POINTER,           m_pFile->CreateReadStream());
        SDL_SetBooleanProperty(oProps, TTF_PROP_FONT_CREATE_IOSTREAM_AUTOCLOSE_BOOLEAN, true);
        SDL_SetFloatProperty  (oProps, TTF_PROP_FONT_CREATE_SIZE_FLOAT,                 I_TO_F(iHeight));
    }
    else
    {
        SDL_SetPointerProperty(oProps, TTF_PROP_FONT_CREATE_EXISTING_FONT,              m_aapFont.front().front());
        SDL_SetFloatProperty  (oProps, TTF_PROP_FONT_CREATE_SIZE_FLOAT,                 I_TO_F(iHeight));
    }

    // create new sub-font
    TTF_Font* pNewFont = TTF_OpenFontWithProperties(oProps);
    if(!pNewFont)
    {
        Core::Log->Warning("Sub-Font (%s, %u height, %u outline) could not be loaded", m_sName.c_str(), iHeight, iOutline);
        return false;
    }

    // enable font hinting and kerning
    TTF_SetFontHinting(pNewFont, m_eHinting);
    TTF_SetFontKerning(pNewFont, m_bKerning);

    // enable outlining
    TTF_SetFontOutline(pNewFont, iOutline);

    // save sub-font
    m_aapFont[iHeight].emplace(iOutline, pNewFont);

    Core::Log->Info("Sub-Font (%s, %u height, %u outline) loaded", m_sName.c_str(), iHeight, iOutline);
    return true;
}


// ****************************************************************
/* ensure font with specific properties */
coreBool coreFont::__EnsureHeight(const coreUint16 iHeight, const coreUint8 iOutline)
{
    if((m_iLastHeight == iHeight) && (m_iLastOutline == iOutline)) return true;

    // check for requested height and outline
    if(!m_aapFont.count(iHeight) || !m_aapFont.at(iHeight).count(iOutline))
    {
        if(!this->__InitHeight(iHeight, iOutline)) return false;
    }

    // cache last request
    m_iLastHeight  = iHeight;
    m_iLastOutline = iOutline;
    return true;
}