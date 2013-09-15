//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef GUARD_CORE_SHADER_H
#define GUARD_CORE_SHADER_H


// ****************************************************************
// shader object class
class coreShader final : public coreResource
{
private:
    GLuint m_iShader;   //!< shader identifier/OpenGL name


public:
    coreShader();
    coreShader(const char* pcPath);
    coreShader(coreFile* pFile);
    ~coreShader();

    //! load and unload vertex-shader resource data
    //! @{
    coreError Load(coreFile* pFile)override;
    coreError Unload()override;
    //! @}

    //! get attributes
    //! @{
    inline const GLuint& GetShader()const {return m_iShader;}
    //! @}

    //! get relative path to NULL resource
    //! @{
    static inline const char* GetNullPath() {return "data/shader/default.fs";}
    //! @}
};


// ****************************************************************
// shader resource access type
typedef coreResourcePtr<coreShader> coreShaderPtr;


// ****************************************************************
// shader-program object class
// \todo cache and check shader uniform values
class coreProgram final : public coreReset
{
private:
    GLuint m_iProgram;                            //!< shader-program identifier/OpenGL name

    std::vector<coreShaderPtr> m_apShader;        //!< attached shader objects
    int m_iStatus;                                //!< current status (0 = new, 1 = ready for linking, 2 = successfully linked)

    std::u_map<std::string, int> m_aiUniform;     //!< cached identifiers for uniform variables
    std::u_map<std::string, int> m_aiAttribute;   //!< cached identifiers for attribute variables

    static std::vector<coreProgram*> s_apStack;   //!< stack of currently active shader-programs


public:
    coreProgram();
    ~coreProgram();

    //! reset the object with the resource manager
    //! @{
    void Reset(const bool& bInit)override;
    //! @}

    //! enable and disable the shader-program
    //! @{
    void Enable();
    inline void Disable() {SDL_assert(s_apStack.back() == this); s_apStack.pop_back(); glUseProgram(s_apStack.empty() ? 0 : s_apStack.back()->GetProgram());}
    //! @}

    //! control shader object linking
    //! @{
    void AttachShader(const char* pcPath);
    inline void Link()        {if(m_iStatus) return; m_iStatus = 1; this->Reset(true);}
    inline bool Linked()const {return (m_iStatus == 2) ? true : false;}
    //! @}

    //! write current error-log to log file
    //! @{
    void LogError(const char* pcText)const;
    //! @}

    //! set uniform variables
    //! @{
    inline void SetUniform(const char* pcName, const int& iA)                                     {glUniform1i(this->GetUniform(pcName), iA);}
    inline void SetUniform(const char* pcName, const int& iA, const int& iB)                      {glUniform2i(this->GetUniform(pcName), iA, iB);}
    inline void SetUniform(const char* pcName, const int& iA, const int& iB, const int& iC)       {glUniform3i(this->GetUniform(pcName), iA, iB, iC);}
    inline void SetUniform(const char* pcName, const float& fA)                                   {glUniform1f(this->GetUniform(pcName), fA);}
    inline void SetUniform(const char* pcName, const float& fA, const float& fB)                  {glUniform2f(this->GetUniform(pcName), fA, fB);}
    inline void SetUniform(const char* pcName, const float& fA, const float& fB, const float& fC) {glUniform3f(this->GetUniform(pcName), fA, fB, fC);}
    inline void SetUniform(const char* pcName, const coreMatrix& mMatrix, const bool& bTranspose) {glUniformMatrix4fv(this->GetUniform(pcName), 1, bTranspose, mMatrix);}
    //! @}

    //! get attributes
    //! @{
    inline const GLuint& GetProgram()const             {return m_iProgram;}
    inline const int& GetUniform(const char* pcName)   {if(!m_aiUniform.count(pcName))   m_aiUniform[pcName]   = glGetUniformLocation(m_iProgram, pcName); return m_aiUniform.at(pcName);}
    inline const int& GetAttribute(const char* pcName) {if(!m_aiAttribute.count(pcName)) m_aiAttribute[pcName] = glGetAttribLocation(m_iProgram,  pcName); return m_aiAttribute.at(pcName);}
    //! @}
};


#endif // GUARD_CORE_SHADER_H