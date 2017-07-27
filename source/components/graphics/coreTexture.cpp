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

coreUintW    coreTexture::s_iActiveUnit                 = 0u;
coreTexture* coreTexture::s_apBound[CORE_TEXTURE_UNITS] = {};


// ****************************************************************
// constructor
coreTexture::coreTexture(const coreBool bLoadCompressed)noexcept
: coreResource  ()
, m_iTexture    (0u)
, m_vResolution (coreVector2(0.0f,0.0f))
, m_iLevels     (0u)
, m_iCompressed (bLoadCompressed ? 0 : -1)
, m_iMode       (CORE_TEXTURE_MODE_DEFAULT)
, m_Spec        (coreTextureSpec(0u, 0u, 0u))
, m_Sync        ()
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

    coreFileUnload oUnload(pFile);

    WARN_IF(m_iTexture)   return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetData()) return CORE_ERROR_FILE;

    // decompress file to plain pixel data
    SDL_Surface* pData = IMG_LoadTyped_RW(SDL_RWFromConstMem(pFile->GetData(), pFile->GetSize()), true, coreData::StrExtension(pFile->GetPath()));
    if(!pData)
    {
        Core::Log->Warning("Texture (%s) could not be loaded (SDL: %s)", pFile->GetPath(), SDL_GetError());
        return CORE_INVALID_DATA;
    }

    // calculate data size
    const coreUint32 iDataSize = pData->w * pData->h * pData->format->BytesPerPixel;

    // check for compression capability
    const coreTextureMode iCompress = (coreMath::IsPot(pData->w) && coreMath::IsPot(pData->h) && !m_iCompressed) ? CORE_TEXTURE_MODE_COMPRESS : CORE_TEXTURE_MODE_DEFAULT;

    // create texture
    this->Create(pData->w, pData->h, CORE_TEXTURE_SPEC_COMPONENTS(pData->format->BytesPerPixel), iCompress | CORE_TEXTURE_MODE_FILTER | CORE_TEXTURE_MODE_REPEAT);
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
    m_iMode       = CORE_TEXTURE_MODE_DEFAULT;
    m_Spec        = coreTextureSpec(0u, 0u, 0u);

    return CORE_OK;
}


// ****************************************************************
// create texture memory
void coreTexture::Create(const coreUint32 iWidth, const coreUint32 iHeight, const coreTextureSpec& oSpec, const coreTextureMode iMode)
{
    WARN_IF(m_iTexture) this->Unload();

    // check for OpenGL extensions
    const coreBool bAnisotropic = CORE_GL_SUPPORT(EXT_texture_filter_anisotropic)                && CONTAINS_FLAG(iMode, CORE_TEXTURE_MODE_FILTER);
    const coreBool bMipMap      = CORE_GL_SUPPORT(EXT_framebuffer_object)                        && CONTAINS_FLAG(iMode, CORE_TEXTURE_MODE_FILTER);
    const coreBool bCompress    = Core::Config->GetBool(CORE_CONFIG_GRAPHICS_TEXTURECOMPRESSION) && CONTAINS_FLAG(iMode, CORE_TEXTURE_MODE_COMPRESS);

    // save properties
    m_vResolution = coreVector2(I_TO_F(iWidth), I_TO_F(iHeight));
    m_iLevels     = bMipMap ? F_TO_UI(coreMath::Log<2u>(m_vResolution.Max())) + 1u : 1u;
    m_iMode       = iMode;
    m_Spec        = oSpec;

    // set wrap mode
    const GLenum iWrapMode = CONTAINS_FLAG(iMode, CORE_TEXTURE_MODE_REPEAT) ? GL_REPEAT : GL_CLAMP_TO_EDGE;

    // set compression
    if(bCompress)
    {
        WARN_IF(!coreMath::IsPot(iWidth) || !coreMath::IsPot(iHeight)) {}
        else
        {
            // overwrite with appropriate compressed texture format (RGTC or S3TC)
            GLenum iNewFormat = 0u;
            switch(m_Spec.iInternal)
            {
            default: ASSERT(false)
            case GL_R8:    if(CORE_GL_SUPPORT(EXT_texture_compression_rgtc)) iNewFormat = GL_COMPRESSED_RED_RGTC1_EXT;       break;
            case GL_RG8:   if(CORE_GL_SUPPORT(EXT_texture_compression_rgtc)) iNewFormat = GL_COMPRESSED_RED_GREEN_RGTC2_EXT; break;
            case GL_RGB8:  if(CORE_GL_SUPPORT(EXT_texture_compression_s3tc)) iNewFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;   break;
            case GL_RGBA8: if(CORE_GL_SUPPORT(EXT_texture_compression_s3tc)) iNewFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;  break;
            }
            if(iNewFormat) {m_Spec.iInternal = m_Spec.iFormat = iNewFormat; m_iCompressed = 1;}
        }
    }

    // generate texture
    coreGenTextures2D(1u, &m_iTexture);
    glBindTexture(GL_TEXTURE_2D, m_iTexture);
    s_apBound[s_iActiveUnit] = NULL;

    // set sampling parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bMipMap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     iWrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     iWrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  m_iLevels - 1);
    if(bAnisotropic) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, I_TO_F(Core::Config->GetInt(CORE_CONFIG_GRAPHICS_TEXTUREFILTER)));

    if(CORE_GL_SUPPORT(ARB_texture_storage))
    {
        // allocate immutable texture memory
        glTexStorage2D(GL_TEXTURE_2D, m_iLevels, m_Spec.iInternal, iWidth, iHeight);
    }
    else
    {
        // allocate normal texture memory
        glTexImage2D(GL_TEXTURE_2D, 0, DEFINED(_CORE_GLES_) ? m_Spec.iFormat : m_Spec.iInternal, iWidth, iHeight, 0, m_Spec.iFormat, m_Spec.iType, NULL);
        if(bMipMap) glGenerateMipmap(GL_TEXTURE_2D);
    }
}


