///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

#if defined(_CORE_WINDOWS_)
    #include <shellapi.h>
    #include <Psapi.h>
    #include <Shlobj.h>
#elif defined(_CORE_LINUX_)
    #include <dirent.h>
    #include <sys/utsname.h>
    #include <pwd.h>
#elif defined(_CORE_MACOS_)
    #include <mach-o/dyld.h>
    #include <pwd.h>
#elif defined(_CORE_ANDROID_)
    #include <dirent.h>
    #include <sys/system_properties.h>
#endif

thread_local coreData::coreTempString coreData::s_TempString     = {};
thread_local coreUintW                coreData::s_iCurString     = 0u;
coreLookupStr<const coreChar*>        coreData::s_apcCommandLine = {};
std::string                           coreData::s_sUserFolder    = "";

extern "C" const coreChar* g_pcUserFolder = "";   // to allow access from C files


// ****************************************************************
/* get amount of memory physically mapped to the application */
coreUint64 coreData::AppMemory()
{
#if defined(_CORE_WINDOWS_)

    PROCESS_MEMORY_COUNTERS oMemory;

    // retrieve memory usage information
    if(GetProcessMemoryInfo(GetCurrentProcess(), &oMemory, sizeof(oMemory)))
    {
        // return current working set size
        return oMemory.WorkingSetSize;
    }

#elif defined(_CORE_LINUX_)

    // open memory pseudo-file
    std::FILE* pFile = std::fopen("/proc/self/statm", "r");
    if(pFile)
    {
        coreUint64 iPages;

        // read resident set size (in pages)
        std::fscanf(pFile, "%*s %lu", &iPages);
        std::fclose(pFile);

        // multiply with page-size and return
        return iPages * sysconf(_SC_PAGESIZE);
    }

#endif

    return 0u;
}


// ****************************************************************
/* get full application path */
const coreChar* coreData::AppPath()
{
    UNUSED coreChar* pcString = coreData::__NextTempString();

#if defined(_CORE_WINDOWS_)

    // get path of the current executable
    if(GetModuleFileNameA(NULL, pcString, CORE_DATA_STRING_LEN))
    {
        // return path
        return pcString;
    }

#elif defined(_CORE_LINUX_)

    // get content of the symbolic link
    const coreInt32 iLen = readlink("/proc/self/exe", pcString, CORE_DATA_STRING_LEN - 1u);
    if(iLen >= 0)
    {
        // add missing null-terminator and return
        pcString[iLen] = '\0';
        return pcString;
    }

#elif defined(_CORE_MACOS_)

    // get path of the current executable
    coreUint32 iLen = CORE_DATA_STRING_LEN;
    if(!_NSGetExecutablePath(pcString, &iLen))
    {
        // return path
        return pcString;
    }

#endif

    return "";
}


