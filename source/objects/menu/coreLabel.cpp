//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

#define CORE_LABEL_DETAIL 0.95f


// ****************************************************************    
// constructor
coreLabel::coreLabel(const char* pcFont, const int& iHeight, const coreUint& iLength)
: m_iHeight     (int(float(iHeight) * (Core::System->GetResolution().x/800.0f) * CORE_LABEL_DETAIL))
, m_vResolution (coreVector2(0.0f,0.0f))
, m_iLength     (iLength)
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
        if(m_iGenerate & 2) this->__Generate(m_sText.c_str(), m_iLength ? true : false);
        if(m_iGenerate & 1)
        {
            // update the object size
            this->SetSize(m_fScale * m_vTexSize * (m_vResolution/Core::System->GetResolution().x) / CORE_LABEL_DETAIL);
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
    const int iNewLength = std::strlen(pcText);

    // adjust the length
    if(iNum < 0 || iNum > iNewLength)         iNum = iNewLength;
    ASSERT_IF(iNum >= m_iLength && m_iLength) iNum = m_iLength;

    // check for new text
    if(std::strcmp(m_sText.c_str(), pcText) || m_sText.length() != iNum)
    {
        m_iGenerate |= 3; 

        // change the current text
        m_sText.assign(pcText, iNum);
        return true;
    }

    return false;
}


// ****************************************************************
// reset with the resource manager
void coreLabel::__Reset(const bool& bInit)
{
    if(bInit)
    {
        // regenerate empty base texture
        m_apTexture[0]->Generate();

        // invoke texture update
        m_vResolution = coreVector2(0.0f,0.0f);
        m_iGenerate   = 3;
    }
    else m_apTexture[0]->Unload();
}


// ****************************************************************    
// update the texture of the label 
void coreLabel::__Generate(const char* pcText, const bool& bSub)
{
    // create text surface with the font
    SDL_Surface* pSurface = m_pFont->Create(pcText, coreVector3(1.0f,1.0f,1.0f), m_iHeight);
    SDL_assert(pSurface->format->BitsPerPixel == 32);

    // convert the data format
    SDL_Surface* pConvert = SDL_CreateRGBSurface(0, pSurface->w, pSurface->h, 32, CORE_TEXTURE_MASK);
    SDL_BlitSurface(pSurface, NULL, pConvert, NULL);

    // get new texture resolution
    const coreVector2 vNewResolution = coreVector2(float(pConvert->w), float(pConvert->h));

    // update the texture
    m_apTexture[0]->Enable(0);
    if(bSub)
    {
        if(!m_vResolution.x)
        {
            // assemble string for maximum size
            std::string sMaxText(MAX(m_iLength-1, 1), 'W');
            sMaxText.append(1, 'j');

            // create static texture
            this->__Generate(sMaxText.c_str(), false);
        }

        coreTexture::Lock();
        {
            // update only a specific area of the texture
            SDL_assert((vNewResolution.x <= m_vResolution.x) && (vNewResolution.y <= m_vResolution.y));
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pConvert->w, pConvert->h, GL_RGBA, GL_UNSIGNED_BYTE, pConvert->pixels);
        }
        coreTexture::Unlock();

        // display only the specific area
        this->SetTexSize(vNewResolution/m_vResolution);
    }
    else
    {
        coreTexture::Lock();
        {
            // completely create the texture
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pConvert->w, pConvert->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pConvert->pixels);
        }
        coreTexture::Unlock();

        // save the new texture resolution
        m_vResolution = vNewResolution;
    }

    // delete text surface data
    SDL_FreeSurface(pSurface);
    SDL_FreeSurface(pConvert);
}