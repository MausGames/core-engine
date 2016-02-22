//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_FONT_H_
#define _CORE_GUARD_FONT_H_

// TODO: distance fields for sharper text
// TODO: 4-byte UTF-8 is beyond U+10000 and therefore bigger than uint16


// ****************************************************************
// font class
class coreFont final : public coreResource
{
private:
    coreLookup<coreUint8, coreLookup<coreUint8, TTF_Font*>> m_aapFont;   //!< list with sub-fonts in different heights <height, <outline> >
    coreFile* m_pFile;                                                   //!< file object with resource data


public:
    coreFont()noexcept;
    ~coreFont()override;

    DISABLE_COPY(coreFont)

    //! load and unload font resource data
    //! @{
    coreStatus Load(coreFile* pFile)override;
    coreStatus Unload()override;
    //! @}

    //! create solid text with the font
    //! @{
    SDL_Surface* CreateText (const coreChar*  pcText, const coreUint8 iHeight);
    SDL_Surface* CreateGlyph(const coreUint16 iGlyph, const coreUint8 iHeight);
    //! @}

    //! create outlined text with the font
    //! @{
    SDL_Surface* CreateTextOutline (const coreChar*  pcText, const coreUint8 iHeight, const coreUint8 iOutline);
    SDL_Surface* CreateGlyphOutline(const coreUint16 iGlyph, const coreUint8 iHeight, const coreUint8 iOutline);
    //! @}

    //! retrieve font-related attributes
    //! @{
    inline const coreChar* RetrieveFamilyName() {ASSERT(!m_aapFont.empty()) return TTF_FontFaceFamilyName(m_aapFont.at(1u).at(0u));}
    inline const coreChar* RetrieveStyleName () {ASSERT(!m_aapFont.empty()) return TTF_FontFaceStyleName (m_aapFont.at(1u).at(0u));}
    void      RetrieveGlyphMetrics(const coreUint16 iGlyph,      const coreUint8 iHeight, const coreUint8 iOutline, coreInt32* OUTPUT piMinX, coreInt32* OUTPUT piMaxX, coreInt32* OUTPUT piMinY, coreInt32* OUTPUT piMaxY, coreInt32* OUTPUT piAdvance);
    coreUint8 RetrieveGlyphMetrics(const coreChar*  pcMultiByte, const coreUint8 iHeight, const coreUint8 iOutline, coreInt32* OUTPUT piMinX, coreInt32* OUTPUT piMaxX, coreInt32* OUTPUT piMinY, coreInt32* OUTPUT piMaxY, coreInt32* OUTPUT piAdvance);
    //! @}


private:
    //! init the font in a specific height
    //! @{
    coreBool __InitHeight(const coreUint8 iHeight, const coreUint8 iOutline);
    //! @}
};


// ****************************************************************
// font resource access type
using coreFontPtr = coreResourcePtr<coreFont>;


#endif // _CORE_GUARD_FONT_H_