// ****************************************************************
/* get operating system name */
const coreChar* coreData::SystemName()
{
#if defined(_CORE_WINDOWS_)

    coreUint16 iMajor    = 0u;
    coreUint16 iMinor    = 0u;
    coreUint16 iBuild    = 0u;
    coreUint16 iRevision = 0u;

    // fetch version from system library
    coreChar acPath[MAX_PATH];
    if(GetModuleFileNameA(GetModuleHandleA("kernel32.dll"), acPath, MAX_PATH))
    {
        // get file version info size
        const coreUint32 iDataSize = GetFileVersionInfoSizeA(acPath, NULL);
        if(iDataSize)
        {
            // get file version info
            coreByte* pData = new coreByte[iDataSize];
            if(GetFileVersionInfoA(acPath, 0u, iDataSize, pData))
            {
                // access root block
                VS_FIXEDFILEINFO* pInfo;
                coreUint32        iInfoSize;
                if(VerQueryValueA(pData, "\\", r_cast<void**>(&pInfo), &iInfoSize) && (pInfo->dwSignature == 0xFEEF04BDu))
                {
                    // extract version numbers
                    iMajor    = HIWORD(pInfo->dwProductVersionMS);
                    iMinor    = LOWORD(pInfo->dwProductVersionMS);
                    iBuild    = HIWORD(pInfo->dwProductVersionLS);
                    iRevision = LOWORD(pInfo->dwProductVersionLS);
                }
            }
            SAFE_DELETE_ARRAY(pData)
        }
    }

    // map to corresponding sub-name
    const coreChar* pcSubName;
    switch(iMajor*10u + iMinor)
    {
    case 100u: pcSubName = "10";    break;
    case  63u: pcSubName = "8.1";   break;
    case  62u: pcSubName = "8";     break;
    case  61u: pcSubName = "7";     break;
    case  60u: pcSubName = "Vista"; break;
    case  51u: pcSubName = "XP";    break;
    case  50u: pcSubName = "2000";  break;
    default:   pcSubName = "-";     break;
    }

    // return full operating system name
    return PRINT("Windows %s (%u.%u.%u.%u)", pcSubName, iMajor, iMinor, iBuild, iRevision);

#elif defined(_CORE_LINUX_)

    // fetch kernel information
    utsname oInfo;
    if(uname(&oInfo)) return "Linux";

    // return full operating system name
    return PRINT("%s %s (%s)", oInfo.sysname, oInfo.release, oInfo.version);

#elif defined(_CORE_MACOS_)

    return "macOS";

#elif defined(_CORE_ANDROID_)

    coreChar acOS[PROP_VALUE_MAX], acSDK[PROP_VALUE_MAX];

    // fetch operating system and SDK version strings
    __system_property_get("ro.build.version.release", acOS);
    __system_property_get("ro.build.version.sdk",     acSDK);

    // return full operating system name
    return PRINT("Android %s (API Level %s)", acOS, acSDK);

#endif
}


// ****************************************************************
/* get user name */
const coreChar* coreData::SystemUserName()
{
#if defined(_CORE_WINDOWS_)

    coreChar* pcString = coreData::__NextTempString();

    // get user name associated with current thread
    DWORD nSize = CORE_DATA_STRING_LEN;
    if(GetUserNameA(pcString, &nSize)) return pcString;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    // get user name from password database
    const passwd* pRecord = getpwuid(geteuid());
    if(pRecord && pRecord->pw_name) return pRecord->pw_name;

    // get user name from controlling terminal
    const coreChar* pcLogin = getlogin();
    if(pcLogin) return pcLogin;

#endif

    return "";
}


// ****************************************************************
/* get path to store application data */
const coreChar* coreData::SystemDirAppData()
{
#if defined(_CORE_WINDOWS_)

    // get default roaming directory
    wchar_t* pcRoamingPath;
    if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT, NULL, &pcRoamingPath)))
    {
        coreChar* pcString = coreData::__NextTempString();

        // convert characters
        const coreInt32 iLen = WideCharToMultiByte(CP_UTF8, 0u, pcRoamingPath, -1, pcString, CORE_DATA_STRING_LEN - 1, NULL, NULL);
        CoTaskMemFree(pcRoamingPath);

        if(iLen)
        {
            // prepare path and return
            const coreChar* pcPath = coreData::__PrepareSystemDir(pcString);
            if(pcPath) return pcPath;
        }
    }

#elif defined(_CORE_LINUX_)

    const coreChar* pcPath;

    // get directory from XDG variable
    if((pcPath = std::getenv("XDG_DATA_HOME")) && (pcPath = coreData::__PrepareSystemDir(pcPath)))
        return pcPath;

    // get directory from home variable
    if((pcPath = std::getenv("HOME")) && (pcPath = coreData::__PrepareSystemDir(PRINT("%s/.local/share", pcPath))))
        return pcPath;

    // get directory from password database
    const passwd* pRecord = getpwuid(geteuid());
    if(pRecord && pRecord->pw_dir && (pcPath = coreData::__PrepareSystemDir(PRINT("%s/.local/share", pRecord->pw_dir))))
        return pcPath;

#endif

    return "";
}


