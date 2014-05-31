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

// TODO: define more log capturing spots, not only resource-loading/unloading and (few) errors


// ****************************************************************
// log definitions
#define __CORE_LOG_STRING      (std::string(coreData::Print(pcText, vArgs...)))
#define __CORE_LOG_ALLOW_INFO  (m_iLevel >=  0)
#define __CORE_LOG_ALLOW_LIST  (m_iLevel >= -1)
#define __CORE_LOG_ALLOW_ERROR (m_iLevel ==  0 || m_iLevel == -1)

enum coreLogLevel
{
    CORE_LOG_LEVEL_ALL        =  0,   //!< log all messages
    CORE_LOG_LEVEL_ONLY_INFO  =  1,   //!< log only headers, infos and lists
    CORE_LOG_LEVEL_ONLY_ERROR = -1,   //!< log only errors and lists
    CORE_LOG_LEVEL_NOTHING    = -2    //!< log nothing
};


// ****************************************************************
// log file class
class coreLog final
{
private:
    std::string m_sPath;     //!< relative path of the file
    coreLogLevel m_iLevel;   //!< logging level

    SDL_threadID m_iMain;    //!< thread-ID of the creator of this log
    SDL_SpinLock m_iLock;    //!< spinlock to prevent asynchronous log access


public:
    explicit coreLog(const char* pcPath)noexcept;

    //! message functions
    //! @{
    template <typename... A> inline void Header(const char* pcText, const A&... vArgs) {if(__CORE_LOG_ALLOW_INFO) this->__Write(false, "<hr /><span class=\"header\">" + __CORE_LOG_STRING + "</span><br />");}
    template <typename... A> inline void Info(const char* pcText, const A&... vArgs)   {if(__CORE_LOG_ALLOW_INFO) this->__Write(true,                                    __CORE_LOG_STRING + "<br />");}
    template <typename... A> void Error(const bool& bShutdown, const char* pcText, const A&... vArgs);
    //! @}

    //! list functions
    //! @{
    template <typename... A> inline void ListStart(const char* pcText, const A&... vArgs) {if(__CORE_LOG_ALLOW_LIST) this->__Write(true,  "<span class=\"list\">" + __CORE_LOG_STRING + "</span><ul>");}
    template <typename... A> inline void ListEntry(const char* pcText, const A&... vArgs) {if(__CORE_LOG_ALLOW_LIST) this->__Write(false, "<li>"                  + __CORE_LOG_STRING + "</li>");}
    inline void ListEnd()                                                                 {if(__CORE_LOG_ALLOW_LIST) this->__Write(false, "</ul>");}
    //! @}

    //! control logging level
    //! @{
    inline void SetLevel(const coreLogLevel& iLevel) {m_iLevel = iLevel;}
    inline const coreLogLevel& GetLevel()const       {return m_iLevel;}
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
    void __Write(const bool& bTime, std::string sText) cold_func;
    //! @}
};


// ****************************************************************
// write error message and shut down the application
template <typename... A> void coreLog::Error(const bool& bShutdown, const char* pcText, const A&... vArgs)
{
    // write error message
    if(__CORE_LOG_ALLOW_ERROR) this->__Write(true, "<span class=\"error\">" + __CORE_LOG_STRING + "</span><br />");

    if(bShutdown)
    {
        // show critical error message
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", __CORE_LOG_STRING.c_str(), NULL);

        // trigger breakpoint or shut down the application
#if defined(_CORE_DEBUG_)
        SDL_TriggerBreakpoint();
#else
        _exit(1);
#endif
    }
}


#endif // _CORE_GUARD_LOG_H_