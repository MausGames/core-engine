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
// TODO: load, check proper use of PBO, maybe implement static buffer(s!)
// TODO: load, allow 1-channel textures (GLES uses GL_ALPHA/GL_LUMINANCE, GL uses GL_RED/GL_DEPTH_COMPONENT ?)
// TODO: load, check performance of 24bit formats, mind texture alignment of 4 (also for frame buffers and labels)
// TODO: load, implement texture-compressions


// ****************************************************************
// texture definitions
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define CORE_TEXTURE_MASK 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
#else
    #define CORE_TEXTURE_MASK 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
#endif

#define CORE_TEXTURE_SPEC_RGB           GL_RGB8,              GL_RGB,             GL_UNSIGNED_BYTE
#define CORE_TEXTURE_SPEC_RGBA          GL_RGBA8,             GL_RGBA,            GL_UNSIGNED_BYTE
#define CORE_TEXTURE_SPEC_DEPTH         GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT
#define CORE_TEXTURE_SPEC_STENCIL       GL_STENCIL_INDEX8,    GL_STENCIL_INDEX,   GL_UNSIGNED_BYTE
#define CORE_TEXTURE_SPEC_DEPTH_STENCIL GL_DEPTH24_STENCIL8,  GL_DEPTH_STENCIL,   GL_UNSIGNED_INT_24_8

#define CORE_TEXTURE_UNITS_2D     (4u)                                                //!< number of 2d texture units (sampler2D)
#define CORE_TEXTURE_UNITS_SHADOW (1u)                                                //!< number of shadow texture units (sampler2DShadow)
#define CORE_TEXTURE_UNITS        (CORE_TEXTURE_UNITS_2D+CORE_TEXTURE_UNITS_SHADOW)   //!< total number of texture units
#define CORE_TEXTURE_SHADOW       (CORE_TEXTURE_UNITS_2D)                             //!< first shadow texture unit


// ****************************************************************
// texture class
class coreTexture final : public coreResource
{
private:
    GLuint m_iTexture;                                   //!< texture identifier

    coreVector2 m_vResolution;                           //!< resolution of the base level
    coreByte    m_iLevels;                               //!< number of texture levels

    GLenum m_iInternal;                                  //!< internal memory format (e.g. GL_RGBA8)
    GLenum m_iFormat;                                    //!< pixel data format (e.g. GL_RGBA)
    GLenum m_iType;                                      //!< pixel data type (e.g. GL_UNSIGNED_BYTE)

    coreSync m_Sync;                                     //!< sync object for asynchronous texture loading

    static int s_iActiveUnit;                            //!< active texture unit
    static coreTexture* s_apBound[CORE_TEXTURE_UNITS];   //!< texture objects currently associated with texture units


public:
    coreTexture()noexcept;
    ~coreTexture();

    DISABLE_COPY(coreTexture)

    //! load and unload texture resource data
    //! @{
    coreError Load(coreFile* pFile)override;
    coreError Unload()override;
    //! @}

    //! handle texture memory
    //! @{
    void Create(const coreUint& iWidth, const coreUint& iHeight, const GLenum& iInternal, const GLenum& iFormat, const GLenum& iType, const GLenum& iWrapMode, const bool& bFilter);
    void Modify(const coreUint& iOffsetX, const coreUint& iOffsetY, const coreUint& iWidth, const coreUint& iHeight, const coreUint& iDataSize, const void* pData);
    //! @}

    //! configure shadow sampling
    //! @{
    void ShadowSampling(const bool& bStatus);
    //! @}

    //! enable and disable the texture
    //! @{
    inline        void Enable (const coreByte& iUnit) {coreTexture::__BindTexture(iUnit, this);}
    static inline void Disable(const coreByte& iUnit) {coreTexture::__BindTexture(iUnit, NULL);}
    static inline void DisableAll()                   {for(int i = CORE_TEXTURE_UNITS-1; i >= 0; --i) coreTexture::Disable(i);}
    //! @}

    //! reset content of the texture
    //! @{
    void Clear     (const GLint& iLevel, const GLenum& iFormat, const GLenum& iType, const void* pData);
    void Invalidate(const GLint& iLevel);
    //! @}

    //! get object properties
    //! @{
    inline const GLuint&      GetTexture   ()const {return m_iTexture;}
    inline const coreVector2& GetResolution()const {return m_vResolution;}
    inline const coreByte&    GetLevels    ()const {return m_iLevels;}
    inline const GLenum&      GetInternal  ()const {return m_iInternal;}
    inline const GLenum&      GetFormat    ()const {return m_iFormat;}
    inline const GLenum&      GetType      ()const {return m_iType;}
    //! @}


private:
    //! bind texture to texture unit
    //! @{
    static void __BindTexture(const coreByte& iUnit, coreTexture* pTexture);
    //! @}
};


// ****************************************************************
// texture resource access type
typedef coreResourcePtr<coreTexture> coreTexturePtr;


#endif // _CORE_GUARD_TEXTURE_H_