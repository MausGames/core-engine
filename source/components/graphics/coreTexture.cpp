///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"
#include <stb_dxt.h>
#include <bc7enc.h>

coreUintW    coreTexture::s_iActiveUnit                 = 0u;
coreTexture* coreTexture::s_apBound[CORE_TEXTURE_UNITS] = {};


// ****************************************************************
/* constructor */
coreTexture::coreTexture(const coreTextureLoad eLoad)noexcept
: coreResource  ()
, m_iIdentifier (0u)
, m_vResolution (coreVector2(0.0f,0.0f))
, m_iLevels     (0u)
, m_iCompressed (0u)
, m_eMode       (CORE_TEXTURE_MODE_DEFAULT)
, m_Spec        (coreTextureSpec(0u, 0u, 0u, 0u, 0u))
, m_eLoad       (eLoad)
, m_Sync        ()
{
}


// ****************************************************************
/* destructor */
coreTexture::~coreTexture()
{
    this->Unload();
}


// ****************************************************************
/* load texture resource data */
coreStatus coreTexture::Load(coreFile* pFile)
{
    // check for sync object status
    const coreStatus eCheck = m_Sync.Check(0u);
    if(eCheck >= CORE_OK) return eCheck;

    WARN_IF(m_iIdentifier) return CORE_INVALID_CALL;
    if(!pFile)             return CORE_INVALID_INPUT;
    if(!pFile->GetSize())  return CORE_ERROR_FILE;   // do not load file data

    // decompress file to plain pixel data
    coreSurfaceScope pData = IMG_LoadTyped_IO(pFile->CreateReadStream(), true, coreData::StrExtension(pFile->GetPath()));
    WARN_IF(!pData || !pData->w || !pData->h)
    {
        Core::Log->Warning("Texture (%s) could not be loaded (SDL: %s)", m_sName.c_str(), SDL_GetError());
        return CORE_INVALID_DATA;
    }

    // convert from low-channel data (if not supported)
    if((SDL_BYTESPERPIXEL(pData->format) < 3u) && !CORE_GL_SUPPORT(ARB_texture_rg))
    {
        pData = SDL_ConvertSurface(pData, SDL_PIXELFORMAT_RGB24);
    }

    // convert to smaller texture format (if supported)
    if((HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_RG) || HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_R)) && CORE_GL_SUPPORT(ARB_texture_rg))
    {
        WARN_IF(SDL_BYTESPERPIXEL(pData->format) != 3u) {}
        else pData = coreTexture::CreateReduction(HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_RG) ? 2u : 1u, pData);
    }

    ASSERT(!SDL_MUSTLOCK(pData))

    // calculate data size
    const coreUint8  iComponents = SDL_BYTESPERPIXEL(pData->format);
    const coreUint32 iDataSize   = pData->pitch * pData->h;
    ASSERT(iComponents && iDataSize)

    // check load configuration
    const coreTextureMode eMode = ((!HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_NO_COMPRESS) && coreMath::IsPot(pData->w) && coreMath::IsPot(pData->h)) ? (HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_COMPRESS_HIGH) ? CORE_TEXTURE_MODE_COMPRESS_HIGH : CORE_TEXTURE_MODE_COMPRESS) : CORE_TEXTURE_MODE_DEFAULT) |
                                  ((!HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_NO_FILTER)) ? CORE_TEXTURE_MODE_FILTER  : CORE_TEXTURE_MODE_DEFAULT) |
                                  ((!HAS_FLAG(m_eLoad, CORE_TEXTURE_LOAD_NEAREST))   ? CORE_TEXTURE_MODE_DEFAULT : CORE_TEXTURE_MODE_NEAREST);

    // create texture
    this->Create(pData->w, pData->h, CORE_TEXTURE_SPEC_COMPONENTS(iComponents), eMode | CORE_TEXTURE_MODE_REPEAT);
    this->Modify(0u, 0u, pData->w, pData->h, iDataSize, s_cast<coreByte*>(pData->pixels));

    // add debug label
    Core::Graphics->LabelOpenGL(GL_TEXTURE, m_iIdentifier, m_sName.c_str());

    Core::Log->Info("Texture (%s, %.0f x %.0f, %u components, %u levels, %s) loaded", m_sName.c_str(), m_vResolution.x, m_vResolution.y, iComponents, m_iLevels, m_iCompressed ? "compressed" : "standard");
    return m_Sync.Create(CORE_SYNC_CREATE_FLUSHED) ? CORE_BUSY : CORE_OK;
}


// ****************************************************************
/* unload texture resource data */
coreStatus coreTexture::Unload()
{
    if(!m_iIdentifier) return CORE_INVALID_CALL;

    // disable still active texture bindings
    for(coreUintW i = CORE_TEXTURE_UNITS; i--; )
    {
        if(s_apBound[i] == this) coreTexture::Disable(i);
    }

    // delete texture
    coreDelTextures2D(1u, &m_iIdentifier);
    if(!m_sName.empty()) Core::Log->Info("Texture (%s) unloaded", m_sName.c_str());

    // delete sync object
    m_Sync.Delete();

    // reset properties
    m_iIdentifier = 0u;
    m_vResolution = coreVector2(0.0f,0.0f);
    m_iLevels     = 0u;
    m_iCompressed = 0u;
    m_eMode       = CORE_TEXTURE_MODE_DEFAULT;
    m_Spec        = coreTextureSpec(0u, 0u, 0u, 0u, 0u);

    return CORE_OK;
}


