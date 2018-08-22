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
// TODO: implement stack trace, described in OpenGL Insights (tested on MSVC, works only with available debug symbols, and not with incremental linking or fast-linking)
// TODO: coreLog 2.0 with graphs, statistics, categories, interactivity/scripts, bacon
// TODO: better specify, should it be possible to create many log files ? what about OpenGL debugging, console output and file header then ?


// ****************************************************************
/* log definitions */
#define __CORE_LOG_STRING (std::string(PRINT(pcText, std::forward<A>(vArgs)...)))

#define CORE_LOG_BOLD(s)       "<b>" s "</b>"   //!< display text bold
#define CORE_LOG_ITALIC(s)     "<i>" s "</i>"   //!< display text italic
#define CORE_LOG_UNDERLINED(s) "<u>" s "</u>"   //!< display text underlined

enum coreLogLevel : coreUint8
{
    CORE_LOG_LEVEL_NOTHING = 0x00u,   //!< log nothing
    CORE_LOG_LEVEL_INFO    = 0x01u,   //!< log info messages and headers
    CORE_LOG_LEVEL_WARNING = 0x02u,   //!< log warning messages
    CORE_LOG_LEVEL_ERROR   = 0x04u,   //!< log error messages
    CORE_LOG_LEVEL_ALL     = 0x07u    //!< log everything
};
ENABLE_BITWISE(coreLogLevel)


// ****************************************************************
/* log file class */
class coreLog final
{
private:
    std::FILE* m_pFile;           //!< log file stream handle

    std::string  m_sPath;         //!< relative path of the file
    coreLogLevel m_iLevel;        //!< logging level

    coreUint8 m_iListStatus;       //!< currently writing a list

    coreUint32   m_iLastTime;     //!< last time-value for duration approximations
    SDL_threadID m_iMainThread;   //!< thread-ID from the creator of this log
    SDL_SpinLock m_iLock;         //!< spinlock to prevent concurrent log access


public:
    explicit coreLog(const coreChar* pcPath)noexcept;
    ~coreLog();

    DISABLE_COPY(coreLog)

    /*! message functions */
    //! @{
    template <typename... A> inline         void Info   (const coreChar* pcText, A&&... vArgs) {if(CONTAINS_FLAG(m_iLevel, CORE_LOG_LEVEL_INFO))    this->__Write(true,                              __CORE_LOG_STRING + "<br />");}
    template <typename... A> inline         void Warning(const coreChar* pcText, A&&... vArgs) {if(CONTAINS_FLAG(m_iLevel, CORE_LOG_LEVEL_WARNING)) this->__Write(true, "<span class=\"warning\">" + __CORE_LOG_STRING + "</span><br />");}
    template <typename... A> FUNC_TERMINATE void Error  (const coreChar* pcText, A&&... vArgs);
    //! @}

    /*! special functions */
    //! @{
    template <typename... A> inline void Header          (const coreChar* pcText, A&&... vArgs) {if(CONTAINS_FLAG(m_iLevel, CORE_LOG_LEVEL_INFO))     this->__Write(false, "<hr /><span class=\"header\">" + __CORE_LOG_STRING + "</span><br />");}
    template <typename... A> inline void ListStartInfo   (const coreChar* pcText, A&&... vArgs) {if(CONTAINS_FLAG(m_iLevel, CORE_LOG_LEVEL_INFO))    {this->__Write(true,  "<span class=\"list\">"         + __CORE_LOG_STRING + "</span><ul>"); ++m_iListStatus;}}
    template <typename... A> inline void ListStartWarning(const coreChar* pcText, A&&... vArgs) {if(CONTAINS_FLAG(m_iLevel, CORE_LOG_LEVEL_WARNING)) {this->__Write(true,  "<span class=\"list\">"         + __CORE_LOG_STRING + "</span><ul>"); ++m_iListStatus;}}
    template <typename... A> inline void ListDeeper      (const coreChar* pcText, A&&... vArgs) {if(m_iListStatus)                                   {this->__Write(false, "<li>"                          + __CORE_LOG_STRING + "</li><ul>");   ++m_iListStatus;}}
    template <typename... A> inline void ListAdd         (const coreChar* pcText, A&&... vArgs) {if(m_iListStatus)                                    this->__Write(false, "<li>"                          + __CORE_LOG_STRING + "</li>");}
    inline                          void ListEnd         ()                                     {if(m_iListStatus)                                   {this->__Write(false, "</ul>"); --m_iListStatus;}}
    //! @}

    /*! handle OpenGL debug output */
    //! @{
    friend void GL_APIENTRY WriteOpenGL(GLenum iSource, GLenum iType, GLuint iID, GLenum iSeverity, GLsizei iLength, const GLchar* pcMessage, const void* pUserParam);
    void DebugOpenGL();
    //! @}

    /*! set object properties */
    //! @{
    inline void SetLevel(const coreLogLevel iLevel) {m_iLevel = iLevel;}
    //! @}

    /*! get object properties */
    //! @{
    inline const coreChar*     GetPath ()const {return m_sPath.c_str();}
    inline const coreLogLevel& GetLevel()const {return m_iLevel;}
    //! @}


private:
    /*! write text to the log file */
    //! @{
    void __Write(const coreBool bTimeStamp, std::string sText);
    //! @}
};


// ****************************************************************
/* write error message and shut down the application */
template <typename... A> FUNC_TERMINATE void coreLog::Error(const coreChar* pcText, A&&... vArgs)
{
    // write error message
    if(CONTAINS_FLAG(m_iLevel, CORE_LOG_LEVEL_ERROR)) this->__Write(true, "<span class=\"error\">" + __CORE_LOG_STRING + "</span><br />");

    // also show message box
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Error", __CORE_LOG_STRING.c_str(), NULL);
    WARN_IF(true) {}

    // shut down the application
    std::exit(EXIT_FAILURE);
}


#endif /* _CORE_GUARD_LOG_H_ */