///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_TEXTURE_H_
#define _CORE_GUARD_TEXTURE_H_

// TODO 3: implement sampler objects
// TODO 3: implement light unbind (like in model and program)
// TODO 3: load, check proper use of PBO, maybe implement static buffer(s!) -> PBO seems to work, but not with automatic mipmap generation (stalls there)
// TODO 3: improve mipmap filtering quality (replace box filter with better filter (e.g. gaussian, lanczos), check for border (repeat or clamp depending on sampling), consider overflow) (what about stb_image_resize.h ? in a (real) test the library took 3-20 times as long)
// TODO 3: implement specialized mipmap generation for normal maps
// TODO 3: implement normal map compression (e.g. use DXT5 for normal maps (g and a channel (max precision) for x and y -> reconstruct z in shader))
// TODO 5: check out AMD compressonator, BPTC (BC6 (HDR), BC7 (HQ)), ASTC (4x4, 6x6, 8x8) or even ETC1 (RGB) and ETC2 (RGB, RGBA)
// TODO 5: <old comment style>
// TODO 3: fill remaining mip-maps on compressed textures
// TODO 3: distribute texture-loading across multiple iterations (maybe only decompression<>processing)
// TODO 3: allow updating of all cube-map faces at once (native for direct-state, fallback for old function)
// TODO 3: upload mipmaps with a single PBO (offset?)


// ****************************************************************
/* texture definitions */
#define GL_RED_LUMINANCE   (CORE_GL_SUPPORT(ARB_texture_rg) ? GL_RED : GL_LUMINANCE)
#define GL_R8_LUMINANCE8   (CORE_GL_SUPPORT(ARB_texture_rg) ? GL_R8  : GL_LUMINANCE8)
#define GL_R16_LUMINANCE16 (CORE_GL_SUPPORT(ARB_texture_rg) ? GL_R16 : GL_LUMINANCE16)

