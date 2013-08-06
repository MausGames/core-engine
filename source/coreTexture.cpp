#include "Core.h"

int          coreTexture::s_iCurUnit = -1;
coreTexture* coreTexture::s_apActive[32]; // = NULL


// ****************************************************************
// constructor
coreTexture::coreTexture(const bool &bGenerate)
: coreResource  ("")
, m_iID         (0)
, m_vResolution (coreVector2(0.0f,0.0f))
, m_iSize       (0)
{
    // generate base texture
    if(bGenerate) glGenTextures(1, &m_iID);
}

coreTexture::coreTexture(const char* pcPath)
: coreResource  ("")
, m_iID         (0)
, m_vResolution (coreVector2(0.0f,0.0f))
, m_iSize       (0)
{
    this->Load(pcPath);
}

coreTexture::coreTexture(coreFile* pFile)
: coreResource  ("")
, m_iID         (0)
, m_vResolution (coreVector2(0.0f,0.0f))
, m_iSize       (0)
{
    this->Load(pFile);
}


// ****************************************************************
// destructor
coreTexture::~coreTexture()
{
    this->Unload();
}


// ****************************************************************
// load resource data
void coreTexture::Load(const char* pcPath)
{
    coreFile pFile(pcPath);
    this->Load(&pFile);
}

void coreTexture::Load(coreFile* pFile)
{
    assert(m_iID == 0);

    //// Datei laden
    //SDL_RWops* pWops   = SDL_RWFromMem(pData, iSize);
    //SDL_Surface* pData = IMG_LoadTyped_RW(pWops, FALSE, strrchr(m_sPath.c_str(), '.')+1);
    //SDL_FreeRW(pWops);
    //
    //if(!pData)
    //{
    //    m_Texture = NULL;
    //    if(pFile->pcName) Core::Log->Error(0, "Texture (%s) could not be loaded!", pFile->pcName);
    //    return FALSE;
    //}
    //if(pFile->pArchive) sprintf(m_pcPath, "%s/%s", pFile->pArchive->GetName(), pFile->pcName);
    //               else sprintf(m_pcPath, pFile->pcName);

    //// Format konvertieren
    //SDL_Surface* pConvert = SDL_CreateRGBSurface(SDL_SWSURFACE, pData->w, pData->h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    //SDL_BlitSurface(pData, 0, pConvert, 0);

    //// Texturobjekt generieren
    //glGenTextures(1, &m_Texture);
    //glBindTexture(GL_TEXTURE_2D, m_Texture);
    //
    //// MipMapping prüfen
    //const BOOL bMipMap = Core::Graphic->GetOpenGL3Support();

    //// Texturparameter setzen
    //m_bClamp = bClamp;
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bMipMap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_bClamp ? GL_CLAMP : GL_REPEAT);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_bClamp ? GL_CLAMP : GL_REPEAT);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)Core::Config->Get()->Graphic.TextureFilter);

    //// Textur laden
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pConvert->w, pConvert->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pConvert->pixels);
    //if(bMipMap) glGenerateMipmap(GL_TEXTURE_2D);
    //else glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

    //// Daten freigeben
    //SDL_FreeSurface(pData);
    //SDL_FreeSurface(pConvert);
    //glBindTexture(GL_TEXTURE_2D, NULL);
}


// ****************************************************************
// unload resource data
void coreTexture::Unload()
{
    // delete texture
    if(m_iID) glDeleteTextures(1, &m_iID);

    // reset attributes
    m_sPath       = "";
    m_vResolution = coreVector2(0.0f,0.0f);
    m_iSize       = 0;
}


// ****************************************************************
// enable texture
void coreTexture::Enable(const coreByte& iUnit)
{
    assert(s_apActive[iUnit] == NULL);
    assert(m_iID != 0);

    // bind texture to texture unit
    if(s_iCurUnit != iUnit)
    {
        glActiveTexture(GL_TEXTURE0+iUnit);
        s_iCurUnit = iUnit;
    }
    glBindTexture(GL_TEXTURE_2D, m_iID);

    // save active texture
    s_apActive[iUnit] = this;
}


// ****************************************************************
// disable texture
void coreTexture::Disable(const coreByte& iUnit)
{
    assert(s_apActive[iUnit] != NULL);

    // unbind texture from texture unit
    if(s_iCurUnit != iUnit)
    {
        glActiveTexture(GL_TEXTURE0+iUnit);
        s_iCurUnit = iUnit;
    }
    glBindTexture(GL_TEXTURE_2D, NULL);

    // reset active texture
    s_apActive[iUnit] = NULL;
}