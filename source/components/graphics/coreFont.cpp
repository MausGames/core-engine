//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
coreFont::coreFont()noexcept
: m_pFile (NULL)
{
}


// ****************************************************************
// destructor
coreFont::~coreFont()
{
    this->Unload();
}


// ****************************************************************
// load font resource data
coreError coreFont::Load(coreFile* pFile)
{
    ASSERT_IF(m_pFile)    return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetData()) return CORE_ERROR_FILE;

    // copy the input file for later font creations
    m_pFile = new coreFile(pFile->GetPath(), pFile->MoveData(), pFile->GetSize());

    // create test font
    if(!this->__InitHeight(1)) return CORE_INVALID_DATA;

    // save properties
    m_sPath = pFile->GetPath();
    m_iSize = pFile->GetSize();

    Core::Log->Info("Font (%s) loaded", pFile->GetPath());
    return CORE_OK;
}


// ****************************************************************    
// unload font resource data
coreError coreFont::Unload()
{
    if(!m_pFile) return CORE_INVALID_CALL;

    // delete all sub-fonts
    FOR_EACH(it, m_apFont) TTF_CloseFont(it->second);
    m_apFont.clear();

    // delete file
    SAFE_DELETE(m_pFile)
    if(!m_sPath.empty()) Core::Log->Info("Font (%s) unloaded", m_sPath.c_str());

    // reset properties
    m_sPath = "";
    m_iSize = 0;

    return CORE_OK;
}


// ****************************************************************   
// create text surface with the font
SDL_Surface* coreFont::CreateText(const char* pcText, const int& iHeight)
{
    ASSERT(pcText)

    // check for specific height
    if(!m_apFont.count(iHeight)) this->__InitHeight(iHeight);

    // define color
    constexpr_var SDL_Color aiFont = {0xFF, 0xFF, 0xFF};
    constexpr_var SDL_Color aiBack = {0x00, 0x00, 0x00};

    // render and return the text surface
    return TTF_RenderUTF8_Shaded(m_apFont.at(iHeight), (pcText[0] == '\0') ? " " : pcText, aiFont, aiBack);
}

SDL_Surface* coreFont::CreateGlyph(const coreUshort& iGlyph,  const int& iHeight)
{
    // check for specific height
    if(!m_apFont.count(iHeight)) this->__InitHeight(iHeight);

    // define color
    constexpr_var SDL_Color aiFont = {0xFF, 0xFF, 0xFF};
    constexpr_var SDL_Color aiBack = {0x00, 0x00, 0x00};

    // render and return the text surface
    return TTF_RenderGlyph_Shaded(m_apFont.at(iHeight), iGlyph, aiFont, aiBack);
}


// ****************************************************************   
// init font in a specific height
bool coreFont::__InitHeight(const int& iHeight)
{
    ASSERT(!m_apFont.count(iHeight))

    // create virtual file as rendering source
    SDL_RWops* pSource = SDL_RWFromConstMem(m_pFile->GetData(), m_pFile->GetSize());

    // create new sub-font
    TTF_Font* pNewFont = TTF_OpenFontRW(pSource, true, iHeight);
    if(!pNewFont)
    {
        Core::Log->Warning("Font (%s:%d) could not be loaded", m_pFile->GetPath(), iHeight);
        return false;
    }

    // enable light font hinting
    TTF_SetFontHinting(pNewFont, TTF_HINTING_LIGHT);

    // save sub-font
    m_apFont[iHeight] = pNewFont;
    return true;
}