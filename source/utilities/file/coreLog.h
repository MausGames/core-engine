//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_LOG_H_
#define _CORE_GUARD_LOG_H_

#if !defined(APIENTRY)
    #define APIENTRY
#endif


// ****************************************************************
// log file class
class coreLog final
{
private:
    std::string m_sPath;    //!< relative path of the log file
    int m_iLevel;           //!< logging level (0 = all | 1 = only non-errors | -1 = only errors)

    SDL_threadID m_iMain;   //!< thread-ID of the creator of this log
    SDL_SpinLock m_iLock;   //!< spinlock to prevent asynchronous log access


public:
    explicit coreLog(const char* pcPath)noexcept;

    //! message functions
    //! @{
    inline void Header(const char* pcText) {if(m_iLevel >= 0) this->__Write(false, "<hr /><span class=\"header\">" + std::string(pcText) + "</span><br />");}
    inline void Info(const char* pcText)   {if(m_iLevel >= 0) this->__Write(true, std::string(pcText) + "<br />");}
    void Error(const bool& bShutdown, const char* pcText);
    //! @}

    //! list functions
    //! @{
    inline void ListStart(const char* pcText) {if(m_iLevel >= 0) this->__Write(true, "<span class=\"list\">" + std::string(pcText) + "</span><ul>");}
    inline void ListEntry(const char* pcText) {if(m_iLevel >= 0) this->__Write(false, "<li>" + std::string(pcText) + "</li>");}
    inline void ListEnd()                     {if(m_iLevel >= 0) this->__Write(false, "</ul>");}
    //! @}

    //! set logging level
    //! @{
    inline void SetLevel(const int& iLevel) {m_iLevel = iLevel;}
    //! @}

    //! control OpenGL debugging
    //! @{
    friend void APIENTRY WriteOpenGL(GLenum iSource, GLenum iType, GLuint iID, GLenum iSeverity, GLsizei iLength, const GLchar* pcMessage, void* pUserParam);
    void EnableOpenGL();
    //! @}


private:
    DISABLE_COPY(coreLog)

    //! write text to the log file
    //! @{
    void __Write(const bool& bTime, std::string sText);
    //! @}
};


#endif // _CORE_GUARD_LOG_H_