// ****************************************************************
/* create texture memory */
void coreTexture::Create(const coreUint32 iWidth, const coreUint32 iHeight, const coreTextureSpec& oSpec, const coreTextureMode eMode)
{
    WARN_IF(m_iIdentifier) this->Unload();
    ASSERT(iWidth && iHeight)

    // check for OpenGL extensions
    const coreBool bFilterable  = !DEFINED(_CORE_GLES_) || ((oSpec.iFormat != GL_DEPTH_COMPONENT) && (oSpec.iFormat != GL_DEPTH_STENCIL));
    const coreBool bAnisotropic = CORE_GL_SUPPORT(ARB_texture_filter_anisotropic)                && HAS_FLAG(eMode, CORE_TEXTURE_MODE_FILTER);
    const coreBool bMipMap      = CORE_GL_SUPPORT(EXT_framebuffer_object)                        && HAS_FLAG(eMode, CORE_TEXTURE_MODE_FILTER);
    const coreBool bMipMapOld   = CORE_GL_SUPPORT(CORE_gl2_compatibility) && !bMipMap            && HAS_FLAG(eMode, CORE_TEXTURE_MODE_FILTER);
    const coreBool bTarget      = CORE_GL_SUPPORT(ANGLE_texture_usage)                           && HAS_FLAG(eMode, CORE_TEXTURE_MODE_TARGET);
    const coreBool bCompress    = Core::Config->GetBool(CORE_CONFIG_GRAPHICS_TEXTURECOMPRESSION) && HAS_FLAG(eMode, CORE_TEXTURE_MODE_COMPRESS);
    const coreBool bTrilinear   = Core::Config->GetBool(CORE_CONFIG_GRAPHICS_TEXTURETRILINEAR)   && bFilterable;

    // save properties
    m_vResolution = coreVector2(I_TO_F(iWidth), I_TO_F(iHeight));
    m_iLevels     = (bMipMap || bMipMapOld) ? F_TO_UI(LOG2(m_vResolution.Max())) + 1u : 1u;
    m_eMode       = eMode;
    m_Spec        = oSpec;

    // set filter mode
    const GLenum iMagFilter = (HAS_FLAG(eMode, CORE_TEXTURE_MODE_NEAREST) || !bFilterable) ? GL_NEAREST : GL_LINEAR;
    const GLenum iMinFilter = (HAS_FLAG(eMode, CORE_TEXTURE_MODE_NEAREST) || !bFilterable) ? ((bMipMap || bMipMapOld) ? (bTrilinear ? GL_NEAREST_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST) : GL_NEAREST) :
                                                                                             ((bMipMap || bMipMapOld) ? (bTrilinear ? GL_LINEAR_MIPMAP_LINEAR  : GL_LINEAR_MIPMAP_NEAREST)  : GL_LINEAR);

    // set wrap mode
    const GLenum iWrapMode = HAS_FLAG(eMode, CORE_TEXTURE_MODE_REPEAT) ? GL_REPEAT : GL_CLAMP_TO_EDGE;

    // set compression
    if(bCompress)
    {
        WARN_IF(!coreMath::IsPot(iWidth) || !coreMath::IsPot(iHeight) || (iWidth < 4u) || (iHeight < 4u)) {}
        else
        {
            if(HAS_FLAG(eMode, CORE_TEXTURE_MODE_COMPRESS_HIGH))
            {
                // overwrite with appropriate compressed texture format (BPTC)
                switch(m_Spec.iInternal)
                {
                default: UNREACHABLE
                case GL_RGB8:  if(CORE_GL_SUPPORT(ARB_texture_compression_bptc)) {m_Spec = CORE_TEXTURE_SPEC_COMPRESSED_BPTC3; m_iCompressed = 2u;} break;
                case GL_RGBA8: if(CORE_GL_SUPPORT(ARB_texture_compression_bptc)) {m_Spec = CORE_TEXTURE_SPEC_COMPRESSED_BPTC4; m_iCompressed = 2u;} break;
                }
            }
            else
            {
                // overwrite with appropriate compressed texture format (RGTC or S3TC)
                switch(m_Spec.iInternal)
                {
                default: UNREACHABLE
                case GL_LUMINANCE8:
                case GL_R8:    if(CORE_GL_SUPPORT(ARB_texture_compression_rgtc)) {m_Spec = CORE_TEXTURE_SPEC_COMPRESSED_RGTC1; m_iCompressed = 1u;} break;
                case GL_RG8:   if(CORE_GL_SUPPORT(ARB_texture_compression_rgtc)) {m_Spec = CORE_TEXTURE_SPEC_COMPRESSED_RGTC2; m_iCompressed = 1u;} break;
                case GL_RGB8:  if(CORE_GL_SUPPORT(EXT_texture_compression_s3tc)) {m_Spec = CORE_TEXTURE_SPEC_COMPRESSED_DXT1;  m_iCompressed = 1u;} break;
                case GL_RGBA8: if(CORE_GL_SUPPORT(EXT_texture_compression_s3tc)) {m_Spec = CORE_TEXTURE_SPEC_COMPRESSED_DXT5;  m_iCompressed = 1u;} break;
                }
            }
            if(m_iCompressed) {if((bMipMap || bMipMapOld) && !CORE_GL_SUPPORT(CORE_es2_restriction)) m_iLevels = F_TO_UI(LOG2(m_vResolution.Min())) - 1u;}
        }
    }

    // generate texture
    coreGenTextures2D(1u, &m_iIdentifier);
    glBindTexture(GL_TEXTURE_2D, m_iIdentifier);
    s_apBound[s_iActiveUnit] = NULL;

    // set sampling parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, iMagFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, iMinFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     iWrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     iWrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  m_iLevels - 1);
    if(bAnisotropic)          glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, I_TO_F(CLAMP(Core::Config->GetInt(CORE_CONFIG_GRAPHICS_TEXTUREANISOTROPY), 1, Core::Graphics->GetMaxAnisotropy())));
    if(bMipMap || bMipMapOld) glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS,       CORE_TEXTURE_LOD_BIAS);
    if(bMipMapOld)            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP,        GL_TRUE);
    if(bTarget)               glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_USAGE_ANGLE,    GL_FRAMEBUFFER_ATTACHMENT_ANGLE);

    if(CORE_GL_SUPPORT(ARB_texture_storage))
    {
        // allocate immutable texture memory
        glTexStorage2D(GL_TEXTURE_2D, m_iLevels, m_Spec.iInternal, iWidth, iHeight);
    }
    else
    {
        // allocate mutable texture memory
        if(m_iCompressed)
        {
            const coreUint32 iBlockSize = (m_iCompressed == 2u) ? bc7enc_blocksize(m_Spec.iComponents) : stb_dxt_blocksize(m_Spec.iComponents);

            for(coreUintW i = 0u, ie = LOOP_NONZERO(m_iLevels); i < ie; ++i)
            {
                const coreUint32 iCurWidth  = MAX(iWidth  >> i, 1u);
                const coreUint32 iCurHeight = MAX(iHeight >> i, 1u);
                const coreUint32 iCurSize   = MAX(iCurWidth / 4u, 1u) * MAX(iCurHeight / 4u, 1u) * iBlockSize;

                // create all compressed levels
                glCompressedTexImage2D(GL_TEXTURE_2D, i, m_Spec.iInternal, iCurWidth, iCurHeight, 0, iCurSize, NULL);
            }
        }
        else
        {
            // create only first standard level
            glTexImage2D(GL_TEXTURE_2D, 0, CORE_GL_SUPPORT(CORE_es2_restriction) ? m_Spec.iFormat : m_Spec.iInternal, iWidth, iHeight, 0, m_Spec.iFormat, m_Spec.iType, NULL);
        }
    }
}


