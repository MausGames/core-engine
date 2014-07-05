//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SHADER_H_
#define _CORE_GUARD_SHADER_H_


// ****************************************************************
// shader definitions
// TODO: document definitions
// TODO: implement standard time uniform (ID?)
// TODO: increase number-performance with constexpr-array int-to-char or char+# (everywhere)
#define CORE_SHADER_BUFFER_GLOBAL              "b_Global"
#define CORE_SHADER_BUFFER_GLOBAL_NUM          0

#define CORE_SHADER_UNIFORM_VIEWPROJ           "u_m4ViewProj"
#define CORE_SHADER_UNIFORM_CAMERA             "u_m4Camera"
#define CORE_SHADER_UNIFORM_PERSPECTIVE        "u_m4Perspective"
#define CORE_SHADER_UNIFORM_ORTHO              "u_m4Ortho"
#define CORE_SHADER_UNIFORM_RESOLUTION         "u_v4Resolution"

#define CORE_SHADER_UNIFORM_LIGHT_POSITION(i)  PRINT("u_asLight[%d].v3Position",  i)
#define CORE_SHADER_UNIFORM_LIGHT_DIRECTION(i) PRINT("u_asLight[%d].v4Direction", i)
#define CORE_SHADER_UNIFORM_LIGHT_VALUE(i)     PRINT("u_asLight[%d].v4Value",     i)

#define CORE_SHADER_UNIFORM_3D_MODELVIEW       "u_m4ModelView"
#define CORE_SHADER_UNIFORM_3D_MODELVIEWPROJ   "u_m4ModelViewProj"
#define CORE_SHADER_UNIFORM_3D_NORMAL          "u_m3Normal"
#define CORE_SHADER_UNIFORM_2D_SCREENVIEW      "u_m4ScreenView"
#define CORE_SHADER_UNIFORM_COLOR              "u_v4Color"
#define CORE_SHADER_UNIFORM_TEXSIZE            "u_v2TexSize"
#define CORE_SHADER_UNIFORM_TEXOFFSET          "u_v2TexOffset"
#define CORE_SHADER_UNIFORM_TEXTURE(i)         PRINT("u_as2Texture[%d]", i)
                                               
#define CORE_SHADER_ATTRIBUTE_POSITION         "a_v3Position"
#define CORE_SHADER_ATTRIBUTE_TEXTURE          "a_v2Texture"
#define CORE_SHADER_ATTRIBUTE_NORMAL           "a_v3Normal"
#define CORE_SHADER_ATTRIBUTE_TANGENT          "a_v4Tangent"
#define CORE_SHADER_ATTRIBUTE_POSITION_NUM     0
#define CORE_SHADER_ATTRIBUTE_TEXTURE_NUM      1
#define CORE_SHADER_ATTRIBUTE_NORMAL_NUM       2
#define CORE_SHADER_ATTRIBUTE_TANGENT_NUM      3

#define CORE_SHADER_OUTPUT_COLOR(i)            PRINT("o_av4Color[%d]", i)
#define CORE_SHADER_OUTPUT_COLORS              4 

enum coreShaderStatus
{
    CORE_SHADER_NEW      = 0,   //!< new and empty
    CORE_SHADER_DEFINED  = 1,   //!< ready for linking
    CORE_SHADER_LINKED   = 2,   //!< successfully linked
    CORE_SHADER_FINISHED = 3    //!< texture units bound
};


// ****************************************************************
// shader class
// TODO: implement simple define-injection interface
class coreShader final : public coreResource
{
private:
    GLuint m_iShader;                   //!< shader identifier
    static std::string s_asGlobal[2];   //!< global shader code (0 = version | 1 = global shader file)


public:
    coreShader()noexcept;
    ~coreShader();

    //! load and unload shader resource data
    //! @{
    coreError Load(coreFile* pFile)override;
    coreError Unload()override;
    //! @}

    //! get object properties
    //! @{
    inline const GLuint& GetShader()const {return m_iShader;}
    //! @}

    //! get relative path to default resource
    //! @{
    static inline const char* GetDefaultPath() {return "data/shaders/default.fs";}
    //! @}


private:
    //! load global shader data
    //! @{
    static void __LoadGlobal();
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
    GLuint m_iProgram;                                //!< shader-program identifier
                                                         
    std::vector<coreShaderPtr> m_apShader;            //!< attached shader objects
    coreShaderStatus m_iStatus;                       //!< current status

