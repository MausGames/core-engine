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
// TODO: implement stack trace, described in OpenGL Insights (tested on MSVC, works only with available debug symbols)
// TODO: coreLog 2.0 with graphs, statistics, categories, interactivity/scripts, bacon


// ****************************************************************
/* log definitions */
#define __CORE_LOG_STRING (std::string(PRINT(pcText, std::forward<A>(vArgs)...)))

#define CORE_LOG_BOLD(s)       "<b>" s "</b>"   //!< display text bold
#define CORE_LOG_ITALIC(s)     "<i>" s "</i>"   //!< display text italic
#define CORE_LOG_UNDERLINED(s) "<u>" s "</u>"   //!< display text underlined

enum coreLogLevel : coreByte
{
    CORE_LOG_LEVEL_NOTHING = 0x00,   //!< log nothing
    CORE_LOG_LEVEL_INFO    = 0x01,   //!< log info messages and headers
    CORE_LOG_LEVEL_WARNING = 0x02,   //!< log warning messages
    CORE_LOG_LEVEL_ERROR   = 0x04,   //!< log error messages
    CORE_LOG_LEVEL_ALL     = 0xFF    //!< log everything
};
EXTEND_ENUM(coreLogLevel)


// ****************************************************************
/* log file class */
class coreLog final
{
private:
    std::string  m_sPath;         //!< relative path of the file
    coreLogLevel m_iLevel;        //!< logging level

    SDL_threadID m_iMainThread;   //!< thread-ID from the creator of this log
    SDL_SpinLock m_iLock;         //!< spinlock to prevent asynchronous log access

    bool m_bListStatus;           //!< currently writing a list


public:
    explicit coreLog(const char* pcPath)noexcept;

    /*! message functions */
    //! @{
    template <typename... A> inline void Info   (const char* pcText, A&&... vArgs) {if(m_iLevel & CORE_LOG_LEVEL_INFO)    this->__Write(true,                              __CORE_LOG_STRING + "<br />");}
    template <typename... A> inline void Warning(const char* pcText, A&&... vArgs) {if(m_iLevel & CORE_LOG_LEVEL_WARNING) this->__Write(true, "<span class=\"warning\">" + __CORE_LOG_STRING + "</span><br />");}
    template <typename... A>        void Error  (const char* pcText, A&&... vArgs);
    //! @}

    /*! special functions */
    //! @{
    template <typename... A> inline void Header          (const char* pcText, A&&... vArgs) {if(m_iLevel & CORE_LOG_LEVEL_INFO)     this->__Write(false, "<hr /><span class=\"header\">" + __CORE_LOG_STRING + "</span><br />");}
    template <typename... A> inline void ListStartInfo   (const char* pcText, A&&... vArgs) {if(m_iLevel & CORE_LOG_LEVEL_INFO)    {this->__Write(true,  "<span class=\"list\">"         + __CORE_LOG_STRING + "</span><ul>"); m_bListStatus = true;}}
    template <typename... A> inline void ListStartWarning(const char* pcText, A&&... vArgs) {if(m_iLevel & CORE_LOG_LEVEL_WARNING) {this->__Write(true,  "<span class=\"list\">"         + __CORE_LOG_STRING + "</span><ul>"); m_bListStatus = true;}}
    template <typename... A> inline void ListAdd         (const char* pcText, A&&... vArgs) {if(m_bListStatus)                      this->__Write(false, "<li>"                          + __CORE_LOG_STRING + "</li>");}
    inline                          void ListEnd         ()                                 {if(m_bListStatus)                     {this->__Write(false, "</ul>"); m_bListStatus = false;}}
    //! @}

    /*! control logging level */
    //! @{
    inline void SetLevel(const coreLogLevel& iLevel) {m_iLevel = iLevel;}
    inline const coreLogLevel& GetLevel()const       {return m_iLevel;}
    //! @}

    /*! handle OpenGL debug output */
    //! @{
    friend void GLAPIENTRY WriteOpenGL(GLenum iSource, GLenum iType, GLuint iID, GLenum iSeverity, GLsizei iLength, const GLchar* pcMessage, const void* pUserParam);
    void DebugOpenGL();
    //! @}


private:
    DISABLE_COPY(coreLog)

    /*! write text to the log file */
    //! @{
    void __Write(const bool& bTime, std::string sText)cold_func;
    //! @}
};


// ****************************************************************
/* write error message and shut down the application */
template <typename... A> void coreLog::Error(const char* pcText, A&&... vArgs)
{
    // write error message
    if(m_iLevel & CORE_LOG_LEVEL_ERROR) this->__Write(true, "<span class=\"error\">" + __CORE_LOG_STRING + "</span><br />");

    // show critical error message
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", __CORE_LOG_STRING.c_str(), NULL);
    ASSERT(false)

    // shut down the application
    std::exit(-1);
}


#endif /* _CORE_GUARD_LOG_H_ */