// ****************************************************************
/* modify texture memory */
void coreTexture::Modify(const coreUint32 iOffsetX, const coreUint32 iOffsetY, const coreUint32 iWidth, const coreUint32 iHeight, const coreUint32 iDataSize, const coreByte* pData)
{
    WARN_IF(!m_iIdentifier) return;
    ASSERT(iWidth && iHeight && (iOffsetX + iWidth <= F_TO_UI(m_vResolution.x)) && (iOffsetY + iHeight <= F_TO_UI(m_vResolution.y)))

    // check for OpenGL extensions
    const coreBool bPixelBuffer = CORE_GL_SUPPORT(ARB_pixel_buffer_object) && iDataSize && pData;
    const coreBool bMipMap      = CORE_GL_SUPPORT(EXT_framebuffer_object)  && (m_iLevels > 1u);

    // adjust row unpack alignment
    const coreUint32 iUnaligned = (iDataSize / iHeight) % 4u;
    if(iUnaligned) glPixelStorei(GL_UNPACK_ALIGNMENT, (iUnaligned == 2u) ? 2 : 1);

    if(m_iCompressed)
    {
        ASSERT((iWidth == F_TO_UI(m_vResolution.x)) && (iHeight == F_TO_UI(m_vResolution.y)) && iDataSize && pData)

        // bind texture (simple)
        glBindTexture(GL_TEXTURE_2D, m_iIdentifier);
        s_apBound[s_iActiveUnit] = NULL;

        // calculate components and compressed size
        const coreUint32 iComponents = iDataSize / (iWidth * iHeight);
        const coreUint32 iPackedSize = iDataSize / ((m_iCompressed == 2u) ? bc7enc_ratio(iComponents) : stb_dxt_ratio(iComponents));
        ASSERT(iComponents == m_Spec.iComponents)

        // allocate required image memory
        coreByte* pPackedData  = TEMP_NEW(coreByte, iPackedSize);
        coreByte* apMipData[2] = {NULL, NULL};
        if(bMipMap)
        {
            apMipData[0] = TEMP_NEW(coreByte, iDataSize >> 2u);
            apMipData[1] = TEMP_NEW(coreByte, iDataSize >> 4u);
        }

        // process all available texture levels
        for(coreUintW i = 0u, ie = LOOP_NONZERO(m_iLevels); i < ie; ++i)
        {
            const coreUint32 iCurWidth  = iWidth  >> i;
            const coreUint32 iCurHeight = iHeight >> i;
            const coreUint32 iCurSize   = iPackedSize >> i >> i;

            // create compressed image
            if(m_iCompressed == 2u) coreTexture::CreateCompressedBC7(iCurWidth, iCurHeight, iComponents, pData, pPackedData);
                               else coreTexture::CreateCompressedDXT(iCurWidth, iCurHeight, iComponents, pData, pPackedData);

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
        TEMP_DELETE(pPackedData)
        TEMP_DELETE(apMipData[0])
        TEMP_DELETE(apMipData[1])
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
            glTextureSubImage2D(m_iIdentifier, 0, iOffsetX, iOffsetY, iWidth, iHeight, m_Spec.iFormat, m_Spec.iType, pData);
            if(bMipMap) glGenerateTextureMipmap(m_iIdentifier);
        }
        else if(CORE_GL_SUPPORT(EXT_direct_state_access))
        {
            // update texture data directly (old)
            glTextureSubImage2DEXT(m_iIdentifier, GL_TEXTURE_2D, 0, iOffsetX, iOffsetY, iWidth, iHeight, m_Spec.iFormat, m_Spec.iType, pData);
            if(bMipMap) glGenerateTextureMipmapEXT(m_iIdentifier, GL_TEXTURE_2D);
        }
        else
        {
            // bind texture (simple)
            glBindTexture(GL_TEXTURE_2D, m_iIdentifier);
            s_apBound[s_iActiveUnit] = NULL;

            // update texture data
            glTexSubImage2D(GL_TEXTURE_2D, 0, iOffsetX, iOffsetY, iWidth, iHeight, m_Spec.iFormat, m_Spec.iType, pData);
            if(bMipMap) glGenerateMipmap(GL_TEXTURE_2D);
        }
    }

    // reset row unpack alignment
    if(iUnaligned) glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}


