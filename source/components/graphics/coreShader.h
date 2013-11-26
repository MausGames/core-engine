//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SHADER_H_
#define _CORE_GUARD_SHADER_H_


// ****************************************************************
// shader definitions
// TODO: document definitions
#define CORE_SHADER_BUFFER_GLOBAL              "b_Global"
#define CORE_SHADER_BUFFER_GLOBAL_NUM          0

#define CORE_SHADER_UNIFORM_LIGHT_POSITION(i)  coreData::Print("u_asLight[%d].v3Position",  i)
#define CORE_SHADER_UNIFORM_LIGHT_DIRECTION(i) coreData::Print("u_asLight[%d].v3Direction", i)
#define CORE_SHADER_UNIFORM_LIGHT_RANGE(i)     coreData::Print("u_asLight[%d].fRange",      i)
#define CORE_SHADER_UNIFORM_LIGHT_VALUE(i)     coreData::Print("u_asLight[%d].v4Value",     i)

#define CORE_SHADER_UNIFORM_3D_MODELVIEW       "u_m4ModelView"
#define CORE_SHADER_UNIFORM_3D_MODELVIEWPROJ   "u_m4ModelViewProj"
#define CORE_SHADER_UNIFORM_3D_NORMAL          "u_m3Normal"
#define CORE_SHADER_UNIFORM_2D_SCREENVIEW      "u_m3ScreenView"
#define CORE_SHADER_UNIFORM_COLOR              "u_v4Color"
#define CORE_SHADER_UNIFORM_TEXSIZE            "u_v2TexSize"
#define CORE_SHADER_UNIFORM_TEXOFFSET          "u_v2TexOffset"
#define CORE_SHADER_UNIFORM_TEXTURE(i)         coreData::Print("u_s2Texture[%d]", i)
                                               
#define CORE_SHADER_ATTRIBUTE_POSITION         "a_v3Position"
#define CORE_SHADER_ATTRIBUTE_TEXTURE          "a_v2Texture"
#define CORE_SHADER_ATTRIBUTE_NORMAL           "a_v3Normal"
#define CORE_SHADER_ATTRIBUTE_TANGENT          "a_v4Tangent"
#define CORE_SHADER_ATTRIBUTE_POSITION_NUM     0
#define CORE_SHADER_ATTRIBUTE_TEXTURE_NUM      1
#define CORE_SHADER_ATTRIBUTE_NORMAL_NUM       2
#define CORE_SHADER_ATTRIBUTE_TANGENT_NUM      3

#define CORE_SHADER_OUTPUT_COLOR(i)            coreData::Print("o_v4Color%d", i)
#define CORE_SHADER_OUTPUT_COLORS              4   


// ****************************************************************
// shader class
class coreShader final : public coreResource
{
private:
    GLuint m_iShader;                   //!< shader identifier/OpenGL name
    static std::string s_asGlobal[3];   //!< global shader data (0 = defines | 1 = vertex | 2 = fragment)


public:
    coreShader()noexcept;
    explicit coreShader(const char* pcPath)noexcept;
    explicit coreShader(coreFile* pFile)noexcept;
    ~coreShader();

    //! load and unload shader resource data
    //! @{
    coreError Load(coreFile* pFile)override;
    coreError Unload()override;
    //! @}

    //! get object attributes
    //! @{
    inline const GLuint& GetShader()const {return m_iShader;}
    //! @}

    //! get relative path to default resource
    //! @{
    static inline const char* GetDefaultPath() {return "data/shaders/default.fs";}
    //! @}


private:
    //! init global shader data
    //! @{
    static void __InitGlobal();
    //! @}
};


// ****************************************************************
// shader resource access type
typedef coreResourcePtr<coreShader> coreShaderPtr;


// ****************************************************************
// shader-program class
// TODO: material data in UBO
// TODO: use variable template with uniform cache (C++14)
class coreProgram final : public coreReset
{
private:
    GLuint m_iProgram;                       //!< shader-program identifier/OpenGL name

    std::vector<coreShaderPtr> m_apShader;   //!< attached shader objects
    int m_iStatus;                           //!< current status (0 = new | 1 = ready for linking | 2 = successfully linked | 3 = texture units bound)

