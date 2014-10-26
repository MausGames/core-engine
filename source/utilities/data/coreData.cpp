//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

#if defined(_CORE_WINDOWS_)
    #include <shellapi.h>
#elif defined(_CORE_LINUX_)
    #include <dirent.h>
#elif defined(_CORE_OSX_)
    #include <mach-o/dyld.h>
#endif

thread_local char     coreData::m_aacString[CORE_DATA_STRING_NUM][CORE_DATA_STRING_LEN]; // = "";
thread_local coreUint coreData::m_iCurString                                                = 0;


// ****************************************************************
/* get full application path */
const char* coreData::AppPath()
{
    char* pcString = coreData::__NextString();

#if defined(_CORE_WINDOWS_)
    GetModuleFileName(NULL, pcString, CORE_DATA_STRING_LEN);
#elif defined(_CORE_LINUX_) && !defined(_CORE_ANDROID_)
    readlink("/proc/self/exe", pcString, CORE_DATA_STRING_LEN);
#elif defined(_CORE_OSX_)
    coreUint iLen = CORE_DATA_STRING_LEN;
    _NSGetExecutablePath(pcString, &iLen);
#else
    return "";
#endif

    return pcString;
}


// ****************************************************************
/* set current working directory */
coreError coreData::SetCurDir(const char* pcPath)
{
#if defined(_CORE_WINDOWS_)
    if(SetCurrentDirectory(pcPath)) return CORE_OK;
#elif !defined(_CORE_ANDROID_)
    if(!chdir(pcPath)) return CORE_OK;
#endif

    return CORE_ERROR_SYSTEM;
}


// ****************************************************************
/* get current working directory */
const char* coreData::GetCurDir()
{
    char* pcString = coreData::__NextString();

#if defined(_CORE_WINDOWS_)
    GetCurrentDirectory(CORE_DATA_STRING_LEN - 1, pcString);
#elif !defined(_CORE_ANDROID_)
    getcwd(pcString, CORE_DATA_STRING_LEN - 1);
#else
    return "";
#endif

    return std::strcat(pcString, CORE_DATA_SLASH);
}


// ******************************************************************
/* open URL with default web-browser */
coreError coreData::OpenURL(const char* pcURL)
{
#if defined(_CORE_WINDOWS_)
    if(int(ShellExecute(NULL, "open", pcURL, NULL, NULL, SW_SHOWNORMAL)) > 32) return CORE_OK;
#elif defined(_CORE_LINUX_)
    if(system(NULL)) if(!system(PRINT("xdg-open %s", pcURL))) return CORE_OK;
#else
    if(system(NULL)) if(!system(PRINT("open %s",     pcURL))) return CORE_OK;
#endif

    return CORE_ERROR_SYSTEM;
}


// ****************************************************************
/* check if file exists */
bool coreData::FileExists(const char* pcPath)
{
    // open file
    SDL_RWops* pFile = SDL_RWFromFile(pcPath, "r");
    if(pFile)
    {
        // file exists
        SDL_RWclose(pFile);
        return true;
    }

    return false;
}


// ****************************************************************
/* retrieve relative paths of all files from a folder */
coreError coreData::ScanFolder(const char* pcPath, const char* pcFilter, std::vector<std::string>* pasOutput)
{
    WARN_IF(!pcPath || !pcFilter || !pasOutput) return CORE_INVALID_INPUT;

#if defined(_CORE_WINDOWS_)

    HANDLE hFolder;
    WIN32_FIND_DATA hFile;

    // open folder
    hFolder = FindFirstFile(PRINT("%s/%s", pcPath, pcFilter), &hFile);
    if(hFolder == INVALID_HANDLE_VALUE)
    {
        Core::Log->Warning("Folder (%s) could not be opened", pcPath);
        return CORE_ERROR_FILE;
    }

    do
    {
        // check and add file path
        if(hFile.cFileName[0] != '.')
            pasOutput->push_back(PRINT("%s/%s", pcPath, hFile.cFileName));
    }
    while(FindNextFile(hFolder, &hFile));

    // close folder
    FindClose(hFolder);

#else

    DIR* pDir;
    dirent* pDirent;

    // open folder
    #if defined(_CORE_ANDROID_)
        pDir = opendir(PRINT("%s/%s", SDL_AndroidGetInternalStoragePath(), pcPath));
    #else
        pDir = opendir(pcPath);
    #endif
    if(!pDir)
    {
        Core::Log->Warning("Folder (%s) could not be opened", pcPath);
        return CORE_ERROR_FILE;
    }

    while((pDirent = readdir(pDir)))
    {
        // check and add file path
        if(pDirent->d_name[0] != '.')
        {
            if(coreData::StrCmpLike(pDirent->d_name, pcFilter))
                pasOutput->push_back(PRINT("%s/%s", pcPath, pDirent->d_name));
        }
    }

    // close folder
    closedir(pDir);

#endif

    return CORE_OK;
}


