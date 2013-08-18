#include "Core.h"

int          coreTexture::s_iActiveUnit                     = 0;
coreTexture* coreTexture::s_apBound[CORE_TEXTURE_UNITS]; // = NULL;
SDL_SpinLock coreTexture::s_iLock                           = 0;


// ****************************************************************
// constructor
coreTexture::coreTexture(const bool bGenerate)
: m_iID         (0)
, m_vResolution (coreVector2(0.0f,0.0f))
, m_iSize       (0)
, m_iUnit       (-1)
, m_pSync       (NULL)
{
    // generate base texture
    if(bGenerate) glGenTextures(1, &m_iID);
}

coreTexture::coreTexture(const char* pcPath)
: m_iID         (0)
, m_vResolution (coreVector2(0.0f,0.0f))
, m_iSize       (0)
, m_iUnit       (-1)
, m_pSync       (NULL)
{
    // load from path
    this->coreResource::Load(pcPath);
}

coreTexture::coreTexture(coreFile* pFile)
: m_iID         (0)
, m_vResolution (coreVector2(0.0f,0.0f))
, m_iSize       (0)
, m_iUnit       (-1)
, m_pSync       (NULL)
{
    // load from file
    this->Load(pFile);
}


// ****************************************************************
// destructor
coreTexture::~coreTexture()
{
    this->Unload();
}


// ****************************************************************
// load texture resource data
int coreTexture::Load(coreFile* pFile)
{
    // check sync object status
    const int iStatus = this->CheckSync();
    if(iStatus != CORE_INVALID_CALL) return iStatus;

    SDL_assert(m_iID == 0);
    SDL_assert(pFile != NULL);

    // decompress file data
    SDL_Surface* pData = IMG_LoadTyped_RW(SDL_RWFromConstMem(pFile->GetData(), pFile->GetSize()), true, strrchr(pFile->GetPath(), '.')+1);
    if(!pData)
    {
        Core::Log->Error(0, coreUtils::Print("Texture (%s) could not be loaded", pFile->GetPath()));
        return CORE_FILE_ERROR;
    }

    const coreUint iDataSize = pData->w * pData->h * 4;
    const bool bMipMap       = Core::Graphic->SupportFeature("GL_ARB_framebuffer_object");

    // save texture attributes
    m_sPath       = pFile->GetPath();
    m_vResolution = coreVector2(float(pData->w), float(pData->h));
    m_iSize       = pData->w * pData->h * (bMipMap ? 4 : 3);
    
    // convert data format
    SDL_Surface* pConvert = SDL_CreateRGBSurface(0, pData->w, pData->h, 32, CORE_TEXTURE_MASK);
    SDL_BlitSurface(pData, NULL, pConvert, NULL);

    // generate pixel buffer object for asynchronous texture loading
    GLuint iBuffer;
    glGenBuffers(1, &iBuffer);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, iBuffer);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, iDataSize, 0, GL_STREAM_DRAW);

    // copy texture data into PBO
    GLubyte* ptr = (GLubyte*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, iDataSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    memcpy(ptr, pConvert->pixels, iDataSize);
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

    // generate texture
    glGenTextures(1, &m_iID);

    SDL_AtomicLock(&s_iLock);
    {
        // bind texture to free texture unit
        int iLoadUnit = -1;
        for(int i = 0; i < CORE_TEXTURE_UNITS; ++i)
        {
            if(!s_apBound[i])
            {
                if(i != s_iActiveUnit)
                {
                    // switch active texture unit
                    iLoadUnit = i;
                    glActiveTexture(GL_TEXTURE0+iLoadUnit);
                }
                break;
            }
        }
        glBindTexture(GL_TEXTURE_2D, m_iID);

        // set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,         bMipMap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,         GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,             GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,             GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)Core::Config->GetInt(CORE_CONFIG_GRAPHIC_TEXTUREFILTER, 0));

        // load texture data from PBO
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, pConvert->w, pConvert->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        if(bMipMap) glGenerateMipmap(GL_TEXTURE_2D);

        // unbind texture from texture unit
        glBindTexture(GL_TEXTURE_2D, 0);
        if(iLoadUnit >= 0) glActiveTexture(GL_TEXTURE0+s_iActiveUnit);
    }
    SDL_AtomicUnlock(&s_iLock);

    // generate sync object or flush all commands
    if(Core::Graphic->SupportFeature("GL_ARB_sync")) m_pSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, NULL);
    else glFlush();

    // delete PBO
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glDeleteBuffers(1, &iBuffer);
    
    // delete file data
    SDL_FreeSurface(pData);
    SDL_FreeSurface(pConvert);

    if(m_pSync) return CORE_BUSY;
    Core::Log->Info(coreUtils::Print("Texture (%s) loaded", m_sPath.c_str()));
    return CORE_OK;
}


// ****************************************************************
// unload texture resource data
int coreTexture::Unload()
{
    SDL_assert(m_iUnit < 0);

    if(!m_iID) return CORE_INVALID_CALL;

    // delete texture
    glDeleteTextures(1, &m_iID);
    Core::Log->Info(coreUtils::Print("Texture (%s) unloaded", m_sPath.c_str()));

    // delete sync object
    if(m_pSync)
    {
        glDeleteSync(m_pSync);
        m_pSync = NULL;
    }

    // reset attributes
    m_sPath       = "";
    m_iID         = 0;
    m_vResolution = coreVector2(0.0f,0.0f);
    m_iSize       = 0;

    return CORE_OK;
}


// ****************************************************************
// enable texture
void coreTexture::Enable(const coreByte& iUnit)
{
    SDL_assert(m_iUnit < 0);
    SDL_assert(s_apBound[iUnit] == NULL);

    // save texture binding
    m_iUnit            = iUnit;
    s_apBound[m_iUnit] = this;

    SDL_AtomicLock(&s_iLock);
    {
        // bind texture to texture unit
        if(s_iActiveUnit != m_iUnit)
        {
            glActiveTexture(GL_TEXTURE0+m_iUnit);
            s_iActiveUnit = m_iUnit;
        }
        glBindTexture(GL_TEXTURE_2D, m_iID);
    }
    SDL_AtomicUnlock(&s_iLock);
}


// ****************************************************************
// disable texture
void coreTexture::Disable()
{
    SDL_assert(m_iUnit >= 0);
    SDL_assert(s_apBound[m_iUnit] != NULL);

    SDL_AtomicLock(&s_iLock);
    {
        // unbind texture from texture unit
        if(s_iActiveUnit != m_iUnit)
        {
            glActiveTexture(GL_TEXTURE0+m_iUnit);
            s_iActiveUnit = m_iUnit;
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    SDL_AtomicUnlock(&s_iLock);

    // reset texture binding
    s_apBound[m_iUnit] = NULL;
    m_iUnit            = -1;
}


// ****************************************************************
// disable all textures
void coreTexture::DisableAll()
{
    // traverse all texture units
    for(int i = 0; i < CORE_TEXTURE_UNITS; ++i)
    {
        if(s_apBound[i])
            s_apBound[i]->Disable();
    }
}


// ****************************************************************
// check sync object status
int coreTexture::CheckSync()
{
    if(!m_iID || !m_pSync) return CORE_INVALID_CALL;

    // check for finished texture loading
    if(glClientWaitSync(m_pSync, NULL, 0) != GL_TIMEOUT_EXPIRED)
    {
        // delete sync object
        glDeleteSync(m_pSync);
        m_pSync = NULL;

        Core::Log->Info(coreUtils::Print("Texture (%s) loaded asynchronous", m_sPath.c_str()));
        return CORE_OK;
    }

    return CORE_BUSY;
}