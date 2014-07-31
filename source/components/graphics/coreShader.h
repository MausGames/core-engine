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
// TODO: _STATUS_
#define CORE_SHADER_BUFFER_GLOBAL               "b_Global"
#define CORE_SHADER_BUFFER_GLOBAL_NUM           (0)
                                                
#define CORE_SHADER_UNIFORM_VIEWPROJ            "u_m4ViewProj"
#define CORE_SHADER_UNIFORM_CAMERA              "u_m4Camera"
#define CORE_SHADER_UNIFORM_PERSPECTIVE         "u_m4Perspective"
#define CORE_SHADER_UNIFORM_ORTHO               "u_m4Ortho"
#define CORE_SHADER_UNIFORM_RESOLUTION          "u_v4Resolution"
                                                
#define CORE_SHADER_UNIFORM_LIGHT_POSITION(i)   PRINT("u_asLight[%d].v3Position",  i)
#define CORE_SHADER_UNIFORM_LIGHT_DIRECTION(i)  PRINT("u_asLight[%d].v4Direction", i)
#define CORE_SHADER_UNIFORM_LIGHT_VALUE(i)      PRINT("u_asLight[%d].v4Value",     i)
                                                
#define CORE_SHADER_UNIFORM_3D_MODELVIEW        "u_m4ModelView"
#define CORE_SHADER_UNIFORM_3D_MODELVIEWPROJ    "u_m4ModelViewProj"
#define CORE_SHADER_UNIFORM_3D_NORMAL           "u_m3Normal"
#define CORE_SHADER_UNIFORM_2D_SCREENVIEW       "u_m3ScreenView"
#define CORE_SHADER_UNIFORM_COLOR               "u_v4Color"
#define CORE_SHADER_UNIFORM_TEXPARAM            "u_v4TexParam"
#define CORE_SHADER_UNIFORM_TEXTURE(i)          PRINT("u_as2Texture[%d]", i)

#define CORE_SHADER_ATTRIBUTE_POSITION          "a_v3Position"
#define CORE_SHADER_ATTRIBUTE_TEXTURE           "a_v2Texture"
#define CORE_SHADER_ATTRIBUTE_NORMAL            "a_v3Normal"
#define CORE_SHADER_ATTRIBUTE_TANGENT           "a_v4Tangent"
#define CORE_SHADER_ATTRIBUTE_POSITION_NUM      (0)
#define CORE_SHADER_ATTRIBUTE_TEXTURE_NUM       (1)
#define CORE_SHADER_ATTRIBUTE_NORMAL_NUM        (2)
#define CORE_SHADER_ATTRIBUTE_TANGENT_NUM       (3)

#define CORE_SHADER_ATTRIBUTE_DIV_MODELVIEW     "a_m4DivModelView"
#define CORE_SHADER_ATTRIBUTE_DIV_POSITION      "a_v3DivPosition"
#define CORE_SHADER_ATTRIBUTE_DIV_DATA          "a_v3DivData"
#define CORE_SHADER_ATTRIBUTE_DIV_COLOR         "a_iDivColor"
#define CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM      "a_v4DivTexParam"
#define CORE_SHADER_ATTRIBUTE_DIV_MODELVIEW_NUM (4)
#define CORE_SHADER_ATTRIBUTE_DIV_POSITION_NUM  (4)
#define CORE_SHADER_ATTRIBUTE_DIV_DATA_NUM      (5)
#define CORE_SHADER_ATTRIBUTE_DIV_COLOR_NUM     (8)
#define CORE_SHADER_ATTRIBUTE_DIV_TEXPARAM_NUM  (9)
                                                
#define CORE_SHADER_OUTPUT_COLOR(i)             PRINT("o_av4OutColor[%d]", i)
#define CORE_SHADER_OUTPUT_COLORS               (4u) 

#define CORE_SHADER_OPTION_INSTANCING           "#define _CORE_OPTION_INSTANCING_           (1) \n"
#define CORE_SHADER_OPTION_NO_TEXTURE_TRANSFORM "#define _CORE_OPTION_NO_TEXTURE_TRANSFORM_ (1) \n"
#define CORE_SHADER_OPTION_NO_PARTICLE_ROTATION "#define _CORE_OPTION_NO_PARTICLE_ROTATION_ (1) \n"

enum coreShaderStatus : coreByte
{
    CORE_SHADER_NEW      = 0,   //!< new and empty
    CORE_SHADER_DEFINED  = 1,   //!< ready for linking
    CORE_SHADER_FINISHED = 2    //!< texture units bound
};


// ****************************************************************
// shader class
// TODO: check out include-replace
class coreShader final : public coreResource
{
private:
    GLuint m_iShader;                       //!< shader identifier

    std::string m_sCustomCode;              //!< custom shader code added to the beginning of the shader                     
    static std::string s_asGlobalCode[2];   //!< global shader code (0 = version | 1 = global shader file)


public:
    coreShader()noexcept;
    explicit coreShader(const char* pcCustomCode)noexcept;
    ~coreShader();

    //! load and unload shader resource data
    //! @{
    coreError Load(coreFile* pFile)override;
    coreError Unload()override;
    //! @}

    //! set object properties
    //! @{
    inline void SetCustomCode(const char* pcCustomCode) {m_sCustomCode = pcCustomCode;}
    //! @}

    //! get object properties
    //! @{
    inline const GLuint& GetShader    ()const {return m_iShader;}
    inline const char*   GetCustomCode()const {return m_sCustomCode.c_str();}
    //! @}


private:
    //! load global shader code
    //! @{
    static void __LoadGlobalCode();
    //! @}
};


// ****************************************************************
// shader resource access type
typedef coreResourcePtr<coreShader> coreShaderPtr;