// ****************************************************************
/* get path to store temporary data */
const coreChar* coreData::SystemDirTemp()
{
#if defined(_CORE_WINDOWS_)

    // get default temporary directory
    coreChar acTempPath[MAX_PATH];
    if(GetTempPathA(MAX_PATH, acTempPath))
    {
        // prepare path and return
        const coreChar* pcPath = coreData::__PrepareSystemDir(acTempPath);
        if(pcPath) return pcPath;
    }

#elif defined(_CORE_LINUX_)

    const coreChar* pcPath;

    // get directory from XDG variable
    if((pcPath = std::getenv("XDG_CACHE_HOME")) && (pcPath = coreData::__PrepareSystemDir(pcPath)))
        return pcPath;

    // get directory from home variable
    if((pcPath = std::getenv("HOME")) && (pcPath = coreData::__PrepareSystemDir(PRINT("%s/.cache", pcPath))))
        return pcPath;

    // get directory from password database
    const passwd* pRecord = getpwuid(geteuid());
    if(pRecord && pRecord->pw_dir && (pcPath = coreData::__PrepareSystemDir(PRINT("%s/.cache", pRecord->pw_dir))))
        return pcPath;

#endif

    return "";
}


// ****************************************************************
/* set current working directory */
coreStatus coreData::SetCurDir(const coreChar* pcPath)
{
#if defined(_CORE_WINDOWS_)

    if(SetCurrentDirectoryA(pcPath)) return CORE_OK;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    if(!chdir(pcPath)) return CORE_OK;

#endif

    return CORE_ERROR_SYSTEM;
}


// ****************************************************************
/* get current working directory */
const coreChar* coreData::GetCurDir()
{
    UNUSED coreChar* pcString = coreData::__NextTempString();

#if defined(_CORE_WINDOWS_)

    // get raw working directory
    const coreUint32 iLen = GetCurrentDirectoryA(CORE_DATA_STRING_LEN - 1u, pcString);
    if(iLen)
    {
        // add path delimiter and return (with known length)
        std::memcpy(pcString + iLen, CORE_DATA_SLASH, ARRAY_SIZE(CORE_DATA_SLASH));
        return pcString;
    }

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    // get raw working directory
    if(getcwd(pcString, CORE_DATA_STRING_LEN - 1u))
    {
        // add path delimiter and return
        return std::strcat(pcString, CORE_DATA_SLASH);
    }

#endif

    return "";
}


// ****************************************************************
/* set command line arguments */
void coreData::SetCommandLine(const coreInt32 iArgc, coreChar** ppcArgv)
{
    ASSERT(ppcArgv)

    // loop through all arguments
    for(coreUintW i = 1u, ie = iArgc; i < ie; ++i)
    {
        const coreChar* pcCurrent = ppcArgv[i];
        ASSERT(pcCurrent)

        // insert valid entries
        if((*pcCurrent) == '-')
        {
            while((*(++pcCurrent)) == '-') {}
            s_apcCommandLine[coreData::StrLower(pcCurrent)] = ((i+1u < ie) && ((*ppcArgv[i+1u]) != '-')) ? ppcArgv[i+1u] : "";
        }
    }
}


// ****************************************************************
/* initialize user folder */
void coreData::InitUserFolder()
{
    // get command line argument
    const coreChar* pcPath = coreData::GetCommandLine("user-folder-path");

    // use specific user folder
    if(pcPath && pcPath[0])
    {
             if(!std::strcmp(pcPath, "!appdata")) pcPath = coreData::SystemDirAppData();
        else if(!std::strcmp(pcPath, "!temp"))    pcPath = coreData::SystemDirTemp();
        else                                      pcPath = PRINT("%s" CORE_DATA_SLASH, pcPath);
    }

    // use default user folder (and create folder hierarchy)
    if(!pcPath || !pcPath[0] || (coreData::CreateFolder(pcPath) != CORE_OK))
    {
        pcPath = "user/";
        coreData::CreateFolder(pcPath);
    }

    // save selected user folder
    s_sUserFolder  = pcPath;
    g_pcUserFolder = s_sUserFolder.c_str();

    // copy configuration file
    const coreChar* pcSource = "user/config.ini";
    const coreChar* pcTarget = coreData::UserFolder("config.ini");
    if(!coreData::FileExists(pcTarget)) coreData::FileCopy(pcSource, pcTarget);
}


