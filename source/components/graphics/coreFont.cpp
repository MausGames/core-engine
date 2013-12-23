//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
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
    if(!pFile->GetData()) return CORE_FILE_ERROR;

    // copy the input file for later font creations
    m_pFile = new coreFile(pFile->GetPath(), pFile->MoveData(), pFile->GetSize());

    // create test font
    if(!this->__InitHeight(1)) return CORE_INVALID_DATA;

    // save attributes
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
    FOR_EACH(it, m_apFont)
        TTF_CloseFont(it->second);
    m_apFont.clear();

    // delete file
    SAFE_DELETE(m_pFile)
    Core::Log->Info("Font (%s) unloaded", m_sPath.c_str());

    // reset attributes
    m_sPath = "";
    m_iSize = 0;

    return CORE_OK;
}


// ****************************************************************   
// create text surface with the font
SDL_Surface* coreFont::Create(const char* pcText, const coreVector3& vColor, const int& iHeight)
{
    SDL_assert(pcText);

    // check for specific height
    if(!m_apFont.count(iHeight)) this->__InitHeight(iHeight);

    // convert color
    const coreVector3 vConvert = vColor*255.0f;
    SDL_Color aiColor = {coreByte(vConvert.r),
                         coreByte(vConvert.g),
                         coreByte(vConvert.b),
                         255};

    // render and return the text surface
    return TTF_RenderText_Blended(m_apFont.at(iHeight), (pcText[0] == '\0') ? " " : pcText, aiColor);
}


// ****************************************************************   
// init font in a specific height
bool coreFont::__InitHeight(const int& iHeight)
{
    SDL_assert(!m_apFont.count(iHeight));

    // create virtual file as rendering source
    SDL_RWops* pSource = SDL_RWFromConstMem(m_pFile->GetData(), m_pFile->GetSize());

    // create new sub-font
    TTF_Font* pNewFont = TTF_OpenFontRW(pSource, true, iHeight);
    if(!pNewFont)
    {
        Core::Log->Error(false, "Font (%s:%d) could not be loaded", m_pFile->GetPath(), iHeight);
        return false;
    }

    // save sub-font
    m_apFont[iHeight] = pNewFont;
    return true;
}