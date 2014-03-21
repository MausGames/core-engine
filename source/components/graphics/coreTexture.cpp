//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

int          coreTexture::s_iActiveUnit                     = 0;
coreTexture* coreTexture::s_apBound[CORE_TEXTURE_UNITS]; // = NULL;
SDL_SpinLock coreTexture::s_iLock                           = 0;


// ****************************************************************
// constructor
coreTexture::coreTexture()noexcept
: m_iTexture    (0)
, m_vResolution (coreVector2(0.0f,0.0f))
{
}


// ****************************************************************
// destructor
coreTexture::~coreTexture()
{
    this->Unload();
}


// ****************************************************************
// load texture resource data
// TODO: check proper use of PBO, maybe implement static buffer(s!)
// TODO: allow 1-channel textures (GLES uses GL_ALPHA/GL_LUMINANCE, GL uses GL_RED/GL_DEPTH_COMPONENT)
// TODO: check performance of 24bit formats, mind texture alignment of 4 (also for frame buffers and labels)
coreError coreTexture::Load(coreFile* pFile)
{
    // check for sync object status
    const coreError iStatus = m_Sync.Check(0);
    if(iStatus >= 0) return iStatus;

    coreFileUnload Unload(pFile);

    ASSERT_IF(m_iTexture) return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetData()) return CORE_FILE_ERROR;

    // decompress file data
    SDL_Surface* pData = IMG_LoadTyped_RW(SDL_RWFromConstMem(pFile->GetData(), pFile->GetSize()), true, coreData::StrExtension(pFile->GetPath()));
    if(!pData)
    {
        Core::Log->Error(false, "Texture (%s) could not be loaded", pFile->GetPath());
        return CORE_INVALID_DATA;
    }

    // check for OpenGL extensions
    const GLboolean& bPixelBuffer = GLEW_ARB_pixel_buffer_object;
    const GLboolean& bAnisotropic = GLEW_EXT_texture_filter_anisotropic;
    const GLboolean& bStorage     = GLEW_ARB_texture_storage;
    const GLboolean& bMipMap      = GLEW_ARB_framebuffer_object;

    // calculate data size and texture format
    const coreUint iDataSize = pData->w * pData->h * pData->format->BytesPerPixel;
    const GLenum iInternal   = (pData->format->BytesPerPixel == 4) ? GL_RGBA8 : GL_RGB8;
    const GLenum iFormat     = (pData->format->BytesPerPixel == 4) ? GL_RGBA  : GL_RGB;

    // save properties
    m_vResolution = coreVector2(float(pData->w), float(pData->h));
    m_sPath       = pFile->GetPath();
    m_iSize       = (iDataSize * 4) / (bMipMap ? 3 : 4);

    // convert the data format
    SDL_Surface* pConvert = SDL_CreateRGBSurface(0, pData->w, pData->h, pData->format->BitsPerPixel, CORE_TEXTURE_MASK);
    SDL_BlitSurface(pData, NULL, pConvert, NULL);

    coreDataBuffer iBuffer;
    if(bPixelBuffer)
    {
        // generate pixel buffer object for asynchronous texture loading
        iBuffer.Create(GL_PIXEL_UNPACK_BUFFER, iDataSize, NULL, GL_STREAM_DRAW);

        // copy texture data into PBO
        coreByte* pRange = iBuffer.Map(0, iDataSize);
        std::memcpy(pRange, pConvert->pixels, iDataSize);
        iBuffer.Unmap(pRange);
    }

    coreTexture::Lock();
    {
        // generate texture
        glGenTextures(1, &m_iTexture);

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
        glBindTexture(GL_TEXTURE_2D, m_iTexture);

        // set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bMipMap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);
        if(bAnisotropic) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)Core::Config->GetInt(CORE_CONFIG_GRAPHICS_TEXTUREFILTER));

        // allocate texture memory
        const void* pPixels = bPixelBuffer ? 0 : pConvert->pixels;
        if(bStorage)
        {
            glTexStorage2D(GL_TEXTURE_2D, (int)std::log2(MAX(pConvert->w, pConvert->h)), iInternal, pConvert->w, pConvert->h);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pConvert->w, pConvert->h, iFormat, GL_UNSIGNED_BYTE, pPixels);
        }
        else glTexImage2D(GL_TEXTURE_2D, 0, iInternal, pConvert->w, pConvert->h, 0, iFormat, GL_UNSIGNED_BYTE, pPixels);
        
        // generate mipmaps
        if(bMipMap) glGenerateMipmap(GL_TEXTURE_2D);

        // unbind texture from texture unit
        glBindTexture(GL_TEXTURE_2D, 0);
        if(iLoadUnit >= 0) glActiveTexture(GL_TEXTURE0+s_iActiveUnit);
    }
    coreTexture::Unlock();

    // create sync object
    const bool bSync = m_Sync.Create();

    // delete PBO
    if(bPixelBuffer) iBuffer.Delete();

    // delete file data
    SDL_FreeSurface(pData);
    SDL_FreeSurface(pConvert);

    Core::Log->Info("Texture (%s) loaded", pFile->GetPath());
    return bSync ? CORE_BUSY : CORE_OK;
}


// ****************************************************************
// unload texture resource data
coreError coreTexture::Unload()
{
    if(!m_iTexture) return CORE_INVALID_CALL;

    // disable still active texture bindings
    for(int i = CORE_TEXTURE_UNITS-1; i >= 0; --i)
        if(s_apBound[i] == this) coreTexture::Disable(i);

    // delete texture
    glDeleteTextures(1, &m_iTexture);
    if(!m_sPath.empty()) Core::Log->Info("Texture (%s) unloaded", m_sPath.c_str());

    // delete sync object
    m_Sync.Delete();

    // reset properties
    m_sPath       = "";
    m_iSize       = 0;
    m_iTexture    = 0;
    m_vResolution = coreVector2(0.0f,0.0f);

    return CORE_OK;
}


// ****************************************************************
// bind texture to texture unit
void coreTexture::__BindTexture(const coreByte& iUnit, coreTexture* pTexture)
{
    SDL_assert(iUnit < CORE_TEXTURE_UNITS);

    // check texture binding
    if(s_apBound[iUnit] == pTexture) return;
    
    coreTexture::Lock();
    {
        // activate texture unit
        if(s_iActiveUnit != iUnit)
        {
            s_iActiveUnit = iUnit;
            glActiveTexture(GL_TEXTURE0+iUnit);
        }

        // bind texture to texture unit
        s_apBound[iUnit] = pTexture;
        glBindTexture(GL_TEXTURE_2D, pTexture ? pTexture->GetTexture() : 0);
    }
    coreTexture::Unlock();
}