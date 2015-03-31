//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"
#include <stb_dxt.h>

coreUintW    coreTexture::s_iActiveUnit                     = 0u;
coreTexture* coreTexture::s_apBound[CORE_TEXTURE_UNITS]; // = NULL;


// ****************************************************************
// constructor
coreTexture::coreTexture()noexcept
: coreTexture (true)
{
}

coreTexture::coreTexture(const bool& bLoadCompressed)noexcept
: m_iTexture    (0u)
, m_vResolution (coreVector2(0.0f,0.0f))
, m_iLevels     (0u)
, m_iCompressed (bLoadCompressed ? 0 : -1)
, m_iInternal   (0u)
, m_iFormat     (0u)
, m_iType       (0u)
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
coreStatus coreTexture::Load(coreFile* pFile)
{
    // check for sync object status
    const coreStatus iCheck = m_Sync.Check(0u, CORE_SYNC_CHECK_FLUSHED);
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
    const coreUint32 iDataSize =  pData->w * pData->h * pData->format->BytesPerPixel;
    const GLenum     iInternal = (pData->format->BytesPerPixel == 4u) ? GL_RGBA8 : GL_RGB8;
    const GLenum     iFormat   = (pData->format->BytesPerPixel == 4u) ? GL_RGBA  : GL_RGB;

    // check for compression capability
    const coreTextureMode bCompress = (coreMath::IsPOT(pData->w) && coreMath::IsPOT(pData->h) && !m_iCompressed) ? CORE_TEXTURE_MODE_COMPRESS : CORE_TEXTURE_MODE_DEFAULT;

    // create texture
    this->Create(pData->w, pData->h, iInternal, iFormat, GL_UNSIGNED_BYTE, bCompress | CORE_TEXTURE_MODE_FILTER | CORE_TEXTURE_MODE_REPEAT);
    this->Modify(0u, 0u, pData->w, pData->h, iDataSize, s_cast<coreByte*>(pData->pixels));

    // save properties
    m_sPath = pFile->GetPath();

    // delete pixel data
    SDL_FreeSurface(pData);

    Core::Log->Info("Texture (%s:%u) loaded", pFile->GetPath(), m_iTexture);
    return m_Sync.Create() ? CORE_BUSY : CORE_OK;
}


// ****************************************************************
// unload texture resource data
coreStatus coreTexture::Unload()
{
    if(!m_iTexture) return CORE_INVALID_CALL;

    // disable still active texture bindings
    for(coreUintW i = CORE_TEXTURE_UNITS; i--; )
        if(s_apBound[i] == this) coreTexture::Disable(i);

    // delete texture
    glDeleteTextures(1, &m_iTexture);
    if(!m_sPath.empty()) Core::Log->Info("Texture (%s) unloaded", m_sPath.c_str());

    // delete sync object
    m_Sync.Delete();

    // reset properties
    m_sPath       = "";
    m_iTexture    = 0u;
    m_vResolution = coreVector2(0.0f,0.0f);
    m_iLevels     = 0u;
    m_iCompressed = MIN(m_iCompressed, 0);
    m_iInternal   = 0u;
    m_iFormat     = 0u;
    m_iType       = 0u;

    return CORE_OK;
}


// ****************************************************************
// create texture memory
void coreTexture::Create(const coreUint32& iWidth, const coreUint32& iHeight, GLenum iInternal, GLenum iFormat, const GLenum& iType, const coreTextureMode& iTextureMode)
{
    WARN_IF(m_iTexture) this->Unload();

    // check for OpenGL extensions
    const coreBool& bStorage     = CORE_GL_SUPPORT(ARB_texture_storage);
    const coreBool  bAnisotropic = CORE_GL_SUPPORT(EXT_texture_filter_anisotropic) && CONTAINS_VALUE(iTextureMode, CORE_TEXTURE_MODE_FILTER);
    const coreBool  bMipMap      = CORE_GL_SUPPORT(EXT_framebuffer_object)         && CONTAINS_VALUE(iTextureMode, CORE_TEXTURE_MODE_FILTER);
    const coreBool  bCompress    = CORE_GL_SUPPORT(EXT_texture_compression_s3tc)   && CONTAINS_VALUE(iTextureMode, CORE_TEXTURE_MODE_COMPRESS) && Core::Config->GetBool(CORE_CONFIG_GRAPHICS_TEXTURECOMPRESSION);

    // set wrap mode
    const GLenum iWrapMode = CONTAINS_VALUE(iTextureMode, CORE_TEXTURE_MODE_REPEAT) ? GL_REPEAT : GL_CLAMP_TO_EDGE;

    // set compression
    if(bCompress)
    {
        WARN_IF(!coreMath::IsPOT(iWidth) || !coreMath::IsPOT(iHeight)) {}
        else
        {
            // overwrite with appropriate compressed texture format (S3TC)
            switch(iFormat)
            {
            default: ASSERT(false)
            case GL_RGB:  iInternal = iFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; m_iCompressed = 1; break;
            case GL_RGBA: iInternal = iFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; m_iCompressed = 1; break;
            }
        }
    }

    // save properties
    m_vResolution = coreVector2(I_TO_F(iWidth), I_TO_F(iHeight));
    m_iLevels     = bMipMap ? F_TO_UI(coreMath::Log<2u>(m_vResolution.Max())) + 1u : 1u;
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
        glTexImage2D(GL_TEXTURE_2D, 0, DEFINED(_CORE_GLES_) ? m_iFormat : m_iInternal, iWidth, iHeight, 0, m_iFormat, m_iType, NULL);
        if(bMipMap) glGenerateMipmap(GL_TEXTURE_2D);
    }
}


