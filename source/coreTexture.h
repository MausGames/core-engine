//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
// TODO: compile SDL_image with png-support
// TODO: use libjpeg-turbo instead of the normal libjpeg
// TODO: check for max available texture units


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
class coreTexture final : public coreResource
{
private:
    GLuint m_iID;                                        // texture identifier/OpenGL name                                     
    coreVector2 m_vResolution;                           // texture resolution
                                                         
    static int s_iActiveUnit;                            // current active texture unit
    static coreTexture* s_apBound[CORE_TEXTURE_UNITS];   // texture objects currently associated with texture units

    GLsync m_pSync;                                      // sync object for asynchronous texture loading
    static SDL_SpinLock s_iLock;                         // spinlock to prevent asynchronous texture unit access


public:
    coreTexture(const bool bGenerate = false);
    coreTexture(const char* pcPath);
    coreTexture(coreFile* pFile);
    ~coreTexture();

    // load and unload texture resource data
    coreError Load(coreFile* pFile)override;
    coreError Unload()override;

    // enable and disable the texture
    void Enable(const coreByte& iUnit);
    static void Disable(const coreByte& iUnit);
    static void DisableAll();

    // check sync object status
    coreError CheckSync(); 

    // get attributes
    inline const GLuint& GetID()const              {return m_iID;}
    inline const coreVector2& GetResolution()const {return m_vResolution;}

    // get relative path to NULL resource
    static inline const char* GetNullPath() {return "data/textures/cursor_diff.tga";}
};


// ****************************************************************
// texture resource access type
typedef coreResourcePtr<coreTexture> coreTexturePtr;