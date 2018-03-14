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
    #include <Psapi.h>
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

thread_local coreChar  coreData::s_aacString[CORE_DATA_STRING_NUM][CORE_DATA_STRING_LEN] = {{}};
thread_local coreUintW coreData::s_iCurString                                            = 0u;


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
        std::fscanf(pFile, "%*s %llu", &iPages);
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
    UNUSED coreChar* pcString = coreData::__NextString();

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

#elif defined(_CORE_OSX_)

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

    // detect actual Windows version (GetVersionEx() is deprecated)
    coreUint32 i, j, k;
    for(i = 5u; IsWindowsVersionOrGreater(i, 0u, 0u); ++i) {} --i; const coreUint32& iMajor = i;
    for(j = 0u; IsWindowsVersionOrGreater(i,  j, 0u); ++j) {} --j; const coreUint32& iMinor = j;
    for(k = 0u; IsWindowsVersionOrGreater(i,  j,  k); ++k) {} --k; const coreUint32& iPack  = k;

    // map to corresponding sub-name
    const coreChar* pcSubString;
    switch(iMajor*10u + iMinor)
    {
    case 100u: pcSubString = "10";    break;
    case  63u: pcSubString = "8.1";   break;
    case  62u: pcSubString = "8";     break;
    case  61u: pcSubString = "7";     break;
    case  60u: pcSubString = "Vista"; break;
    case  51u: pcSubString = "XP";    break;
    case  50u: pcSubString = "2000";  break;

    default:
        pcSubString = PRINT("v%u.%u", iMajor, iMinor);
        break;
    }

    // add service pack string when available
    const coreChar* pcPackString = iPack ? PRINT(" (Service Pack %u)", iPack) : "";

    // return full operating system name
    return PRINT("Windows %s%s", pcSubString, pcPackString);

#elif defined(_CORE_LINUX_)

    // fetch kernel information
    utsname oInfo;
    if(uname(&oInfo)) return "Linux";

    // return full operating system name
    return PRINT("%s %s (%s)", oInfo.sysname, oInfo.release, oInfo.version);

#elif defined(_CORE_ANDROID_)

    coreChar acOS[PROP_VALUE_MAX], acSDK[PROP_VALUE_MAX];

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
coreStatus coreData::SetCurDir(const coreChar* pcPath)
{
#if defined(_CORE_WINDOWS_)

    if(SetCurrentDirectoryA(pcPath)) return CORE_OK;

#elif defined(_CORE_LINUX_) || defined(_CORE_OSX_)

    if(!chdir(pcPath)) return CORE_OK;

#endif

    return CORE_ERROR_SYSTEM;
}


