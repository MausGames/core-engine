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
coreStatus coreFont::Load(coreFile* pFile)
{
    WARN_IF(m_pFile)      return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetData()) return CORE_ERROR_FILE;

    // copy the input file for later font creations
    m_pFile = new coreFile(pFile->GetPath(), pFile->MoveData(), pFile->GetSize());

    // create test font
    if(!this->__InitHeight(1u, 0u)) return CORE_INVALID_DATA;

    // save properties
    m_sPath = pFile->GetPath();

    Core::Log->Info("Font (%s) loaded", pFile->GetPath());
    return CORE_OK;
}


// ****************************************************************
// unload font resource data
coreStatus coreFont::Unload()
{
    if(!m_pFile) return CORE_INVALID_CALL;

    // delete all sub-fonts
    FOR_EACH(it, m_aapFont) FOR_EACH(et, *it) TTF_CloseFont(*et);
    m_aapFont.clear();

    // delete file
    SAFE_DELETE(m_pFile)
    if(!m_sPath.empty()) Core::Log->Info("Font (%s) unloaded", m_sPath.c_str());

    // reset properties
    m_sPath = "";

    return CORE_OK;
}


// ****************************************************************
// create solid text with the font
SDL_Surface* coreFont::CreateText(const coreChar* pcText, const coreUint8& iHeight)
{
    // render and return the text surface
    return this->CreateTextOutline(pcText, iHeight, 0u);
}

SDL_Surface* coreFont::CreateGlyph(const coreUint16& iGlyph, const coreUint8& iHeight)
{
    // render and return the text surface
    return this->CreateGlyphOutline(iGlyph, iHeight, 0u);
}


// ****************************************************************
// create outlined text with the font
SDL_Surface* coreFont::CreateTextOutline(const coreChar* pcText, const coreUint8& iHeight, const coreUint8& iOutline)
{
    ASSERT(pcText)

    // check for specific height and outline
    if(!m_aapFont.count(iHeight) || !m_aapFont.at(iHeight).count(iOutline))
        this->__InitHeight(iHeight, iOutline);

    // define color
    constexpr_var SDL_Color aiFront = {0xFFu, 0xFFu, 0xFFu, 0xFFu};
    constexpr_var SDL_Color aiBack  = {0x00u, 0x00u, 0x00u, 0xFFu};

    // render and return the text surface
    return TTF_RenderUTF8_Shaded(m_aapFont.at(iHeight).at(iOutline), (pcText[0] == '\0') ? " " : pcText, aiFront, aiBack);
}

SDL_Surface* coreFont::CreateGlyphOutline(const coreUint16& iGlyph, const coreUint8& iHeight, const coreUint8& iOutline)
{
    // check for specific height
    if(!m_aapFont.count(iHeight) || !m_aapFont.at(iHeight).count(iOutline))
        this->__InitHeight(iHeight, iOutline);

    // define color
    constexpr_var SDL_Color aiFront = {0xFFu, 0xFFu, 0xFFu, 0xFFu};
    constexpr_var SDL_Color aiBack  = {0x00u, 0x00u, 0x00u, 0xFFu};

    // render and return the text surface
    return TTF_RenderGlyph_Shaded(m_aapFont.at(iHeight).at(iOutline), iGlyph, aiFront, aiBack);
}


// ****************************************************************
// retrieve the dimensions of a glyph
void coreFont::RetrieveGlyphMetrics(const coreUint16& iGlyph, const coreUint8& iHeight, const coreUint8& iOutline, coreInt32* OUTPUT piMinX, coreInt32* OUTPUT piMaxX, coreInt32* OUTPUT piMinY, coreInt32* OUTPUT piMaxY, coreInt32* OUTPUT piAdvance)
{
    // check for specific height
    if(!m_aapFont.count(iHeight) || !m_aapFont.at(iHeight).count(iOutline))
        this->__InitHeight(iHeight, iOutline);

    // retrieve the dimensions of a glyph
    TTF_GlyphMetrics(m_aapFont.at(iHeight).at(iOutline), iGlyph, piMinX, piMaxX, piMinY, piMaxY, piAdvance);
}


// ****************************************************************
// init font in a specific height
coreBool coreFont::__InitHeight(const coreUint8& iHeight, const coreUint8& iOutline)
{
    ASSERT(!m_aapFont.count(iHeight) || !m_aapFont.at(iHeight).count(iOutline))

    // create virtual file as rendering source
    SDL_RWops* pSource = SDL_RWFromConstMem(m_pFile->GetData(), m_pFile->GetSize());

    // create new sub-font
    TTF_Font* pNewFont = TTF_OpenFontRW(pSource, true, iHeight);
    if(!pNewFont)
    {
        Core::Log->Warning("Font (%s:%u:%u) could not be loaded", m_pFile->GetPath(), iHeight, iOutline);
        return false;
    }

    // enable font kerning and hinting
    TTF_SetFontKerning(pNewFont, 1);
    TTF_SetFontHinting(pNewFont, TTF_HINTING_MONO);

    // 
    if(iOutline) TTF_SetFontOutline(pNewFont, iOutline);

    // save sub-font
    m_aapFont[iHeight][iOutline] = pNewFont;
    return true;
}