//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
coreLog::coreLog(const char* pcPath)noexcept
: m_sPath       (pcPath)
, m_iLevel      (CORE_LOG_LEVEL_ALL)
, m_iMainThread (0)
, m_iLock       (0)
, m_bListStatus (false)
{
    // open and reset log file
    std::FILE* pFile = std::fopen(m_sPath.c_str(), "w");
    if(pFile)
    {
        // write basic style sheet
        std::fputs("<!DOCTYPE html>                                 \n", pFile);
        std::fputs("<style type=\"text/css\">                       \n", pFile);
        std::fputs("  body    {font: 0.95em courier new;}           \n", pFile);
        std::fputs(" .time    {color: #AAA;}                        \n", pFile);
        std::fputs(" .thread1 {color: green;}                       \n", pFile);
        std::fputs(" .thread2 {color: olive;}                       \n", pFile);
        std::fputs(" .data    {color: teal;}                        \n", pFile);
        std::fputs(" .gl      {color: purple;}                      \n", pFile);
        std::fputs(" .warning {font-weight: bold; color: coral;}    \n", pFile);
        std::fputs(" .error   {font-weight: bold; color: red;}      \n", pFile);
        std::fputs(" .header  {font-weight: bold; font-size: 1.4em;}\n", pFile);
        std::fputs(" .list    {font-weight: bold;}                  \n", pFile);
        std::fputs("</style>                                        \n", pFile);

        // write application data and timestamp
        std::fprintf(pFile, CORE_LOG_BOLD("Executable:") " %s/%s %s %s <br />\n", DEFINED(_CORE_X64_) ? "x64" : "x86", coreData::AppName(), __DATE__, __TIME__);
        std::fprintf(pFile, CORE_LOG_BOLD("Started on:") " %s %s       <br />\n", coreData::DateString(), coreData::TimeString());

        // close log file
        std::fclose(pFile);

        // save thread-ID from the creator
        m_iMainThread = SDL_ThreadID() % 10000;
    }
}


// ****************************************************************
/* write an OpenGL debug message */
void GL_APIENTRY WriteOpenGL(GLenum iSource, GLenum iType, GLuint iID, GLenum iSeverity, GLsizei iLength, const GLchar* pcMessage, const void* pUserParam)
{
    coreLog* pLog = s_cast<coreLog*>(c_cast<void*>(pUserParam));

    // write message
    pLog->ListStartWarning("OpenGL Debug Message");
    {
        pLog->ListAdd("<span class=\"gl\">" CORE_LOG_BOLD("ID:")       "     %d</span>", iID);
        pLog->ListAdd("<span class=\"gl\">" CORE_LOG_BOLD("Source:")   " 0x%04X</span>", iSource);
        pLog->ListAdd("<span class=\"gl\">" CORE_LOG_BOLD("Type:")     " 0x%04X</span>", iType);
        pLog->ListAdd("<span class=\"gl\">" CORE_LOG_BOLD("Severity:") " 0x%04X</span>", iSeverity);
        pLog->ListAdd(pcMessage);
    }
    pLog->ListEnd();

    WARN_IF(true) {}
}


// ****************************************************************
/* enable OpenGL debug output */
void coreLog::DebugOpenGL()
{
    if(!Core::Config->GetBool(CORE_CONFIG_SYSTEM_DEBUGMODE) && !DEFINED(_CORE_DEBUG_)) return;

    if(CORE_GL_SUPPORT(KHR_debug))
    {
        // enable synchronous debug output
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        // set callback function and filter
        glDebugMessageCallback(&WriteOpenGL, this);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }
}


// ****************************************************************
/* write text to the log file */
void coreLog::__Write(const bool& bTime, std::string sText)
{
    SDL_AtomicLock(&m_iLock);
    {
#if defined(_CORE_ANDROID_) || defined(_CORE_DEBUG_)

        // also write text to the standard output
        SDL_Log(sText.substr(0, SDL_MAX_LOG_MESSAGE).c_str());

#endif
        // open log file
        std::FILE* pFile = std::fopen(m_sPath.c_str(), "a");
        if(pFile)
        {
            // color brackets and convert new lines
            coreData::StrReplace(&sText, "(",  "<span class=\"data\">(");
            coreData::StrReplace(&sText, ")",  ")</span>");
            coreData::StrReplace(&sText, "\n", "<br />");

            if(bTime)
            {
                // get thread-ID
                const SDL_threadID iThread = SDL_ThreadID() % 10000;

                // write timestamp and thread-ID
                std::fprintf(pFile, "<span class=\"time\">[%s]</span> <span class=\"%s\">[%04lu]</span> ",
                             coreData::TimeString(), (iThread == m_iMainThread) ? "thread1" : "thread2", iThread);
            }

            // write text
            std::fputs(sText.c_str(), pFile);
            std::fputs("\n",          pFile);

            // close log file
            std::fclose(pFile);
        }
    }
    SDL_AtomicUnlock(&m_iLock);
}