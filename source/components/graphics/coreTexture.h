//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_TEXTURE_H_
#define _CORE_GUARD_TEXTURE_H_

// TODO: check for max available texture units (only at start?)
// TODO: implement sampler objects
// TODO: implement light unbind (like in model and program)
// TODO: add option/config for trilinear filtering
// TODO: load, check proper use of PBO, maybe implement static buffer(s!) -> PBO seems to work, but not with automatic mipmap generation (stalls there)
// TODO: load, check performance of 24bit formats, mind texture alignment of 4 (also for frame buffers and labels)
// TODO: last few compressed mipmap levels contain only garbage
// TODO: replace box filter in mipmap generation with better filter (e.g. gaussian, lanczos), check for border (repeat or clamp depending on sampling)
// TODO: specialized mipmap generation for normal maps
// TODO: use DXT5 for normal maps (g and a channel (max precision) for x and y -> reconstruct z in shader)
// TODO: check out AMD compress library, when other formats are required (e.g. for Android)


// ****************************************************************
// texture definitions
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define CORE_TEXTURE_MASK 0xFF000000u, 0x00FF0000u, 0x0000FF00u, 0x000000FFu
#else
    #define CORE_TEXTURE_MASK 0x000000FFu, 0x0000FF00u, 0x00FF0000u, 0xFF000000u
#endif

#define GL_RED_R 0x1903
#undef  GL_RED
#define GL_RED (CORE_GL_SUPPORT(ARB_texture_rg) ? GL_RED_R : GL_LUMINANCE)

#define CORE_TEXTURE_SPEC_R             (coreTextureSpec(GL_R8,                GL_RED,             GL_UNSIGNED_BYTE))
#define CORE_TEXTURE_SPEC_RG            (coreTextureSpec(GL_RG8,               GL_RG,              GL_UNSIGNED_BYTE))
#define CORE_TEXTURE_SPEC_RGB           (coreTextureSpec(GL_RGB8,              GL_RGB,             GL_UNSIGNED_BYTE))
#define CORE_TEXTURE_SPEC_RGBA          (coreTextureSpec(GL_RGBA8,             GL_RGBA,            GL_UNSIGNED_BYTE))
#define CORE_TEXTURE_SPEC_R_16F         (coreTextureSpec(GL_R16F,              GL_RED,             GL_HALF_FLOAT))
#define CORE_TEXTURE_SPEC_RG_16F        (coreTextureSpec(GL_RG16F,             GL_RG,              GL_HALF_FLOAT))
#define CORE_TEXTURE_SPEC_RGB_16F       (coreTextureSpec(GL_RGB16F,            GL_RGB,             GL_HALF_FLOAT))
#define CORE_TEXTURE_SPEC_RGBA_16F      (coreTextureSpec(GL_RGBA16F,           GL_RGBA,            GL_HALF_FLOAT))
#define CORE_TEXTURE_SPEC_DEPTH         (coreTextureSpec(GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT))
#define CORE_TEXTURE_SPEC_STENCIL       (coreTextureSpec(GL_STENCIL_INDEX8,    GL_STENCIL_INDEX,   GL_UNSIGNED_BYTE))
#define CORE_TEXTURE_SPEC_DEPTH_STENCIL (coreTextureSpec(GL_DEPTH24_STENCIL8,  GL_DEPTH_STENCIL,   GL_UNSIGNED_INT_24_8))

#define CORE_TEXTURE_SPEC_COMPONENTS(i)   \
    ((i == 4u) ? CORE_TEXTURE_SPEC_RGBA : \
     (i == 3u) ? CORE_TEXTURE_SPEC_RGB  : \
     (i == 2u) ? CORE_TEXTURE_SPEC_RG   : \
                 CORE_TEXTURE_SPEC_R)

#define CORE_TEXTURE_UNITS_2D     (4u)                                                //!< number of 2d texture units (sampler2D)
#define CORE_TEXTURE_UNITS_SHADOW (1u)                                                //!< number of shadow texture units (sampler2DShadow)
#define CORE_TEXTURE_UNITS        (CORE_TEXTURE_UNITS_2D+CORE_TEXTURE_UNITS_SHADOW)   //!< total number of texture units
#define CORE_TEXTURE_SHADOW       (CORE_TEXTURE_UNITS_2D)                             //!< first shadow texture unit

enum coreTextureMode : coreUint8
{
    CORE_TEXTURE_MODE_DEFAULT  = 0x00u,   //!< do nothing special
    CORE_TEXTURE_MODE_COMPRESS = 0x01u,   //!< perform color texture compression
    CORE_TEXTURE_MODE_FILTER   = 0x02u,   //!< perform anisotropic filtering and mipmapping
    CORE_TEXTURE_MODE_REPEAT   = 0x04u    //!< perform repeating texture sampling
};
ENABLE_BITWISE(coreTextureMode)


