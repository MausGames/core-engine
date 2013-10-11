//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
coreLog::coreLog(const char* pcPath)noexcept
: m_sPath  (pcPath)
, m_iLevel (0)
, m_iLock  (0)
{
    // open and reset log file
    std::FILE* pFile = std::fopen(m_sPath.c_str(), "w");
    if(!pFile) return;

    // write basic style sheet
    std::fprintf(pFile, "<style type=\"text/css\">                        \n");
    std::fprintf(pFile, "  body      {font-family: courier new;}          \n");
    std::fprintf(pFile, " .time      {color: #AAAAAA;}                    \n");
    std::fprintf(pFile, " .thread    {color: green;}                      \n");
    std::fprintf(pFile, " .data      {color: teal;}                       \n");
    std::fprintf(pFile, " .header    {font-weight: bold; font-size: 22px;}\n");
    std::fprintf(pFile, " .liststart {font-weight: bold;}                 \n");
    std::fprintf(pFile, " .error     {font-weight: bold; color: red;}     \n");
    std::fprintf(pFile, "</style>                                         \n");

    // close log file
    std::fclose(pFile);
}


// ****************************************************************
// write error message and shut down the application
void coreLog::Error(const bool& bShutdown, const std::string& sText)
{
    // write error message
    if(m_iLevel <= 0) this->__Write(true, "<span class=\"error\">" + sText + "</span><br />");

    if(bShutdown)
    {
        // show critical error message
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", sText.c_str(), Core::System->GetWindow());

        // trigger breakpoint or shut down the application
#if defined(_CORE_DEBUG_)
        SDL_TriggerBreakpoint();
#else
        _exit(1);
#endif
    }
}


// ****************************************************************
// write text to the log file
void coreLog::__Write(const bool& bTime, std::string sText)
{
    SDL_AtomicLock(&m_iLock);
    {
        // write text also to standard output
        SDL_Log(sText.c_str());

        // open log file
        std::FILE* pFile = std::fopen(m_sPath.c_str(), "a");
        if(!pFile) {SDL_AtomicUnlock(&m_iLock); return;}

        // color brackets and convert new lines
        int iPos = -1;
        while((iPos = sText.find("("))  >= 0) sText.replace(iPos, 1, "<span class=\"data\">&#40;");
        while((iPos = sText.find(")"))  >= 0) sText.replace(iPos, 1, "&#41;</span>");
        while((iPos = sText.find("\n")) >= 0) sText.replace(iPos, 1, "<br />");

        // write timestamp
        if(bTime)
        {
            coreUint awTime[3];
            coreUtils::DateTime(&awTime[0], &awTime[1], &awTime[2], NULL, NULL, NULL);
            std::fprintf(pFile, "<span class=\"time\">[%02d:%02d:%02d]</span> <span class=\"thread\">[%04lu]</span> ", awTime[2], awTime[1], awTime[0], SDL_ThreadID());
        }

        // write text
        std::fprintf(pFile, "%s\n", sText.c_str());

        // close log file
        std::fclose(pFile);
    }
    SDL_AtomicUnlock(&m_iLock);
}