// ****************************************************************
/* copy content from current read frame buffer */
void coreTexture::CopyFrameBuffer(const coreUint32 iSrcX, const coreUint32 iSrcY, const coreUint32 iDstX, const coreUint32 iDstY, const coreUint32 iWidth, const coreUint32 iHeight)
{
    ASSERT(m_iIdentifier)
    ASSERT(((iSrcX + iWidth) <= F_TO_UI(Core::Graphics->GetViewResolution().x)) && ((iSrcY + iHeight) <= F_TO_UI(Core::Graphics->GetViewResolution().y)) &&
           ((iDstX + iWidth) <= F_TO_UI(m_vResolution                      .x)) && ((iDstY + iHeight) <= F_TO_UI(m_vResolution                      .y)))

    if(CORE_GL_SUPPORT(ARB_direct_state_access))
    {
        // copy frame buffer directly (new)
        glCopyTextureSubImage2D(m_iIdentifier, 0, iDstX, iDstY, iSrcX, iSrcY, iWidth, iHeight);
    }
    else if(CORE_GL_SUPPORT(EXT_direct_state_access))
    {
        // copy frame buffer directly (old)
        glCopyTextureSubImage2DEXT(m_iIdentifier, GL_TEXTURE_2D, 0, iDstX, iDstY, iSrcX, iSrcY, iWidth, iHeight);
    }
    else
    {
        // bind texture and copy frame buffer
        this->Enable(0u);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, iDstX, iDstY, iSrcX, iSrcY, iWidth, iHeight);
    }
}

void coreTexture::CopyFrameBuffer()
{
    this->CopyFrameBuffer(0u, 0u, 0u, 0u, F_TO_UI(m_vResolution.x), F_TO_UI(m_vResolution.y));
}


// ****************************************************************
/* bind texture level to image unit */
coreStatus coreTexture::BindImage(const coreUintW iUnit, const coreUint8 iLevel, const GLenum iAccess)
{
    ASSERT(m_iIdentifier && (iLevel < m_iLevels))

    if(CORE_GL_SUPPORT(ARB_shader_image_load_store))
    {
        // bind directly without layering
        glBindImageTexture(iUnit, m_iIdentifier, iLevel, false, 0, iAccess, m_Spec.iInternal);

        return CORE_OK;
    }

    return CORE_ERROR_SUPPORT;
}


// ****************************************************************
/* read image data into buffer */
void coreTexture::ReadImage(const coreUint8 iLevel, const coreUint32 iDataSize, coreByte* OUTPUT pData)
{
    ASSERT(m_iIdentifier && (iLevel < m_iLevels) && iDataSize && pData)

    if(CORE_GL_SUPPORT(ARB_direct_state_access))
    {
        // read image data directly (new)
        glGetTextureImage(m_iIdentifier, iLevel, m_Spec.iFormat, m_Spec.iType, iDataSize, pData);
    }
    else if(CORE_GL_SUPPORT(EXT_direct_state_access))
    {
        // read image data directly (old)
        glGetTextureImageEXT(m_iIdentifier, GL_TEXTURE_2D, iLevel, m_Spec.iFormat, m_Spec.iType, pData);
    }
    else
    {
        // bind texture and read image data
        this->Enable(0u);
        glGetTexImage(GL_TEXTURE_2D, iLevel, m_Spec.iFormat, m_Spec.iType, pData);
    }
}


// ****************************************************************
/* copy image data to another image */
coreStatus coreTexture::CopyImage(coreTexture* OUTPUT pDestination, const coreUint8 iSrcLevel, const coreUint32 iSrcX, const coreUint32 iSrcY, const coreUint8 iDstLevel, const coreUint32 iDstX, const coreUint32 iDstY, const coreUint32 iWidth, const coreUint32 iHeight)const
{
    ASSERT(m_iIdentifier)
    ASSERT((iSrcLevel < m_iLevels) && (iDstLevel < pDestination->GetLevels()) &&
           ((iSrcX + iWidth) <= F_TO_UI(m_vResolution                .x)) && ((iSrcY + iHeight) <= F_TO_UI(m_vResolution                .y)) &&
           ((iDstX + iWidth) <= F_TO_UI(pDestination->GetResolution().x)) && ((iDstY + iHeight) <= F_TO_UI(pDestination->GetResolution().y)))

    if(CORE_GL_SUPPORT(ARB_copy_image))
    {
        // copy directly to another texture unit
        glCopyImageSubData(m_iIdentifier,                 GL_TEXTURE_2D, iSrcLevel, iSrcX, iSrcY, 0,
                           pDestination->GetIdentifier(), GL_TEXTURE_2D, iDstLevel, iDstX, iDstY, 0,
                           iWidth, iHeight, 0);

        return CORE_OK;
    }

    return CORE_ERROR_SUPPORT;
}

coreStatus coreTexture::CopyImage(coreTexture* OUTPUT pDestination)const
{
    return this->CopyImage(pDestination, 0u, 0u, 0u, 0u, 0u, 0u, F_TO_UI(m_vResolution.x), F_TO_UI(m_vResolution.y));
}


// ****************************************************************
/* configure shadow sampling */
void coreTexture::EnableShadowSampling()
{
    ASSERT(m_iIdentifier && ((m_Spec.iFormat == GL_DEPTH_COMPONENT) || (m_Spec.iFormat == GL_DEPTH_STENCIL)))

    // bind texture
    this->Enable(0u);

    // enable depth value comparison (with sampler2DShadow)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,   GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,   (m_iLevels > 1u) ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
}


