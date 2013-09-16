//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

#if defined(_CORE_WINDOWS_)
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
#if defined(_CORE_WINDOWS_)
    GetModuleFileName(NULL, pcString, 255);
#else
    readlink("/proc/self/exe", pcString, 255);
#endif

    return strrchr(pcString, CORE_UTILS_SLASH[0])+1;
}


// ****************************************************************
// get application path
const char* coreUtils::AppPath()
{
    char* pcString = __NextString();

    // receive path
#if defined(_CORE_WINDOWS_)
    GetCurrentDirectory(255, pcString);
    strcat(pcString, CORE_UTILS_SLASH);
#else
    readlink("/proc/self/exe", pcString, 255);
    (*(strrchr(pcString, CORE_UTILS_SLASH[0])+1)) = '\0';
#endif

    return pcString;
}


// ****************************************************************
// retrieve current date and time
void coreUtils::DateTime(coreUint* piSec, coreUint* piMin, coreUint* piHou, coreUint* piDay, coreUint* piMon, coreUint* piYea)
{
    // format the current time
    const time_t iTime = time(NULL);
    tm* pFormat = localtime(&iTime);

    // forward data
    if(piSec) *piSec = pFormat->tm_sec;
    if(piMin) *piMin = pFormat->tm_min;
    if(piHou) *piHou = pFormat->tm_hour;
    if(piDay) *piDay = pFormat->tm_mday;
    if(piMon) *piMon = pFormat->tm_mon+1;
    if(piYea) *piYea = pFormat->tm_year+1900;
}


// ****************************************************************
// create formated string
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
bool coreUtils::StrCmp(const char* s, const char* t)
{
    return *t-'*' ? *s ? (*t=='?') | (toupper(*s)==toupper(*t)) && StrCmp(s+1,t+1) : !*t : StrCmp(s,t+1) || (*s && StrCmp(s+1,t));
}


// ****************************************************************
// get last characters of a string
const char* coreUtils::StrRight(const char* pcInput, const coreUint& iNum)
{
    const coreUint iLen = strlen(pcInput);
    return pcInput + (iLen-coreMath::Min(iLen, iNum));
}


// ****************************************************************
// safely get file extension
const char* coreUtils::StrExt(const char* pcInput)
{
    const char* pcExtension = strrchr(pcInput, '.');
    return pcExtension ? pcExtension+1 : pcInput;
}


// ****************************************************************
// move string pointer and skip comments
void coreUtils::StrSkip(const char** ppcInput, const int &iNum)
{
    int n = iNum;
    char c = '\0';

    do
    {
        // move string pointer
        *ppcInput += n;

        // check for comments and skip them
        sscanf(*ppcInput, "%c %*[^\n] %n", &c, &n);
    }
    while(c == '/' || c == '#');
}


// ******************************************************************
// open URL with the web-browser
void coreUtils::OpenURL(const char* pcURL)
{
#if defined(_CORE_WINDOWS_)
    ShellExecute(NULL, "open", pcURL, NULL, NULL, SW_SHOWNORMAL);
#else
    if(system(NULL)) system(coreUtils::Print("xdg-open %s", pcURL));
#endif
}