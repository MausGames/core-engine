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
// TODO: better specify, should it be possible to create many log files ? what about OpenGL debugging, console output and file header then ?


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
    CORE_LOG_LEVEL_ALL     = 0x07    //!< log everything
};
ENABLE_BITWISE(coreLogLevel)


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
    ~coreLog();

    DISABLE_COPY(coreLog)

    /*! message functions */
    //! @{
    template <typename... A> inline        void Info   (const char* pcText, A&&... vArgs) {if(CONTAINS_VALUE(m_iLevel, CORE_LOG_LEVEL_INFO))    this->__Write(true,                              __CORE_LOG_STRING + "<br />");}
    template <typename... A> inline        void Warning(const char* pcText, A&&... vArgs) {if(CONTAINS_VALUE(m_iLevel, CORE_LOG_LEVEL_WARNING)) this->__Write(true, "<span class=\"warning\">" + __CORE_LOG_STRING + "</span><br />");}
    template <typename... A> FUNC_NORETURN void Error  (const char* pcText, A&&... vArgs);
    //! @}

    /*! special functions */
    //! @{
    template <typename... A> inline void Header          (const char* pcText, A&&... vArgs) {if(CONTAINS_VALUE(m_iLevel, CORE_LOG_LEVEL_INFO))     this->__Write(false, "<hr /><span class=\"header\">" + __CORE_LOG_STRING + "</span><br />");}
    template <typename... A> inline void ListStartInfo   (const char* pcText, A&&... vArgs) {if(CONTAINS_VALUE(m_iLevel, CORE_LOG_LEVEL_INFO))    {this->__Write(true,  "<span class=\"list\">"         + __CORE_LOG_STRING + "</span><ul>"); m_bListStatus = true;}}
    template <typename... A> inline void ListStartWarning(const char* pcText, A&&... vArgs) {if(CONTAINS_VALUE(m_iLevel, CORE_LOG_LEVEL_WARNING)) {this->__Write(true,  "<span class=\"list\">"         + __CORE_LOG_STRING + "</span><ul>"); m_bListStatus = true;}}
    template <typename... A> inline void ListAdd         (const char* pcText, A&&... vArgs) {if(m_bListStatus)                                     this->__Write(false, "<li>"                          + __CORE_LOG_STRING + "</li>");}
    inline                          void ListEnd         ()                                 {if(m_bListStatus)                                    {this->__Write(false, "</ul>"); m_bListStatus = false;}}
    //! @}

    /*! handle OpenGL debug output */
    //! @{
    friend void GL_APIENTRY WriteOpenGL(GLenum iSource, GLenum iType, GLuint iID, GLenum iSeverity, GLsizei iLength, const GLchar* pcMessage, const void* pUserParam);
    void DebugOpenGL();
    //! @}

    /*! set object properties */
    //! @{
    inline void SetLevel(const coreLogLevel& iLevel) {m_iLevel = iLevel;}
    //! @}

    /*! get object properties */
    //! @{
    inline const char*         GetPath ()const {return m_sPath.c_str();}
    inline const coreLogLevel& GetLevel()const {return m_iLevel;}
    //! @}


private:
    /*! write text to the log file */
    //! @{
    void __Write(const bool& bTime, std::string sText);
    //! @}
};


// ****************************************************************
/* write error message and shut down the application */
template <typename... A> FUNC_NORETURN void coreLog::Error(const char* pcText, A&&... vArgs)
{
    // write error message
    if(CONTAINS_VALUE(m_iLevel, CORE_LOG_LEVEL_ERROR)) this->__Write(true, "<span class=\"error\">" + __CORE_LOG_STRING + "</span><br />");

    // show critical error message
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", __CORE_LOG_STRING.c_str(), NULL);
    WARN_IF(true) {}

    // shut down the application
    std::exit(EXIT_FAILURE);
}


#endif /* _CORE_GUARD_LOG_H_ */