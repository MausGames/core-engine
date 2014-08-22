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


// ****************************************************************
// texture definitions
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define CORE_TEXTURE_MASK 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
#else
    #define CORE_TEXTURE_MASK 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
#endif

#define CORE_TEXTURE_UNITS (4u)   //!< number of texture units


// ****************************************************************
// texture class
// TODO: check for max available texture units (only at start?)
// TODO: implement sampler objects
// TODO: implement light unbind (like in model and program)
class coreTexture final : public coreResource
{
private:
    GLuint m_iTexture;                                   //!< texture identifier

    coreVector2 m_vResolution;                           //!< resolution of the base level
    coreByte    m_iLevels;                               //!< number of texture levels
    GLenum      m_iFormat;                               //!< pixel data format (e.g. GL_RGBA)
    GLenum      m_iType;                                 //!< pixel data type (e.g. GL_UNSIGNED_BYTE)

    coreSync m_Sync;                                     //!< sync object for asynchronous texture loading

    static int s_iActiveUnit;                            //!< active texture unit
    static coreTexture* s_apBound[CORE_TEXTURE_UNITS];   //!< texture objects currently associated with texture units


public:
    coreTexture()noexcept;
    ~coreTexture();

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

    //! enable and disable the texture
    //! @{
    inline        void Enable (const coreByte& iUnit) {coreTexture::__BindTexture(iUnit, this);}
    static inline void Disable(const coreByte& iUnit) {coreTexture::__BindTexture(iUnit, NULL);}
    static inline void DisableAll()                   {for(int i = CORE_TEXTURE_UNITS-1; i >= 0; --i) coreTexture::Disable(i);}
    //! @}

    //! get object properties
    //! @{
    inline const GLuint&      GetTexture   ()const {return m_iTexture;}
    inline const coreVector2& GetResolution()const {return m_vResolution;}
    inline const coreByte&    GetLevels    ()const {return m_iLevels;}
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