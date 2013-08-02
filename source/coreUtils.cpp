#include "Core.h"

#ifdef _WIN32
    #include <Shellapi.h>
#endif

char     coreUtils::m_aacString[16][256];
coreUint coreUtils::m_iIndex = 0;


// ****************************************************************
// get application name
const char* coreUtils::AppName()
{
    char* pcString = __NextString();

    // receive name
#ifdef _WIN32
    GetModuleFileName(NULL, pcString, 256);
    return strrchr(pcString, '\\')+1;
#else
    readlink("/proc/self/exe", pcString, 256);
    return strrchr(pcString, '/')+1;
#endif
}


// ****************************************************************
// get application path
const char* coreUtils::AppPath()
{
    char* pcString = __NextString();

    // receive path
#ifdef _WIN32
    GetCurrentDirectory(256, pcString);
    strcat(pcString, "\\");
#else
    readlink("/proc/self/exe", pcString, 256);
    char* pcEnd = strrchr(pcString, '/')+1;
    *pcEnd = '\0';
#endif
    
    return pcString;
}


// ****************************************************************
// check if file exists
bool coreUtils::FileExists(const char* pcPath)
{
    // open file
    FILE* pFile = fopen(pcPath, "r");
    if(pFile)
    {
        // file exists
        fclose(pFile);
        return true;
    }

    return false;
}


// ****************************************************************
// get file data and size
void coreUtils::FileDataSize(FILE* pFile, coreByte** ppDataOut, coreUint* piSizeOut)
{
    // get file size
    fseek(pFile, 0, SEEK_END);
    const coreUint iSize = (coreUint)ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    if(ppDataOut)
    {
        // get and save file data
        (*ppDataOut) = new coreByte[iSize];
        fread((*ppDataOut), sizeof(coreByte), iSize, pFile);
    }
    if(piSizeOut)
    {
        // save file size
        (*piSizeOut) = iSize;
    }
}


// ****************************************************************
// retrieve current date and time
void coreUtils::DateTime(coreUint* piSec, coreUint* piMin, coreUint* piHou, coreUint* piDay, coreUint* piMon, coreUint* piYea)
{
    // get current time
    const time_t Time = time(NULL);

    // format the time value
    tm* pFormat = localtime(&Time);

    // forward data
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

    // assemble the string fragments
    va_list pList;
    va_start(pList, pcMessage);
    vsnprintf(pcString, 256, pcMessage, pList);
    va_end(pList);

    return pcString;
}


// ******************************************************************
// open URL with webbrowser
void coreUtils::OpenURL(const char* pcURL)
{
#ifdef _WIN32
    ShellExecute(NULL, "open", pcURL, NULL, NULL, SW_SHOWNORMAL);
#else
    if(system(NULL)) system(coreUtils::Print("xdg-open %s", pcURL));
#endif
}