// ****************************************************************
// shader-program class
// TODO: material data in UBO
// TODO: use variable template with uniform cache (C++14)
// TODO: assert-check for new shaders while already finished
// TODO: allow additional shaders and attributes in between
class coreProgram final : public coreResource
{
private:
    GLuint m_iProgram;                            //!< shader-program identifier
                                                         
    std::vector<coreShaderPtr> m_apShader;        //!< attached shader objects
    coreShaderStatus m_iStatus;                   //!< current status

    coreLookup<const char*, int> m_aiUniform;     //!< uniform locations
    coreLookup<const char*, int> m_aiAttribute;   //!< attribute locations
    coreLookup<int, coreVector4> m_avCache;       //!< cached uniform values

    coreSync m_Sync;                              //!< sync object for asynchronous shader-program loading

    static coreProgram* s_pCurrent;               //!< currently active shader-program

    
public:
    coreProgram()noexcept;
    ~coreProgram();

    //! load and unload shader-program
    //! @{
    coreError Load(coreFile* pFile)override;
    coreError Unload()override;
    //! @}

    //! enable and disable the shader-program
    //! @{
    bool Enable();
    static void Disable(const bool& bFull);
    //! @}

    //! define shader objects and attribute locations
    //! @{
    inline coreProgram* AttachShader (const coreShaderPtr& pShader)                      {if(!m_iStatus) {m_apShader.push_back(pShader);                                          m_apShader.back().SetActive(false);} return this;}
    inline coreProgram* AttachShader (const char*          pcName)                       {if(!m_iStatus) {m_apShader.push_back(Core::Manager::Resource->Get<coreShader>(pcName)); m_apShader.back().SetActive(false);} return this;}
    inline coreProgram* BindAttribute(const char*          pcName, const int& iLocation) {if(!m_iStatus)  m_aiAttribute[pcName] = iLocation;                                                                           return this;}
    inline void Finish ()                                                                {if(!m_iStatus)  m_iStatus = CORE_SHADER_DEFINED;}
    inline void Restart()                                                                {this->Unload(); m_apShader.clear(); m_aiAttribute.clear(); m_iStatus = CORE_SHADER_NEW;}
    //! @}

    //! send new uniform values
    //! @{
    inline void SendUniform(const char* pcName, const int&         iInt)                            {const int iLocation = this->GetUniform(pcName); if(this->CheckCache(iLocation, coreVector4(float(iInt), 0.0f, 0.0f, 0.0f))) glUniform1i (iLocation,    iInt);}
    inline void SendUniform(const char* pcName, const float&       fFloat)                          {const int iLocation = this->GetUniform(pcName); if(this->CheckCache(iLocation, coreVector4(fFloat,      0.0f, 0.0f, 0.0f))) glUniform1f (iLocation,    fFloat);}
    inline void SendUniform(const char* pcName, const coreVector2& vVector)                         {const int iLocation = this->GetUniform(pcName); if(this->CheckCache(iLocation, coreVector4(vVector,     0.0f, 0.0f)))       glUniform2fv(iLocation, 1, vVector);}
    inline void SendUniform(const char* pcName, const coreVector3& vVector)                         {const int iLocation = this->GetUniform(pcName); if(this->CheckCache(iLocation, coreVector4(vVector,     0.0f)))             glUniform3fv(iLocation, 1, vVector);}
    inline void SendUniform(const char* pcName, const coreVector4& vVector)                         {const int iLocation = this->GetUniform(pcName); if(this->CheckCache(iLocation, vVector))                                    glUniform4fv(iLocation, 1, vVector);}
    inline void SendUniform(const char* pcName, const coreMatrix3& mMatrix, const bool& bTranspose) {const int iLocation = this->GetUniform(pcName); if(this->CheckCache(iLocation, coreVector4(mMatrix._11 + mMatrix._12 + mMatrix._13, mMatrix._21 + mMatrix._22 + mMatrix._23, mMatrix._31 + mMatrix._32 + mMatrix._33, 0.0f))) glUniformMatrix3fv(iLocation, 1, bTranspose, mMatrix);}
    inline void SendUniform(const char* pcName, const coreMatrix4& mMatrix, const bool& bTranspose) {glUniformMatrix4fv(this->GetUniform(pcName), 1, bTranspose, mMatrix);}
    //! @}

    //! check for cached uniform values
    //! @{
    inline bool CheckCache(const int& iLocation, const coreVector4& vVector)hot_func {if(iLocation < 0) return false; if(m_avCache.count(iLocation)) {if(m_avCache.at(iLocation) == vVector) return false;} m_avCache[iLocation] = vVector; return true;}
    //! @}

    //! get object properties
    //! @{
    inline const GLuint& GetProgram  ()const              {return m_iProgram;}
    inline const int&    GetUniform  (const char* pcName) {if(!m_aiUniform.count  (pcName)) {ASSERT(m_iStatus >= CORE_SHADER_FINISHED && s_pCurrent == this) m_aiUniform  [pcName] = glGetUniformLocation(m_iProgram, pcName);} return m_aiUniform.at  (pcName);}
    inline const int&    GetAttribute(const char* pcName) {if(!m_aiAttribute.count(pcName)) {ASSERT(m_iStatus >= CORE_SHADER_FINISHED && s_pCurrent == this) m_aiAttribute[pcName] = glGetAttribLocation (m_iProgram, pcName);} return m_aiAttribute.at(pcName);}
    //! @}

    //! get currently active shader-program
    //! @{
    static inline coreProgram* GetCurrent() {return s_pCurrent;}
    //! @}


private:
    //! write error-log
    //! @{
    void __WriteLog()const;
    //! @}
};


// ****************************************************************
// shader-program resource access type
typedef coreResourcePtr<coreProgram> coreProgramPtr;


#endif // _CORE_GUARD_SHADER_H_