// ****************************************************************
/* enable default array of textures */
void coreTexture::EnableAll(const coreResourcePtr<coreTexture>* ppTextureArray)
{
    if(CORE_GL_SUPPORT(ARB_multi_bind))
    {
        GLuint aiIdentifier[CORE_TEXTURE_UNITS] = {};

        coreInt8 iStart = -1;
        coreInt8 iEnd   = -1;

        // loop through all textures
        for(coreUintW i = 0u; i < CORE_TEXTURE_UNITS; ++i)
        {
            if(ppTextureArray[i].IsUsable())
            {
                coreTexture* pTexture = ppTextureArray[i].GetResource();

                // insert texture identifier
                aiIdentifier[i] = pTexture->GetIdentifier();

                // check texture binding
                if(s_apBound[i] == pTexture) continue;
                s_apBound[i] = pTexture;

                // set range of texture units
                if(iStart < 0) iStart = i;
                iEnd = i;
            }
            else
            {
                coreTexture* pTexture = s_apBound[i];

                // keep current texture identifier
                if(pTexture) aiIdentifier[i] = pTexture->GetIdentifier();
            }
        }

        // enable all at once
        if(iStart >= 0) glBindTextures(iStart, iEnd - iStart + 1, aiIdentifier + iStart);
    }
    else
    {
        // enable all separately
        for(coreUintW i = 0u; i < CORE_TEXTURE_UNITS; ++i)
        {
            if(ppTextureArray[i].IsUsable()) ppTextureArray[i]->Enable(i);
        }
    }
}


// ****************************************************************
/* disable all textures */
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
        // disable all separately (end with first texture unit)
        for(coreUintW i = CORE_TEXTURE_UNITS; i--; )
        {
            coreTexture::Disable(i);
        }
    }
}


// ****************************************************************
/* clear content of the texture */
void coreTexture::Clear(const coreUint8 iLevel)
{
    ASSERT(m_iIdentifier && (iLevel < m_iLevels))

    if(CORE_GL_SUPPORT(ARB_clear_texture))
    {
        // clear the whole texture
        glClearTexImage(m_iIdentifier, iLevel, m_Spec.iFormat, m_Spec.iType, NULL);
    }
    else
    {
        coreByte* pEmpty = TEMP_ZERO_NEW(coreByte, F_TO_UI(m_vResolution.x) * F_TO_UI(m_vResolution.y) * (m_Spec.iBytes ? m_Spec.iBytes : 4u));

        if(CORE_GL_SUPPORT(ARB_direct_state_access))
        {
            // reset texture data directly (new)
            glTextureSubImage2D(m_iIdentifier, iLevel, 0, 0, F_TO_UI(m_vResolution.x), F_TO_UI(m_vResolution.y), m_Spec.iFormat, m_Spec.iType, pEmpty);
        }
        else if(CORE_GL_SUPPORT(EXT_direct_state_access))
        {
            // reset texture data directly (old)
            glTextureSubImage2DEXT(m_iIdentifier, GL_TEXTURE_2D, iLevel, 0, 0, F_TO_UI(m_vResolution.x), F_TO_UI(m_vResolution.y), m_Spec.iFormat, m_Spec.iType, pEmpty);
        }
        else
        {
            // bind texture and reset texture data
            this->Enable(0u);
            glTexSubImage2D(GL_TEXTURE_2D, iLevel, 0, 0, F_TO_UI(m_vResolution.x), F_TO_UI(m_vResolution.y), m_Spec.iFormat, m_Spec.iType, pEmpty);
        }

        TEMP_ZERO_DELETE(pEmpty)
    }
}


// ****************************************************************
/* invalidate content of the texture */
coreStatus coreTexture::Invalidate(const coreUint8 iLevel)
{
    ASSERT(m_iIdentifier && (iLevel < m_iLevels))

    if(CORE_GL_SUPPORT(ARB_invalidate_subdata))
    {
        // invalidate the whole texture
        glInvalidateTexImage(m_iIdentifier, iLevel);

        return CORE_OK;
    }

    return CORE_ERROR_SUPPORT;
}


// ****************************************************************
/* create next level-of-detail image */
void coreTexture::CreateNextLevel(const coreUintW iInWidth, const coreUintW iInHeight, const coreUintW iComponents, const coreByte* pInput, coreByte* OUTPUT pOutput)
{
    ASSERT((coreMath::IsPot(iInWidth)  && (iInWidth  >= 2u)) &&
           (coreMath::IsPot(iInHeight) && (iInHeight >= 2u)) &&
           (iComponents <= 4u) && pInput && pOutput)

    // assume pointer alignment
    pInput  = ASSUME_ALIGNED(pInput,  ALIGNMENT_NEW);
    pOutput = ASSUME_ALIGNED(pOutput, ALIGNMENT_NEW);

    // save output texture size
    const coreUintW iOutWidth  = iInWidth  >> 1u;
    const coreUintW iOutHeight = iInHeight >> 1u;

    // save input memory offsets
    const coreUintW iInOffset1 = iComponents;
    const coreUintW iInOffset2 = iComponents * iInWidth;
    const coreUintW iInOffset3 = iInOffset1 + iInOffset2;

    // loop through all output texels
    for(coreUintW y = 0u; y < LOOP_NONZERO(iOutHeight); ++y)
    {
        for(coreUintW x = 0u; x < LOOP_NONZERO(iOutWidth); ++x)
        {
            // calculate base memory positions
            const coreUintW iInBase  = (x + y*iInWidth)  * iComponents * 2u;
            const coreUintW iOutBase = (x + y*iOutWidth) * iComponents;

            for(coreUintW i = 0u; i < LOOP_NONZERO(iComponents); ++i)
            {
                // gather all required input values
                const coreUint16 A = pInput[iInBase + i];
                const coreUint16 B = pInput[iInBase + i + iInOffset1];
                const coreUint16 C = pInput[iInBase + i + iInOffset2];
                const coreUint16 D = pInput[iInBase + i + iInOffset3];

                // calculate average output value (bilinear, rounding-error)
                pOutput[iOutBase + i] = ((A + B + C + D + 1u) >> 2u) & 0xFFu;
            }
        }
    }
}