// ****************************************************************
/* open URL with default web-browser */
coreStatus coreData::OpenURL(const coreChar* pcURL)
{
#if defined(_CORE_WINDOWS_)

    // delegate request to the Windows Shell
    if(P_TO_SI(ShellExecuteA(NULL, "open", pcURL, NULL, NULL, SW_SHOWNORMAL)) > 32) return CORE_OK;

#elif defined(_CORE_LINUX_)

    // delegate request to the Linux command processor (/bin/sh)
    if(std::system(NULL) && !std::system(PRINT("xdg-open %s", pcURL))) return CORE_OK;

#elif defined(_CORE_MACOS_)

    // delegate request to the OSX command processor
    if(std::system(NULL) && !std::system(PRINT("open %s", pcURL))) return CORE_OK;

#endif

    return CORE_ERROR_SYSTEM;
}


// ****************************************************************
/* check if file exists */
coreBool coreData::FileExists(const coreChar* pcPath)
{
#if defined(_CORE_WINDOWS_)

    // quick Windows check
    if(GetFileAttributesA(pcPath) != INVALID_FILE_ATTRIBUTES) return true;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    struct stat oBuffer;

    // quick POSIX check
    if(!stat(pcPath, &oBuffer)) return true;

#else

    // open file
    SDL_RWops* pFile = SDL_RWFromFile(pcPath, "rb");
    if(pFile)
    {
        // file exists
        SDL_RWclose(pFile);
        return true;
    }

#endif

    return false;
}


// ****************************************************************
/* retrieve file size */
coreInt64 coreData::FileSize(const coreChar* pcPath)
{
#if defined(_CORE_WINDOWS_)

    WIN32_FILE_ATTRIBUTE_DATA oAttributes;

    // get extended file attributes
    if(GetFileAttributesExA(pcPath, GetFileExInfoStandard, &oAttributes))
    {
        // return combined file size
        return (coreInt64(oAttributes.nFileSizeHigh) << 32u) |
               (coreInt64(oAttributes.nFileSizeLow));
    }

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    struct stat oBuffer;

    // get POSIX file info
    if(!stat(pcPath, &oBuffer))
    {
        // return file size
        return oBuffer.st_size;
    }

#else

    // open file
    SDL_RWops* pFile = SDL_RWFromFile(pcPath, "rb");
    if(pFile)
    {
        // get size from stream
        const coreInt64 iSize = SDL_RWsize(pFile);
        SDL_RWclose(pFile);

        return iSize;
    }

#endif

    return -1;
}


// ****************************************************************
/* retrieve file write time */
std::time_t coreData::FileWriteTime(const coreChar* pcPath)
{
#if defined(_CORE_WINDOWS_)

    WIN32_FILE_ATTRIBUTE_DATA oAttributes;

    // get extended file attributes
    if(GetFileAttributesExA(pcPath, GetFileExInfoStandard, &oAttributes))
    {
        // return converted file write time
        return r_cast<coreUint64&>(oAttributes.ftLastWriteTime) / 10000000ull - 11644473600ull;
    }

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    struct stat oBuffer;

    // get POSIX file info
    if(!stat(pcPath, &oBuffer))
    {
        // return file write time
        return oBuffer.st_mtime;
    }

#endif

    return -1;
}