    coreLookup<int> m_aiUniform;             //!< identifiers for uniform variables
    coreLookup<int> m_aiAttribute;           //!< identifiers for attribute variables
    coreLookup<coreVector4> m_avCache;       //!< cached uniform variable values

    static coreProgram* s_pCurrent;          //!< currently active shader-program


public:
    coreProgram()noexcept;
    ~coreProgram();

    //! init and exit the shader-program
    //! @{
    coreError Init();
    coreError Exit();
    //! @}

    //! enable and disable the shader-program
    //! @{
    bool Enable();
    static void Disable();
    //! @}

    //! attach and link shader objects
    //! @{
    coreProgram* AttachShaderFile(const char* pcPath);
    coreProgram* AttachShaderLink(const char* pcName);
    inline void Finish()          {if(m_iStatus) return; m_iStatus = 1; this->Init();}
    inline bool IsFinished()const {return (m_iStatus >= 2) ? true : false;}
    //! @}

    //! set uniform variables
    //! @{
    inline void SetUniform(const char* pcName, const int& iInt)                                    {const int iID = this->CheckCache(pcName, coreVector4(float(iInt), 0.0f, 0.0f, 0.0f)); if(iID >= 0) glUniform1i (iID, iInt);}
    inline void SetUniform(const char* pcName, const float& fFloat)                                {const int iID = this->CheckCache(pcName, coreVector4(fFloat,      0.0f, 0.0f, 0.0f)); if(iID >= 0) glUniform1f (iID, fFloat);}
    inline void SetUniform(const char* pcName, const coreVector2& vVector)                         {const int iID = this->CheckCache(pcName, coreVector4(vVector,     0.0f, 0.0f));       if(iID >= 0) glUniform2fv(iID, 1, vVector);}
    inline void SetUniform(const char* pcName, const coreVector3& vVector)                         {const int iID = this->CheckCache(pcName, coreVector4(vVector,     0.0f));             if(iID >= 0) glUniform3fv(iID, 1, vVector);}
    inline void SetUniform(const char* pcName, const coreVector4& vVector)                         {const int iID = this->CheckCache(pcName, vVector);                                    if(iID >= 0) glUniform4fv(iID, 1, vVector);}
    inline void SetUniform(const char* pcName, const coreMatrix3& mMatrix, const bool& bTranspose) {glUniformMatrix3fv(this->GetUniform(pcName), 1, bTranspose, mMatrix);}
    inline void SetUniform(const char* pcName, const coreMatrix4& mMatrix, const bool& bTranspose) {glUniformMatrix4fv(this->GetUniform(pcName), 1, bTranspose, mMatrix);}
    //! @}

    //! check for cached uniform variables
    //! @{
    inline int CheckCache(const char* pcName, const coreVector4& vVector) {if(m_avCache.count(pcName)) {if(m_avCache.at(pcName) == vVector) return -1;} m_avCache[pcName] = vVector; return this->GetUniform(pcName);}
    //! @}

    //! get object attributes
    //! @{
    inline const GLuint& GetProgram()const             {return m_iProgram;}
    inline const int& GetUniform(const char* pcName)   {if(!m_aiUniform.count(pcName))   {SDL_assert(this->IsFinished() && s_pCurrent == this); m_aiUniform[pcName]   = glGetUniformLocation(m_iProgram, pcName);} return m_aiUniform.at(pcName);}
    inline const int& GetAttribute(const char* pcName) {if(!m_aiAttribute.count(pcName)) {SDL_assert(this->IsFinished() && s_pCurrent == this); m_aiAttribute[pcName] = glGetAttribLocation (m_iProgram, pcName);} return m_aiAttribute.at(pcName);}
    //! @}

    //! get currently active shader-program
    //! @{
    static inline coreProgram* GetCurrent() {return s_pCurrent;}
    //! @}


private:
    //! reset with the resource manager
    //! @{
    void __Reset(const bool& bInit)override;
    //! @}
};


// ****************************************************************
// shader-program shared memory type
typedef std::shared_ptr<coreProgram> coreProgramShr;


#endif // _CORE_GUARD_SHADER_H_