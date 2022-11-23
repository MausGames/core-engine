///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"
#include <convert_utf.h>


// ****************************************************************
/* constructor */
coreFont::coreFont(const coreUint8 iHinting, const coreBool bKerning)noexcept
: coreResource ()
, m_aapFont    {}
, m_pFile      (NULL)
, m_iHinting   (iHinting)
, m_bKerning   (bKerning)
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

#if defined(_CORE_DEBUG_)

    // create test sub-font
    WARN_IF(!this->__InitHeight(1u, 0u)) {}

#endif

    // save properties
    m_sPath = pFile->GetPath();

    Core::Log->Info("Font (%s) loaded", pFile->GetPath());
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
    if(!m_sPath.empty()) Core::Log->Info("Font (%s) unloaded", m_sPath.c_str());

    // reset properties
    m_sPath = "";

    return CORE_OK;
}


// ****************************************************************
/* create solid text with the font */
SDL_Surface* coreFont::CreateText(const coreChar* pcText, const coreUint16 iHeight)
{
    // render and return the text surface
    return this->CreateTextOutline(pcText, iHeight, 0u);
}

SDL_Surface* coreFont::CreateGlyph(const coreUint16 iGlyph, const coreUint16 iHeight)
{
    // render and return the text surface
    return this->CreateGlyphOutline(iGlyph, iHeight, 0u);
}


// ****************************************************************
/* create outlined text with the font */
SDL_Surface* coreFont::CreateTextOutline(const coreChar* pcText, const coreUint16 iHeight, const coreUint8 iOutline)
{
    ASSERT(pcText)

    // check for requested height and outline
    this->__EnsureHeight(iHeight, iOutline);

    // render and return the text surface
    return TTF_RenderUTF8_Shaded(m_aapFont.at(iHeight).at(iOutline), (pcText[0] == '\0') ? " " : pcText, CORE_FONT_COLOR_FRONT, CORE_FONT_COLOR_BACK);
}

SDL_Surface* coreFont::CreateGlyphOutline(const coreUint16 iGlyph, const coreUint16 iHeight, const coreUint8 iOutline)
{
    // check for requested height and outline
    this->__EnsureHeight(iHeight, iOutline);

    // render and return the text surface
    return TTF_RenderGlyph_Shaded(m_aapFont.at(iHeight).at(iOutline), iGlyph, CORE_FONT_COLOR_FRONT, CORE_FONT_COLOR_BACK);
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
    TTF_SizeUTF8(m_aapFont.at(iHeight).at(iOutline), pcText, &iX, &iY);

    return coreVector2(I_TO_F(iX), I_TO_F(iY));
}


// ****************************************************************
/* check if a glyph if provided by the font */
coreBool coreFont::IsGlyphProvided(const coreUint16 iGlyph)
{
    ASSERT(!m_aapFont.empty())

    // check for the glyph with first available sub-font
    return (TTF_GlyphIsProvided(m_aapFont.front().front(), iGlyph) != 0);
}

coreBool coreFont::IsGlyphProvided(const coreChar* pcMultiByte)
{
    // convert multibyte UTF-8 character to UTF-16 glyph
    coreUint16 iGlyph;
    coreFont::__ConvertToGlyph(pcMultiByte, &iGlyph);

    // check for the glyph
    return this->IsGlyphProvided(iGlyph);
}


// ****************************************************************
/* retrieve the dimensions of a glyph */
void coreFont::RetrieveGlyphMetrics(const coreUint16 iGlyph, const coreUint16 iHeight, const coreUint8 iOutline, coreInt32* OUTPUT piMinX, coreInt32* OUTPUT piMaxX, coreInt32* OUTPUT piMinY, coreInt32* OUTPUT piMaxY, coreInt32* OUTPUT piAdvance)
{
    // check for requested height and outline
    this->__EnsureHeight(iHeight, iOutline);

    // retrieve dimensions
    TTF_GlyphMetrics(m_aapFont.at(iHeight).at(iOutline), iGlyph, piMinX, piMaxX, piMinY, piMaxY, piAdvance);
}

coreUint8 coreFont::RetrieveGlyphMetrics(const coreChar* pcMultiByte, const coreUint16 iHeight, const coreUint8 iOutline, coreInt32* OUTPUT piMinX, coreInt32* OUTPUT piMaxX, coreInt32* OUTPUT piMinY, coreInt32* OUTPUT piMaxY, coreInt32* OUTPUT piAdvance)
{
    // convert multibyte UTF-8 character to UTF-16 glyph
    coreUint16 iGlyph;
    const coreUint8 iBytes = coreFont::__ConvertToGlyph(pcMultiByte, &iGlyph);

    // retrieve dimensions and return number of bytes
    this->RetrieveGlyphMetrics(iGlyph, iHeight, iOutline, piMinX, piMaxX, piMinY, piMaxY, piAdvance);
    return iBytes;
}


// ****************************************************************
/* init font with specific properties */
coreBool coreFont::__InitHeight(const coreUint16 iHeight, const coreUint8 iOutline)
{
    ASSERT(!m_aapFont.count(iHeight) || !m_aapFont.at(iHeight).count(iOutline))

    // create new sub-font
    TTF_Font* pNewFont = TTF_OpenFontRW(m_pFile->CreateReadStream(), 1, iHeight);
    if(!pNewFont)
    {
        Core::Log->Warning("Sub-Font (%s, %u height, %u outline) could not be loaded", m_pFile->GetPath(), iHeight, iOutline);
        return false;
    }

    // enable font hinting and kerning
    TTF_SetFontHinting(pNewFont, m_iHinting);
    TTF_SetFontKerning(pNewFont, m_bKerning ? 1 : 0);

    // enable outlining
    if(iOutline) TTF_SetFontOutline(pNewFont, iOutline);

    // save sub-font
    m_aapFont[iHeight].emplace(iOutline, pNewFont);

    Core::Log->Info("Sub-Font (%s, %u height, %u outline) loaded", m_pFile->GetPath(), iHeight, iOutline);
    return true;
}


// ****************************************************************
/* ensure font with specific properties */
coreBool coreFont::__EnsureHeight(const coreUint16 iHeight, const coreUint8 iOutline)
{
    // check for requested height and outline
    if(!m_aapFont.count(iHeight) || !m_aapFont.at(iHeight).count(iOutline))
        return this->__InitHeight(iHeight, iOutline);

    return true;
}


// ****************************************************************
/* convert multibyte UTF-8 character to UTF-16 glyph */
coreUint8 coreFont::__ConvertToGlyph(const coreChar* pcMultiByte, coreUint16* OUTPUT piGlyph)
{
    ASSERT(pcMultiByte && piGlyph)

    // handle UTF-8 encoding
    if(HAS_FLAG((*pcMultiByte), 0x80u))
    {
        // count number of bytes
        const coreUint8 iBytes = 2u + HAS_FLAG((*pcMultiByte), 0xE0u) + HAS_FLAG((*pcMultiByte), 0xF0u);
        ASSERT(iBytes <= 4u)

        // convert character
        const ConversionResult eResult = ConvertUTF8toUTF16(r_cast<const coreUint8*>(pcMultiByte), r_cast<const coreUint8*>(pcMultiByte) + iBytes, piGlyph, piGlyph + 1u, lenientConversion);
        ASSERT(eResult == conversionOK)

        return iBytes;
    }

    // just forward the character
    (*piGlyph) = (*pcMultiByte);

    return 1u;
}