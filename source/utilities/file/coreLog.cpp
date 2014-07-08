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
: m_sPath  (pcPath)
, m_iLevel (CORE_LOG_LEVEL_ALL)
, m_iMain  (0)
, m_iLock  (0)
{
    // open and reset log file
    std::FILE* pFile = std::fopen(m_sPath.c_str(), "w");
    if(pFile)
    {
        // write basic style sheet
        std::fputs("<!DOCTYPE html>                                \n", pFile);
        std::fputs("<style type=\"text/css\">                      \n", pFile);
        std::fputs("  body    {font-family: courier new;}          \n", pFile);
        std::fputs(" .time    {color: #AAA;}                       \n", pFile);
        std::fputs(" .thread  {color: green;}                      \n", pFile);
        std::fputs(" .thread2 {color: olive;}                      \n", pFile);
        std::fputs(" .data    {color: teal;}                       \n", pFile);
        std::fputs(" .gl      {color: purple;}                     \n", pFile);
        std::fputs(" .header  {font-weight: bold; font-size: 22px;}\n", pFile);
        std::fputs(" .list    {font-weight: bold;}                 \n", pFile);
        std::fputs(" .error   {font-weight: bold; color: red;}     \n", pFile);
        std::fputs("</style>                                       \n", pFile);

        // write application data and timestamp
        std::fprintf(pFile, CORE_LOG_BOLD("Executable:") " %s %s %s    <br />\n", coreData::AppName(), __DATE__, __TIME__);
        std::fprintf(pFile, CORE_LOG_BOLD("Started on:") " %s %s <br /><br />\n", coreData::DateString(), coreData::TimeString());

        // close log file
        std::fclose(pFile);

        // save thread-ID from the creator
        m_iMain = SDL_ThreadID() % 10000;
    }
}


// ****************************************************************
/* write an OpenGL debug message */
void APIENTRY WriteOpenGL(GLenum iSource, GLenum iType, GLuint iID, GLenum iSeverity, GLsizei iLength, const GLchar* pcMessage, void* pUserParam)
{
    coreLog* pLog = s_cast<coreLog*>(pUserParam);

    // write message
    pLog->ListStart("OpenGL Debug Message");
    pLog->ListEntry("<span class=\"gl\">" CORE_LOG_BOLD("ID:")       "     %d</span>", iID);
    pLog->ListEntry("<span class=\"gl\">" CORE_LOG_BOLD("Source:")   " 0x%04X</span>", iSource);
    pLog->ListEntry("<span class=\"gl\">" CORE_LOG_BOLD("Type:")     " 0x%04X</span>", iType);
    pLog->ListEntry("<span class=\"gl\">" CORE_LOG_BOLD("Severity:") " 0x%04X</span>", iSeverity);
    pLog->ListEntry("<span class=\"gl\">                                   %s</span>", pcMessage);
    pLog->ListEnd();

    ASSERT(false)
}


// ****************************************************************
/* enable OpenGL debug output */
void coreLog::DebugOpenGL()
{
    if(!Core::Config->GetBool(CORE_CONFIG_SYSTEM_DEBUG) && !DEFINED(_CORE_DEBUG_)) return;

    if(GLEW_KHR_debug)
    {
        // enable synchronous debug output
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        // set callback function and filter
        glDebugMessageCallback(&WriteOpenGL, this);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }
    else if(GLEW_ARB_debug_output)
    {
        // enable synchronous debug output
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

        // set callback function and filter
        glDebugMessageCallbackARB(&WriteOpenGL, this);
        glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
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
        SDL_Log(sText.c_str());

#endif
        // open log file
        std::FILE* pFile = std::fopen(m_sPath.c_str(), "a");
        if(pFile)
        {
            // color brackets and convert new lines
            int iPos = -1;
            while((iPos = sText.find("("))  >= 0) sText.replace(iPos, 1, "<span class=\"data\">&#40;");
            while((iPos = sText.find(")"))  >= 0) sText.replace(iPos, 1, "&#41;</span>");
            while((iPos = sText.find("\n")) >= 0) sText.replace(iPos, 1, "<br />");

            if(bTime)
            {
                // get thread-ID
                const SDL_threadID iThread = SDL_ThreadID() % 10000;

                // write timestamp and thread-ID
                std::fprintf(pFile, "<span class=\"time\">[%s]</span> <span class=\"%s\">[%04lu]</span> ",
                             coreData::TimeString(), (m_iMain == iThread) ? "thread" : "thread2", iThread);
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