// ****************************************************************
/* create folder hierarchy */
void coreData::CreateFolder(const std::string& sPath)
{
    int iPos = 0;

    // loop through path
    while((iPos = sPath.find_first_of("/\\", iPos+2)) >= 0)
    {
        const std::string sSubFolder = sPath.substr(0, iPos);

        // create subfolder
#if defined(_CORE_WINDOWS_)
        CreateDirectory(sSubFolder.c_str(), NULL);
#else
        mkdir(sSubFolder.c_str(), S_IRWXU);
#endif
    }
}


// ****************************************************************
/* retrieve current date and time as values */
void coreData::DateTimeValue(coreUint* piYea, coreUint* piMon, coreUint* piDay, coreUint* piHou, coreUint* piMin, coreUint* piSec)
{
    // format current time
    const time_t iTime = std::time(NULL);
    tm* pLocal = std::localtime(&iTime);

    // forward values
    if(piYea) *piYea = pLocal->tm_year + 1900;
    if(piMon) *piMon = pLocal->tm_mon  + 1;
    if(piDay) *piDay = pLocal->tm_mday;
    if(piHou) *piHou = pLocal->tm_hour;
    if(piMin) *piMin = pLocal->tm_min;
    if(piSec) *piSec = pLocal->tm_sec;
}


// ****************************************************************
/* retrieve current date and time as formatted string */
const char* coreData::DateTimePrint(const char* pcFormat)
{
    char* pcString = coreData::__NextString();

    // format current time
    const time_t iTime = std::time(NULL);
    tm* pLocal = std::localtime(&iTime);
    
    // assemble string
    const coreUint iReturn = std::strftime(pcString, CORE_DATA_STRING_LEN, pcFormat, pLocal);

    ASSERT(iReturn)
    return iReturn ? pcString : pcFormat;
}


// ****************************************************************
/* safely get last characters of a string */
const char* coreData::StrRight(const char* pcInput, const coreUint& iNum)
{
    WARN_IF(!pcInput) return "";

    const coreUint iLen = std::strlen(pcInput);
    return pcInput + (iLen - MIN(iLen, iNum));
}


// ****************************************************************
/* safely get file extension */
const char* coreData::StrExtension(const char* pcInput)
{
    WARN_IF(!pcInput) return "";

    const char* pcDot = std::strrchr(pcInput, '.');
    return pcDot ? pcDot+1 : pcInput;
}


// ****************************************************************
/* safely get version number */
float coreData::StrVersion(const char* pcInput)
{
    WARN_IF(!pcInput) return 0.0f;

    const char* pcDot = std::strchr(pcInput, '.');
    return pcDot ? (I_TO_F((pcDot-1)[0] - '0') + 0.1f * I_TO_F((pcDot+1)[0] - '0')) : 0.0f;
}


// ****************************************************************
/* trim a standard string on both sides */
void coreData::StrTrim(std::string* psInput)
{
    // trim right
    const int iLast = psInput->find_last_not_of(" \n\r\t");
    if(iLast >= 0) psInput->erase(iLast+1);

    // trim left
    const int iFirst = psInput->find_first_not_of(" \n\r\t");
    if(iFirst >= 0) psInput->erase(0, iFirst);
}