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

// TODO 3: distance fields for sharper text (TTF_SetFontSDF + TTF_RenderGlyph_Blended)
// TODO 5: clear-type font rendering (TTF_RenderGlyph_LCD) -> 4 channel, but how to add (clear-type) outlines?
// TODO 5: <old comment style>
// TODO 3: TTF_AddFallbackFont
// TODO 3: check support for color emojis
// TODO 3: allow floating-point font height, but that would increase permutations significantly (maybe somehow use unmodified base-height as key)


// ****************************************************************
/* font definitions */
#define CORE_FONT_COLOR_FRONT (SDL_Color {0xFFu, 0xFFu, 0xFFu, 0xFFu})
#define CORE_FONT_COLOR_BACK  (SDL_Color {0x00u, 0x00u, 0x00u, 0xFFu})

#define CORE_FONT_GLYPH_NBSP  (U'\u00A0')   // no-break space
#define CORE_FONT_GLYPH_NNBSP (U'\u202F')   // narrow no-break space
#define CORE_FONT_GLYPH_WJ    (U'\u2060')   // word joiner


// ****************************************************************
/* font class */
class coreFont final : public coreResource
{
private:
    coreMap<coreUint16, coreMap<coreUint8, TTF_Font*>> m_aapFont;   // list with sub-fonts in different heights <height, <outline>>
    coreFile* m_pFile;                                              // file object with resource data

    TTF_HintingFlags m_eHinting;                                    // hinting-algorithm to use
    coreBool         m_bKerning;                                    // apply kerning if available

    coreUint16 m_iLastHeight;                                       // last requested height
    coreUint8  m_iLastOutline;                                      // last requested outline


public:
    explicit coreFont(const TTF_HintingFlags eHinting = TTF_HINTING_LIGHT, const coreBool bKerning = true)noexcept;
    ~coreFont()final;

    DISABLE_COPY(coreFont)

    /* load and unload font resource data */
    coreStatus Load(coreFile* pFile)final;
    coreStatus Unload()final;

    /* get resource type */
    inline coreResourceType GetResourceType()const final {return CORE_RESOURCE_TYPE_DEFAULT;}

    /* create solid text with the font */
    SDL_Surface* CreateText (const coreChar*  pcText, const coreUint16 iHeight);
    SDL_Surface* CreateGlyph(const coreChar32 cGlyph, const coreUint16 iHeight);

    /* create outlined text with the font */
    SDL_Surface* CreateTextOutline (const coreChar*  pcText, const coreUint16 iHeight, const coreUint8 iOutline);
    SDL_Surface* CreateGlyphOutline(const coreChar32 cGlyph, const coreUint16 iHeight, const coreUint8 iOutline);

    /* retrieve text-related attributes */
    coreBool    AreGlyphsProvided     (const coreChar* pcText);
    coreVector2 RetrieveTextDimensions(const coreChar* pcText, const coreUint16 iHeight, const coreUint8 iOutline);
    coreInt8    RetrieveTextShift     (const coreChar* pcText, const coreUint16 iHeight, const coreUint8 iOutline, coreInt8* OUTPUT piTop, coreInt8* OUTPUT piBottom);

    /* retrieve glyph-related attributes */
    coreBool  IsGlyphProvided     (const coreChar32 cGlyph);
    coreBool  IsGlyphProvided     (const coreChar*  pcMultiByte);
    void      RetrieveGlyphMetrics(const coreChar32 cGlyph,                                      const coreUint16 iHeight, const coreUint8 iOutline, coreInt32* OUTPUT piMinX, coreInt32* OUTPUT piMaxX, coreInt32* OUTPUT piMinY, coreInt32* OUTPUT piMaxY, coreInt32* OUTPUT piAdvance);
    coreUint8 RetrieveGlyphMetrics(const coreChar*  pcMultiByte,                                 const coreUint16 iHeight, const coreUint8 iOutline, coreInt32* OUTPUT piMinX, coreInt32* OUTPUT piMaxX, coreInt32* OUTPUT piMinY, coreInt32* OUTPUT piMaxY, coreInt32* OUTPUT piAdvance);
    coreInt32 RetrieveGlyphKerning(const coreChar32 cGlyph1,      const coreChar32 cGlyph2,      const coreUint16 iHeight, const coreUint8 iOutline);
    coreInt32 RetrieveGlyphKerning(const coreChar*  pcMultiByte1, const coreChar*  pcMultiByte2, const coreUint16 iHeight, const coreUint8 iOutline);

    /* retrieve font-related attributes */
    inline const coreChar* RetrieveFamilyName()const                                              {ASSERT(!m_aapFont.empty()) return TTF_GetFontFamilyName(m_aapFont.front().front());}
    inline const coreChar* RetrieveStyleName ()const                                              {ASSERT(!m_aapFont.empty()) return TTF_GetFontStyleName (m_aapFont.front().front());}
    inline       coreInt32 RetrieveAscent    (const coreUint16 iHeight, const coreUint8 iOutline) {this->__EnsureHeight(iHeight, iOutline); return TTF_GetFontAscent  (m_aapFont.at(iHeight).at(iOutline));}
    inline       coreInt32 RetrieveDescent   (const coreUint16 iHeight, const coreUint8 iOutline) {this->__EnsureHeight(iHeight, iOutline); return TTF_GetFontDescent (m_aapFont.at(iHeight).at(iOutline));}
    inline       coreInt32 RetrieveRealHeight(const coreUint16 iHeight, const coreUint8 iOutline) {this->__EnsureHeight(iHeight, iOutline); return TTF_GetFontHeight  (m_aapFont.at(iHeight).at(iOutline));}
    inline       coreInt32 RetrieveLineSkip  (const coreUint16 iHeight, const coreUint8 iOutline) {this->__EnsureHeight(iHeight, iOutline); return TTF_GetFontLineSkip(m_aapFont.at(iHeight).at(iOutline));}

    /* convert multibyte UTF-8 character to UTF-32 glyph */
    static coreUint8 ConvertToGlyph(const coreChar* pcMultiByte, coreChar32* OUTPUT pcGlyph);


private:
    /* handle font with specific properties */
    coreBool __InitHeight  (const coreUint16 iHeight, const coreUint8 iOutline);
    coreBool __EnsureHeight(const coreUint16 iHeight, const coreUint8 iOutline);
};


// ****************************************************************
/* font resource access type */
using coreFontPtr = coreResourcePtr<coreFont>;


#endif /* _CORE_GUARD_FONT_H_ */