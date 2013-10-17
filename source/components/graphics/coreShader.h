//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SHADER_H_
#define _CORE_GUARD_SHADER_H_


// ****************************************************************
// shader definitions
// TODO: define shaders with different versions and levels
// TODO: document definitions
#define CORE_SHADER_UNIFORM_PERSPECTIVE "u_mPerspective"
#define CORE_SHADER_UNIFORM_ORTHO       "u_mOrtho"
#define CORE_SHADER_UNIFORM_CAMERA      "u_mCamera"
#define CORE_SHADER_UNIFORM_TRANSFORM   "u_mTransform"
#define CORE_SHADER_UNIFORM_LIGHT0      "u_v3Light0"

#define CORE_SHADER_ATTRIBUTE_POSITION     "a_v3Position"
#define CORE_SHADER_ATTRIBUTE_POSITION_NUM 0
#define CORE_SHADER_ATTRIBUTE_TEXTURE      "a_v2Texture"
#define CORE_SHADER_ATTRIBUTE_TEXTURE_NUM  1
#define CORE_SHADER_ATTRIBUTE_NORMAL       "a_v3Normal"
#define CORE_SHADER_ATTRIBUTE_NORMAL_NUM   2
#define CORE_SHADER_ATTRIBUTE_TANGENT      "a_v4Tangent"
#define CORE_SHADER_ATTRIBUTE_TANGENT_NUM  3

#define CORE_SHADER_OUT_COLOR0     "o_v4Color0"
#define CORE_SHADER_OUT_COLOR0_NUM 0


// ****************************************************************
// shader class
class coreShader final : public coreResource
{
private:
    GLuint m_iShader;   //!< shader identifier/OpenGL name


public:
    coreShader()noexcept;
    explicit coreShader(const char* pcPath)noexcept;
    explicit coreShader(coreFile* pFile)noexcept;
    ~coreShader();

    //! load and unload vertex-shader resource data
    //! @{
    coreError Load(coreFile* pFile)override;
    coreError Unload()override;
    //! @}

    //! get object attributes
    //! @{
    inline const GLuint& GetShader()const {return m_iShader;}
    //! @}

    //! get relative path to NULL resource
    //! @{
    static inline const char* GetNullPath() {return "data/shaders/default.fs";}
    //! @}
};


// ****************************************************************
// shader resource access type
typedef coreResourcePtr<coreShader> coreShaderPtr;


// ****************************************************************
// shader-program class
// TODO: cache and check shader uniform values
class coreProgram final : public coreReset
{
private:
    GLuint m_iProgram;                            //!< shader-program identifier/OpenGL name

    std::vector<coreShaderPtr> m_apShader;        //!< attached shader objects
    int m_iStatus;                                //!< current status (0 = new | 1 = ready for linking | 2 = successfully linked)

    std::u_map<std::string, int> m_aiUniform;     //!< cached identifiers for uniform variables
    std::u_map<std::string, int> m_aiAttribute;   //!< cached identifiers for attribute variables

    static coreProgram* s_pCurrent;               //!< currently active shader-program


public:
    coreProgram()noexcept;
    ~coreProgram();

    //! reset with the resource manager
    //! @{
    void Reset(const bool& bInit)override;
    //! @}

    //! init and exit the shader-program
    //! @{
    coreError Init();
    coreError Exit();
    //! @}

    //! enable and disable the shader-program
    //! @{
    void Enable();
    static void Disable();
    //! @}

    //! attach and link shader objects
    //! @{
    coreProgram* AttachShaderFile(const char* pcPath);
    coreProgram* AttachShaderLink(const char* pcName);
    inline void Finish()          {if(m_iStatus) return; m_iStatus = 1; this->Init();}
    inline bool IsFinished()const {return (m_iStatus == 2) ? true : false;}
    //! @}

    //! set uniform variables
    //! @{
    inline void SetUniform(const char* pcName, const int& iInt)                                   {glUniform1i(this->GetUniform(pcName), iInt);}
    inline void SetUniform(const char* pcName, const float& fFloat)                               {glUniform1f(this->GetUniform(pcName), fFloat);}
    inline void SetUniform(const char* pcName, const coreVector2& vVector)                        {glUniform2fv(this->GetUniform(pcName), 1, vVector);}
    inline void SetUniform(const char* pcName, const coreVector3& vVector)                        {glUniform3fv(this->GetUniform(pcName), 1, vVector);}
    inline void SetUniform(const char* pcName, const coreVector4& vVector)                        {glUniform4fv(this->GetUniform(pcName), 1, vVector);}
    inline void SetUniform(const char* pcName, const coreMatrix& mMatrix, const bool& bTranspose) {glUniformMatrix4fv(this->GetUniform(pcName), 1, bTranspose, mMatrix);}
    //! @}

    //! get object attributes
    //! @{
    inline const GLuint& GetProgram()const             {return m_iProgram;}
    inline const int& GetUniform(const char* pcName)   {if(!m_aiUniform.count(pcName))   m_aiUniform[pcName]   = glGetUniformLocation(m_iProgram, pcName); return m_aiUniform.at(pcName);}
    inline const int& GetAttribute(const char* pcName) {if(!m_aiAttribute.count(pcName)) m_aiAttribute[pcName] = glGetAttribLocation(m_iProgram,  pcName); return m_aiAttribute.at(pcName);}
    //! @}

    //! get currently active shader-program
    //! @{
    static inline coreProgram* GetCurrent() {return s_pCurrent;}
    //! @}
};


// ****************************************************************
// shader-program shared memory type
typedef std::shared_ptr<coreProgram> coreProgramShr;


#endif // _CORE_GUARD_SHADER_H_