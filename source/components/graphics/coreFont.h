///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_FONT_H_
#define _CORE_GUARD_FONT_H_

// TODO 5: distance fields for sharper text
// TODO 5: <old comment style>


// ****************************************************************
/* font definitions */
#define CORE_FONT_COLOR_FRONT (SDL_Color {0xFFu, 0xFFu, 0xFFu, 0xFFu})
#define CORE_FONT_COLOR_BACK  (SDL_Color {0x00u, 0x00u, 0x00u, 0xFFu})


// ****************************************************************
/* font class */
class coreFont final : public coreResource
{
private:
    coreMap<coreUint16, coreMap<coreUint8, TTF_Font*>> m_aapFont;   // list with sub-fonts in different heights <height, <outline>>
    coreFile* m_pFile;                                              // file object with resource data

    coreUint8 m_iHinting;                                           // hinting-algorithm to use (NORMAL, LIGHT, MONO, NONE)
    coreBool  m_bKerning;                                           // apply kerning if available


public:
    explicit coreFont(const coreUint8 iHinting = TTF_HINTING_NORMAL, const coreBool bKerning = true)noexcept;
    ~coreFont()final;

    DISABLE_COPY(coreFont)

    /* load and unload font resource data */
    coreStatus Load(coreFile* pFile)final;
    coreStatus Unload()final;

    /* create solid text with the font */
    SDL_Surface* CreateText (const coreChar*  pcText, const coreUint16 iHeight);
    SDL_Surface* CreateGlyph(const coreChar32 iGlyph, const coreUint16 iHeight);

    /* create outlined text with the font */
    SDL_Surface* CreateTextOutline (const coreChar*  pcText, const coreUint16 iHeight, const coreUint8 iOutline);
    SDL_Surface* CreateGlyphOutline(const coreChar32 iGlyph, const coreUint16 iHeight, const coreUint8 iOutline);

    /* retrieve text-related attributes */
    coreVector2 RetrieveTextDimensions(const coreChar* pcText, const coreUint16 iHeight, const coreUint8 iOutline);

    /* retrieve glyph-related attributes */
    coreBool  IsGlyphProvided     (const coreChar32 iGlyph);
    coreBool  IsGlyphProvided     (const coreChar*  pcMultiByte);
    void      RetrieveGlyphMetrics(const coreChar32 iGlyph,      const coreUint16 iHeight, const coreUint8 iOutline, coreInt32* OUTPUT piMinX, coreInt32* OUTPUT piMaxX, coreInt32* OUTPUT piMinY, coreInt32* OUTPUT piMaxY, coreInt32* OUTPUT piAdvance);
    coreUint8 RetrieveGlyphMetrics(const coreChar*  pcMultiByte, const coreUint16 iHeight, const coreUint8 iOutline, coreInt32* OUTPUT piMinX, coreInt32* OUTPUT piMaxX, coreInt32* OUTPUT piMinY, coreInt32* OUTPUT piMaxY, coreInt32* OUTPUT piAdvance);

    /* retrieve font-related attributes */
    inline const coreChar* RetrieveFamilyName() {ASSERT(!m_aapFont.empty()) return TTF_FontFaceFamilyName(m_aapFont.front().front());}
    inline const coreChar* RetrieveStyleName () {ASSERT(!m_aapFont.empty()) return TTF_FontFaceStyleName (m_aapFont.front().front());}


private:
    /* handle font with specific properties */
    coreBool __InitHeight  (const coreUint16 iHeight, const coreUint8 iOutline);
    coreBool __EnsureHeight(const coreUint16 iHeight, const coreUint8 iOutline);

    /* convert multibyte UTF-8 character to UTF-32 glyph */
    static coreUint8 __ConvertToGlyph(const coreChar* pcMultiByte, coreChar32* OUTPUT piGlyph);
};


// ****************************************************************
/* font resource access type */
using coreFontPtr = coreResourcePtr<coreFont>;


#endif /* _CORE_GUARD_FONT_H_ */