// ****************************************************************
// modify texture memory
void coreTexture::Modify(const coreUint32& iOffsetX, const coreUint32& iOffsetY, const coreUint32& iWidth, const coreUint32& iHeight, const coreUint32& iDataSize, const coreByte* pData)
{
    WARN_IF(!m_iTexture) return;
    ASSERT(((iOffsetX + iWidth) <= F_TO_UI(m_vResolution.x)) && ((iOffsetY + iHeight) <= F_TO_UI(m_vResolution.y)))

    // check for OpenGL extensions
    const coreBool& bDirectState = CORE_GL_SUPPORT(ARB_direct_state_access);
    const coreBool  bPixelBuffer = CORE_GL_SUPPORT(ARB_pixel_buffer_object) && iDataSize && pData;
    const coreBool  bMipMap      = CORE_GL_SUPPORT(EXT_framebuffer_object)  && (m_iLevels > 1u);

    if(m_iCompressed > 0)
    {
        ASSERT((iWidth == F_TO_UI(m_vResolution.x)) && (iHeight == F_TO_UI(m_vResolution.y)))

        // bind texture (simple)
        glBindTexture(GL_TEXTURE_2D, m_iTexture);

        // calculate components and compressed size
        const coreUint32 iComponents = iDataSize / (iWidth * iHeight);
        const coreUint32 iPackedSize = CORE_TEXTURE_COMPRESSED_SIZE(iDataSize, iComponents);

        // allocate required image memory
        coreByte* pPackedData  = new coreByte[iPackedSize];
        coreByte* apMipData[2] = {NULL, NULL};
        if(bMipMap)
        {
            apMipData[0] = new coreByte[iDataSize >> 2u];
            apMipData[1] = new coreByte[iDataSize >> 4u];
        }

        // process all available texture levels
        for(coreUintW i = 0u; i < m_iLevels; ++i)
        {
            const coreUint32 iCurWidth  = iWidth  >> i;
            const coreUint32 iCurHeight = iHeight >> i;
            const coreUint32 iCurSize   = iPackedSize >> i >> i;

            // create compressed image
            coreTexture::CreateCompressed(iCurWidth, iCurHeight, iComponents, pData, pPackedData);

            // upload image to texture
            coreDataBuffer iBuffer;
            if(bPixelBuffer) iBuffer.Create(GL_PIXEL_UNPACK_BUFFER, iCurSize, pPackedData, CORE_DATABUFFER_STORAGE_STREAM);
            glCompressedTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, iCurWidth, iCurHeight, m_iFormat, iCurSize, bPixelBuffer ? NULL : pPackedData);

            // check for still valid resolution
            if((iCurWidth <= 4u) || (iCurHeight <= 4u) || !bMipMap) break;

            // create next level-of-detail image
            coreTexture::CreateNextLevel(iCurWidth, iCurHeight, iComponents, pData, apMipData[i % 2u]);
            pData = apMipData[i % 2u];
        }

        // free required image memory
        SAFE_DELETE_ARRAY(pPackedData)
        SAFE_DELETE_ARRAY(apMipData[0])
        SAFE_DELETE_ARRAY(apMipData[1])
    }
    else
    {
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
}


