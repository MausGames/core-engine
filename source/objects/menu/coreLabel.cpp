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
coreLabel::coreLabel(const char* pcFont, const int& iHeight, const coreUint& iLength)
: m_iHeight     (iHeight)
, m_vResolution (coreVector2(0.0f,0.0f))
, m_iLength     (0)
, m_sText       ("")
, m_fScale      (1.0f)
, m_iGenerate   (0)
{
    // set font object
    m_pFont = Core::Manager::Resource->LoadFile<coreFont>(pcFont);
    
    // create own texture to display text
    m_apTexture[0] = Core::Manager::Resource->LoadNew<coreTexture>();
    m_apTexture[0]->Generate();

    // load shaders
    this->DefineProgramShare(CORE_MEMORY_SHARED)
          ->AttachShaderFile("data/shaders/default_2d.vs")
          ->AttachShaderFile("data/shaders/default.fs")
          ->Finish();

    // check and set max number of characters
    if(iLength)
    {
        // create static texture
        m_sText.assign(MAX(iLength-1, (coreUint)1), 'W');
        m_sText.append(1, 'j');
        this->__Generate();
        m_sText = "";
    }
    m_iLength = iLength;
}


// ****************************************************************    
// destructor
coreLabel::~coreLabel()
{
    // free own texture
    Core::Manager::Resource->Free(&m_apTexture[0]);
}


// ****************************************************************    
// draw the label
void coreLabel::Render()
{
    if(m_sText.empty()) return;
    
    if(m_iGenerate)
    {
        // check if requested font is loaded
        if(!m_pFont.IsLoaded()) return;

        // update the texture of the label
        if(m_iGenerate & 2) this->__Generate();
        if(m_iGenerate & 1)
        {
            // update the object size
            this->SetSize(m_fScale * m_vTexSize * m_vResolution/Core::System->GetResolution().x);
            coreObject2D::Move();
        }
        m_iGenerate = 0;
    }

    // render the object
    coreObject2D::Render();
}


// ****************************************************************    
// move the label
// TODO: transformation matrix is not always immediately updated
void coreLabel::Move()
{
    if(m_sText.empty()) return;

    // move the object
    if(!(m_iGenerate & 1)) coreObject2D::Move();
}


// ****************************************************************    
// change the current text
bool coreLabel::SetText(const char* pcText, int iNum)
{
    const int iTextLength = std::strlen(pcText);

    // adjust the length
    if(iNum < 0 || iNum > iTextLength)        iNum = iTextLength;
    ASSERT_IF(iNum >= m_iLength && m_iLength) iNum = m_iLength;

    // check for new text
    const int iMaxNum = MAX((int)m_sText.length(), iNum);
    if(std::strncmp(m_sText.c_str(), pcText, iMaxNum) || !iMaxNum)
    {
        m_iGenerate |= 3; 

        // change the current text
        m_sText.assign(pcText, iNum);
        return true;
    }

    return false;
}


// ****************************************************************    
// update the texture of the label 
void coreLabel::__Generate()
{
    // create text surface with the font
    SDL_Surface* pSurface = m_pFont->Create(m_sText.c_str(), coreVector3(1.0f,1.0f,1.0f), m_iHeight);
    SDL_assert(pSurface->format->BitsPerPixel == 32);

    // convert the data format
    SDL_Surface* pConvert = SDL_CreateRGBSurface(0, pSurface->w, pSurface->h, 32, CORE_TEXTURE_MASK);
    SDL_BlitSurface(pSurface, NULL, pConvert, NULL);

    // get new texture resolution
    const coreVector2 vNewResolution = coreVector2(float(pConvert->w), float(pConvert->h));

    // update the texture
    m_apTexture[0]->Enable(0);
    if(m_iLength)
    {
        // update only a specific area of the texture
        SDL_assert((vNewResolution.x <= m_vResolution.x) && (vNewResolution.y <= m_vResolution.y));
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pConvert->w, pConvert->h, GL_RGBA, GL_UNSIGNED_BYTE, pConvert->pixels);

        // display only the specific area
        this->SetTexSize(vNewResolution/m_vResolution);
    }
    else
    {
        // completely create the texture
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pConvert->w, pConvert->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pConvert->pixels);

        // save the new texture resolution
        m_vResolution = vNewResolution;
    }

    // delete text surface data
    SDL_FreeSurface(pSurface);
    SDL_FreeSurface(pConvert);
}