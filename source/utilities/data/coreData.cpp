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
    #include <Shellapi.h>
#endif
#if defined(_CORE_LINUX_)
    #include <dirent.h>
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
// check if file exists
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
// retrieve relative paths of all files from a folder
// TODO: implement for Android
coreError coreData::FolderSearch(const char* pcPath, const char* pcFilter, std::vector<std::string>* pasOutput)
{
    if(!pcPath || !pasOutput) return CORE_INVALID_INPUT;

#if defined(_CORE_WINDOWS_)

    HANDLE hFolder;
    WIN32_FIND_DATA hFile;

    // open folder
    hFolder = FindFirstFile(coreData::Print("%s/%s/%s", coreData::AppPath(), pcPath, pcFilter), &hFile);
    if(hFolder == INVALID_HANDLE_VALUE)
    {
        Core::Log->Error(false, "Folder (%s) could not be opened", pcPath);
        return CORE_FILE_ERROR;
    }

    do
    {
        // check and add file path
        if(hFile.cFileName[0] != '.')
            pasOutput->push_back(coreData::Print("%s/%s", pcPath, hFile.cFileName));
    }
    while(FindNextFile(hFolder, &hFile));

    // close folder
    FindClose(hFolder);

#else

    DIR* pDir;
    struct dirent* pDirent;

    // open folder
    pDir = opendir(pcPath);
    if(!pDir)
    {
        Core::Log->Error(false, "Folder (%s) could not be opened", pcPath);
        return CORE_FILE_ERROR;
    }

    while((pDirent = readdir(pDir)) != NULL)
    {
        // check and add file path
        if(pDirent->d_name[0] != '.')
        {
            if(coreData::StrCompare(pDirent->d_name, pcFilter))
                pasOutput->push_back(coreData::Print("%s/%s", pcPath, pDirent->d_name));
        }
    }

    // close folder
    closedir(pDir);

#endif

    return CORE_OK;
}


// ****************************************************************
// create folder hierarchy
// TODO: remove std::string
void coreData::FolderCreate(const std::string& sPath)
{
    int iPos = 0;

    // loop through path
    while((iPos = sPath.find_first_of("/\\", iPos+2)) >= 0)
    {
        const std::string sSubFolder = sPath.substr(0, iPos);

        // create subfolder
#if defined(_CORE_WINDOWS_)
        CreateDirectoryA(sSubFolder.c_str(), NULL);
#else
        mkdir(sSubFolder.c_str(), S_IRWXU);
#endif
    }
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
// compare strings with wildcards
bool coreData::StrCompare(const char* s, const char* t)
{
    return *t-'*' ? *s ? (*t=='?') | (toupper(*s)==toupper(*t)) && StrCompare(s+1,t+1) : !*t : StrCompare(s,t+1) || (*s && StrCompare(s+1,t));
}


// ****************************************************************
// get last characters of a string
const char* coreData::StrRight(const char* pcInput, const coreUint& iNum)
{
    SDL_assert(pcInput);

    const coreUint iLen = std::strlen(pcInput);
    return pcInput + (iLen-coreMath::Min(iLen, iNum));
}


// ****************************************************************
// safely get file extension
const char* coreData::StrExtension(const char* pcInput)
{
    ASSERT_IF(!pcInput) return "";

    const char* pcDot = std::strrchr(pcInput, '.');
    return pcDot ? pcDot+1 : pcInput;
}


// ****************************************************************
// safely get version number
float coreData::StrVersion(const char* pcInput)
{
    ASSERT_IF(!pcInput) return 0.0f;

    const char* pcDot = std::strchr(pcInput, '.');
    return pcDot ? (float((pcDot-1)[0]-'0') + 0.1f*float((pcDot+1)[0]-'0')) : 0.0f;
}


// ****************************************************************
// move string pointer and skip comments
void coreData::StrSkip(const char** ppcInput, const int &iNum)
{
    SDL_assert(*ppcInput);

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


// ****************************************************************
// trim a standard string on both sides
void coreData::StrTrim(std::string* psInput)
{
    const int iFirst = psInput->find_first_not_of(" \n\r\t");
    if(iFirst >= 0) psInput->erase(0, iFirst);

    const int iLast = psInput->find_last_not_of(" \n\r\t");
    if(iLast >= 0) psInput->erase(iLast+1);
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