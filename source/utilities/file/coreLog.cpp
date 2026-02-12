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
, m_iWarnLimit  (1000u)
, m_iThisThread (0u)
, m_aiLastTime  {}
, m_Lock        ()
, m_sWorkString ("")
{
#if !defined(_CORE_EMSCRIPTEN_) && !defined(_CORE_SWITCH_)

    // open and reset log file
    m_pFile = coreData::FileOpen(m_sPath.c_str(), "wb");
    if(m_pFile)
    {
        // retrieve current date and time
        const coreChar* pcDateTime = coreData::DateTimeString();

        // write basic style sheet
        std::fputs("<!DOCTYPE html>"                                                 "\n", m_pFile);
        std::fputs("<meta charset=\"utf-8\">"                                        "\n", m_pFile);
        std::fputs("<style>"                                                         "\n", m_pFile);
        std::fputs(" :root    {font: 0.95em courier new; color-scheme: dark light;}" "\n", m_pFile);
        std::fputs(" .time    {color: #AAA; white-space: pre;}"                      "\n", m_pFile);
        std::fputs(" .thread1 {color: green;}"                                       "\n", m_pFile);
        std::fputs(" .thread2 {color: olive;}"                                       "\n", m_pFile);
        std::fputs(" .data    {color: teal;}"                                        "\n", m_pFile);
        std::fputs(" .warning {color: coral;}"                                       "\n", m_pFile);
        std::fputs(" .error   {color: red;}"                                         "\n", m_pFile);
        std::fputs(" .header  {font-weight: bold; font-size: 1.4em;}"                "\n", m_pFile);
        std::fputs(" .list    {font-weight: bold;}"                                  "\n", m_pFile);
        std::fputs("</style>"                                                        "\n", m_pFile);

        // write application data
        std::fprintf(m_pFile, "<title>%s | %s</title>"                                 "\n", CoreApp::Settings::Name, pcDateTime);
        std::fprintf(m_pFile, CORE_LOG_BOLD("Executable:") " %s (%s %s %s, %s %s)" "<br>\n", coreData::ProcessPath(), CoreApp::Settings::Name, CoreApp::Settings::IsDemo() ? "Demo" : "", CoreApp::Settings::Version, __DATE__, __TIME__);
        std::fprintf(m_pFile, CORE_LOG_BOLD("Built with:") " %s, %s, %s"           "<br>\n", coreData::BuildCompiler(), coreData::BuildLibraryC(), coreData::BuildLibraryCpp());
        std::fprintf(m_pFile, CORE_LOG_BOLD("Started on:") " %s (PID %u)"          "<br>\n", pcDateTime, coreData::ProcessID());

        // flush log file
        std::fflush(m_pFile);

        // save thread-ID from the creator
        m_iThisThread = SDL_GetCurrentThreadID();
    }

#endif
}


// ****************************************************************
/* destructor */
coreLog::~coreLog()
{
    const coreSpinLocker oLocker(&m_Lock);

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
            const coreUint64   iTime   = SDL_GetTicks();
            const SDL_ThreadID iThread = SDL_GetCurrentThreadID();

            // retrieve thread-specific last time-value
            coreUint64& iLastTime = m_aiLastTime.count(iThread) ? m_aiLastTime.at(iThread) : m_aiLastTime.emplace(iThread, iTime);

            // write time-value and thread-ID
            std::fprintf(m_pFile, "<span class=\"time\">[%02llu:%02llu.%03llu - %3llu]</span> <span class=\"%s\">[%04llX]</span> ",
                         (iTime / 1000u) / 60u, (iTime / 1000u) % 60u, (iTime % 1000u), coreUint64(MIN(iTime - iLastTime, 999u)),
                         (iThread == m_iThisThread) ? "thread1" : "thread2", coreUint64(iThread));

            // save time-value (for duration approximations)
            iLastTime = iTime;
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
    this->__WriteStandard(sMessage);

#endif
}


// ****************************************************************
/* write text to the standard output */
void coreLog::__WriteStandard(coreWorkString& sMessage)
{
    ASSERT(m_Lock.IsLocked())

    coreUintW iFrom = SIZE_MAX;

    // remove all HTML tags
    for(coreUintW i = 0u, ie = sMessage.length(); i < ie; ++i)
    {
        if(sMessage[i] == '<')
        {
            iFrom = i;
        }
        else if((sMessage[i] == '>') && (iFrom != SIZE_MAX))   // don't remove greater-than symbols in regular text
        {
            const coreUintW iLen = i - iFrom + 1u;

            sMessage.erase(iFrom, iLen);
            i  -= iLen;
            ie -= iLen;

            iFrom = SIZE_MAX;
        }
    }

    // write text
    if(!sMessage.empty()) SDL_Log("%s", sMessage.c_str());
}