#define CORE_TEXTURE_SPEC_R8               (coreTextureSpec(GL_R8_LUMINANCE8,                 GL_RED_LUMINANCE,                 GL_UNSIGNED_BYTE,                1u, 1u))   // ARB_texture_rg (wrapped)
#define CORE_TEXTURE_SPEC_RG8              (coreTextureSpec(GL_RG8,                           GL_RG,                            GL_UNSIGNED_BYTE,                2u, 2u))   // ARB_texture_rg
#define CORE_TEXTURE_SPEC_RGB8             (coreTextureSpec(GL_RGB8,                          GL_RGB,                           GL_UNSIGNED_BYTE,                3u, 3u))   // -
#define CORE_TEXTURE_SPEC_RGBA8            (coreTextureSpec(GL_RGBA8,                         GL_RGBA,                          GL_UNSIGNED_BYTE,                4u, 4u))   // -
#define CORE_TEXTURE_SPEC_R16              (coreTextureSpec(GL_R16_LUMINANCE16,               GL_RED_LUMINANCE,                 GL_UNSIGNED_SHORT,               1u, 2u))   // EXT_texture_norm16 ARB_texture_rg (wrapped)
#define CORE_TEXTURE_SPEC_RG16             (coreTextureSpec(GL_RG16,                          GL_RG,                            GL_UNSIGNED_SHORT,               2u, 4u))   // EXT_texture_norm16 ARB_texture_rg
#define CORE_TEXTURE_SPEC_RGB16            (coreTextureSpec(GL_RGB16,                         GL_RGB,                           GL_UNSIGNED_SHORT,               3u, 6u))   // EXT_texture_norm16
#define CORE_TEXTURE_SPEC_RGBA16           (coreTextureSpec(GL_RGBA16,                        GL_RGBA,                          GL_UNSIGNED_SHORT,               4u, 8u))   // EXT_texture_norm16
#define CORE_TEXTURE_SPEC_R16F             (coreTextureSpec(GL_R16F,                          GL_RED,                           GL_HALF_FLOAT,                   1u, 2u))   // ARB_texture_float ARB_texture_rg
#define CORE_TEXTURE_SPEC_RG16F            (coreTextureSpec(GL_RG16F,                         GL_RG,                            GL_HALF_FLOAT,                   2u, 4u))   // ARB_texture_float ARB_texture_rg
#define CORE_TEXTURE_SPEC_RGB16F           (coreTextureSpec(GL_RGB16F,                        GL_RGB,                           GL_HALF_FLOAT,                   3u, 6u))   // ARB_texture_float
#define CORE_TEXTURE_SPEC_RGBA16F          (coreTextureSpec(GL_RGBA16F,                       GL_RGBA,                          GL_HALF_FLOAT,                   4u, 8u))   // ARB_texture_float
#define CORE_TEXTURE_SPEC_RGB10_A2         (coreTextureSpec(GL_RGB10_A2,                      GL_RGBA,                          GL_UNSIGNED_INT_2_10_10_10_REV,  4u, 4u))   // EXT_texture_type_2_10_10_10_rev
#define CORE_TEXTURE_SPEC_R11F_G11F_B10F   (coreTextureSpec(GL_R11F_G11F_B10F,                GL_RGB,                           GL_UNSIGNED_INT_10F_11F_11F_REV, 3u, 4u))   // EXT_packed_float
#define CORE_TEXTURE_SPEC_DEPTH16          (coreTextureSpec(GL_DEPTH_COMPONENT16,             GL_DEPTH_COMPONENT,               GL_UNSIGNED_SHORT,               1u, 2u))   // -
#define CORE_TEXTURE_SPEC_DEPTH24          (coreTextureSpec(GL_DEPTH_COMPONENT24,             GL_DEPTH_COMPONENT,               GL_UNSIGNED_INT,                 1u, 4u))   // -
#define CORE_TEXTURE_SPEC_DEPTH32F         (coreTextureSpec(GL_DEPTH_COMPONENT32F,            GL_DEPTH_COMPONENT,               GL_FLOAT,                        1u, 4u))   // ARB_depth_buffer_float
#define CORE_TEXTURE_SPEC_STENCIL8         (coreTextureSpec(GL_STENCIL_INDEX8,                GL_STENCIL_INDEX,                 GL_UNSIGNED_BYTE,                1u, 1u))   // ARB_texture_stencil8 (as texture)
#define CORE_TEXTURE_SPEC_DEPTH24_STENCIL8 (coreTextureSpec(GL_DEPTH24_STENCIL8,              GL_DEPTH_STENCIL,                 GL_UNSIGNED_INT_24_8,            2u, 4u))   // EXT_packed_depth_stencil
#define CORE_TEXTURE_SPEC_COMPRESSED_RGTC1 (coreTextureSpec(GL_COMPRESSED_RED_RGTC1,          GL_COMPRESSED_RED_RGTC1,          GL_UNSIGNED_BYTE,                1u, 0u))   // ARB_texture_compression_rgtc
#define CORE_TEXTURE_SPEC_COMPRESSED_RGTC2 (coreTextureSpec(GL_COMPRESSED_RG_RGTC2,           GL_COMPRESSED_RG_RGTC2,           GL_UNSIGNED_BYTE,                2u, 0u))   // ARB_texture_compression_rgtc
#define CORE_TEXTURE_SPEC_COMPRESSED_DXT1  (coreTextureSpec(GL_COMPRESSED_RGB_S3TC_DXT1_EXT,  GL_COMPRESSED_RGB_S3TC_DXT1_EXT,  GL_UNSIGNED_BYTE,                3u, 0u))   // EXT_texture_compression_s3tc
#define CORE_TEXTURE_SPEC_COMPRESSED_DXT5  (coreTextureSpec(GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_UNSIGNED_BYTE,                4u, 0u))   // EXT_texture_compression_s3tc
#define CORE_TEXTURE_SPEC_COMPRESSED_BPTC3 (coreTextureSpec(GL_COMPRESSED_RGBA_BPTC_UNORM,    GL_COMPRESSED_RGBA_BPTC_UNORM,    GL_UNSIGNED_BYTE,                3u, 0u))   // ARB_texture_compression_bptc
#define CORE_TEXTURE_SPEC_COMPRESSED_BPTC4 (coreTextureSpec(GL_COMPRESSED_RGBA_BPTC_UNORM,    GL_COMPRESSED_RGBA_BPTC_UNORM,    GL_UNSIGNED_BYTE,                4u, 0u))   // ARB_texture_compression_bptc

