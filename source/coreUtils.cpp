//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

#if defined(_WIN32)
    #include <Shellapi.h>
#endif

char     coreUtils::m_aacString[16][256]; // = "";
coreUint coreUtils::m_iIndex                 = 0;


// ****************************************************************
// get application name
const char* coreUtils::AppName()
{
    char* pcString = __NextString();

    // receive name
#if defined(_WIN32)
    GetModuleFileName(NULL, pcString, 255);
    return strrchr(pcString, '\\')+1;
#else
    readlink("/proc/self/exe", pcString, 255);
    return strrchr(pcString, '/')+1;
#endif
}


// ****************************************************************
// get application path
const char* coreUtils::AppPath()
{
    char* pcString = __NextString();

    // receive path
#if defined(_WIN32)
    GetCurrentDirectory(255, pcString);
    strcat(pcString, "\\");
#else
    readlink("/proc/self/exe", pcString, 255);
    char* pcEnd = strrchr(pcString, '/')+1;
    *pcEnd = '\0';
#endif
    
    return pcString;
}


// ****************************************************************
// retrieve current date and time
void coreUtils::DateTime(coreUint* piSec, coreUint* piMin, coreUint* piHou, coreUint* piDay, coreUint* piMon, coreUint* piYea)
{
    // get current time
    const time_t Time = time(NULL);

    // format the time value
    tm* pFormat = localtime(&Time);

    // forward dataz
    if(piSec) *piSec = pFormat->tm_sec;
    if(piMin) *piMin = pFormat->tm_min;
    if(piHou) *piHou = pFormat->tm_hour;
    if(piDay) *piDay = pFormat->tm_mday;
    if(piMon) *piMon = pFormat->tm_mon+1;
    if(piYea) *piYea = pFormat->tm_year+1900;
}


// ****************************************************************
// create printed string
const char* coreUtils::Print(const char* pcMessage, ...)
{
    char* pcString = __NextString();

    // assemble string
    va_list pList;
    va_start(pList, pcMessage);
    vsnprintf(pcString, 255, pcMessage, pList);
    va_end(pList);

    return pcString;
}


// ****************************************************************
// compare strings with wildcards
bool coreUtils::WildCmp(const char* s, const char* t)
{
	return *t-'*' ? *s ? (*t=='?') | (toupper(*s)==toupper(*t)) && WildCmp(s+1,t+1) : !*t : WildCmp(s,t+1) || (*s && WildCmp(s+1,t));
}


// ******************************************************************
// open URL with webbrowser
void coreUtils::OpenURL(const char* pcURL)
{
#if defined(_WIN32)
    ShellExecute(NULL, "open", pcURL, NULL, NULL, SW_SHOWNORMAL);
#else
    if(system(NULL)) system(coreUtils::Print("xdg-open %s", pcURL));
#endif
}