// ****************************************************************
/* create compressed image */
void coreTexture::CreateCompressedDXT(const coreUintW iInWidth, const coreUintW iInHeight, const coreUintW iComponents, const coreByte* pInput, coreByte* OUTPUT pOutput)
{
    ASSERT(coreMath::IsPot(iInWidth)  && (iInWidth  >= 4u) &&
           coreMath::IsPot(iInHeight) && (iInHeight >= 4u) &&
           (iComponents <= 4u) && pInput && pOutput)

    // assume pointer alignment
    pInput  = ASSUME_ALIGNED(pInput,  ALIGNMENT_NEW);
    pOutput = ASSUME_ALIGNED(pOutput, ALIGNMENT_NEW);

    // save memory offsets
    const coreUintW iInOffsetX =  4u * iComponents;
    const coreUintW iInOffsetY =  3u * iComponents * iInWidth;
    const coreUintW iOutOffset = 16u * iComponents / stb_dxt_ratio(iComponents);   // size per block

    // loop through all input texels
    for(coreUintW y = 0u; y < LOOP_NONZERO(iInHeight); y += 4u)
    {
        for(coreUintW x = 0u; x < LOOP_NONZERO(iInWidth); x += 4u)
        {
            alignas(ALIGNMENT_CACHE) coreByte aBlock[64];

            // copy data into 4x4 RGBA block
            for(coreUintW i = 0u; i < 4u; ++i) std::memcpy(aBlock + (i)       * 4u, pInput + (i)               * iComponents, 4u);
            for(coreUintW i = 0u; i < 4u; ++i) std::memcpy(aBlock + (i +  4u) * 4u, pInput + (i + 1u*iInWidth) * iComponents, 4u);
            for(coreUintW i = 0u; i < 4u; ++i) std::memcpy(aBlock + (i +  8u) * 4u, pInput + (i + 2u*iInWidth) * iComponents, 4u);
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

void coreTexture::CreateCompressedBC7(const coreUintW iInWidth, const coreUintW iInHeight, const coreUintW iComponents, const coreByte* pInput, coreByte* OUTPUT pOutput)
{
    ASSERT(coreMath::IsPot(iInWidth)  && (iInWidth  >= 4u) &&
           coreMath::IsPot(iInHeight) && (iInHeight >= 4u) &&
           (iComponents >= 3u) && (iComponents <= 4u) && pInput && pOutput)

    // assume pointer alignment
    pInput  = ASSUME_ALIGNED(pInput,  ALIGNMENT_NEW);
    pOutput = ASSUME_ALIGNED(pOutput, ALIGNMENT_NEW);

    // save memory offsets
    const coreUintW iInOffsetX =  4u * iComponents;
    const coreUintW iInOffsetY =  3u * iComponents * iInWidth;
    const coreUintW iOutOffset = 16u * iComponents / bc7enc_ratio(iComponents);   // size per block

    // loop through all input texels
    for(coreUintW y = 0u; y < LOOP_NONZERO(iInHeight); y += 4u)
    {
        for(coreUintW x = 0u; x < LOOP_NONZERO(iInWidth); x += 4u)
        {
            alignas(ALIGNMENT_CACHE) coreByte aBlock[64];

            // copy data into 4x4 RGBA block
            for(coreUintW i = 0u; i < 4u; ++i) std::memcpy(aBlock + (i)       * 4u, pInput + (i)               * iComponents, 4u);
            for(coreUintW i = 0u; i < 4u; ++i) std::memcpy(aBlock + (i +  4u) * 4u, pInput + (i + 1u*iInWidth) * iComponents, 4u);
            for(coreUintW i = 0u; i < 4u; ++i) std::memcpy(aBlock + (i +  8u) * 4u, pInput + (i + 2u*iInWidth) * iComponents, 4u);
            for(coreUintW i = 0u; i < 4u; ++i) std::memcpy(aBlock + (i + 12u) * 4u, pInput + (i + 3u*iInWidth) * iComponents, iComponents);

            // compress block (RGB -> BPTC/BC7, RGBA -> BPTC/BC7)
            bc7enc_compress_block(pOutput, aBlock, iComponents);

            // increment pointers
            pInput  += iInOffsetX;
            pOutput += iOutOffset;
        }

        // skip already processed texture lines
        pInput += iInOffsetY;
    }
}


// ****************************************************************
/* convert to smaller texture format */
SDL_Surface* coreTexture::CreateReduction(const coreUintW iComponents, const SDL_Surface* pInput, SDL_Surface* OUTPUT pOutput)
{
    ASSERT(iComponents && pInput)

    const coreUintW iSource = SDL_BYTESPERPIXEL(pInput->format);
    const coreUintW iTarget = iComponents;
    ASSERT(iTarget < iSource)

    // create SDL surface (on demand)
    if(!pOutput) pOutput = SDL_CreateSurface(pInput->w, pInput->h, SDL_PixelFormat(SDL_DEFINE_PIXELFORMAT(SDL_PIXELTYPE_ARRAYU8, SDL_PACKEDORDER_NONE, SDL_PACKEDLAYOUT_NONE, 8 * iTarget, iTarget)));
    ASSERT((pOutput->w == pInput->w) && (pOutput->h == pInput->h) && (SDL_BYTESPERPIXEL(pOutput->format) == iTarget))

    // assume no memory aliasing
    const coreByte*  pInMemory  = ASSUME_ALIGNED(s_cast<const coreByte*>(pInput ->pixels), ALIGNMENT_NEW);
    coreByte* OUTPUT pOutMemory = ASSUME_ALIGNED(s_cast<coreByte*>      (pOutput->pixels), ALIGNMENT_NEW);

    // manually copy texels
    for(coreUintW i = 0u, ie = LOOP_NONZERO(pInput->h); i < ie; ++i)
    {
        for(coreUintW j = 0u, je = LOOP_NONZERO(pInput->w); j < je; ++j)
        {
            const coreUintW iInOffset  = (j * iSource) + (i * pInput ->pitch);
            const coreUintW iOutOffset = (j * iTarget) + (i * pOutput->pitch);
            std::memcpy(pOutMemory + iOutOffset, pInMemory + iInOffset, iTarget);
        }
    }

    return pOutput;
}


// ****************************************************************
/* bind texture to texture unit */
void coreTexture::__BindTexture(const coreUintW iUnit, coreTexture* pTexture)
{
    ASSERT(iUnit < CORE_TEXTURE_UNITS)
    ASSERT(iUnit < Core::Graphics->GetMaxTextures())

    // check texture binding
    if(s_apBound[iUnit] == pTexture) return;
    s_apBound[iUnit] = pTexture;

    if(CORE_GL_SUPPORT(ARB_direct_state_access))
    {
        // bind texture directly (new)
        glBindTextureUnit(iUnit, pTexture ? pTexture->GetIdentifier() : 0u);
    }
    else if(CORE_GL_SUPPORT(EXT_direct_state_access))
    {
        // bind texture directly (old)
        glBindMultiTextureEXT(GL_TEXTURE0 + iUnit, GL_TEXTURE_2D, pTexture ? pTexture->GetIdentifier() : 0u);
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
        glBindTexture(GL_TEXTURE_2D, pTexture ? pTexture->GetIdentifier() : 0u);
    }
}


// ****************************************************************
/* constructor */
coreTextureVolume::coreTextureVolume()noexcept
: m_iIdentifier (0u)
, m_vResolution (coreVector3(0.0f,0.0f,0.0f))
, m_eMode       (CORE_TEXTURE_MODE_DEFAULT)
, m_Spec        (coreTextureSpec(0u, 0u, 0u, 0u, 0u))
{
}


// ****************************************************************
/* destructor */
coreTextureVolume::~coreTextureVolume()
{
    this->Delete();
}


// ****************************************************************
/* create texture memory */
coreStatus coreTextureVolume::Create(const coreUint32 iWidth, const coreUint32 iHeight, const coreUint32 iDepth, const coreTextureSpec& oSpec, const coreTextureMode eMode)
{
    WARN_IF(m_iIdentifier) this->Delete();
    ASSERT(iWidth && iHeight && iDepth)

    // check for OpenGL extensions
    if(!CORE_GL_SUPPORT(EXT_texture3D)) return CORE_ERROR_SUPPORT;

    // save properties
    m_vResolution = coreVector3(I_TO_F(iWidth), I_TO_F(iHeight), I_TO_F(iDepth));
    m_eMode       = eMode;
    m_Spec        = oSpec;

    // set filter mode
    const GLenum iMagFilter = HAS_FLAG(eMode, CORE_TEXTURE_MODE_NEAREST) ? GL_NEAREST : GL_LINEAR;
    const GLenum iMinFilter = HAS_FLAG(eMode, CORE_TEXTURE_MODE_NEAREST) ? GL_NEAREST : GL_LINEAR;

    // set wrap mode
    const GLenum iWrapMode = HAS_FLAG(eMode, CORE_TEXTURE_MODE_REPEAT) ? GL_REPEAT : GL_CLAMP_TO_EDGE;

    // generate texture
    glGenTextures(1u, &m_iIdentifier);
    glBindTexture(GL_TEXTURE_3D, m_iIdentifier);

    // set sampling parameters
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, iMagFilter);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, iMinFilter);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S,     iWrapMode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T,     iWrapMode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R,     iWrapMode);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL,  0);

    if(CORE_GL_SUPPORT(ARB_texture_storage))
    {
        // allocate immutable texture memory
        glTexStorage3D(GL_TEXTURE_3D, 1, m_Spec.iInternal, iWidth, iHeight, iDepth);
    }
    else
    {
        // allocate mutable texture memory
        glTexImage3D(GL_TEXTURE_3D, 0, CORE_GL_SUPPORT(CORE_es2_restriction) ? m_Spec.iFormat : m_Spec.iInternal, iWidth, iHeight, iDepth, 0, m_Spec.iFormat, m_Spec.iType, NULL);
    }

    return CORE_OK;
}


