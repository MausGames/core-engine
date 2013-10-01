//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_TEXTURE_H_
#define _CORE_GUARD_TEXTURE_H_


// ****************************************************************
// texture definitions
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    #define CORE_TEXTURE_MASK 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
#else
    #define CORE_TEXTURE_MASK 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000
#endif

#define CORE_TEXTURE_UNITS 32


// ****************************************************************
// texture object class
// TODO: check for max available texture units
class coreTexture final : public coreResource
{
private:
    GLuint m_iTexture;                                   //!< texture identifier/OpenGL name
    coreVector2 m_vResolution;                           //!< texture resolution

    static int s_iActiveUnit;                            //!< current active texture unit
    static coreTexture* s_apBound[CORE_TEXTURE_UNITS];   //!< texture objects currently associated with texture units

    GLsync m_pSync;                                      //!< sync object for asynchronous texture loading
    static SDL_SpinLock s_iLock;                         //!< spinlock to prevent asynchronous texture unit access


public:
    coreTexture();
    explicit coreTexture(const char* pcPath);
    explicit coreTexture(coreFile* pFile);
    ~coreTexture();

    //! load and unload texture resource data
    //! @{
    coreError Load(coreFile* pFile)override;
    coreError Unload()override;
    //! @}

    //! enable and disable the texture
    //! @{
    void Enable(const coreByte& iUnit);
    static void Disable(const coreByte& iUnit);
    static void DisableAll();
    //! @}

    //! check sync object status
    //! @{
    coreError CheckSync();
    //! @}

    //! generate empty base texture
    //! @{
    inline void Generate() {SDL_assert(!m_iTexture); if(!m_iTexture) glGenTextures(1, &m_iTexture);}
    //! @}

    //! get attributes
    inline const GLuint& GetTexture()const         {return m_iTexture;}
    inline const coreVector2& GetResolution()const {return m_vResolution;}
    //! @}

    //! get relative path to NULL resource
    //! @{
    static inline const char* GetNullPath() {return "data/textures/default.png";}
    //! @}
};


// ****************************************************************
// texture resource access type
typedef coreResourcePtr<coreTexture> coreTexturePtr;


#endif // _CORE_GUARD_TEXTURE_H_