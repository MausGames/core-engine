///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_LOG_H_
#define _CORE_GUARD_LOG_H_

// TODO 4: define more log capturing spots, not only resource-loading/unloading and (few) errors
// TODO 5: coreLog 2.0 with graphs, statistics, categories, interactivity/scripts
// TODO 2: should it be possible to create many log files ? console output and file header then ?
// TODO 3: escape special HTML characters as normal text, <, >, &


// ****************************************************************
/* log definitions */
#define __CORE_LOG_PRINT (m_sWorkString.print(pcText, std::forward<A>(vArgs)...), m_sWorkString)
#define __CORE_LOG_CLEAR (m_sWorkString.clear(), m_sWorkString)

#define CORE_LOG_BOLD(s)       "<b>" s "</b>"   // display text bold
#define CORE_LOG_ITALIC(s)     "<i>" s "</i>"   // display text italic
#define CORE_LOG_UNDERLINED(s) "<u>" s "</u>"   // display text underlined

enum coreLogLevel : coreUint8
{
    CORE_LOG_LEVEL_NOTHING = 0x00u,   // log nothing
    CORE_LOG_LEVEL_INFO    = 0x01u,   // log info messages and headers
    CORE_LOG_LEVEL_WARNING = 0x02u,   // log warning messages
    CORE_LOG_LEVEL_ERROR   = 0x04u,   // log error messages
    CORE_LOG_LEVEL_ALL     = 0x07u    // log everything
};
ENABLE_BITWISE(coreLogLevel)


// ****************************************************************
/* log file class */
class coreLog final
{
private:
    std::FILE* m_pFile;             // log file stream handle

    coreString   m_sPath;           // relative path of the file
    coreLogLevel m_eLevel;          // logging level

    coreUint8 m_iListStatus;        // currently writing a list

    coreUint32   m_iLastTime;       // last time-value for duration approximations
    SDL_threadID m_iThisThread;     // thread-ID from the creator of this log
    coreSpinLock m_Lock;            // spinlock to prevent concurrent log access

    coreWorkString m_sWorkString;   // pre-allocated string for assembling messages


public:
    explicit coreLog(const coreChar* pcPath)noexcept;
    ~coreLog();

    DISABLE_COPY(coreLog)

    /* message functions */
    template <typename... A> inline         void Info   (const coreChar* pcText, A&&... vArgs) {if(HAS_FLAG(m_eLevel, CORE_LOG_LEVEL_INFO))    {coreSpinLocker oLocker(&m_Lock); this->__Write(true, __CORE_LOG_PRINT, "[I] ",                         "<br>");}}
    template <typename... A> inline         void Warning(const coreChar* pcText, A&&... vArgs) {if(HAS_FLAG(m_eLevel, CORE_LOG_LEVEL_WARNING)) {coreSpinLocker oLocker(&m_Lock); this->__Write(true, __CORE_LOG_PRINT, "[W] <span class=\"warning\">", "</span><br>");}}
    template <typename... A> FUNC_TERMINATE void Error  (const coreChar* pcText, A&&... vArgs);

    /* special functions */
    template <typename... A> inline void Header          (const coreChar* pcText, A&&... vArgs) {if(HAS_FLAG(m_eLevel, CORE_LOG_LEVEL_INFO))    {coreSpinLocker oLocker(&m_Lock); this->__Write(false, __CORE_LOG_PRINT, "<hr><span class=\"header\">",                       "</span><br>");}}
    template <typename... A> inline void ListStartInfo   (const coreChar* pcText, A&&... vArgs) {if(HAS_FLAG(m_eLevel, CORE_LOG_LEVEL_INFO))    {m_Lock.Lock();                   this->__Write(true,  __CORE_LOG_PRINT, "[I] <span><span class=\"list\">",                   "</span><ul>"); ++m_iListStatus;}}
    template <typename... A> inline void ListStartWarning(const coreChar* pcText, A&&... vArgs) {if(HAS_FLAG(m_eLevel, CORE_LOG_LEVEL_WARNING)) {m_Lock.Lock();                   this->__Write(true,  __CORE_LOG_PRINT, "[W] <span class=\"warning\"><span class=\"list\">", "</span><ul>"); ++m_iListStatus;}}
    template <typename... A> inline void ListDeeper      (const coreChar* pcText, A&&... vArgs) {if(m_iListStatus)                                                               {this->__Write(false, __CORE_LOG_PRINT, "<li>",                                              "</li><ul>");   ++m_iListStatus;}}
    template <typename... A> inline void ListAdd         (const coreChar* pcText, A&&... vArgs) {if(m_iListStatus)                                                               {this->__Write(false, __CORE_LOG_PRINT, "<li>",                                              "</li>");}}
    inline                          void ListEnd         ()                                     {if(m_iListStatus)                                                               {this->__Write(false, __CORE_LOG_CLEAR, "</ul></span>",                                      ""); if(!(--m_iListStatus)) m_Lock.Unlock();}}

    /* set object properties */
    inline void SetLevel(const coreLogLevel eLevel) {m_eLevel = eLevel;}

    /* get object properties */
    inline const coreChar*     GetPath ()const {return m_sPath.c_str();}
    inline const coreLogLevel& GetLevel()const {return m_eLevel;}


private:
    /* write text to the log file */
    void __Write(const coreBool bTimeStamp, coreWorkString& sMessage, const coreChar* pcPre, const coreChar* pcPost);

    /* write text to the standard output */
    void __WriteStandard(coreWorkString& sMessage);
};


// ****************************************************************
/* write error message and shut down the application */
template <typename... A> FUNC_TERMINATE void coreLog::Error(const coreChar* pcText, A&&... vArgs)
{
    m_Lock.Lock();
    {
        // write error message
        if(HAS_FLAG(m_eLevel, CORE_LOG_LEVEL_ERROR)) this->__Write(true, __CORE_LOG_PRINT, "[E] <span class=\"error\">", "</span><br>");

        // also show message box
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical Error", __CORE_LOG_PRINT.c_str(), NULL);
        WARN_IF(true) {}
    }
    m_Lock.Unlock();

    // shut down the application
    std::exit(EXIT_FAILURE);
}


#endif /* _CORE_GUARD_LOG_H_ */