// ****************************************************************
// modify texture memory
void coreTexture::Modify(const coreUint32 iOffsetX, const coreUint32 iOffsetY, const coreUint32 iWidth, const coreUint32 iHeight, const coreUint32 iDataSize, const coreByte* pData)
{
    WARN_IF(!m_iTexture) return;
    ASSERT(((iOffsetX + iWidth) <= F_TO_UI(m_vResolution.x)) && ((iOffsetY + iHeight) <= F_TO_UI(m_vResolution.y)))

    // check for OpenGL extensions
    const coreBool bPixelBuffer = CORE_GL_SUPPORT(ARB_pixel_buffer_object) && iDataSize && pData;
    const coreBool bMipMap      = CORE_GL_SUPPORT(EXT_framebuffer_object)  && (m_iLevels > 1u);

    if(m_iCompressed > 0)
    {
        ASSERT((iWidth == F_TO_UI(m_vResolution.x)) && (iHeight == F_TO_UI(m_vResolution.y)))

        // bind texture (simple)
        glBindTexture(GL_TEXTURE_2D, m_iTexture);
        s_apBound[s_iActiveUnit] = NULL;

        // calculate components and compressed size
        const coreUint32 iComponents = iDataSize / (iWidth * iHeight);
        const coreUint32 iPackedSize = iDataSize / stb_compress_dxt_ratio(iComponents);

        // allocate required image memory
        coreByte* pPackedData  = new coreByte[iPackedSize];
        coreByte* apMipData[2] = {NULL, NULL};
        if(bMipMap)
        {
            apMipData[0] = new coreByte[iDataSize >> 2u];
            apMipData[1] = new coreByte[iDataSize >> 4u];
        }

        // process all available texture levels
        for(coreUintW i = 0u, ie = m_iLevels; i < ie; ++i)
        {
            const coreUint32 iCurWidth  = iWidth  >> i;
            const coreUint32 iCurHeight = iHeight >> i;
            const coreUint32 iCurSize   = iPackedSize >> i >> i;

            // create compressed image
            coreTexture::CreateCompressed(iCurWidth, iCurHeight, iComponents, pData, pPackedData);

            // upload image to texture
            coreDataBuffer oBuffer;
            if(bPixelBuffer) oBuffer.Create(GL_PIXEL_UNPACK_BUFFER, iCurSize, pPackedData, CORE_DATABUFFER_STORAGE_STREAM);
            glCompressedTexSubImage2D(GL_TEXTURE_2D, i, 0, 0, iCurWidth, iCurHeight, m_Spec.iFormat, iCurSize, bPixelBuffer ? NULL : pPackedData);

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
        coreDataBuffer oBuffer;
        if(bPixelBuffer)
        {
            // create pixel buffer object for asynchronous texture loading
            oBuffer.Create(GL_PIXEL_UNPACK_BUFFER, iDataSize, pData, CORE_DATABUFFER_STORAGE_STREAM);

            // use PBO instead of client memory
            pData = NULL;
        }

        if(CORE_GL_SUPPORT(ARB_direct_state_access))
        {
            // update texture data directly (new)
            glTextureSubImage2D(m_iTexture, 0, iOffsetX, iOffsetY, iWidth, iHeight, m_Spec.iFormat, m_Spec.iType, pData);
            if(bMipMap) glGenerateTextureMipmap(m_iTexture);
        }
        else if(CORE_GL_SUPPORT(EXT_direct_state_access))
        {
            // update texture data directly (old)
            glTextureSubImage2DEXT(m_iTexture, GL_TEXTURE_2D, 0, iOffsetX, iOffsetY, iWidth, iHeight, m_Spec.iFormat, m_Spec.iType, pData);
            if(bMipMap) glGenerateTextureMipmapEXT(m_iTexture, GL_TEXTURE_2D);
        }
        else
        {
            // bind texture (simple)
            glBindTexture(GL_TEXTURE_2D, m_iTexture);
            s_apBound[s_iActiveUnit] = NULL;

            // update texture data
            glTexSubImage2D(GL_TEXTURE_2D, 0, iOffsetX, iOffsetY, iWidth, iHeight, m_Spec.iFormat, m_Spec.iType, pData);
            if(bMipMap) glGenerateMipmap(GL_TEXTURE_2D);
        }
    }
}


// ****************************************************************
// copy content from current read frame buffer
void coreTexture::CopyFrameBuffer(const coreUint32 iSrcX, const coreUint32 iSrcY, const coreUint32 iDstX, const coreUint32 iDstY, const coreUint32 iWidth, const coreUint32 iHeight)
{
    ASSERT(m_iTexture)
    ASSERT(((iSrcX + iWidth) <= F_TO_UI(Core::Graphics->GetViewResolution().x)) && ((iSrcY + iHeight) <= F_TO_UI(Core::Graphics->GetViewResolution().y)) &&
           ((iDstX + iWidth) <= F_TO_UI(m_vResolution                      .x)) && ((iDstY + iHeight) <= F_TO_UI(m_vResolution                      .y)))

    if(CORE_GL_SUPPORT(ARB_direct_state_access))
    {
        // copy frame buffer directly (new)
        glCopyTextureSubImage2D(m_iTexture, 0, iDstX, iDstY, iSrcX, iSrcY, iWidth, iHeight);
    }
    else if(CORE_GL_SUPPORT(EXT_direct_state_access))
    {
        // copy frame buffer directly (old)
        glCopyTextureSubImage2DEXT(m_iTexture, GL_TEXTURE_2D, 0, iDstX, iDstY, iSrcX, iSrcY, iWidth, iHeight);
    }
    else
    {
        // bind and copy frame buffer
        this->Enable(0);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, iDstX, iDstY, iSrcX, iSrcY, iWidth, iHeight);
    }
}

