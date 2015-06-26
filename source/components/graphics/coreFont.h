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


// ****************************************************************
// font definitions
#define FONT_OUTLINE_SIZE (1)   // default outline size (in pixels)


// ****************************************************************
// font class
class coreFont final : public coreResource
{
private:
    coreLookup<coreInt16, TTF_Font*> m_apFont;   //!< list with sub-fonts in different heights
    coreFile* m_pFile;                           //!< file object with resource data


public:
    coreFont()noexcept;
    ~coreFont();

    DISABLE_COPY(coreFont)

    //! load and unload font resource data
    //! @{
    coreStatus Load(coreFile* pFile)override;
    coreStatus Unload()override;
    //! @}

    //! create solid text with the font
    //! @{
    SDL_Surface* CreateText (const coreChar*   pcText, const coreUint8& iHeight);
    SDL_Surface* CreateGlyph(const coreUint16& iGlyph, const coreUint8& iHeight);
    //! @}

    //! create outlined text with the font
    //! @{
    SDL_Surface* CreateTextOutline(const coreChar* pcText, const coreUint8& iHeight);
    //! @}

    //! retrieve font-related attributes
    //! @{
    inline const coreChar* RetrieveFamilyName() {ASSERT(!m_apFont.empty()) return TTF_FontFaceFamilyName(m_apFont.at(1));}
    inline const coreChar* RetrieveStyleName () {ASSERT(!m_apFont.empty()) return TTF_FontFaceStyleName (m_apFont.at(1));}
    void RetrieveGlyphMetrics(const coreUint16& iGlyph, const coreUint8& iHeight, coreInt32* OUTPUT piMinX, coreInt32* OUTPUT piMaxX, coreInt32* OUTPUT piMinY, coreInt32* OUTPUT piMaxY, coreInt32* OUTPUT piAdvance);
    //! @}


private:
    //! init the font in a specific height
    //! @{
    coreBool __InitHeight(const coreInt16& iHeight);
    //! @}
};


// ****************************************************************
// font resource access type
using coreFontPtr = coreResourcePtr<coreFont>;


#endif // _CORE_GUARD_FONT_H_