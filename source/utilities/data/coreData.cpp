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

char     coreData::m_aacString[16][256]; // = "";
coreUint coreData::m_iIndex                 = 0;


// ****************************************************************
// get application name
const char* coreData::AppName()
{
    char* pcString = __NextString();

    // receive name
#if defined(_CORE_WINDOWS_)
    GetModuleFileName(NULL, pcString, 255);
#else
    readlink("/proc/self/exe", pcString, 255);
#endif

    return std::strrchr(pcString, CORE_DATA_SLASH[0])+1;
}


// ****************************************************************
// get application path
const char* coreData::AppPath()
{
    char* pcString = __NextString();

    // receive path
#if defined(_CORE_WINDOWS_)
    GetCurrentDirectory(255, pcString);
#else
    getcwd(pcString, 255);
#endif

    std::strcat(pcString, CORE_DATA_SLASH);
    return pcString;
}


// ****************************************************************
// retrieve current date and time
void coreData::DateTime(coreUint* piSec, coreUint* piMin, coreUint* piHou, coreUint* piDay, coreUint* piMon, coreUint* piYea)
{
    // format the current time
    const time_t iTime = std::time(NULL);
    tm* pFormat = std::localtime(&iTime);

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
const char* coreData::Print(const char* pcMessage, ...)
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
bool coreData::StrCompare(const char* s, const char* t)
{
    return *t-'*' ? *s ? (*t=='?') | (toupper(*s)==toupper(*t)) && StrCompare(s+1,t+1) : !*t : StrCompare(s,t+1) || (*s && StrCompare(s+1,t));
}


// ****************************************************************
// get last characters of a string
const char* coreData::StrRight(const char* pcInput, const coreUint& iNum)
{
    if(!pcInput) return NULL;

    const coreUint iLen = std::strlen(pcInput);
    return pcInput + (iLen-coreMath::Min(iLen, iNum));
}


// ****************************************************************
// safely get file extension
const char* coreData::StrExtension(const char* pcInput)
{
    if(!pcInput) return NULL;

    const char* pcDot = std::strrchr(pcInput, '.');
    return pcDot ? pcDot+1 : pcInput;
}


// ****************************************************************
// safely get version number
float coreData::StrVersion(const char* pcInput)
{
    if(!pcInput) return 0.0f;

    const char* pcDot = std::strchr(pcInput, '.');
    return pcDot ? (float((pcDot-1)[0]-'0') + 0.1f*float((pcDot+1)[0]-'0')) : 0.0f;
}


// ****************************************************************
// move string pointer and skip comments
void coreData::StrSkip(const char** ppcInput, const int &iNum)
{
    if(!*ppcInput) return;

    int  n = iNum;
    char c = '\0';

    do
    {
        // move string pointer
        *ppcInput += n;

        // check for comments and skip them
         std::sscanf(*ppcInput, "%c %*[^\n] %n", &c, &n);
    }
    while(c == '/' || c == '#');
}


// ******************************************************************
// open URL with the web-browser
void coreData::OpenURL(const char* pcURL)
{
#if defined(_CORE_WINDOWS_)
    ShellExecute(NULL, "open", pcURL, NULL, NULL, SW_SHOWNORMAL);
#else
    if(system(NULL)) system(coreData::Print("xdg-open %s", pcURL));
#endif
}