#define CORE_TEXTURE_SPEC_COMPONENTS(i)    \
    ((i == 4u) ? CORE_TEXTURE_SPEC_RGBA8 : \
     (i == 3u) ? CORE_TEXTURE_SPEC_RGB8  : \
     (i == 2u) ? CORE_TEXTURE_SPEC_RG8   : \
                 CORE_TEXTURE_SPEC_R8)

#define CORE_TEXTURE_UNITS_2D     (4u)                                                  // number of 2d texture units     (sampler2D)
#define CORE_TEXTURE_UNITS_SHADOW (1u)                                                  // number of shadow texture units (sampler2DShadow)
#define CORE_TEXTURE_UNITS        (CORE_TEXTURE_UNITS_2D + CORE_TEXTURE_UNITS_SHADOW)   // total number of texture units
#define CORE_TEXTURE_SHADOW       (CORE_TEXTURE_UNITS_2D)                               // first shadow texture unit
#define CORE_TEXTURE_LOD_BIAS     (-0.2f)                                               // make all textures a little bit sharper (if possible)

enum coreTextureMode : coreUint8
{
    CORE_TEXTURE_MODE_DEFAULT       = 0x00u,   // do nothing special
    CORE_TEXTURE_MODE_COMPRESS      = 0x01u,   // perform fast color texture compression   (DXT, 1-4 channels)
    CORE_TEXTURE_MODE_COMPRESS_HIGH = 0x03u,   // perform high quality texture compression (BC7, 3-4 channels)
    CORE_TEXTURE_MODE_FILTER        = 0x04u,   // perform anisotropic filtering and mipmapping
    CORE_TEXTURE_MODE_NEAREST       = 0x08u,   // perform nearest texture sampling (instead of linear)
    CORE_TEXTURE_MODE_REPEAT        = 0x10u,   // perform repeating texture sampling (for coordinates outside 0.0 and 1.0)
    CORE_TEXTURE_MODE_TARGET        = 0x20u    // use texture as render target
};
ENABLE_BITWISE(coreTextureMode)

enum coreTextureLoad : coreUint8
{
    CORE_TEXTURE_LOAD_DEFAULT       = 0x00u,   // use default configuration
    CORE_TEXTURE_LOAD_NO_COMPRESS   = 0x01u,   // disable color texture compression
    CORE_TEXTURE_LOAD_NO_FILTER     = 0x02u,   // disable anisotropic filtering and mipmapping
    CORE_TEXTURE_LOAD_NEAREST       = 0x04u,   // enable nearest texture sampling (instead of linear)
    CORE_TEXTURE_LOAD_COMPRESS_HIGH = 0x08u,   // enable high quality texture compression
    CORE_TEXTURE_LOAD_R             = 0x10u,   // convert to R texture (if supported)
    CORE_TEXTURE_LOAD_RG            = 0x20u    // convert to RG texture (if supported)
};
ENABLE_BITWISE(coreTextureLoad)


// ****************************************************************
/* texture specification structure */
struct coreTextureSpec final
{
    GLenum    iInternal;     // internal memory format (e.g. GL_RGBA8)
    GLenum    iFormat;       // pixel data format (e.g. GL_RGBA)
    GLenum    iType;         // pixel data type (e.g. GL_UNSIGNED_BYTE)
    coreUint8 iComponents;   // number of components
    coreUint8 iBytes;        // size per texel (0 = nontrivial/compressed)

