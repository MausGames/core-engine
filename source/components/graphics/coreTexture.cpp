//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

coreByte     coreTexture::s_iActiveUnit                     = 0;
coreTexture* coreTexture::s_apBound[CORE_TEXTURE_UNITS]; // = NULL;


// ****************************************************************
// constructor
coreTexture::coreTexture()noexcept
: m_iTexture    (0)
, m_vResolution (coreVector2(0.0f,0.0f))
, m_iLevels     (0)
, m_iInternal   (0)
, m_iFormat     (0)
, m_iType       (0)
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
coreError coreTexture::Load(coreFile* pFile)
{
    // check for sync object status
    const coreError iCheck = m_Sync.Check(0, CORE_SYNC_CHECK_FLUSHED);
    if(iCheck >= CORE_OK) return iCheck;

    coreFileUnload Unload(pFile);

    WARN_IF(m_iTexture)   return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetData()) return CORE_ERROR_FILE;

    // decompress file to plain pixel data
    SDL_Surface* pData = IMG_LoadTyped_RW(SDL_RWFromConstMem(pFile->GetData(), pFile->GetSize()), true, coreData::StrExtension(pFile->GetPath()));
    if(!pData)
    {
        Core::Log->Warning("Texture (%s) could not be loaded", pFile->GetPath());
        return CORE_INVALID_DATA;
    }

    // calculate data size and texture format
    const coreUint iDataSize =  pData->w * pData->h * pData->format->BytesPerPixel;
    const GLenum   iInternal = (pData->format->BytesPerPixel == 4) ? GL_RGBA8 : GL_RGB8;
    const GLenum   iFormat   = (pData->format->BytesPerPixel == 4) ? GL_RGBA  : GL_RGB;

    // create new texture
    this->Create(pData->w, pData->h, iInternal, iFormat, GL_UNSIGNED_BYTE, GL_REPEAT, true);
    this->Modify(0, 0, pData->w, pData->h, iDataSize, pData->pixels);

    // save properties
    m_sPath = pFile->GetPath();
    m_iSize = (iDataSize * 4) / ((m_iLevels > 1) ? 3 : 4);

    // delete pixel data
    SDL_FreeSurface(pData);

    Core::Log->Info("Texture (%s:%u) loaded", pFile->GetPath(), m_iTexture);
    return m_Sync.Create() ? CORE_BUSY : CORE_OK;
}


// ****************************************************************
// unload texture resource data
coreError coreTexture::Unload()
{
    if(!m_iTexture) return CORE_INVALID_CALL;

    // disable still active texture bindings
    for(coreByte i = CORE_TEXTURE_UNITS; i--; )
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
    m_iInternal   = 0;
    m_iFormat     = 0;
    m_iType       = 0;

    return CORE_OK;
}


// ****************************************************************
// create texture memory
void coreTexture::Create(const coreUint& iWidth, const coreUint& iHeight, const GLenum& iInternal, const GLenum& iFormat, const GLenum& iType, const GLenum& iWrapMode, const bool& bFilter)
{
    WARN_IF(m_iTexture) this->Unload();

    // check for OpenGL extensions
    const bool& bStorage     = CORE_GL_SUPPORT(ARB_texture_storage);
    const bool  bAnisotropic = CORE_GL_SUPPORT(EXT_texture_filter_anisotropic) && bFilter;
    const bool  bMipMap      = CORE_GL_SUPPORT(EXT_framebuffer_object)         && bFilter;

    // save properties
    m_vResolution = coreVector2(I_TO_F(iWidth), I_TO_F(iHeight));
    m_iLevels     = bMipMap ? F_TO_SI(coreMath::Log<2>(m_vResolution.Max())) + 1 : 1;
    m_iInternal   = iInternal;
    m_iFormat     = iFormat;
    m_iType       = iType;

    // generate texture
    glGenTextures(1, &m_iTexture);
    glBindTexture(GL_TEXTURE_2D, m_iTexture);

    // set sampling parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bMipMap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     iWrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     iWrapMode);
    if(bAnisotropic) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, I_TO_F(Core::Config->GetInt(CORE_CONFIG_GRAPHICS_TEXTUREFILTER)));

    if(bStorage)
    {
        // allocate immutable texture memory
        glTexStorage2D(GL_TEXTURE_2D, m_iLevels, m_iInternal, iWidth, iHeight);
    }
    else
    {
        // allocate normal texture memory
        glTexImage2D(GL_TEXTURE_2D, 0, DEFINED(_CORE_GLES_) ? m_iFormat : m_iInternal, iWidth, iHeight, 0, m_iFormat, m_iType, 0);
        if(bMipMap) glGenerateMipmap(GL_TEXTURE_2D);
    }
}