// ****************************************************************
/* copy file from source to destination (and replace) */
coreStatus coreData::FileCopy(const coreChar* pcFrom, const coreChar* pcTo)
{
#if defined(_CORE_WINDOWS_)

    // copy directly (with attributes)
    if(CopyFileA(pcFrom, pcTo, false)) return CORE_OK;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    // open source file
    std::FILE* pFileFrom = std::fopen(pcFrom, "rb");
    if(pFileFrom)
    {
        // open destination file
        std::FILE* pFileTo = std::fopen(pcTo, "wb");
        if(pFileTo)
        {
            alignas(ALIGNMENT_PAGE) BIG_STATIC coreByte aBuffer[0x4000u];

            // copy all data
            while(!std::feof(pFileFrom))
            {
                const coreUintW iResult = std::fread(aBuffer, 1u, ARRAY_SIZE(aBuffer), pFileFrom);
                std::fwrite(aBuffer, 1u, iResult, pFileTo);
            }

            // close both files
            std::fclose(pFileTo);
            std::fclose(pFileFrom);

            return CORE_OK;
        }

        // close source file
        std::fclose(pFileFrom);
    }

#endif

    return CORE_ERROR_FILE;
}


// ****************************************************************
/* move file from source to destination (and replace) */
coreStatus coreData::FileMove(const coreChar* pcFrom, const coreChar* pcTo)
{
#if defined(_CORE_WINDOWS_)

    if(MoveFileExA(pcFrom, pcTo, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) return CORE_OK;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    if(!std::rename(pcFrom, pcTo)) return CORE_OK;

#endif

    return CORE_ERROR_FILE;
}


// ****************************************************************
/* delete existing file */
coreStatus coreData::FileDelete(const coreChar* pcPath)
{
#if defined(_CORE_WINDOWS_)

    if(DeleteFileA(pcPath)) return CORE_OK;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    if(!unlink(pcPath)) return CORE_OK;

#endif

    return CORE_ERROR_FILE;
}


// ****************************************************************
/* retrieve relative paths of all files from a folder */
coreStatus coreData::ScanFolder(const coreChar* pcPath, const coreChar* pcFilter, std::vector<std::string>* OUTPUT pasOutput)
{
    WARN_IF(!pcPath || !pcFilter || !pasOutput) return CORE_INVALID_INPUT;

#if defined(_CORE_WINDOWS_)

    HANDLE pFolder;
    WIN32_FIND_DATAA oFile;

    // open folder
    pFolder = FindFirstFileExA(PRINT("%s/%s", pcPath, pcFilter), FindExInfoBasic, &oFile, FindExSearchNameMatch, NULL, 0u);
    if(pFolder == INVALID_HANDLE_VALUE)
    {
        Core::Log->Warning("Folder (%s/%s) could not be opened", pcPath, pcFilter);
        return CORE_ERROR_FILE;
    }

    do
    {
        // check and add file path
        if(oFile.cFileName[0] != '.')
            pasOutput->push_back(PRINT("%s/%s", pcPath, oFile.cFileName));
    }
    while(FindNextFileA(pFolder, &oFile));

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
        Core::Log->Warning("Folder (%s/%s) could not be opened", pcPath, pcFilter);
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
coreStatus coreData::CreateFolder(const coreChar* pcPath)
{
    coreChar* pcString = coreData::__NextTempString();
    coreChar* pcCursor = pcString;

    // make local copy
    coreData::StrCopy(pcPath, pcString, CORE_DATA_STRING_LEN);

    // loop through all sub-folders
    for(; (*pcCursor) != '\0'; ++pcCursor)
    {
        if(((*pcCursor) == '/') || ((*pcCursor) == '\\'))
        {
            // build temporary path
            const coreChar cTemp = (*pcCursor);
            (*pcCursor) = '\0';

            // create sub-folder
#if defined(_CORE_WINDOWS_)
            if(!CreateDirectoryA(pcString, NULL) && (GetLastError() != ERROR_ALREADY_EXISTS)) return CORE_ERROR_FILE;
#else
            if(mkdir(pcString, S_IRWXU) && (errno != EEXIST)) return CORE_ERROR_FILE;
#endif
            // reset path
            (*pcCursor) = cTemp;
        }
    }

    return CORE_OK;
}


// ****************************************************************
/* retrieve date and time as values */
void coreData::DateTimeValue(coreUint16* OUTPUT piYea, coreUint16* OUTPUT piMon, coreUint16* OUTPUT piDay, coreUint16* OUTPUT piHou, coreUint16* OUTPUT piMin, coreUint16* OUTPUT piSec, const std::tm* pTimeMap)
{
    // forward values
    if(piYea) (*piYea) = pTimeMap->tm_year + 1900u;
    if(piMon) (*piMon) = pTimeMap->tm_mon  + 1u;
    if(piDay) (*piDay) = pTimeMap->tm_mday;
    if(piHou) (*piHou) = pTimeMap->tm_hour;
    if(piMin) (*piMin) = pTimeMap->tm_min;
    if(piSec) (*piSec) = pTimeMap->tm_sec;
}


// ****************************************************************
/* retrieve date and time as formatted string */
const coreChar* coreData::DateTimePrint(const coreChar* pcFormat, const std::tm* pTimeMap)
{
    coreChar* pcString = coreData::__NextTempString();

    // read arguments and assemble string
    const coreUintW iReturn = std::strftime(pcString, CORE_DATA_STRING_LEN, pcFormat, pTimeMap);
    WARN_IF(!iReturn) return pcFormat;

    return pcString;
}


// ****************************************************************
/* compress data with Zstandard library */
coreStatus coreData::Compress(const coreByte* pInput, const coreUint32 iInputSize, coreByte** OUTPUT ppOutput, coreUint32* OUTPUT piOutputSize, const coreInt32 iLevel)
{
    ASSERT(pInput && iInputSize && ppOutput && piOutputSize && (iLevel >= ZSTD_minCLevel()) && (iLevel <= ZSTD_maxCLevel()))

    // retrieve required output size
    const coreUintW iBound  = ZSTD_compressBound(iInputSize);
    coreByte*       pBuffer = new coreByte[iBound + sizeof(coreUint32)];

    // compress data
    const coreUintW iWritten = ZSTD_compress(pBuffer + sizeof(coreUint32), iBound, pInput, iInputSize, iLevel);
    if(ZSTD_isError(iWritten))
    {
        Core::Log->Warning("Error compressing data (ZSTD: %s)", ZSTD_getErrorName(iWritten));
        SAFE_DELETE_ARRAY(pBuffer)
        return CORE_INVALID_INPUT;
    }

    // store original size and return compressed data
    (*r_cast<coreUint32*>(pBuffer)) = iInputSize;
    (*ppOutput)     = pBuffer;
    (*piOutputSize) = iWritten + sizeof(coreUint32);

    return CORE_OK;
}


// ****************************************************************
/* decompress data with Zstandard library */
coreStatus coreData::Decompress(const coreByte* pInput, const coreUint32 iInputSize, coreByte** OUTPUT ppOutput, coreUint32* OUTPUT piOutputSize)
{
    ASSERT(pInput && iInputSize && ppOutput && piOutputSize)

    // retrieve original size
    const coreUint32 iBound  = (*r_cast<const coreUint32*>(pInput));
    coreByte*        pBuffer = new coreByte[iBound];

    // decompress data
    const coreUintW iWritten = ZSTD_decompress(pBuffer, iBound, pInput + sizeof(coreUint32), iInputSize - sizeof(coreUint32));
    if(ZSTD_isError(iWritten))
    {
        Core::Log->Warning("Error decompressing data (ZSTD: %s)", ZSTD_getErrorName(iWritten));
        SAFE_DELETE_ARRAY(pBuffer)
        return CORE_INVALID_INPUT;
    }

    // return decompressed data
    (*ppOutput)     = pBuffer;
    (*piOutputSize) = iBound;

    return CORE_OK;
}


// ****************************************************************
/* scramble data with 32-bit key */
void coreData::Scramble(coreByte* OUTPUT pData, const coreUintW iSize, const coreUint32 iKey)
{
    // create sequence
    coreRand oRand(iKey);

    // combine sequence with original data
    for(coreUintW i = 0u; i < iSize; ++i)
        pData[i] = (oRand.Raw() & 0xFFu) - pData[i];
}

void coreData::Unscramble(coreByte* OUTPUT pData, const coreUintW iSize, const coreUint32 iKey)
{
    // scramble again (operation is involutory)
    coreData::Scramble(pData, iSize, iKey);
}


// ****************************************************************
/* safely get last characters of a string */
const coreChar* coreData::StrRight(const coreChar* pcInput, const coreUintW iNum)
{
    WARN_IF(!pcInput) return "";

    const coreUintW iLen = std::strlen(pcInput);
    return pcInput + (iLen - MIN(iLen, iNum));
}


// ****************************************************************
/* safely get file name from path */
const coreChar* coreData::StrFilename(const coreChar* pcInput)
{
    WARN_IF(!pcInput) return "";

    const coreChar* pcSlash = std::strrchr(pcInput, '/');
    if(!pcSlash)    pcSlash = std::strrchr(pcInput, '\\');
    return pcSlash ? (pcSlash + 1u) : pcInput;
}


// ****************************************************************
/* safely get file extension */
const coreChar* coreData::StrExtension(const coreChar* pcInput)
{
    WARN_IF(!pcInput) return "";

    const coreChar* pcDot = std::strrchr(pcInput, '.');
    return pcDot ? (pcDot + 1u) : "";
}


// ****************************************************************
/* safely get version number */
coreFloat coreData::StrVersion(const coreChar* pcInput)
{
    WARN_IF(!pcInput) return 0.0f;

    const coreChar* pcDot = std::strchr(pcInput, '.');
    return pcDot ? (I_TO_F((pcDot - 1u)[0] - '0') + 0.1f * I_TO_F((pcDot + 1u)[0] - '0')) : 0.0f;
}


// ****************************************************************
/* copy string into another buffer */
void coreData::StrCopy(const coreChar* pcInput, coreChar* OUTPUT pcOutput, const coreUintW iMaxLen)
{
    ASSERT(pcInput && pcOutput)

    // calculate string length
    const coreUintW iLen = MIN(std::strlen(pcInput), iMaxLen - 1u);
    WARN_IF(std::strlen(pcInput) >= iMaxLen) {}

    // copy string with guaranteed null-termination
    std::memcpy(pcOutput, pcInput, iLen);
    pcOutput[iLen] = '\0';
}


// ****************************************************************
/* trim a standard string on both sides */
void coreData::StrTrim(std::string* OUTPUT psInput)
{
    STATIC_ASSERT(std::string::npos == -1)

    // trim right
    const coreUintW iLast = psInput->find_last_not_of(" \n\r\t");
    psInput->erase(iLast + 1u);

    // trim left
    const coreUintW iFirst = psInput->find_first_not_of(" \n\r\t");
    if(iFirst != std::string::npos) psInput->erase(0u, iFirst);
}


// ****************************************************************
/* replace all occurrences of a sub-string with another one */
void coreData::StrReplace(std::string* OUTPUT psInput, const coreChar* pcOld, const coreChar* pcNew)
{
    coreUintW iPos = 0u;

    // save length of both sub-strings
    const coreUintW iOldLen = std::strlen(pcOld);
    const coreUintW iNewLen = std::strlen(pcNew);

    // loop only once and replace all findings
    while((iPos = psInput->find(pcOld, iPos, iOldLen)) != std::string::npos)
    {
        psInput->replace(iPos, iOldLen, pcNew);
        iPos += iNewLen;
    }
}


// ****************************************************************
/* prepare path for system directory */
const coreChar* coreData::__PrepareSystemDir(const coreChar* pcPath)
{
    // get folder name from application name
    static std::string sIdentifier;
    if(sIdentifier.empty())
    {
        sIdentifier = Core::Application->Settings.Name;
        coreData::StrReplace(&sIdentifier, " ", "");
    }

    // create full path
    const coreChar* pcFullPath = PRINT("%s" CORE_DATA_SLASH "%s" CORE_DATA_SLASH, pcPath, sIdentifier.c_str());

    // create folder hierarchy (and check if path is valid)
    if(coreData::CreateFolder(pcFullPath) != CORE_OK) return NULL;

    return pcFullPath;
}