    coreLookup<const char*, int> m_aiUniform;         //!< uniform locations
    coreLookup<const char*, int> m_aiAttribute;       //!< attribute locations
    coreLookup<const char*, coreVector4> m_avCache;   //!< cached uniform values

    static coreProgram* s_pCurrent;                   //!< currently active shader-program


public:
    coreProgram()noexcept;
    ~coreProgram();

    //! enable and disable the shader-program
    //! @{
    bool Enable();
    static void Disable(const bool& bFull);
    //! @}

    //! define shader objects and attribute locations
    //! @{
    coreProgram* AttachShaderFile(const char* pcPath);
    coreProgram* AttachShaderLink(const char* pcName);
    coreProgram* BindAttribute(const char* pcName, const int& iLocation);
    inline void Finish()  {if(m_iStatus) return; m_iStatus = CORE_SHADER_DEFINED; this->__Init();}
    inline void Refresh() {this->__Exit();}
    //! @}

    //! send new uniform values
    //! @{
    inline void SendUniform(const char* pcName, const int& iInt)                                    {const int iID = this->CheckCache(pcName, coreVector4(float(iInt), 0.0f, 0.0f, 0.0f)); if(iID >= 0) glUniform1i (iID, iInt);}
    inline void SendUniform(const char* pcName, const float& fFloat)                                {const int iID = this->CheckCache(pcName, coreVector4(fFloat,      0.0f, 0.0f, 0.0f)); if(iID >= 0) glUniform1f (iID, fFloat);}
    inline void SendUniform(const char* pcName, const coreVector2& vVector)                         {const int iID = this->CheckCache(pcName, coreVector4(vVector,     0.0f, 0.0f));       if(iID >= 0) glUniform2fv(iID, 1, vVector);}
    inline void SendUniform(const char* pcName, const coreVector3& vVector)                         {const int iID = this->CheckCache(pcName, coreVector4(vVector,     0.0f));             if(iID >= 0) glUniform3fv(iID, 1, vVector);}
    inline void SendUniform(const char* pcName, const coreVector4& vVector)                         {const int iID = this->CheckCache(pcName, vVector);                                    if(iID >= 0) glUniform4fv(iID, 1, vVector);}
    inline void SendUniform(const char* pcName, const coreMatrix3& mMatrix, const bool& bTranspose) {const int iID = this->CheckCache(pcName, coreVector4(mMatrix._11 + mMatrix._12 + mMatrix._13, mMatrix._21 + mMatrix._22 + mMatrix._23, mMatrix._31 + mMatrix._32 + mMatrix._33, 0.0f)); if(iID >= 0) glUniformMatrix3fv(iID, 1, bTranspose, mMatrix);}
    inline void SendUniform(const char* pcName, const coreMatrix4& mMatrix, const bool& bTranspose) {glUniformMatrix4fv(this->GetUniform(pcName), 1, bTranspose, mMatrix);}
    //! @}

    //! check for cached uniform values
    //! @{
    inline int CheckCache(const char* pcName, const coreVector4& vVector) hot_func {if(m_avCache.count(pcName)) {if(m_avCache.at(pcName) == vVector) return -1;} m_avCache[pcName] = vVector; return this->GetUniform(pcName);}
    //! @}

    //! get object properties
    //! @{
    inline const GLuint& GetProgram()const             {return m_iProgram;}
    inline const int& GetUniform(const char* pcName)   {if(!m_aiUniform.count(pcName))   {SDL_assert(m_iStatus >= CORE_SHADER_LINKED && s_pCurrent == this); m_aiUniform[pcName]   = glGetUniformLocation(m_iProgram, pcName);} return m_aiUniform.at(pcName);}
    inline const int& GetAttribute(const char* pcName) {if(!m_aiAttribute.count(pcName)) {SDL_assert(m_iStatus >= CORE_SHADER_LINKED && s_pCurrent == this); m_aiAttribute[pcName] = glGetAttribLocation (m_iProgram, pcName);} return m_aiAttribute.at(pcName);}
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

    //! init and exit the shader-program
    //! @{
    coreError __Init();
    coreError __Exit();
    //! @}

    //! write error-log
    //! @{
    void __WriteLog()const;
    //! @}
};


// ****************************************************************
// shader-program shared memory type
typedef std::shared_ptr<coreProgram> coreProgramShr;


#endif // _CORE_GUARD_SHADER_H_