// ****************************************************************
/* get current working directory */
const coreChar* coreData::GetCurDir()
{
    UNUSED coreChar* pcString = coreData::__NextString();

#if defined(_CORE_WINDOWS_)

    // get raw working directory
    const coreUint32 iLen = GetCurrentDirectoryA(CORE_DATA_STRING_LEN - 1u, pcString);
    if(iLen)
    {
        // add path delimiter and return (with known length)
        std::memcpy(pcString + iLen, CORE_DATA_SLASH, ARRAY_SIZE(CORE_DATA_SLASH));
        return pcString;
    }

#elif defined(_CORE_LINUX_) || defined(_CORE_OSX_)

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
/* open URL with default web-browser */
coreStatus coreData::OpenURL(const coreChar* pcURL)
{
#if defined(_CORE_WINDOWS_)

    // delegate request to the Windows Shell
    if(P_TO_SI(ShellExecuteA(NULL, "open", pcURL, NULL, NULL, SW_SHOWNORMAL)) > 32) return CORE_OK;

#elif defined(_CORE_LINUX_)

    // delegate request to the Linux command processor (/bin/sh)
    if(std::system(NULL) && !std::system(PRINT("xdg-open %s", pcURL))) return CORE_OK;

#else defined(_CORE_OSX_)

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

#elif defined(_CORE_LINUX_) || defined(_CORE_OSX_)

    struct stat oBuffer;

    // quick POSIX check
    if(!stat(pcPath, &oBuffer) return true;

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

#elif defined(_CORE_LINUX_) || defined(_CORE_OSX_)

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

#elif defined(_CORE_LINUX_) || defined(_CORE_OSX_)

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
/* retrieve relative paths of all files from a folder */
coreStatus coreData::ScanFolder(const coreChar* pcPath, const coreChar* pcFilter, std::vector<std::string>* OUTPUT pasOutput)
{
    WARN_IF(!pcPath || !pcFilter || !pasOutput) return CORE_INVALID_INPUT;

#if defined(_CORE_WINDOWS_)

    HANDLE pFolder;
    WIN32_FIND_DATAA oFile;

    // improve performance if possible
    static const FINDEX_INFO_LEVELS s_iInfoLevel = IsWindowsVersionOrGreater(6u, 1u, 0u) ? FindExInfoBasic : FindExInfoStandard;

    // open folder
    pFolder = FindFirstFileExA(PRINT("%s/%s", pcPath, pcFilter), s_iInfoLevel, &oFile, FindExSearchNameMatch, NULL, 0u);
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
void coreData::CreateFolder(const coreChar* pcPath)
{
    coreChar* pcString = coreData::__NextString();
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
            CreateDirectoryA(pcString, NULL);
#else
            mkdir(pcString, S_IRWXU);
#endif
            // reset path
            (*pcCursor) = cTemp;
        }
    }
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
    coreChar* pcString = coreData::__NextString();

    // read arguments and assemble string
    const coreUintW iReturn = std::strftime(pcString, CORE_DATA_STRING_LEN, pcFormat, pTimeMap);
    WARN_IF(!iReturn) return pcFormat;

    return pcString;
}


// ****************************************************************
/* compress data with deflate algorithm */
coreStatus coreData::CompressDeflate(const coreByte* pInput, const coreUint32 iInputSize, coreByte** OUTPUT ppOutput, coreUint32* OUTPUT piOutputSize, const coreInt8 iCompression)
{
    ASSERT(pInput && iInputSize && ppOutput && piOutputSize)

    // set input parameters
    z_stream oStream = {};
    oStream.next_in  = pInput;
    oStream.avail_in = iInputSize;

    // initialize compression
    coreInt32 iError = deflateInit(&oStream, iCompression);
    if(iError != Z_OK)
    {
        Core::Log->Warning("Error initializing compression with deflate (ZLIB: %s (0x%08X))", oStream.msg ? oStream.msg : "-", iError);
        return CORE_INVALID_DATA;
    }

    // retrieve required output size
    const coreUint32 iBound  = deflateBound(&oStream, iInputSize);
    coreByte*        pBuffer = new coreByte[iBound + sizeof(coreUint32)];

    // set output parameters
    oStream.next_out  = pBuffer + sizeof(coreUint32);
    oStream.avail_out = iBound;

    // compress data
    iError = deflate(&oStream, Z_FINISH);
    if(iError != Z_STREAM_END)
    {
        Core::Log->Warning("Error compressing data with deflate (ZLIB: %s (0x%08X))", oStream.msg ? oStream.msg : "-", iError);
        SAFE_DELETE_ARRAY(pBuffer)
        deflateEnd(&oStream);
        return CORE_INVALID_INPUT;
    }

    // store original size and return compressed data
    (*r_cast<coreUint32*>(pBuffer)) = oStream.total_in;
    (*ppOutput)     = pBuffer;
    (*piOutputSize) = oStream.total_out + sizeof(coreUint32);

    // compression finished
    deflateEnd(&oStream);
    return CORE_OK;
}


// ****************************************************************
/* decompress data with deflate algorithm */
coreStatus coreData::DecompressDeflate(const coreByte* pInput, const coreUint32 iInputSize, coreByte** OUTPUT ppOutput, coreUint32* OUTPUT piOutputSize)
{
    ASSERT(pInput && iInputSize && ppOutput && piOutputSize)

    // retrieve original size
    const coreUint32 iBound  = (*r_cast<const coreUint32*>(pInput));
    coreByte*        pBuffer = new coreByte[iBound];

    // set input and output parameters
    z_stream oStream  = {};
    oStream.next_in   = pInput     + sizeof(coreUint32);
    oStream.avail_in  = iInputSize - sizeof(coreUint32);
    oStream.next_out  = pBuffer;
    oStream.avail_out = iBound;

    // initialize decompression
    coreInt32 iError = inflateInit(&oStream);
    if(iError != Z_OK)
    {
        Core::Log->Warning("Error initializing decompression with deflate (ZLIB: %s (0x%08X))", oStream.msg ? oStream.msg : "-", iError);
        SAFE_DELETE_ARRAY(pBuffer)
        return CORE_INVALID_DATA;
    }

    // decompress data
    iError = inflate(&oStream, Z_FINISH);
    if(iError != Z_STREAM_END)
    {
        Core::Log->Warning("Error decompressing data with deflate (ZLIB: %s (0x%08X))", oStream.msg ? oStream.msg : "-", iError);
        SAFE_DELETE_ARRAY(pBuffer)
        inflateEnd(&oStream);
        return CORE_INVALID_INPUT;
    }

    // return decompressed data
    (*ppOutput)     = pBuffer;
    (*piOutputSize) = iBound;

    // decompression finished
    inflateEnd(&oStream);
    return CORE_OK;
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

    const coreChar* pcSlash = std::strrchr(pcInput, '\\');
    if(!pcSlash)    pcSlash = std::strrchr(pcInput, '/');
    return pcSlash ? (pcSlash + 1u) : pcInput;
}


// ****************************************************************
/* safely get file extension */
const coreChar* coreData::StrExtension(const coreChar* pcInput)
{
    WARN_IF(!pcInput) return "";

    const coreChar* pcDot = std::strrchr(pcInput, '.');
    return pcDot ? (pcDot + 1u) : pcInput;
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
void coreData::StrCopy(const coreChar* pcInput, coreChar* OUTPUT pcOutput, const coreUintW iSize)
{
    ASSERT(pcInput && pcOutput)

    // calculate string length
    const coreUintW iLen = MIN(std::strlen(pcInput), iSize - 1u);
    WARN_IF(std::strlen(pcInput) >= iSize) {}

    // copy string with guaranteed null-termination
    std::memcpy(pcOutput, pcInput, iLen);
    pcOutput[iLen] = '\0';
}


// ****************************************************************
/* trim a standard string on both sides */
void coreData::StrTrim(std::string* OUTPUT psInput)
{
    // trim right
    const coreUintW iLast = psInput->find_last_not_of(" \n\r\t");
    if(iLast != std::string::npos) psInput->erase(iLast + 1u);

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