    coreTextureSpec() = default;
    constexpr coreTextureSpec(const GLenum iInternal, const GLenum iFormat, const GLenum iType, const coreUint8 iComponents, const coreUint8 iBytes)noexcept;
};


// ****************************************************************
/* texture class */
class coreTexture final : public coreResource
{
private:
    GLuint m_iIdentifier;                                // texture identifier

    coreVector2 m_vResolution;                           // resolution of the base level
    coreUint8   m_iLevels;                               // number of texture levels
    coreUint8   m_iCompressed;                           // compression status (0 = no compression | 1 = fast | 2 = high quality)

    coreTextureMode m_eMode;                             // texture mode (sampling)
    coreTextureSpec m_Spec;                              // texture specification (format)

    coreTextureLoad m_eLoad;                             // resource load configuration

    coreSync m_Sync;                                     // sync object for asynchronous texture loading

    static coreUintW    s_iActiveUnit;                   // active texture unit
    static coreTexture* s_apBound[CORE_TEXTURE_UNITS];   // texture objects currently associated with texture units


public:
    explicit coreTexture(const coreTextureLoad eLoad = CORE_TEXTURE_LOAD_DEFAULT)noexcept;
    ~coreTexture()final;

    DISABLE_COPY(coreTexture)

    /* load and unload texture resource data */
    coreStatus Load(coreFile* pFile)final;
    coreStatus Unload()final;

    /* handle texture memory */
    void Create(const coreUint32 iWidth, const coreUint32 iHeight, const coreTextureSpec& oSpec, const coreTextureMode eMode);
    void Modify(const coreUint32 iOffsetX, const coreUint32 iOffsetY, const coreUint32 iWidth, const coreUint32 iHeight, const coreUint32 iDataSize, const coreByte* pData);
    void CopyFrameBuffer(const coreUint32 iSrcX, const coreUint32 iSrcY, const coreUint32 iDstX, const coreUint32 iDstY, const coreUint32 iWidth, const coreUint32 iHeight);
    void CopyFrameBuffer();

    /* handle texture levels and images */
    coreStatus BindImage(const coreUintW iUnit, const coreUint8 iLevel, const GLenum iAccess);
    void       ReadImage(const coreUint8 iLevel, const coreUint32 iDataSize, coreByte* OUTPUT pData);
    coreStatus CopyImage(coreTexture* OUTPUT pDestination, const coreUint8 iSrcLevel, const coreUint32 iSrcX, const coreUint32 iSrcY, const coreUint8 iDstLevel, const coreUint32 iDstX, const coreUint32 iDstY, const coreUint32 iWidth, const coreUint32 iHeight)const;
    coreStatus CopyImage(coreTexture* OUTPUT pDestination)const;

    /* configure shadow sampling */
    void EnableShadowSampling();

    /* enable and disable the texture */
    inline        void Enable    (const coreUintW iUnit) {coreTexture::__BindTexture(iUnit, this); ASSERT(m_iIdentifier)}
    static inline void Disable   (const coreUintW iUnit) {coreTexture::__BindTexture(iUnit, NULL);}
    static        void EnableAll (const coreResourcePtr<coreTexture>* ppTextureArray);
    static        void DisableAll();

    /* reset content of the texture */
    void       Clear     (const coreUint8 iLevel);
    coreStatus Invalidate(const coreUint8 iLevel);

    /* get object properties */
    inline const GLuint&          GetIdentifier()const {return m_iIdentifier;}
    inline const coreVector2&     GetResolution()const {return m_vResolution;}
    inline const coreUint8&       GetLevels    ()const {return m_iLevels;}
    inline const coreUint8&       GetCompressed()const {return m_iCompressed;}
    inline const coreTextureMode& GetMode      ()const {return m_eMode;}
    inline const coreTextureSpec& GetSpec      ()const {return m_Spec;}