// ****************************************************************
/* modify texture memory */
void coreTextureVolume::Modify(const coreUint32 iOffsetX, const coreUint32 iOffsetY, const coreUint32 iOffsetZ, const coreUint32 iWidth, const coreUint32 iHeight, const coreUint32 iDepth, const coreUint32 iDataSize, const coreByte* pData)
{
    WARN_IF(!m_iIdentifier) return;
    ASSERT(iWidth && iHeight && iDepth && (iOffsetX + iWidth <= F_TO_UI(m_vResolution.x)) && (iOffsetY + iHeight <= F_TO_UI(m_vResolution.y)) && (iOffsetZ + iDepth <= F_TO_UI(m_vResolution.z)))

    // check for OpenGL extensions
    const coreBool bPixelBuffer = CORE_GL_SUPPORT(ARB_pixel_buffer_object) && iDataSize && pData;

    // adjust row unpack alignment
    const coreUint32 iUnaligned = (iDataSize / iHeight / iDepth) % 4u;
    if(iUnaligned) glPixelStorei(GL_UNPACK_ALIGNMENT, (iUnaligned == 2u) ? 2 : 1);

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
        glTextureSubImage3D(m_iIdentifier, 0, iOffsetX, iOffsetY, iOffsetZ, iWidth, iHeight, iDepth, m_Spec.iFormat, m_Spec.iType, pData);
    }
    else if(CORE_GL_SUPPORT(EXT_direct_state_access))
    {
        // update texture data directly (old)
        glTextureSubImage3DEXT(m_iIdentifier, GL_TEXTURE_3D, 0, iOffsetX, iOffsetY, iOffsetZ, iWidth, iHeight, iDepth, m_Spec.iFormat, m_Spec.iType, pData);
    }
    else
    {
        // bind texture (simple)
        glBindTexture(GL_TEXTURE_3D, m_iIdentifier);

        // update texture data
        glTexSubImage3D(GL_TEXTURE_3D, 0, iOffsetX, iOffsetY, iOffsetZ, iWidth, iHeight, iDepth, m_Spec.iFormat, m_Spec.iType, pData);
    }

    // reset row unpack alignment
    if(iUnaligned) glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}


