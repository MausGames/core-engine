//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_LOG_H_
#define _CORE_GUARD_LOG_H_

#if !defined(APIENTRY)
    #define APIENTRY
#endif


// ****************************************************************
// log definitions
#define CORE_LOG_STRING std::string(coreData::Print(pcText, vArgs...))


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
    template <typename... A> inline void Header(const char* pcText, const A&... vArgs) {if(m_iLevel >= 0) this->__Write(false, "<hr /><span class=\"header\">" + CORE_LOG_STRING + "</span><br />");}
    template <typename... A> inline void Info(const char* pcText, const A&... vArgs)   {if(m_iLevel >= 0) this->__Write(true,                                    CORE_LOG_STRING + "<br />");}
    template <typename... A> void Error(const bool& bShutdown, const char* pcText, const A&... vArgs);
    //! @}

    //! list functions
    //! @{
    template <typename... A> inline void ListStart(const char* pcText, const A&... vArgs) {if(m_iLevel >= 0) this->__Write(true,  "<span class=\"list\">" + CORE_LOG_STRING + "</span><ul>");}
    template <typename... A> inline void ListEntry(const char* pcText, const A&... vArgs) {if(m_iLevel >= 0) this->__Write(false, "<li>"                  + CORE_LOG_STRING + "</li>");}
    inline void ListEnd()                                                                 {if(m_iLevel >= 0) this->__Write(false, "</ul>");}
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


// ****************************************************************
// write error message and shut down the application
template <typename... A> void coreLog::Error(const bool& bShutdown, const char* pcText, const A&... vArgs)
{
    // write error message
    if(m_iLevel <= 0) this->__Write(true, "<span class=\"error\">" + CORE_LOG_STRING + "</span><br />");

    if(bShutdown)
    {
        // show critical error message
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", CORE_LOG_STRING.c_str(), NULL);

        // trigger breakpoint or shut down the application
#if defined(_CORE_DEBUG_)
        SDL_TriggerBreakpoint();
#else
        _exit(1);
#endif
    }
}


#endif // _CORE_GUARD_LOG_H_