    /* process and convert image data */
    static void         CreateNextLevel    (const coreUintW iInWidth, const coreUintW iInHeight, const coreUintW iComponents, const coreByte* pInput, coreByte* OUTPUT pOutput);
    static void         CreateCompressedDXT(const coreUintW iInWidth, const coreUintW iInHeight, const coreUintW iComponents, const coreByte* pInput, coreByte* OUTPUT pOutput);
    static void         CreateCompressedBC7(const coreUintW iInWidth, const coreUintW iInHeight, const coreUintW iComponents, const coreByte* pInput, coreByte* OUTPUT pOutput);
    static SDL_Surface* CreateReduction    (const coreUintW iComponents, const SDL_Surface* pInput, SDL_Surface* OUTPUT pOutput = NULL);


private:
    /* bind texture to texture unit */
    static void __BindTexture(const coreUintW iUnit, coreTexture* pTexture);
};


// ****************************************************************
/* texture resource access type */
using coreTexturePtr = coreResourcePtr<coreTexture>;


// ****************************************************************
/* volume texture class */
class coreTextureVolume final
{
private:
    GLuint m_iIdentifier;        // texture identifier

    coreVector3 m_vResolution;   // resolution of the base level

    coreTextureMode m_eMode;     // texture mode (sampling)
    coreTextureSpec m_Spec;      // texture specification (format)


public:
    coreTextureVolume()noexcept;
    ~coreTextureVolume();

    DISABLE_COPY(coreTextureVolume)

    /* handle texture memory */
    coreStatus Create(const coreUint32 iWidth, const coreUint32 iHeight, const coreUint32 iDepth, const coreTextureSpec& oSpec, const coreTextureMode eMode);
    void       Modify(const coreUint32 iOffsetX, const coreUint32 iOffsetY, const coreUint32 iOffsetZ, const coreUint32 iWidth, const coreUint32 iHeight, const coreUint32 iDepth, const coreUint32 iDataSize, const coreByte* pData);
    void       Delete();

    /* get object properties */
    inline const GLuint&          GetIdentifier()const {return m_iIdentifier;}
    inline const coreVector3&     GetResolution()const {return m_vResolution;}
    inline const coreTextureMode& GetMode      ()const {return m_eMode;}
    inline const coreTextureSpec& GetSpec      ()const {return m_Spec;}
};


// ****************************************************************
/* cube texture class */
class coreTextureCube final
{
private:
    GLuint m_iIdentifier;        // texture identifier

    coreVector2 m_vResolution;   // resolution of the base level

    coreTextureMode m_eMode;     // texture mode (sampling)
    coreTextureSpec m_Spec;      // texture specification (format)


public:
    coreTextureCube()noexcept;
    ~coreTextureCube();

    DISABLE_COPY(coreTextureCube)

    /* handle texture memory */
    coreStatus Create(const coreUint32 iWidth, const coreUint32 iHeight, const coreTextureSpec& oSpec, const coreTextureMode eMode);
    void       Modify(const coreUint32 iOffsetX, const coreUint32 iOffsetY, const coreUint32 iWidth, const coreUint32 iHeight, const coreUint8 iFace, const coreUint32 iDataSize, const coreByte* pData);
    void       Delete();

    /* get object properties */
    inline const GLuint&          GetIdentifier()const {return m_iIdentifier;}
    inline const coreVector2&     GetResolution()const {return m_vResolution;}
    inline const coreTextureMode& GetMode      ()const {return m_eMode;}
    inline const coreTextureSpec& GetSpec      ()const {return m_Spec;}
};


// ****************************************************************
/* constructor */
constexpr coreTextureSpec::coreTextureSpec(const GLenum iInternal, const GLenum iFormat, const GLenum iType, const coreUint8 iComponents, const coreUint8 iBytes)noexcept
: iInternal   (iInternal)
, iFormat     (iFormat)
, iType       (iType)
, iComponents (iComponents)
, iBytes      (iBytes)
{
}


#endif /* _CORE_GUARD_TEXTURE_H_ */