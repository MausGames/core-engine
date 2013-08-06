#include "Core.h"

// ****************************************************************
// constructor
coreLog::coreLog(const char* pcName)
: m_sName  (pcName)
, m_iLevel (0)
{
    // open and reset log file
    FILE* pFile = fopen(m_sName.c_str(), "w");
    if(!pFile) return;

    // write basic style sheet
    fprintf(pFile, "<style type=\"text/css\">\n");
    fprintf(pFile, "body       {font-family: courier new;}\n");
    fprintf(pFile, ".time      {color: #AAAAAA;}\n");
    fprintf(pFile, ".header    {font-weight: bold; font-size: 22px;}\n");
    fprintf(pFile, ".liststart {font-weight: bold;}\n");
    fprintf(pFile, ".thread    {color: green;}\n");
    fprintf(pFile, ".warning   {color: blue;}\n");
    fprintf(pFile, ".error     {color: red;}\n");
    fprintf(pFile, "</style>\n");

    // close log file
    fclose(pFile);
}


// ****************************************************************
// destructor
coreLog::~coreLog()
{
}


// ****************************************************************
// write error message to log and shut down the application
void coreLog::Error(const bool& bShutdown, const char* pcText)
{
    // write message
    if(m_iLevel <= 0) this->__Write(true, "<span class=\"thread\">[%04d]</span> <span class=\"%s\">%s</span><br />", SDL_ThreadID(), (bShutdown ? "error" : "warning"), pcText);

    // shut down the application
    if(bShutdown)
    {
        Core::System->MsgBox(pcText, "Error", 3);
        _exit(1);
    }
}


// ****************************************************************
// write text to the log file
void coreLog::__Write(const bool& bTime, const char* pcText, ...)
{
    // open log file
    FILE* pFile = fopen(m_sName.c_str(), "a");
    if(!pFile) return;

    // write timestamp
    if(bTime)
    {
        coreUint awTime[3];
        coreUtils::DateTime(&awTime[0], &awTime[1], &awTime[2], NULL, NULL, NULL);
        fprintf(pFile, "<span class=\"time\">[%02d:%02d:%02d]</span> ", awTime[2], awTime[1], awTime[0]);
    }

    // write assembled text
    va_list pList;
    va_start(pList, pcText);
    vfprintf(pFile, pcText, pList);
    va_end(pList);

    // close log file
    fprintf(pFile, "\n");
    fclose(pFile);
}