void coreTexture::CopyFrameBuffer()
{
    this->CopyFrameBuffer(0u, 0u, 0u, 0u, F_TO_UI(m_vResolution.x), F_TO_UI(m_vResolution.y));
}


// ****************************************************************
// bind texture level to image unit
void coreTexture::BindImage(const coreUintW iUnit, const coreUint8 iLevel, const GLenum iAccess)
{
    ASSERT(m_iTexture && (iLevel < m_iLevels))

    if(CORE_GL_SUPPORT(ARB_shader_image_load_store))
    {
        // bind directly without layering
        glBindImageTexture(iUnit, m_iTexture, iLevel, false, 0, iAccess, m_Spec.iInternal);
    }
}


// ****************************************************************
// copy image data to another image
void coreTexture::CopyImage(coreTexture* OUTPUT pDestination, const coreUint8 iSrcLevel, const coreUint32 iSrcX, const coreUint32 iSrcY, const coreUint8 iDstLevel, const coreUint32 iDstX, const coreUint32 iDstY, const coreUint32 iWidth, const coreUint32 iHeight)const
{
    ASSERT(m_iTexture)
    ASSERT((iSrcLevel < m_iLevels) && (iDstLevel < pDestination->GetLevels()) &&
           ((iSrcX + iWidth) <= F_TO_UI(m_vResolution                .x)) && ((iSrcY + iHeight) <= F_TO_UI(m_vResolution                .y)) &&
           ((iDstX + iWidth) <= F_TO_UI(pDestination->GetResolution().x)) && ((iDstY + iHeight) <= F_TO_UI(pDestination->GetResolution().y)))

    if(CORE_GL_SUPPORT(ARB_copy_image))
    {
        // copy directly to another texture unit
        glCopyImageSubData(m_iTexture,                 GL_TEXTURE_2D, iSrcLevel, iSrcX, iSrcY, 0,
                           pDestination->GetTexture(), GL_TEXTURE_2D, iDstLevel, iDstX, iDstY, 0,
                           iWidth, iHeight, 0);
    }
}