// ****************************************************************
/* delete texture memory */
void coreTextureVolume::Delete()
{
    if(!m_iIdentifier) return;

    // delete texture
    glDeleteTextures(1u, &m_iIdentifier);

    // reset properties
    m_iIdentifier = 0u;
    m_vResolution = coreVector3(0.0f,0.0f,0.0f);
    m_eMode       = CORE_TEXTURE_MODE_DEFAULT;
    m_Spec        = coreTextureSpec(0u, 0u, 0u, 0u, 0u);
}


// ****************************************************************
/* constructor */
coreTextureCube::coreTextureCube()noexcept
: m_iIdentifier (0u)
, m_vResolution (coreVector2(0.0f,0.0f))
, m_eMode       (CORE_TEXTURE_MODE_DEFAULT)
, m_Spec        (coreTextureSpec(0u, 0u, 0u, 0u, 0u))
{
}


// ****************************************************************
/* destructor */
coreTextureCube::~coreTextureCube()
{
    this->Delete();
}


// ****************************************************************
/* create texture memory */
coreStatus coreTextureCube::Create(const coreUint32 iWidth, const coreUint32 iHeight, const coreTextureSpec& oSpec, const coreTextureMode eMode)
{
    WARN_IF(m_iIdentifier) this->Delete();
    ASSERT(iWidth && iHeight)

    // save properties
    m_vResolution = coreVector2(I_TO_F(iWidth), I_TO_F(iHeight));
    m_eMode       = eMode;
    m_Spec        = oSpec;

    // set filter mode
    const GLenum iMagFilter = HAS_FLAG(eMode, CORE_TEXTURE_MODE_NEAREST) ? GL_NEAREST : GL_LINEAR;
    const GLenum iMinFilter = HAS_FLAG(eMode, CORE_TEXTURE_MODE_NEAREST) ? GL_NEAREST : GL_LINEAR;

    // set wrap mode
    const GLenum iWrapMode = HAS_FLAG(eMode, CORE_TEXTURE_MODE_REPEAT) ? GL_REPEAT : GL_CLAMP_TO_EDGE;

    // generate texture
    glGenTextures(1u, &m_iIdentifier);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_iIdentifier);

    // set sampling parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, iMagFilter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, iMinFilter);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,     iWrapMode);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,     iWrapMode);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,     iWrapMode);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL,  0);

    if(CORE_GL_SUPPORT(ARB_texture_storage))
    {
        // allocate immutable texture memory
        glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, m_Spec.iInternal, iWidth, iHeight);
    }
    else
    {
        // allocate mutable texture memory
        glTexImage2D(GL_TEXTURE_CUBE_MAP, 0, CORE_GL_SUPPORT(CORE_es2_restriction) ? m_Spec.iFormat : m_Spec.iInternal, iWidth, iHeight, 0, m_Spec.iFormat, m_Spec.iType, NULL);
    }

    return CORE_OK;
}


// ****************************************************************
/* modify texture memory */
void coreTextureCube::Modify(const coreUint32 iOffsetX, const coreUint32 iOffsetY, const coreUint32 iWidth, const coreUint32 iHeight, const coreUint8 iFace, const coreUint32 iDataSize, const coreByte* pData)
{
    WARN_IF(!m_iIdentifier) return;
    ASSERT(iWidth && iHeight && (iOffsetX + iWidth <= F_TO_UI(m_vResolution.x)) && (iOffsetY + iHeight <= F_TO_UI(m_vResolution.y)) && (iFace < 6u))

    // check for OpenGL extensions
    const coreBool bPixelBuffer = CORE_GL_SUPPORT(ARB_pixel_buffer_object) && iDataSize && pData;

    // adjust row unpack alignment
    const coreUint32 iUnaligned = (iDataSize / iHeight) % 4u;
    if(iUnaligned) glPixelStorei(GL_UNPACK_ALIGNMENT, (iUnaligned == 2u) ? 2 : 1);

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
        glTextureSubImage3D(m_iIdentifier, 0, iOffsetX, iOffsetY, iFace, iWidth, iHeight, 1, m_Spec.iFormat, m_Spec.iType, pData);
    }
    else if(CORE_GL_SUPPORT(EXT_direct_state_access))
    {
        // update texture data directly (old)
        glTextureSubImage2DEXT(m_iIdentifier, GL_TEXTURE_CUBE_MAP_POSITIVE_X + iFace, 0, iOffsetX, iOffsetY, iWidth, iHeight, m_Spec.iFormat, m_Spec.iType, pData);
    }
    else
    {
        // bind texture (simple)
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_iIdentifier);

        // update texture data
        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + iFace, 0, iOffsetX, iOffsetY, iWidth, iHeight, m_Spec.iFormat, m_Spec.iType, pData);
    }

    // reset row unpack alignment
    if(iUnaligned) glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}


// ****************************************************************
/* delete texture memory */
void coreTextureCube::Delete()
{
    if(!m_iIdentifier) return;

    // delete texture
    glDeleteTextures(1u, &m_iIdentifier);

    // reset properties
    m_iIdentifier = 0u;
    m_vResolution = coreVector2(0.0f,0.0f);
    m_eMode       = CORE_TEXTURE_MODE_DEFAULT;
    m_Spec        = coreTextureSpec(0u, 0u, 0u, 0u, 0u);
}