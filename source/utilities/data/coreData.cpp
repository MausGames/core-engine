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
    #include <VersionHelpers.h>
#elif defined(_CORE_LINUX_)
    #include <dirent.h>
    #include <sys/utsname.h>
#elif defined(_CORE_OSX_)
    #include <mach-o/dyld.h>
#elif defined(_CORE_ANDROID_)
    #include <dirent.h>
    #include <sys/system_properties.h>
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
#elif defined(_CORE_LINUX_)
    const int iLen = readlink("/proc/self/exe", pcString, CORE_DATA_STRING_LEN - 1);
    pcString[MAX(iLen, 0)] = '\0';
#elif defined(_CORE_OSX_)
    coreUint iLen = CORE_DATA_STRING_LEN;
    _NSGetExecutablePath(pcString, &iLen);
#else
    return "";
#endif

    return pcString;
}


// ****************************************************************
/* get operating system name */
const char* coreData::SystemName()
{
#if defined(_CORE_WINDOWS_)

    // detect actual Windows version (GetVersionEx() is deprecated)
    int i, j, k;
    for(i = 5; IsWindowsVersionOrGreater(i, 0, 0); ++i) {} --i; int& iMajor = i;
    for(j = 0; IsWindowsVersionOrGreater(i, j, 0); ++j) {} --j; int& iMinor = j;
    for(k = 0; IsWindowsVersionOrGreater(i, j, k); ++k) {} --k; int& iPack  = k;

    // map to corresponding sub-name
    const char* pcSubString = NULL;
    switch(iMajor*10 + iMinor)
    {
    case 64: pcSubString = "10";    break;
    case 63: pcSubString = "8.1";   break;
    case 62: pcSubString = "8";     break;
    case 61: pcSubString = "7";     break;
    case 60: pcSubString = "Vista"; break;
    case 51: pcSubString = "XP";    break;
    case 50: pcSubString = "2000";  break;

    default:
        pcSubString = PRINT("v%d.%d", iMajor, iMinor);
        break;
    }

    // add service pack string when available
    const char* pcPackString = iPack ? PRINT(" (Service Pack %d)", iPack) : "";

    // return full operating system name
    return PRINT("Windows %s%s", pcSubString, pcPackString);

#elif defined(_CORE_LINUX_)

    // fetch kernel information
    utsname oInfo;
    if(uname(&oInfo)) return "Linux";

    // return full operating system name
    return PRINT("%s %s (%s)", oInfo.sysname, oInfo.release, oInfo.version);

#elif defined(_CORE_ANDROID_)

    char acOS[PROP_VALUE_MAX], acSDK[PROP_VALUE_MAX];

    // fetch operating system and SDK version strings
    __system_property_get("ro.build.version.release", acOS);
    __system_property_get("ro.build.version.sdk",     acSDK);

    // return full operating system name
    return PRINT("Android %s (API level %s)", acOS, acSDK);

#elif defined(_CORE_OSX_)
    return "OSX";
#else
    return "Unknown";
#endif
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


// ****************************************************************
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
coreError coreData::ScanFolder(const char* pcPath, const char* pcFilter, std::vector<std::string>* OUTPUT pasOutput)
{
    WARN_IF(!pcPath || !pcFilter || !pasOutput) return CORE_INVALID_INPUT;

#if defined(_CORE_WINDOWS_)

    HANDLE pFolder;
    WIN32_FIND_DATA oFile;

    // open folder
    pFolder = FindFirstFile(PRINT("%s/%s", pcPath, pcFilter), &oFile);
    if(pFolder == INVALID_HANDLE_VALUE)
    {
        Core::Log->Warning("Folder (%s) could not be opened", pcPath);
        return CORE_ERROR_FILE;
    }

    do
    {
        // check and add file path
        if(oFile.cFileName[0] != '.')
            pasOutput->push_back(PRINT("%s/%s", pcPath, oFile.cFileName));
    }
    while(FindNextFile(pFolder, &oFile));

    // close folder
    FindClose(pFolder);

#else

    DIR* pDir;
    dirent* pEntry;

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

    while((pEntry = readdir(pDir)))
    {
        // check and add file path
        if(pEntry->d_name[0] != '.')
        {
            if(coreData::StrCmpLike(pEntry->d_name, pcFilter))
                pasOutput->push_back(PRINT("%s/%s", pcPath, pEntry->d_name));
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
    std::size_t iPos = 0;

    // loop through path
    while((iPos = sPath.find_first_of("/\\", iPos+2)) != std::string::npos)
    {
        const std::string sSubFolder = sPath.substr(0, iPos);

        // create sub-folder
#if defined(_CORE_WINDOWS_)
        CreateDirectory(sSubFolder.c_str(), NULL);
#else
        mkdir(sSubFolder.c_str(), S_IRWXU);
#endif
    }
}


// ****************************************************************
/* retrieve current date and time as values */
void coreData::DateTimeValue(coreUint* OUTPUT piYea, coreUint* OUTPUT piMon, coreUint* OUTPUT piDay, coreUint* OUTPUT piHou, coreUint* OUTPUT piMin, coreUint* OUTPUT piSec)
{
    // format current time
    const std::time_t iTime = std::time(NULL);
    std::tm* pLocal = std::localtime(&iTime);

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
    const std::time_t iTime = std::time(NULL);
    std::tm* pLocal = std::localtime(&iTime);

    // assemble string
    const std::size_t iReturn = std::strftime(pcString, CORE_DATA_STRING_LEN, pcFormat, pLocal);

    ASSERT(iReturn)
    return iReturn ? pcString : pcFormat;
}


// ****************************************************************
/* safely get last characters of a string */
const char* coreData::StrRight(const char* pcInput, const coreUint& iNum)
{
    WARN_IF(!pcInput) return "";

    const coreUint iLen = coreUint(std::strlen(pcInput));
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
void coreData::StrTrim(std::string* OUTPUT psInput)
{
    // trim right
    const std::size_t iLast = psInput->find_last_not_of(" \n\r\t");
    if(iLast != std::string::npos) psInput->erase(iLast+1);

    // trim left
    const std::size_t iFirst = psInput->find_first_not_of(" \n\r\t");
    if(iFirst != std::string::npos) psInput->erase(0, iFirst);
}


// ****************************************************************
/* replace all occurrences of a sub-string with another one*/
void coreData::StrReplace(std::string* OUTPUT psInput, const char* pcOld, const char* pcNew)
{
    std::size_t iPos = 0;

    // save length of both sub-strings
    const std::size_t iOldLen = std::strlen(pcOld);
    const std::size_t iNewLen = std::strlen(pcNew);

    // loop only once and replace all findings
    while((iPos = psInput->find(pcOld, iPos)) != std::string::npos)
    {
        psInput->replace(iPos, iOldLen, pcNew);
        iPos += iNewLen;
    }
}