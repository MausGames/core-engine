#pragma once
// TODO: compile SDL_image with png-support


// ****************************************************************
// texture object class
class coreTexture : public coreResource
{
private:
    GLuint m_iID;                         // texture memory ID
                                          
    coreVector2 m_vResolution;            // texture resolution
    coreUint m_iSize;                     // texture size in byte
                                          
    static int s_iCurUnit;                // current active texture unit
    static coreTexture* s_apActive[32];   // pointer to active textures
    

public:
    coreTexture(const bool& bGenerate);
    coreTexture(const char* pcPath);
    coreTexture(coreFile* pFile);
    ~coreTexture();

    // load and unload resource data
    void Load(const char* pcPath);
    void Load(coreFile* pFile);
    void Unload();

    // enable and disable the texture
    void Enable(const coreByte& iUnit);
    void Disable(const coreByte& iUnit);

    // get attributes
    inline const GLuint& GetID()const              {return m_iID;}
    inline const coreVector2& GetResolution()const {return m_vResolution;}
    inline const coreUint& GetSize()const          {return m_iSize;}

    // get relative path to NULL resource (required for resource management)
    static const char* GetNullPath() {return "data/textures/default_white.tga";}
};