void coreTexture::CopyImage(coreTexture* OUTPUT pDestination)const
{
    this->CopyImage(pDestination, 0u, 0u, 0u, 0u, 0u, 0u, F_TO_UI(m_vResolution.x), F_TO_UI(m_vResolution.y));
}


// ****************************************************************
// configure shadow sampling
void coreTexture::ShadowSampling(const coreBool bStatus)
{
    ASSERT(m_iTexture && (m_Spec.iFormat == GL_DEPTH_COMPONENT || m_Spec.iFormat == GL_DEPTH_STENCIL))

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
// enable default array of textures
void coreTexture::EnableAll(const coreResourcePtr<coreTexture>* ppTextureArray)
{
    if(CORE_GL_SUPPORT(ARB_multi_bind))
    {
        coreInt8 iStart = -1;
        coreInt8 iEnd   = -1;

        // loop through all textures
        for(coreUintW i = 0u; i < CORE_TEXTURE_UNITS; ++i)
        {
            if(ppTextureArray[i].IsUsable())
            {
                coreTexture* pTexture = s_cast<coreTexture*>(ppTextureArray[i].GetHandle()->GetResource());

                // check texture binding
                if(s_apBound[i] == pTexture) continue;
                s_apBound[i] = pTexture;

                // set range of texture units
                if(iStart < 0) iStart = i;
                iEnd = i;
            }
        }

        if(iStart >= 0)
        {
            GLuint aiIdentifier[CORE_TEXTURE_UNITS] = {};

            // prepare tight traversal
            const coreUintW iCount = iEnd - iStart + 1u;
            ppTextureArray += iStart;

            // arrange texture identifiers
            for(coreUintW i = 0u; i < iCount; ++i)
            {
                if(ppTextureArray[i].IsUsable()) aiIdentifier[i] = ppTextureArray[i]->GetTexture();
                else s_apBound[i] = NULL;
            }

            // enable all at once
            glBindTextures(iStart, iCount, aiIdentifier);
        }
    }
    else
    {
        // enable all separately
        for(coreUintW i = 0u; i < CORE_TEXTURE_UNITS; ++i)
            if(ppTextureArray[i].IsUsable()) ppTextureArray[i]->Enable(i);
    }
}


// ****************************************************************
// disable all textures
void coreTexture::DisableAll()
{
    if(CORE_GL_SUPPORT(ARB_multi_bind))
    {
        // disable all at once
        std::memset(s_apBound, 0, sizeof(s_apBound));
        glBindTextures(0u, CORE_TEXTURE_UNITS, NULL);
    }
    else
    {
        // disable all separately
        for(coreUintW i = CORE_TEXTURE_UNITS; i--; )
            coreTexture::Disable(i);
    }
}


// ****************************************************************
// clear content of the texture
void coreTexture::Clear(const coreUint8 iLevel)
{
    ASSERT(m_iTexture && (iLevel < m_iLevels))

    if(CORE_GL_SUPPORT(ARB_clear_texture))
    {
        // clear the whole texture
        glClearTexImage(m_iTexture, iLevel, m_Spec.iFormat, m_Spec.iType, NULL);
    }
}


// ****************************************************************
// invalidate content of the texture
void coreTexture::Invalidate(const coreUint8 iLevel)
{
    ASSERT(m_iTexture && (iLevel < m_iLevels))

    if(CORE_GL_SUPPORT(ARB_invalidate_subdata))
    {
        // invalidate the whole texture
        glInvalidateTexImage(m_iTexture, iLevel);
    }
}


// ****************************************************************
// create next level-of-detail image
void coreTexture::CreateNextLevel(const coreUintW iInWidth, const coreUintW iInHeight, const coreUintW iComponents, const coreByte* pInput, coreByte* OUTPUT pOutput)
{
    ASSERT(coreMath::IsPot(iInWidth)  && (iInWidth  >= 2u) &&
           coreMath::IsPot(iInHeight) && (iInHeight >= 2u) &&
           (iComponents <= 4u) && pInput && pOutput)

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
void coreTexture::CreateCompressed(const coreUintW iInWidth, const coreUintW iInHeight, const coreUintW iComponents, const coreByte* pInput, coreByte* OUTPUT pOutput)
{
    ASSERT(coreMath::IsPot(iInWidth)  && (iInWidth  >= 4u) &&
           coreMath::IsPot(iInHeight) && (iInHeight >= 4u) &&
           (iComponents <= 4u) && pInput && pOutput)

    // save memory offsets and alpha status
    const coreUintW iInOffsetX = 4u * iComponents;
    const coreUintW iInOffsetY = 3u * iComponents * iInWidth;
    const coreUintW iOutOffset = 8u * (2u - (iComponents % 2u));

    // loop through all input texels
    for(coreUintW y = 0u; y < iInHeight; y += 4u)
    {
        for(coreUintW x = 0u; x < iInWidth; x += 4u)
        {
            alignas(ALIGNMENT_CACHE) coreByte aBlock[64];

            // copy data into 4x4 RGBA block
            for(coreUintW i = 0u; i < 4u; ++i) std::memcpy(aBlock + (i)       * 4u, pInput + (i)               * iComponents, iComponents);
            for(coreUintW i = 0u; i < 4u; ++i) std::memcpy(aBlock + (i +  4u) * 4u, pInput + (i + 1u*iInWidth) * iComponents, iComponents);
            for(coreUintW i = 0u; i < 4u; ++i) std::memcpy(aBlock + (i +  8u) * 4u, pInput + (i + 2u*iInWidth) * iComponents, iComponents);
            for(coreUintW i = 0u; i < 4u; ++i) std::memcpy(aBlock + (i + 12u) * 4u, pInput + (i + 3u*iInWidth) * iComponents, iComponents);

            // compress block (R -> RGTC1/BC4, RG -> RGTC2/BC5, RGB -> DXT1/BC1, RGBA -> DXT5/BC3)
            stb_compress_dxt_block(pOutput, aBlock, iComponents, STB_DXT_HIGHQUAL);

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
void coreTexture::__BindTexture(const coreUintW iUnit, coreTexture* pTexture)
{
    ASSERT(iUnit < CORE_TEXTURE_UNITS)

    // check texture binding
    if(s_apBound[iUnit] == pTexture) return;
    s_apBound[iUnit] = pTexture;

    if(CORE_GL_SUPPORT(ARB_direct_state_access))
    {
        // bind texture directly (new)
        glBindTextureUnit(iUnit, pTexture ? pTexture->GetTexture() : 0u);
    }
    else if(CORE_GL_SUPPORT(EXT_direct_state_access))
    {
        // bind texture directly (old)
        glBindMultiTextureEXT(GL_TEXTURE0 + iUnit, GL_TEXTURE_2D, pTexture ? pTexture->GetTexture() : 0u);
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