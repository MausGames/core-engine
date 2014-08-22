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


// ****************************************************************
// constructor
coreTexture::coreTexture()noexcept
: m_iTexture    (0)
, m_vResolution (coreVector2(0.0f,0.0f))
, m_iLevels     (0)
, m_iFormat     (GL_RGBA)
, m_iType       (GL_UNSIGNED_BYTE)
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
// TODO: allow 1-channel textures (GLES uses GL_ALPHA/GL_LUMINANCE, GL uses GL_RED/GL_DEPTH_COMPONENT ?)
// TODO: check performance of 24bit formats, mind texture alignment of 4 (also for frame buffers and labels)
// TODO: implement texture-compressions
coreError coreTexture::Load(coreFile* pFile)
{
    // check for sync object status
    const coreError iCheck = m_Sync.Check(0);
    if(iCheck >= CORE_OK) return iCheck;

    coreFileUnload Unload(pFile);

    ASSERT_IF(m_iTexture) return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetData()) return CORE_ERROR_FILE;

    // decompress file data
    SDL_Surface* pData = IMG_LoadTyped_RW(SDL_RWFromConstMem(pFile->GetData(), pFile->GetSize()), true, coreData::StrExtension(pFile->GetPath()));
    if(!pData)
    {
        Core::Log->Warning("Texture (%s) could not be loaded", pFile->GetPath());
        return CORE_INVALID_DATA;
    }

    // convert file data
    SDL_Surface* pConvert = SDL_CreateRGBSurface(0, pData->w, pData->h, pData->format->BitsPerPixel, CORE_TEXTURE_MASK);
    SDL_SetSurfaceBlendMode(pData, SDL_BLENDMODE_NONE);
    SDL_BlitSurface(pData, NULL, pConvert, NULL);

    // calculate data size and texture format
    const coreUint iDataSize =  pData->w * pData->h * pData->format->BytesPerPixel;
    const GLenum   iInternal = (pData->format->BytesPerPixel == 4) ? GL_RGBA8 : GL_RGB8;
    const GLenum   iFormat   = (pData->format->BytesPerPixel == 4) ? GL_RGBA  : GL_RGB;

    // create new texture
    this->Create(pConvert->w, pConvert->h, iInternal, iFormat, GL_UNSIGNED_BYTE, GL_REPEAT, true);
    this->Modify(0, 0, pConvert->w, pConvert->h, iDataSize, pConvert->pixels);

    // save properties
    m_sPath = pFile->GetPath();
    m_iSize = (iDataSize * 4) / (GLEW_ARB_framebuffer_object ? 3 : 4);

    // delete file data
    SDL_FreeSurface(pData);
    SDL_FreeSurface(pConvert);

    Core::Log->Info("Texture (%s:%u) loaded", pFile->GetPath(), m_iTexture);
    return m_Sync.Create() ? CORE_BUSY : CORE_OK;
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
    m_iLevels     = 0;
    m_iFormat     = GL_RGBA;
    m_iType       = GL_UNSIGNED_BYTE;

    return CORE_OK;
}


// ****************************************************************
// create texture memory
void coreTexture::Create(const coreUint& iWidth, const coreUint& iHeight, const GLenum& iInternal, const GLenum& iFormat, const GLenum& iType, const GLenum& iWrapMode, const bool& bFilter)
{
    ASSERT_IF(m_iTexture) this->Unload();

    // check for OpenGL extensions
    const GLboolean& bStorage     = GLEW_ARB_texture_storage;
    const GLboolean  bAnisotropic = GLEW_EXT_texture_filter_anisotropic && bFilter;
    const GLboolean  bMipMap      = GLEW_ARB_framebuffer_object         && bFilter;

    // generate texture
    glGenTextures(1, &m_iTexture);
    glBindTexture(GL_TEXTURE_2D, m_iTexture);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bMipMap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     iWrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     iWrapMode);
    if(bAnisotropic) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, (float)Core::Config->GetInt(CORE_CONFIG_GRAPHICS_TEXTUREFILTER));

    // set texture properties
    m_vResolution = coreVector2(float(iWidth), float(iHeight));
    m_iLevels     = bMipMap ? coreByte(CEIL(coreMath::Log<2>(float(MAX(iWidth, iHeight))))) : 1;
    m_iFormat     = iFormat;
    m_iType       = iType;

    if(bStorage)
    {
        // allocate immutable texture memory
        glTexStorage2D(GL_TEXTURE_2D, m_iLevels, iInternal, iWidth, iHeight);
    }
    else
    {
        // allocate default texture memory
        glTexImage2D(GL_TEXTURE_2D, 0, iInternal, iWidth, iHeight, 0, iFormat, iType, 0);
        if(bMipMap) glGenerateMipmap(GL_TEXTURE_2D);
    }
}


// ****************************************************************
// modify texture memory
void coreTexture::Modify(const coreUint& iOffsetX, const coreUint& iOffsetY, const coreUint& iWidth, const coreUint& iHeight, const coreUint& iDataSize, const void* pData)
{
    ASSERT_IF(!m_iTexture) return;
    ASSERT(((iOffsetX + iWidth) <= coreUint(m_vResolution.x)) && ((iOffsetY + iHeight) <= coreUint(m_vResolution.y)))

    // check for OpenGL extensions
    const GLboolean& bDirectState = GLEW_ARB_direct_state_access;
    const GLboolean  bPixelBuffer = GLEW_ARB_pixel_buffer_object && iDataSize && pData;
    const GLboolean  bMipMap      = GLEW_ARB_framebuffer_object  && (m_iLevels > 1);

    coreDataBuffer iBuffer;
    if(bPixelBuffer)
    {
        // generate pixel buffer object for asynchronous texture loading
        iBuffer.Create(GL_PIXEL_UNPACK_BUFFER, iDataSize, pData, CORE_DATABUFFER_STORAGE_DYNAMIC);

        // use PBO instead of client memory
        pData = NULL;
    }

    if(bDirectState)
    {
        // update texture data directly
        glTextureSubImage2D(m_iTexture, 0, iOffsetX, iOffsetY, iWidth, iHeight, m_iFormat, m_iType, pData);
        if(bMipMap) glGenerateTextureMipmap(m_iTexture);
    }
    else
    {
        // bind and update texture data
        this->Enable(0);
        glTexSubImage2D(GL_TEXTURE_2D, 0, iOffsetX, iOffsetY, iWidth, iHeight, m_iFormat, m_iType, pData);
        if(bMipMap) glGenerateMipmap(GL_TEXTURE_2D);
    }
}


// ****************************************************************
// bind texture to texture unit
void coreTexture::__BindTexture(const coreByte& iUnit, coreTexture* pTexture)
{
    ASSERT(iUnit < CORE_TEXTURE_UNITS)

    // check texture binding
    if(s_apBound[iUnit] == pTexture) return;
    s_apBound[iUnit] = pTexture;
    
    if(GLEW_ARB_direct_state_access)
    {
        // bind texture directly
        glBindTextureUnit(GL_TEXTURE0 + iUnit, pTexture ? pTexture->GetTexture() : 0);
    }
    else
    {
        if(s_iActiveUnit != iUnit)
        {
            // activate texture unit
            s_iActiveUnit = iUnit;
            glActiveTexture(GL_TEXTURE0 + iUnit);
        }

        // bind texture to current unit
        glBindTexture(GL_TEXTURE_2D, pTexture ? pTexture->GetTexture() : 0);
    }
}