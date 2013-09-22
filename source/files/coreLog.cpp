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
coreLog::coreLog(const char* pcPath)
: m_sPath  (pcPath)
, m_iLevel (0)
, m_iLock  (0)
{
    // open and reset log file
    FILE* pFile = fopen(m_sPath.c_str(), "w");
    if(!pFile) return;

    // write basic style sheet
    fprintf(pFile, "<style type=\"text/css\">                        \n");
    fprintf(pFile, "  body      {font-family: courier new;}          \n");
    fprintf(pFile, " .time      {color: #AAAAAA;}                    \n");
    fprintf(pFile, " .thread    {color: green;}                      \n");
    fprintf(pFile, " .data      {color: teal;}                       \n");
    fprintf(pFile, " .header    {font-weight: bold; font-size: 22px;}\n");
    fprintf(pFile, " .liststart {font-weight: bold;}                 \n");
    fprintf(pFile, " .error     {font-weight: bold; color: red;}     \n");
    fprintf(pFile, "</style>                                         \n");

    // close log file
    fclose(pFile);
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
        FILE* pFile = fopen(m_sPath.c_str(), "a");
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
            fprintf(pFile, "<span class=\"time\">[%02d:%02d:%02d]</span> <span class=\"thread\">[%04lu]</span> ", awTime[2], awTime[1], awTime[0], SDL_ThreadID());
        }

        // write text
        fprintf(pFile, "%s\n", sText.c_str());

        // close log file
        fclose(pFile);
    }
    SDL_AtomicUnlock(&m_iLock);
}