// ****************************************************************
// modify texture memory
void coreTexture::Modify(const coreUint& iOffsetX, const coreUint& iOffsetY, const coreUint& iWidth, const coreUint& iHeight, const coreUint& iDataSize, const void* pData)
{
    WARN_IF(!m_iTexture) return;
    ASSERT(((iOffsetX + iWidth) <= F_TO_UI(m_vResolution.x)) && ((iOffsetY + iHeight) <= F_TO_UI(m_vResolution.y)))

    // check for OpenGL extensions
    const bool& bDirectState = CORE_GL_SUPPORT(ARB_direct_state_access);
    const bool  bPixelBuffer = CORE_GL_SUPPORT(ARB_pixel_buffer_object) && iDataSize && pData;
    const bool  bMipMap      = CORE_GL_SUPPORT(EXT_framebuffer_object)  && (m_iLevels > 1);

    coreDataBuffer iBuffer;
    if(bPixelBuffer)
    {
        // create pixel buffer object for asynchronous texture loading
        iBuffer.Create(GL_PIXEL_UNPACK_BUFFER, iDataSize, pData, CORE_DATABUFFER_STORAGE_STREAM);

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
        // bind (simple) and update texture data
        glBindTexture  (GL_TEXTURE_2D, m_iTexture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, iOffsetX, iOffsetY, iWidth, iHeight, m_iFormat, m_iType, pData);
        if(bMipMap) glGenerateMipmap(GL_TEXTURE_2D);
    }
}


// ****************************************************************
// copy content from current read frame buffer
void coreTexture::CopyFrameBuffer(const coreUint& iSrcX, const coreUint& iSrcY, const coreUint& iDstX, const coreUint& iDstY, const coreUint& iWidth, const coreUint& iHeight)
{
    ASSERT(m_iTexture)
    ASSERT(((iSrcX + iWidth) <= F_TO_UI(Core::Graphics->GetViewResolution().x)) && ((iSrcY + iHeight) <= F_TO_UI(Core::Graphics->GetViewResolution().y)) &&
           ((iDstX + iWidth) <= F_TO_UI(m_vResolution                      .x)) && ((iDstY + iHeight) <= F_TO_UI(m_vResolution                      .y)))

    if(CORE_GL_SUPPORT(ARB_direct_state_access))
    {
        // copy frame buffer directly
        glCopyTextureSubImage2D(m_iTexture, 0, iDstX, iDstY, iSrcX, iSrcY, iWidth, iHeight);
    }
    else
    {
        // bind and copy frame buffer
        this->Enable(0);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, iDstX, iDstY, iSrcX, iSrcY, iWidth, iHeight);
    }
}


// ****************************************************************
// configure shadow sampling
void coreTexture::ShadowSampling(const bool& bStatus)
{
    ASSERT(m_iFormat == GL_DEPTH_COMPONENT)

    // bind texture
    this->Enable(0);

    if(bStatus)
    {
        // enable depth value comparison (with sampler2DShadow)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
    }
    else glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
}


// ****************************************************************
// clear content of the texture
void coreTexture::Clear(const GLint& iLevel)
{
    ASSERT(m_iTexture)

    if(CORE_GL_SUPPORT(ARB_clear_texture))
    {
        // clear content directly
        glClearTexImage(m_iTexture, iLevel, m_iFormat, m_iType, NULL);
    }
    else
    {
        // bind and clear content (with fallback method)
        this->Enable(0);
        glTexSubImage2D(GL_TEXTURE_2D, iLevel, 0, 0, F_TO_UI(m_vResolution.x), F_TO_UI(m_vResolution.y), m_iFormat, m_iType, NULL);
    }
}


// ****************************************************************
// invalidate content of the texture
void coreTexture::Invalidate(const GLint& iLevel)
{
    ASSERT(m_iTexture)

    // invalidate the whole texture
    if(CORE_GL_SUPPORT(ARB_invalidate_subdata))
        glInvalidateTexImage(m_iTexture, iLevel);
}


// ****************************************************************
// bind texture to texture unit
void coreTexture::__BindTexture(const coreByte& iUnit, coreTexture* pTexture)
{
    ASSERT(iUnit < CORE_TEXTURE_UNITS)

    // check texture binding
    if(s_apBound[iUnit] == pTexture) return;
    s_apBound[iUnit] = pTexture;

    if(CORE_GL_SUPPORT(ARB_direct_state_access))
    {
        // bind texture directly
        glBindTextureUnit(iUnit, pTexture ? pTexture->GetTexture() : 0);
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