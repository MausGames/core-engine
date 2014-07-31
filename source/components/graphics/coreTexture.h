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
class coreTexture final : public coreResource
{
private:
    GLuint m_iTexture;                                   //!< texture identifier
    coreVector2 m_vResolution;                           //!< texture resolution

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

    //! enable and disable the texture
    //! @{
    inline        void Enable (const coreByte& iUnit) {coreTexture::__BindTexture(iUnit, this);}
    static inline void Disable(const coreByte& iUnit) {coreTexture::__BindTexture(iUnit, NULL);}
    static inline void DisableAll()                   {for(int i = CORE_TEXTURE_UNITS-1; i >= 0; --i) coreTexture::Disable(i);}
    //! @}

    //! generate empty base texture
    //! @{
    inline void Generate() {ASSERT(!m_iTexture) glGenTextures(1, &m_iTexture);}
    //! @}

    //! get object properties
    //! @{
    inline const GLuint&      GetTexture   ()const {return m_iTexture;}
    inline const coreVector2& GetResolution()const {return m_vResolution;}
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