// ****************************************************************
// copy content from current read frame buffer
void coreTexture::CopyFrameBuffer(const coreUint32& iSrcX, const coreUint32& iSrcY, const coreUint32& iDstX, const coreUint32& iDstY, const coreUint32& iWidth, const coreUint32& iHeight)
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
void coreTexture::ShadowSampling(const coreBool& bStatus)
{
    ASSERT(m_iTexture && (m_iFormat == GL_DEPTH_COMPONENT || m_iFormat == GL_DEPTH_STENCIL))

    // bind texture
    this->Enable(0u);

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
void coreTexture::Clear(const coreUint8& iLevel)
{
    ASSERT(m_iTexture && (iLevel < m_iLevels))

    if(CORE_GL_SUPPORT(ARB_clear_texture))
    {
        // clear content directly
        glClearTexImage(m_iTexture, iLevel, m_iFormat, m_iType, NULL);
    }
    else
    {
        // bind and clear content (with fallback method)
        this->Enable(0u);
        glTexSubImage2D(GL_TEXTURE_2D, iLevel, 0, 0, F_TO_UI(m_vResolution.x), F_TO_UI(m_vResolution.y), m_iFormat, m_iType, NULL);
    }
}


// ****************************************************************
// invalidate content of the texture
void coreTexture::Invalidate(const coreUint8& iLevel)
{
    ASSERT(m_iTexture && (iLevel < m_iLevels))

    // invalidate the whole texture
    if(CORE_GL_SUPPORT(ARB_invalidate_subdata))
        glInvalidateTexImage(m_iTexture, iLevel);
}


// ****************************************************************
// create next level-of-detail image
void coreTexture::CreateNextLevel(const coreUintW& iInWidth, const coreUintW& iInHeight, const coreUintW& iComponents, const coreByte* pInput, coreByte* OUTPUT pOutput)
{
    ASSERT(coreMath::IsPOT(iInWidth)  && (iInWidth  >= 2u) &&
           coreMath::IsPOT(iInHeight) && (iInHeight >= 2u) &&
           iComponents && pInput && pOutput)

    // save output texture size
    const coreUintW iOutWidth  = iInWidth  >> 1u;
    const coreUintW iOutHeight = iInHeight >> 1u;

    // save input memory offsets
    const coreUintW iInOffset1 = iComponents;
    const coreUintW iInOffset2 = iComponents * iInWidth;
    const coreUintW iInOffset3 = iInOffset1 + iInOffset2;

    // loop through all output texels
    for(coreUintW y = 0u; y < iOutHeight; ++y)
    {
        for(coreUintW x = 0u; x < iOutWidth; ++x)
        {
            // calculate base memory positions
            const coreUintW iInBase  = (x + y*iInWidth)  * iComponents * 2u;
            const coreUintW iOutBase = (x + y*iOutWidth) * iComponents;

            for(coreUintW i = 0u; i < iComponents; ++i)
            {
                // gather all required input values
                const coreUint16 A = pInput[iInBase + i];
                const coreUint16 B = pInput[iInBase + i + iInOffset1];
                const coreUint16 C = pInput[iInBase + i + iInOffset2];
                const coreUint16 D = pInput[iInBase + i + iInOffset3];

                // calculate average output value (bilinear, rounding-error)
                pOutput[iOutBase + i] = ((A + B + C + D) >> 2u) & 0xFFu;
            }
        }
    }
}


// ****************************************************************
// create compressed image
void coreTexture::CreateCompressed(const coreUintW& iInWidth, const coreUintW& iInHeight, const coreUintW& iComponents, const coreByte* pInput, coreByte* OUTPUT pOutput)
{
    ASSERT(coreMath::IsPOT(iInWidth)  && (iInWidth  >= 4u) &&
           coreMath::IsPOT(iInHeight) && (iInHeight >= 4u) &&
           (iComponents == 3u || iComponents == 4u) && pInput && pOutput)

    // save memory offsets and alpha status
    const coreUintW iInOffsetX = 4u  * iComponents;
    const coreUintW iInOffsetY = 3u  * iComponents * iInWidth;
    const coreUintW iOutOffset = 1u << iComponents;
    const coreInt32 iAlpha     = iComponents - 3;

    // loop through all input texels
    for(coreUintW y = 0u; y < iInHeight; y += 4u)
    {
        for(coreUintW x = 0u; x < iInWidth; x += 4u)
        {
            coreByte aBlock[64];

            // copy data into 4x4 RGBA block
            for(coreUintW i = 0u; i < 4u; ++i) std::memcpy(aBlock + (i)       * 4u, pInput + (i)               * iComponents, iComponents);
            for(coreUintW i = 0u; i < 4u; ++i) std::memcpy(aBlock + (i +  4u) * 4u, pInput + (i + 1u*iInWidth) * iComponents, iComponents);
            for(coreUintW i = 0u; i < 4u; ++i) std::memcpy(aBlock + (i +  8u) * 4u, pInput + (i + 2u*iInWidth) * iComponents, iComponents);
            for(coreUintW i = 0u; i < 4u; ++i) std::memcpy(aBlock + (i + 12u) * 4u, pInput + (i + 3u*iInWidth) * iComponents, iComponents);

            // compress block with S3TC DXT1 (RGB) or DXT5 (RGBA)
            stb_compress_dxt_block(pOutput, aBlock, iAlpha, STB_DXT_HIGHQUAL);

            // increment pointers
            pInput  += iInOffsetX;
            pOutput += iOutOffset;
        }

        // skip already processed texture lines
        pInput += iInOffsetY;
    }
}


// ****************************************************************
// bind texture to texture unit
void coreTexture::__BindTexture(const coreUintW& iUnit, coreTexture* pTexture)
{
    ASSERT(iUnit < CORE_TEXTURE_UNITS)

    // check texture binding
    if(s_apBound[iUnit] == pTexture) return;
    s_apBound[iUnit] = pTexture;

    if(CORE_GL_SUPPORT(ARB_direct_state_access))
    {
        // bind texture directly
        glBindTextureUnit(iUnit, pTexture ? pTexture->GetTexture() : 0u);
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
        glBindTexture(GL_TEXTURE_2D, pTexture ? pTexture->GetTexture() : 0u);
    }
}