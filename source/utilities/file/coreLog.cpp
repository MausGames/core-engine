//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
coreLog::coreLog(const char* pcPath)noexcept
: m_sPath  (pcPath)
, m_iLevel (0)
, m_iMain  (0)
, m_iLock  (0)
{
    // open and reset log file
    std::FILE* pFile = std::fopen(m_sPath.c_str(), "w");
    if(!pFile) return;

    // write basic style sheet
    std::fprintf(pFile, "<style type=\"text/css\">                      \n");
    std::fprintf(pFile, "  body    {font-family: courier new;}          \n");
    std::fprintf(pFile, " .time    {color: #AAAAAA;}                    \n");
    std::fprintf(pFile, " .thread  {color: green;}                      \n");
    std::fprintf(pFile, " .thread2 {color: olive;}                      \n");
    std::fprintf(pFile, " .data    {color: teal;}                       \n");
    std::fprintf(pFile, " .header  {font-weight: bold; font-size: 22px;}\n");
    std::fprintf(pFile, " .list    {font-weight: bold;}                 \n");
    std::fprintf(pFile, " .error   {font-weight: bold; color: red;}     \n");
    std::fprintf(pFile, " .gl      {color: purple;}                     \n");
    std::fprintf(pFile, "</style>                                       \n");

    // write application name
    std::fprintf(pFile, "%s\n", coreData::AppName());

    // close log file
    std::fclose(pFile);

    // save thread-ID of the creator
    m_iMain = SDL_ThreadID()%10000;
}


// ****************************************************************
// write error message and shut down the application
void coreLog::Error(const bool& bShutdown, const char* pcText)
{
    // write error message
    if(m_iLevel <= 0) this->__Write(true, "<span class=\"error\">" + std::string(pcText) + "</span><br />");

    if(bShutdown)
    {
        // show critical error message
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", pcText, Core::System->GetWindow());

        // trigger breakpoint or shut down the application
#if defined(_CORE_DEBUG_)
        SDL_TriggerBreakpoint();
#else
        _exit(1);
#endif
    }
}


// ****************************************************************
// write an OpenGL debug message
void APIENTRY WriteOpenGL(GLenum iSource, GLenum iType, GLuint iID, GLenum iSeverity, GLsizei iLength, const GLchar* pcMessage, void* pUserParam)
{
    coreLog* pLog = s_cast<coreLog*>(pUserParam);

    pLog->ListStart("OpenGL Debug Log");
    pLog->ListEntry(coreData::Print("<span class=\"gl\"><b>ID:</b>           %d</span>", iID));
    pLog->ListEntry(coreData::Print("<span class=\"gl\"><b>Source:</b>   0x%04X</span>", iSource));
    pLog->ListEntry(coreData::Print("<span class=\"gl\"><b>Type:</b>     0x%04X</span>", iType));
    pLog->ListEntry(coreData::Print("<span class=\"gl\"><b>Severity:</b> 0x%04X</span>", iSeverity));
    pLog->ListEntry(coreData::Print("<span class=\"gl\">                     %s</span>", pcMessage));
    pLog->ListEnd();
}


// ****************************************************************
// enable OpenGL debugging
void coreLog::EnableOpenGL()
{
    // set callback function and filter
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(&WriteOpenGL, this);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
}


// ****************************************************************
// write text to the log file
void coreLog::__Write(const bool& bTime, std::string sText)
{
    SDL_AtomicLock(&m_iLock);
    {
#if defined(_CORE_ANDROID_)

        // write text also to the standard output
        SDL_Log(sText.c_str());

#endif
        // open log file
        std::FILE* pFile = std::fopen(m_sPath.c_str(), "a");
        if(!pFile) {SDL_AtomicUnlock(&m_iLock); return;}

        // color brackets and convert new lines
        int iPos = -1;
        while((iPos = sText.find("("))  >= 0) sText.replace(iPos, 1, "<span class=\"data\">&#40;");
        while((iPos = sText.find(")"))  >= 0) sText.replace(iPos, 1, "&#41;</span>");
        while((iPos = sText.find("\n")) >= 0) sText.replace(iPos, 1, "<br />");

        if(bTime)
        {
            // get timestamp
            coreUint awTime[3];
            coreData::DateTime(&awTime[0], &awTime[1], &awTime[2], NULL, NULL, NULL);

            // get thread-ID
            const SDL_threadID iThread = SDL_ThreadID()%10000;

            // write timestamp and thread-ID
            std::fprintf(pFile, "<span class=\"time\">[%02u:%02u:%02u]</span> <span class=\"%s\">[%04lu]</span> ", awTime[2], awTime[1], awTime[0], (m_iMain == iThread) ? "thread" : "thread2", iThread);
        }

        // write text
        std::fprintf(pFile, "%s\n", sText.c_str());

        // close log file
        std::fclose(pFile);
    }
    SDL_AtomicUnlock(&m_iLock);
}