// ****************************************************************
// texture specification structure
struct coreTextureSpec
{
    GLenum iInternal;   //!< internal memory format (e.g. GL_RGBA8)
    GLenum iFormat;     //!< pixel data format (e.g. GL_RGBA)
    GLenum iType;       //!< pixel data type (e.g. GL_UNSIGNED_BYTE)

    coreTextureSpec() = default;
    constexpr_func coreTextureSpec(const GLenum& iInternal, const GLenum& iFormat, const GLenum& iType)noexcept;
};


// ****************************************************************
// texture class
class coreTexture final : public coreResource
{
private:
    GLuint m_iTexture;                                   //!< texture identifier

    coreVector2 m_vResolution;                           //!< resolution of the base level
    coreUint8   m_iLevels;                               //!< number of texture levels
    coreInt8    m_iCompressed;                           //!< compression status

    coreTextureSpec m_Spec;                              //!< texture specification (format)
    coreTextureMode m_iMode;                             //!< texture mode (sampling)

    coreSync m_Sync;                                     //!< sync object for asynchronous texture loading

    static coreUintW    s_iActiveUnit;                   //!< active texture unit
    static coreTexture* s_apBound[CORE_TEXTURE_UNITS];   //!< texture objects currently associated with texture units


public:
    coreTexture()noexcept;
    explicit coreTexture(const coreBool& bLoadCompressed)noexcept;
    ~coreTexture();

    DISABLE_COPY(coreTexture)

    //! load and unload texture resource data
    //! @{
    coreStatus Load(coreFile* pFile)override;
    coreStatus Unload()override;
    //! @}

    //! handle texture memory
    //! @{
    void Create(const coreUint32& iWidth, const coreUint32& iHeight, const coreTextureSpec& oSpec, const coreTextureMode& iMode);
    void Modify(const coreUint32& iOffsetX, const coreUint32& iOffsetY, const coreUint32& iWidth, const coreUint32& iHeight, const coreUint32& iDataSize, const coreByte* pData);
    void CopyFrameBuffer(const coreUint32& iSrcX, const coreUint32& iSrcY, const coreUint32& iDstX, const coreUint32& iDstY, const coreUint32& iWidth, const coreUint32& iHeight);
    //! @}

    //! configure shadow sampling
    //! @{
    void ShadowSampling(const coreBool& bStatus);
    //! @}

    //! enable and disable the texture
    //! @{
    inline        void Enable (const coreUintW& iUnit) {coreTexture::__BindTexture(iUnit, this); ASSERT(m_iTexture)}
    static inline void Disable(const coreUintW& iUnit) {coreTexture::__BindTexture(iUnit, NULL);}
    static inline void DisableAll()                    {for(coreUintW i = CORE_TEXTURE_UNITS; i--; ) coreTexture::Disable(i);}
    //! @}

    //! reset content of the texture
    //! @{
    void Clear     (const coreUint8& iLevel);
    void Invalidate(const coreUint8& iLevel);
    //! @}

    //! process and convert image data
    //! @{
    static void CreateNextLevel (const coreUintW& iInWidth, const coreUintW& iInHeight, const coreUintW& iComponents, const coreByte* pInput, coreByte* OUTPUT pOutput);
    static void CreateCompressed(const coreUintW& iInWidth, const coreUintW& iInHeight, const coreUintW& iComponents, const coreByte* pInput, coreByte* OUTPUT pOutput);
    //! @}

    //! get object properties
    //! @{
    inline const GLuint&          GetTexture   ()const {return m_iTexture;}
    inline const coreVector2&     GetResolution()const {return m_vResolution;}
    inline const coreUint8&       GetLevels    ()const {return m_iLevels;}
    inline const coreTextureSpec& GetSpec      ()const {return m_Spec;}
    inline const coreTextureMode& GetMode      ()const {return m_iMode;}
    //! @}


private:
    //! bind texture to texture unit
    //! @{
    static void __BindTexture(const coreUintW& iUnit, coreTexture* pTexture);
    //! @}
};


// ****************************************************************
// texture resource access type
using coreTexturePtr = coreResourcePtr<coreTexture>;


// ****************************************************************
// constructor
constexpr_func coreTextureSpec::coreTextureSpec(const GLenum& iInternal, const GLenum& iFormat, const GLenum& iType)noexcept
: iInternal (iInternal)
, iFormat   (iFormat)
, iType     (iType)
{
}


#endif // _CORE_GUARD_TEXTURE_H_