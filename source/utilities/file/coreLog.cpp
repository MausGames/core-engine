///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
coreLog::coreLog(const coreChar* pcPath)noexcept
: m_pFile       (NULL)
, m_sPath       (pcPath)
, m_eLevel      (CORE_LOG_LEVEL_ALL)
, m_iListStatus (0u)
, m_iLastTime   (0u)
, m_iMainThread (0u)
, m_Lock        ()
, m_sWorkString ("")
{
#if !defined(_CORE_EMSCRIPTEN_)

    // open and reset log file
    m_pFile = coreData::FileOpen(m_sPath.c_str(), "wb");
    if(m_pFile)
    {
        // write basic style sheet
        std::fputs("<!DOCTYPE html>"                                  "\n", m_pFile);
        std::fputs("<meta charset=\"utf-8\">"                         "\n", m_pFile);
        std::fputs("<style>"                                          "\n", m_pFile);
        std::fputs("  body    {font: 0.95em courier new;}"            "\n", m_pFile);
        std::fputs(" .time    {color: #AAA; white-space: pre;}"       "\n", m_pFile);
        std::fputs(" .thread1 {color: green;}"                        "\n", m_pFile);
        std::fputs(" .thread2 {color: olive;}"                        "\n", m_pFile);
        std::fputs(" .data    {color: teal;}"                         "\n", m_pFile);
        std::fputs(" .warning {color: coral;}"                        "\n", m_pFile);
        std::fputs(" .error   {color: red;}"                          "\n", m_pFile);
        std::fputs(" .header  {font-weight: bold; font-size: 1.4em;}" "\n", m_pFile);
        std::fputs(" .list    {font-weight: bold;}"                   "\n", m_pFile);
        std::fputs("</style>"                                         "\n", m_pFile);

        // write application data and timestamp
        std::fprintf(m_pFile, CORE_LOG_BOLD("Executable:") " %s (%s %s)" "<br>\n", coreData::AppPath(), __DATE__, __TIME__);
        std::fprintf(m_pFile, CORE_LOG_BOLD("Built with:") " %s, %s"     "<br>\n", coreData::BuildCompiler(), coreData::BuildLibrary());
        std::fprintf(m_pFile, CORE_LOG_BOLD("Started on:") " %s %s"      "<br>\n", coreData::DateString(), coreData::TimeString());

        // flush log file
        std::fflush(m_pFile);

        // save first time-value (and init tick-counter)
        m_iLastTime = SDL_GetTicks();

        // save thread-ID from the creator
        m_iMainThread = SDL_ThreadID();
    }

#endif
}


// ****************************************************************
/* destructor */
coreLog::~coreLog()
{
    coreSpinLocker oLocker(&m_Lock);

    // append final line
    this->__Write(false, __CORE_LOG_CLEAR, "<hr>", "");

    // close log file
    if(m_pFile) std::fclose(m_pFile);
}


// ****************************************************************
/* write text to the log file */
void coreLog::__Write(const coreBool bTimeStamp, coreWorkString& sMessage, const coreChar* pcPre, const coreChar* pcPost)
{
    ASSERT(m_Lock.IsLocked())

    // check for valid log file
    if(m_pFile)
    {
        // convert new lines and color brackets
        sMessage.replace("\n", "<br>");
        sMessage.replace("(",  "<span class=\"data\">(");
        sMessage.replace(")",  ")</span>");

        if(bTimeStamp)
        {
            // get time-value and thread-ID
            const coreUint32   iTime   = SDL_GetTicks();
            const SDL_threadID iThread = SDL_ThreadID();

            // write time-value and thread-ID
            std::fprintf(m_pFile, "<span class=\"time\">[%02u:%02u.%03u - %3u]</span> <span class=\"%s\">[%04lX]</span> ",
                         (iTime / 1000u) / 60u, (iTime / 1000u) % 60u, (iTime % 1000u), coreUint32(MIN(iTime - m_iLastTime, 999u)),
                         (iThread == m_iMainThread) ? "thread1" : "thread2", iThread);

            // save time-value (for duration approximations)
            m_iLastTime = iTime;
        }

        // write text
        std::fputs(pcPre,            m_pFile);
        std::fputs(sMessage.c_str(), m_pFile);
        std::fputs(pcPost,           m_pFile);
        std::fputc('\n',             m_pFile);

        // flush log file
        std::fflush(m_pFile);
    }

#if defined(_CORE_DEBUG_)

    // also write text to the standard output
    SDL_Log("%s", sMessage.c_str());

#endif
}