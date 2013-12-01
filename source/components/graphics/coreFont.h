//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_FONT_H_
#define _CORE_GUARD_FONT_H_


// ****************************************************************
// font class
class coreFont final : public coreResource
{
private:
    std::u_map<int, TTF_Font*> m_apFont;   //!< list with sub-fonts in different heights
    coreFile* m_pFile;                     //!< file object with resource data


public:
    coreFont()noexcept;
    ~coreFont();

    //! load and unload font resource data
    //! @{
    coreError Load(coreFile* pFile)override;
    coreError Unload()override;
    //! @}

    //! create text surface with the font
    //! @{
    SDL_Surface* Create(const char* pcText, const coreVector3& vColor, const int& iHeight);
    //! @}

    //! get relative path to default resource
    //! @{
    static inline const char* GetDefaultPath() {return "data/fonts/arial.ttf";}
    //! @}


private:
    //! init the font in a specific height
    //! @{
    bool __InitHeight(const int& iHeight);
    //! @}
};


// ****************************************************************
// font resource access type
typedef coreResourcePtr<coreFont> coreFontPtr;


#endif // _CORE_GUARD_FONT_H_