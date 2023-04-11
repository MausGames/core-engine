///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"
#include <stb_sprintf.h>

#if defined(_CORE_WINDOWS_)
    #include <Psapi.h>
    #include <Shlobj.h>
    #include <Lmcons.h>
#elif defined(_CORE_LINUX_)
    #include <gnu/libc-version.h>
#elif defined(_CORE_MACOS_)
    #include <mach/mach.h>
    #include <mach-o/dyld.h>
    #include <sys/sysctl.h>
    #include "additional/macos/cocoa.h"
#elif defined(_CORE_ANDROID_)
    #include <sys/system_properties.h>
#endif
#if !defined(_CORE_WINDOWS_)
    #include <unistd.h>
    #include <dirent.h>
    #include <pwd.h>
    #include <dlfcn.h>
    #include <sys/utsname.h>
    #include <sys/statvfs.h>
    #include <sys/stat.h>
#endif

thread_local coreData::coreTempString coreData::s_TempString     = {};
coreMapStrFull<const coreChar*>       coreData::s_apcCommandLine = {};
coreString                            coreData::s_sUserFolder    = "";

extern "C" const coreChar* g_pcUserFolder = "";   // to allow access from C files


// ****************************************************************
/* create formatted string */
coreInt32 coreData::PrintBase(coreChar* OUTPUT pcOutput, const coreInt32 iMaxLen, const coreChar* pcFormat, ...)
{
    ASSERT(pcFormat)

    // prepare variable arguments
    va_list oArgs;
    va_start(oArgs, pcFormat);

    // assemble string
    const coreInt32 iReturn = stbsp_vsnprintf(pcOutput, iMaxLen, pcFormat, oArgs);
    va_end(oArgs);

    return iReturn;
}

coreInt32 coreData::PrintBaseV(coreChar* OUTPUT pcOutput, const coreInt32 iMaxLen, const coreChar* pcFormat, va_list oArgs)
{
    ASSERT(pcFormat)

    // assemble string
    return stbsp_vsnprintf(pcOutput, iMaxLen, pcFormat, oArgs);
}


// ****************************************************************
/* get process identifier for the current process */
coreUint32 coreData::ProcessID()
{
#if defined(_CORE_WINDOWS_)

    return GetCurrentProcessId();

#else

    return getpid();

#endif
}


// ****************************************************************
/* get amount of memory physically mapped to the application */
coreUint64 coreData::ProcessMemory()
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
    std::FILE* pFile = coreData::FileOpen("/proc/self/statm", "rb");
    if(pFile)
    {
        coreUint64 iPages;

        // read resident set size (in pages)
        std::fscanf(pFile, "%*s %llu", &iPages);
        std::fclose(pFile);

        // multiply with page-size and return
        return iPages * sysconf(_SC_PAGESIZE);
    }

#elif defined(_CORE_MACOS_)

    mach_task_basic_info_data_t oInfo;
    mach_msg_type_number_t iCount = MACH_TASK_BASIC_INFO_COUNT;

    // retrieve basic task information
    if(task_info(mach_task_self(), MACH_TASK_BASIC_INFO, r_cast<task_info_t>(&oInfo), &iCount) == KERN_SUCCESS)
    {
        // return resident set size (in bytes)
        return oInfo.resident_size;
    }

#endif

    return 0u;
}


// ****************************************************************
/* get full application path */
const coreChar* coreData::ProcessPath()
{
    UNUSED coreChar* pcString = coreData::__NextTempString();

#if defined(_CORE_WINDOWS_)

    coreWchar acPath[CORE_DATA_MAX_PATH];

    // get path of the current executable
    if(GetModuleFileNameW(NULL, acPath, CORE_DATA_MAX_PATH))
    {
        // return path
        return coreData::__ToAnsiChar(acPath);
    }

#elif defined(_CORE_LINUX_)

    // get content of the symbolic link
    const coreIntW iLen = readlink("/proc/self/exe", pcString, CORE_DATA_STRING_LEN - 1u);
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
/* get physical system memory */
coreBool coreData::SystemMemory(coreUint64* OUTPUT piAvailable, coreUint64* OUTPUT piTotal)
{
#if defined(_CORE_WINDOWS_)

    MEMORYSTATUSEX oMemory = {sizeof(oMemory)};

    // retrieve global memory status
    if(GlobalMemoryStatusEx(&oMemory))
    {
        if(piAvailable) (*piAvailable) = oMemory.ullAvailPhys;
        if(piTotal)     (*piTotal)     = oMemory.ullTotalPhys;
        return true;
    }

#elif defined(_CORE_LINUX_)

    // retrieve runtime system parameters
    const coreInt64 iPageSize  = sysconf(_SC_PAGESIZE);
    const coreInt64 iAvailable = sysconf(_SC_AVPHYS_PAGES);
    const coreInt64 iTotal     = sysconf(_SC_PHYS_PAGES);

    // only allow positive non-zero values (-1 on error)
    if((iPageSize > 0) && (iAvailable > 0) && (iTotal > 0))
    {
        if(piAvailable) (*piAvailable) = iPageSize * iAvailable;   // without file cache
        if(piTotal)     (*piTotal)     = iPageSize * iTotal;
        return true;
    }

#elif defined(_CORE_MACOS_)

    // retrieve runtime system parameters
    const coreInt64 iPageSize = sysconf(_SC_PAGESIZE);
    const coreInt64 iTotal    = sysconf(_SC_PHYS_PAGES);

    // only allow positive non-zero values (-1 on error)
    if((iPageSize > 0) && (iTotal > 0))
    {
        vm_statistics64_data_t oInfo;
        mach_msg_type_number_t iCount = HOST_VM_INFO64_COUNT;

        // retrieve virtual memory statistics
        if(host_statistics64(mach_host_self(), HOST_VM_INFO64, r_cast<host_info64_t>(&oInfo), &iCount) == KERN_SUCCESS)
        {
            if(piAvailable) (*piAvailable) = iPageSize * oInfo.free_count;   // without file cache
            if(piTotal)     (*piTotal)     = iPageSize * iTotal;
            return true;
        }
    }

#endif

    // could not get physical system memory
    if(piAvailable) (*piAvailable) = 0u;
    if(piTotal)     (*piTotal)     = 1u;
    return false;
}


// ****************************************************************
/* get disk space (for the user folder) */
coreBool coreData::SystemSpace(coreUint64* OUTPUT piAvailable, coreUint64* OUTPUT piTotal)
{
#if defined(_CORE_WINDOWS_)

    ULARGE_INTEGER iAvailable, iTotal;

    // retrieve disk volume information
    if(GetDiskFreeSpaceExW(coreData::__ToWideChar(s_sUserFolder.c_str()), &iAvailable, &iTotal, NULL))
    {
        if(piAvailable) (*piAvailable) = iAvailable.QuadPart;
        if(piTotal)     (*piTotal)     = iTotal    .QuadPart;
        return true;
    }

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    struct statvfs oBuffer;

    // retrieve filesystem statistics
    if(!statvfs(s_sUserFolder.c_str(), &oBuffer))
    {
        if(piAvailable) (*piAvailable) = oBuffer.f_frsize * oBuffer.f_bavail;
        if(piTotal)     (*piTotal)     = oBuffer.f_frsize * oBuffer.f_blocks;
        return true;
    }

#endif

    // could not get disk space
    if(piAvailable) (*piAvailable) = 0u;
    if(piTotal)     (*piTotal)     = 1u;
    return false;
}


// ****************************************************************
/* get operating system name */
const coreChar* coreData::SystemOsName()
{
#if defined(_CORE_WINDOWS_)

    coreUint16 iMajor    = 0u;
    coreUint16 iMinor    = 0u;
    coreUint16 iBuild    = 0u;
    coreUint16 iRevision = 0u;

    // fetch version from system library
    coreWchar acPath[CORE_DATA_MAX_PATH];
    if(GetModuleFileNameW(GetModuleHandleW(L"kernel32.dll"), acPath, CORE_DATA_MAX_PATH))
    {
        // get file version info size
        const coreUint32 iDataSize = GetFileVersionInfoSizeW(acPath, NULL);
        if(iDataSize)
        {
            // get file version info
            coreByte* pData = new coreByte[iDataSize];
            if(GetFileVersionInfoW(acPath, 0u, iDataSize, pData))
            {
                // access root block
                VS_FIXEDFILEINFO* pInfo;
                coreUint32        iInfoSize;
                if(VerQueryValueW(pData, L"\\", r_cast<void**>(&pInfo), &iInfoSize) && (pInfo->dwSignature == 0xFEEF04BDu))
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
    case 100u: pcSubName = (iBuild >= 22000u) ? "11" : "10"; break;
    case  63u: pcSubName = "8.1";                            break;
    case  62u: pcSubName = "8";                              break;
    case  61u: pcSubName = "7";                              break;
    case  60u: pcSubName = "Vista";                          break;
    case  51u: pcSubName = "XP";                             break;
    case  50u: pcSubName = "2000";                           break;
    default:   pcSubName = "-";                              break;
    }

    // return full operating system name
    return PRINT("Windows %s (%u.%u.%u.%u)", pcSubName, iMajor, iMinor, iBuild, iRevision);

#elif defined(_CORE_LINUX_)

    // fetch kernel information
    utsname oInfo;
    if(uname(&oInfo)) return "Linux";

    // return full operating system name
    return PRINT("%s %s (%s, glibc %s %s)", oInfo.sysname, oInfo.release, oInfo.version, gnu_get_libc_version(), gnu_get_libc_release());

#elif defined(_CORE_MACOS_)

    // fetch kernel information
    utsname oInfo;
    if(uname(&oInfo)) return "macOS";

    // fetch model identifier
    coreChar  acModel[64] = {};
    coreUintW iLen        = sizeof(acModel);
    sysctlbyname("hw.model", acModel, &iLen, NULL, 0u);

    // return full operating system name (Darwin)
    return PRINT("%s %s (%s, on %s)", oInfo.sysname, oInfo.release, oInfo.version, acModel);

#elif defined(_CORE_ANDROID_)

    coreChar acOS[PROP_VALUE_MAX] = {}, acSDK[PROP_VALUE_MAX] = {};

    // fetch operating system and SDK version strings
    __system_property_get("ro.build.version.release", acOS);
    __system_property_get("ro.build.version.sdk",     acSDK);

    // return full operating system name
    return PRINT("Android %s (API Level %s)", acOS, acSDK);

#elif defined(_CORE_IOS_)

    return "iOS";

#elif defined(_CORE_EMSCRIPTEN_)

    return "Emscripten " STRING(__EMSCRIPTEN_major__) "." STRING(__EMSCRIPTEN_minor__) "." STRING(__EMSCRIPTEN_tiny__);

#endif
}


// ****************************************************************
/* get user identifier */
coreUint32 coreData::SystemUserID()
{
#if defined(_CORE_WINDOWS_)

    // get user name hash (as substitute)
    return coreHashXXH32(coreData::SystemUserName());

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    // get effective user identifier of calling process
    return geteuid();

#else

    return 0u;

#endif
}


// ****************************************************************
/* get user name */
const coreChar* coreData::SystemUserName()
{
#if defined(_CORE_WINDOWS_)

    coreWchar acName[UNLEN + 1u];
    coreUlong iSize = ARRAY_SIZE(acName);

    // get user name associated with current thread
    if(GetUserNameW(acName, &iSize)) return coreData::__ToAnsiChar(acName);

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
/* get processor vendor string */
const coreChar* coreData::SystemCpuVendor()
{
#if defined(_CORE_MACOS_)

    coreChar* pcString = coreData::__NextTempString();

    // fetch OS processor vendor string (may not exist on ARM)
    coreUintW iLen = CORE_DATA_STRING_LEN;
    if(!sysctlbyname("machdep.cpu.vendor", pcString, &iLen, NULL, 0u))
    {
        return pcString;
    }

#endif

    // return CPUID processor vendor string
    return coreCPUID::Vendor();
}


// ****************************************************************
/* get processor brand string */
const coreChar* coreData::SystemCpuBrand()
{
#if defined(_CORE_MACOS_)

    coreChar* pcString = coreData::__NextTempString();

    // fetch OS processor brand string
    coreUintW iLen = CORE_DATA_STRING_LEN;
    if(!sysctlbyname("machdep.cpu.brand_string", pcString, &iLen, NULL, 0u))
    {
        return pcString;
    }

#endif

    // return CPUID processor brand string
    return coreCPUID::Brand();
}


// ****************************************************************
/* get path to store application data */
const coreChar* coreData::SystemDirAppData()
{
#if defined(_CORE_WINDOWS_)

    // get default roaming directory
    coreWchar* pcRoamingPath = NULL;
    if(SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT, NULL, &pcRoamingPath)) && pcRoamingPath)
    {
        // convert characters
        const coreChar* pcConvert = coreData::__ToAnsiChar(pcRoamingPath);
        CoTaskMemFree(pcRoamingPath);

        // prepare path and return
        const coreChar* pcPath = coreData::__PrepareSystemDir(pcConvert);
        if(pcPath) return pcPath;
    }

#elif defined(_CORE_LINUX_)

    const coreChar* pcPath;

    // get directory from XDG variable
    if((pcPath = coreData::GetEnvironment("XDG_DATA_HOME")) && (pcPath = coreData::__PrepareSystemDir(pcPath)))
        return pcPath;

    // get directory from home variable
    if((pcPath = coreData::GetEnvironment("HOME")) && (pcPath = coreData::__PrepareSystemDir(PRINT("%s/.local/share", pcPath))))
        return pcPath;

    // get directory from password database
    const passwd* pRecord = getpwuid(geteuid());
    if(pRecord && pRecord->pw_dir && (pcPath = coreData::__PrepareSystemDir(PRINT("%s/.local/share", pRecord->pw_dir))))
        return pcPath;

#elif defined(_CORE_MACOS_)

    const coreChar* pcPath;

    // get directory from Cocoa interface
    if((pcPath = coreCocoaPathApplicationSupport()) && (pcPath = coreData::__PrepareSystemDir(pcPath)))
        return pcPath;

    // get directory from home variable
    if((pcPath = coreData::GetEnvironment("HOME")) && (pcPath = coreData::__PrepareSystemDir(PRINT("%s/Library/Application Support", pcPath))))
        return pcPath;

    // get directory from password database
    const passwd* pRecord = getpwuid(geteuid());
    if(pRecord && pRecord->pw_dir && (pcPath = coreData::__PrepareSystemDir(PRINT("%s/Library/Application Support", pRecord->pw_dir))))
        return pcPath;

#elif defined(_CORE_ANDROID_)

    // get internal storage path for this application
    return SDL_AndroidGetInternalStoragePath();

#endif

    return "";
}


// ****************************************************************
/* get path to store temporary data */
const coreChar* coreData::SystemDirTemp()
{
#if defined(_CORE_WINDOWS_)

    // get default temporary directory
    coreWchar acTempPath[CORE_DATA_MAX_PATH];
    if(GetTempPathW(CORE_DATA_MAX_PATH, acTempPath))
    {
        // prepare path and return
        const coreChar* pcPath = coreData::__PrepareSystemDir(coreData::__ToAnsiChar(acTempPath));
        if(pcPath) return pcPath;
    }

#elif defined(_CORE_LINUX_)

    const coreChar* pcPath;

    // get directory from XDG variable
    if((pcPath = coreData::GetEnvironment("XDG_CACHE_HOME")) && (pcPath = coreData::__PrepareSystemDir(pcPath)))
        return pcPath;

    // get directory from home variable
    if((pcPath = coreData::GetEnvironment("HOME")) && (pcPath = coreData::__PrepareSystemDir(PRINT("%s/.cache", pcPath))))
        return pcPath;

    // get directory from password database
    const passwd* pRecord = getpwuid(geteuid());
    if(pRecord && pRecord->pw_dir && (pcPath = coreData::__PrepareSystemDir(PRINT("%s/.cache", pRecord->pw_dir))))
        return pcPath;

#elif defined(_CORE_MACOS_)

    const coreChar* pcPath;

    // get directory from Cocoa interface
    if((pcPath = coreCocoaPathCaches()) && (pcPath = coreData::__PrepareSystemDir(pcPath)))
        return pcPath;

    // get directory from home variable
    if((pcPath = coreData::GetEnvironment("HOME")) && (pcPath = coreData::__PrepareSystemDir(PRINT("%s/Library/Caches", pcPath))))
        return pcPath;

    // get directory from password database
    const passwd* pRecord = getpwuid(geteuid());
    if(pRecord && pRecord->pw_dir && (pcPath = coreData::__PrepareSystemDir(PRINT("%s/Library/Caches", pRecord->pw_dir))))
        return pcPath;

#endif

    return "";
}


// ****************************************************************
/* get compiler name and version */
const coreChar* coreData::BuildCompiler()
{
#if defined(_CORE_MSVC_)

    return PRINT("MSVC %d.%d.%d.%d", (_MSC_VER / 100u) % 100u, (_MSC_VER) % 100u, (_MSC_FULL_VER) % 100000u, _MSC_BUILD);

#elif defined(_CORE_GCC_)

    return "GCC " STRING(__GNUC__) "." STRING(__GNUC_MINOR__) "." STRING(__GNUC_PATCHLEVEL__);

#elif defined(_CORE_CLANG_)

    return "Clang " __clang_version__;

#endif
}


// ****************************************************************
/* get standard library name and version */
const coreChar* coreData::BuildLibrary()
{
#if defined(_CORE_STL_)

    return "STL " STRING(_MSVC_STL_VERSION) " (" STRING(_MSVC_STL_UPDATE) ")";

#elif defined(_CORE_GLIBCXX_)

    return "libstdc++ " STRING(_GLIBCXX_RELEASE) " (" STRING(__GLIBCXX__) ")";

#elif defined(_CORE_LIBCPP_)

    return "libc++ " STRING(_LIBCPP_VERSION) " (ABI " STRING(_LIBCPP_ABI_VERSION) ")";

#endif
}


// ****************************************************************
/* set current working directory */
coreStatus coreData::SetCurDir(const coreChar* pcPath)
{
    ASSERT(pcPath)

#if defined(_CORE_WINDOWS_)

    if(SetCurrentDirectoryW(coreData::__ToWideChar(pcPath))) return CORE_OK;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    if(!chdir(pcPath)) return CORE_OK;

#endif

    return CORE_ERROR_SYSTEM;
}


// ****************************************************************
/* get current working directory */
const coreChar* coreData::GetCurDir()
{
#if defined(_CORE_WINDOWS_)

    coreWchar acPath[CORE_DATA_MAX_PATH];

    // get raw working directory
    const coreUint32 iLen = GetCurrentDirectoryW(CORE_DATA_MAX_PATH - 1u, acPath);
    if(iLen)
    {
        // add path-delimiter and return (with known length)
        std::memcpy(acPath + iLen, L"/", 4u);
        return coreData::__ToAnsiChar(acPath);
    }

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    coreChar* pcString = coreData::__NextTempString();

    // get raw working directory
    if(getcwd(pcString, CORE_DATA_STRING_LEN - 1u))
    {
        // add path-delimiter and return
        return std::strcat(pcString, "/");
    }

#endif

    return "";
}


// ****************************************************************
/* log command line arguments  */
void coreData::LogCommandLine()
{
    if(s_apcCommandLine.empty())
    {
        // log placeholder message
        Core::Log->Info("No command line arguments");
    }
    else
    {
        // log all recognized arguments
        Core::Log->ListStartInfo("Command Line Arguments");
        {
            FOR_EACH(it, s_apcCommandLine)
            {
                Core::Log->ListAdd("%s = %s", s_apcCommandLine.get_string(it), *it);
            }
        }
        Core::Log->ListEnd();
    }
}


// ****************************************************************
/* set command line arguments */
void coreData::SetCommandLine(const coreInt32 iArgc, const coreChar* const* ppcArgv)
{
    ASSERT(!iArgc || ppcArgv)

    // loop through all arguments
    for(coreUintW i = 1u, ie = iArgc; i < ie; ++i)
    {
        const coreChar* pcCurrent = ppcArgv[i];
        ASSERT(pcCurrent)

        // insert valid entries
        if((*pcCurrent) == '-')
        {
            while((*(++pcCurrent)) == '-') {}

            // handle assignment syntax
            const coreChar* pcAssign = std::strchr(pcCurrent, '=');
            if(pcAssign)
            {
                s_apcCommandLine[coreData::StrToLower(coreData::StrLeft(pcCurrent, pcAssign - pcCurrent))] = pcAssign + 1u;
            }
            else
            {
                s_apcCommandLine[coreData::StrToLower(pcCurrent)] = ((i+1u < ie) && ((*ppcArgv[i+1u]) != '-')) ? ppcArgv[i+1u] : "";
            }
        }
    }
}


// ****************************************************************
/* set environment variable (of current process) */
coreStatus coreData::SetEnvironment(const coreChar* pcName, const coreChar* pcValue)
{
    ASSERT(pcName)

#if defined(_CORE_WINDOWS_)

    // create, replace, or remove environment variable
    if(!_wputenv_s(coreData::__ToWideChar(pcName), pcValue ? coreData::__ToWideChar(pcValue) : L"")) return CORE_OK;   // SetEnvironmentVariable is unreliable

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    if(pcValue && pcValue[0])
    {
        // create or replace environment variable
        if(!setenv(pcName, pcValue, 1)) return CORE_OK;
    }
    else
    {
        // remove environment variable
        if(!unsetenv(pcName)) return CORE_OK;
    }

#endif

    return CORE_ERROR_SYSTEM;
}


// ****************************************************************
/* get environment variable (of current process) */
const coreChar* coreData::GetEnvironment(const coreChar* pcName)
{
    ASSERT(pcName)

#if defined(_CORE_WINDOWS_)

    return coreData::__ToAnsiChar(_wgetenv(coreData::__ToWideChar(pcName)));

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    return std::getenv(pcName);

#else

    return NULL;

#endif
}


// ****************************************************************
/* initialize default folders */
void coreData::InitDefaultFolders()
{
    // set working directory
#if defined(_CORE_MACOS_)
    coreData::SetCurDir(coreCocoaPathResource());
#else
    coreData::SetCurDir(coreData::ProcessDir());
    coreData::SetCurDir("../..");
#endif

    // get command line argument
    const coreChar* pcPath = coreData::GetCommandLine("user-folder-path");

    // use specific user folder
    if(pcPath && pcPath[0])
    {
        pcPath = std::strcmp(pcPath, "!appdata") ? PRINT("%s/", pcPath) : coreData::SystemDirAppData();
    }

    // use default user folder (and create folder hierarchy)
    if(!pcPath || !pcPath[0] || (coreData::FolderCreate(pcPath) != CORE_OK) || !coreData::FolderWritable(pcPath))
    {
        #if defined(_CORE_MACOS_) || defined(_CORE_ANDROID_)
            pcPath = coreData::SystemDirAppData();
        #else
            pcPath = CoreApp::Settings::UserManagement ? coreData::SystemDirAppData() : "user/";
        #endif

        coreData::FolderCreate(pcPath);
    }

    // save selected user folder
    s_sUserFolder  = pcPath;
    g_pcUserFolder = s_sUserFolder.c_str();
}


// ****************************************************************
/* create path into selected user folder */
const coreChar* coreData::UserFolderShared(const coreChar* pcPath)
{
    ASSERT(pcPath)

    // forward to user-shared location
    return PRINT("%s%s", s_sUserFolder.c_str(), pcPath);
}

const coreChar* coreData::UserFolderPrivate(const coreChar* pcPath)
{
    ASSERT(pcPath)

    if(CoreApp::Settings::UserManagement)
    {
        // forward to user-private location
        const coreUint32 iUserID = Core::Platform->GetUserID();
        return PRINT("%s%s%s", s_sUserFolder.c_str(), iUserID ? PRINT("user_%u/", iUserID) : "default/", pcPath);
    }
    else
    {
        // forward to user-shared location
        return coreData::UserFolderShared(pcPath);
    }
}


// ****************************************************************
/* open dynamic library */
void* coreData::OpenLibrary(const coreChar* pcName)
{
    ASSERT(pcName)

#if defined(_CORE_WINDOWS_)

    return LoadLibraryW(coreData::__ToWideChar(pcName));

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    // get absolute path near application
    #if defined(_CORE_LINUX_)
        const coreChar* pcLocal = PRINT("%s/%s", coreData::ProcessDir(), pcName);
    #else
        const coreChar* pcLocal = PRINT("%s/%s", coreCocoaPathFramework(), pcName);
    #endif

    // try to open dynamic library
    void* pLibrary = NULL;
    if(!pLibrary) pLibrary = dlopen(pcLocal, RTLD_LAZY);   // independent of current working directory
    if(!pLibrary) pLibrary = dlopen(pcName,  RTLD_LAZY);

    return pLibrary;

#else

    return NULL;

#endif
}


// ****************************************************************
/* find symbol in dynamic library */
void* coreData::GetAddress(void* pLibrary, const coreChar* pcName)
{
    ASSERT(pLibrary && pcName)

#if defined(_CORE_WINDOWS_)

    return GetProcAddress(s_cast<HMODULE>(pLibrary), pcName);

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    return dlsym(pLibrary, pcName);

#else

    return NULL;

#endif
}


// ****************************************************************
/* close dynamic library */
coreStatus coreData::CloseLibrary(void* pLibrary)
{
    ASSERT(pLibrary)

#if defined(_CORE_WINDOWS_)

    if(FreeLibrary(s_cast<HMODULE>(pLibrary))) return CORE_OK;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    if(!dlclose(pLibrary)) return CORE_OK;

#endif

    return CORE_ERROR_SYSTEM;
}


// ****************************************************************
/* create private heap object */
void* coreData::HeapCreate(const coreBool bThreadSafe)
{
#if defined(_CORE_WINDOWS_)

    return ::HeapCreate((bThreadSafe ? 0u : HEAP_NO_SERIALIZE) | (DEFINED(_CORE_DEBUG_) ? HEAP_GENERATE_EXCEPTIONS : 0u), 0u, 0u);

#else

    return I_TO_P(-1);

#endif
}


// ****************************************************************
/* destroy private heap object */
void coreData::HeapDestroy(void* pHeap)
{
    ASSERT(pHeap)

#if defined(_CORE_WINDOWS_)

    #if defined(_CORE_DEBUG_)
        ASSERT(::HeapValidate(pHeap, 0u, NULL))
    #endif

    WARN_IF(!::HeapDestroy(pHeap)) {}

#else

    ASSERT(pHeap == I_TO_P(-1))

#endif
}


// ****************************************************************
/* allocate memory from private heap */
RETURN_RESTRICT void* coreData::HeapMalloc(void* pHeap, const coreUintW iSize)
{
    ASSERT(pHeap && iSize)

#if defined(_CORE_WINDOWS_)

    return ::HeapAlloc(pHeap, 0u, iSize);

#else

    return std::malloc(iSize);

#endif
}

// ****************************************************************
/* allocate zero-initialized memory from private heap */
RETURN_RESTRICT void* coreData::HeapCalloc(void* pHeap, const coreUintW iSize)
{
    ASSERT(pHeap && iSize)

#if defined(_CORE_WINDOWS_)

    return ::HeapAlloc(pHeap, HEAP_ZERO_MEMORY, iSize);

#else

    return std::calloc(iSize, 1u);

#endif
}

// ****************************************************************
/* allocate dynamic-sized memory from private heap */
void coreData::HeapRealloc(void* pHeap, void** OUTPUT ppPointer, const coreUintW iSize)
{
    ASSERT(pHeap && ppPointer && iSize)

#if defined(_CORE_WINDOWS_)

    (*ppPointer) = (*ppPointer) ? ::HeapReAlloc(pHeap, 0u, *ppPointer, iSize) : ::HeapAlloc(pHeap, 0u, iSize);

#else

    (*ppPointer) = std::realloc(*ppPointer, iSize);

#endif
}


// ****************************************************************
/* free memory from private heap */
void coreData::HeapFree(void* pHeap, void** OUTPUT ppPointer)
{
    ASSERT(pHeap && ppPointer)

#if defined(_CORE_WINDOWS_)

    #if defined(_CORE_DEBUG_)
        ASSERT(::HeapValidate(pHeap, 0u, *ppPointer))
    #endif

    WARN_IF(!::HeapFree(pHeap, 0u, *ppPointer)) {}

#else

    std::free(*ppPointer);

#endif

    (*ppPointer) = NULL;
}


// ****************************************************************
/* open regular file stream */
std::FILE* coreData::FileOpen(const coreChar* pcPath, const coreChar* pcMode)
{
    ASSERT(pcPath && pcMode)

#if defined(_CORE_WINDOWS_)

    // optimize caching for sequential access
    return _wfopen(coreData::__ToWideChar(pcPath), coreData::__ToWideChar(PRINT("%sS", pcMode)));

#elif defined(_CORE_LINUX_)

    // disable thread cancellation points
    return std::fopen(pcPath, PRINT("%sc", pcMode));

#else

    // just open regular file stream
    return std::fopen(pcPath, pcMode);

#endif
}


// ****************************************************************
/* check if file exists */
coreBool coreData::FileExists(const coreChar* pcPath)
{
    ASSERT(pcPath)

#if defined(_CORE_WINDOWS_)

    const coreUint32 iAttributes = GetFileAttributesW(coreData::__ToWideChar(pcPath));

    // quick Windows check
    if((iAttributes != INVALID_FILE_ATTRIBUTES) && !HAS_FLAG(iAttributes, FILE_ATTRIBUTE_DIRECTORY)) return true;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_EMSCRIPTEN_)

    struct stat oBuffer;

    // quick POSIX check
    if(!stat(pcPath, &oBuffer) && S_ISREG(oBuffer.st_mode)) return true;

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
    ASSERT(pcPath)

#if defined(_CORE_WINDOWS_)

    WIN32_FILE_ATTRIBUTE_DATA oAttributes;

    // get extended file attributes
    if(GetFileAttributesExW(coreData::__ToWideChar(pcPath), GetFileExInfoStandard, &oAttributes))
    {
        // return combined file size
        return (coreInt64(oAttributes.nFileSizeHigh) << 32u) |
               (coreInt64(oAttributes.nFileSizeLow));
    }

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_EMSCRIPTEN_)

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
    ASSERT(pcPath)

#if defined(_CORE_WINDOWS_)

    WIN32_FILE_ATTRIBUTE_DATA oAttributes;

    // get extended file attributes
    if(GetFileAttributesExW(coreData::__ToWideChar(pcPath), GetFileExInfoStandard, &oAttributes))
    {
        // return converted file write time
        return r_cast<coreUint64&>(oAttributes.ftLastWriteTime) / 10000000ull - 11644473600ull;
    }

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_EMSCRIPTEN_)

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
    ASSERT(pcFrom && pcTo)

#if defined(_CORE_WINDOWS_)

    // copy directly (with attributes)
    if(CopyFileW(coreData::__ToWideChar(pcFrom), coreData::__ToWideChar(pcTo), false)) return CORE_OK;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_EMSCRIPTEN_)

    // open source file
    std::FILE* pFileFrom = coreData::FileOpen(pcFrom, "rb");
    if(pFileFrom)
    {
        // open destination file
        std::FILE* pFileTo = coreData::FileOpen(pcTo, "wb");
        if(pFileTo)
        {
            alignas(ALIGNMENT_PAGE) BIG_STATIC coreByte s_aBuffer[0x4000u];

            // copy all data
            while(!std::feof(pFileFrom))
            {
                const coreUintW iResult = std::fread(s_aBuffer, 1u, ARRAY_SIZE(s_aBuffer), pFileFrom);
                std::fwrite(s_aBuffer, 1u, iResult, pFileTo);
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
    ASSERT(pcFrom && pcTo)

#if defined(_CORE_WINDOWS_)

    if(MoveFileExW(coreData::__ToWideChar(pcFrom), coreData::__ToWideChar(pcTo), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) return CORE_OK;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_EMSCRIPTEN_)

    if(!std::rename(pcFrom, pcTo)) return CORE_OK;

#endif

    return CORE_ERROR_FILE;
}


// ****************************************************************
/* delete existing file */
coreStatus coreData::FileDelete(const coreChar* pcPath)
{
    ASSERT(pcPath)

#if defined(_CORE_WINDOWS_)

    if(DeleteFileW(coreData::__ToWideChar(pcPath))) return CORE_OK;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_EMSCRIPTEN_)

    if(!unlink(pcPath)) return CORE_OK;

#endif

    return CORE_ERROR_FILE;
}


// ****************************************************************
/* check if folder exists */
coreBool coreData::FolderExists(const coreChar* pcPath)
{
    ASSERT(pcPath)

    // current working directory always exists
    if(!pcPath[0]) return true;

#if defined(_CORE_WINDOWS_)

    const coreUint32 iAttributes = GetFileAttributesW(coreData::__ToWideChar(pcPath));

    // quick Windows check
    if((iAttributes != INVALID_FILE_ATTRIBUTES) && HAS_FLAG(iAttributes, FILE_ATTRIBUTE_DIRECTORY)) return true;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_EMSCRIPTEN_)

    struct stat oBuffer;

    // quick POSIX check
    if(!stat(pcPath, &oBuffer) && S_ISDIR(oBuffer.st_mode)) return true;

#else

    // try to write into presumed folder
    if(coreData::FolderWritable(pcPath)) return true;

#endif

    return false;
}


// ****************************************************************
/* check if folder is writable */
coreBool coreData::FolderWritable(const coreChar* pcPath)
{
    ASSERT(pcPath)

    // get temporary file name
    const coreChar* pcTemp = PRINT("%s/check_%s", pcPath, coreData::DateTimePrint("%Y%m%d_%H%M%S"));

#if defined(_CORE_WINDOWS_)

    // create temporary file
    const HANDLE pFile = CreateFileW(coreData::__ToWideChar(pcTemp), GENERIC_WRITE, 0u, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, NULL);
    if(pFile != INVALID_HANDLE_VALUE)
    {
        // close and (automatically) delete file again
        CloseHandle(pFile);

        return true;
    }

#else

    // create temporary file
    SDL_RWops* pFile = SDL_RWFromFile(pcTemp, "wb");
    if(pFile)
    {
        // close and delete file again
        SDL_RWclose(pFile);
        coreData::FileDelete(pcTemp);

        return true;
    }

#endif

    return false;
}


// ****************************************************************
/* create folder hierarchy */
coreStatus coreData::FolderCreate(const coreChar* pcPath)
{
    ASSERT(pcPath)

    // check if folder already exists (less expensive)
    if(coreData::FolderExists(pcPath)) return CORE_OK;
    ASSERT(pcPath[0])

    coreChar  acString[CORE_DATA_MAX_PATH];
    coreChar* pcCursor = acString + 1u;

    // make local copy
    coreData::StrCopy(acString, ARRAY_SIZE(acString), pcPath);

    // loop through all sub-folders
    for(; (*pcCursor) != '\0'; ++pcCursor)
    {
        if(((*pcCursor) == '/') || ((*pcCursor) == '\\'))
        {
            // build temporary path
            (*pcCursor) = '\0';

            // create sub-folder
            #if defined(_CORE_WINDOWS_)
                if(!CreateDirectoryW(coreData::__ToWideChar(acString), NULL) && (GetLastError() != ERROR_ALREADY_EXISTS)) return CORE_ERROR_FILE;
            #else
                if(mkdir(acString, S_IRWXU) && (errno != EEXIST)) return CORE_ERROR_FILE;
            #endif

            // reset path
            (*pcCursor) = '/';
        }
    }

    return CORE_OK;
}


// ****************************************************************
/* retrieve relative paths of all files from a folder */
coreStatus coreData::FolderScan(const coreChar* pcPath, const coreChar* pcFilter, coreList<coreString>* OUTPUT pasOutput)
{
    ASSERT(pcPath && pcFilter && pasOutput)

#if defined(_CORE_WINDOWS_)

    WIN32_FIND_DATAW oFile;

    // open folder
    const HANDLE pFolder = FindFirstFileExW(coreData::__ToWideChar(PRINT("%s/%s", pcPath, pcFilter)), FindExInfoBasic, &oFile, FindExSearchNameMatch, NULL, 0u);
    if(pFolder == INVALID_HANDLE_VALUE)
    {
        Core::Log->Warning("Folder (%s/%s) could not be opened", pcPath, pcFilter);
        return CORE_ERROR_FILE;
    }

    // loop through all files
    do
    {
        // check and add file path
        if(oFile.cFileName[0] != L'.')
        {
            pasOutput->push_back(PRINT("%s/%s", pcPath, coreData::__ToAnsiChar(oFile.cFileName)));
        }
    }
    while(FindNextFileW(pFolder, &oFile));

    // close folder
    FindClose(pFolder);

#else

    dirent* pFile;

    // open folder
    DIR* pFolder = opendir(pcPath);
    if(!pFolder)
    {
        Core::Log->Warning("Folder (%s/%s) could not be opened", pcPath, pcFilter);
        return CORE_ERROR_FILE;
    }

    // loop through all files
    while((pFile = readdir(pFolder)))
    {
        // check and add file path
        if((pFile->d_name[0] != '.') && coreData::StrCmpLike(pFile->d_name, pcFilter))
        {
            pasOutput->push_back(PRINT("%s/%s", pcPath, pFile->d_name));
        }
    }

    // close folder
    closedir(pFolder);

#endif

    return CORE_OK;
}


// ****************************************************************
/* retrieve relative paths of all files from a folder tree */
coreStatus coreData::FolderScanTree(const coreChar* pcPath, const coreChar* pcFilter, coreList<coreString>* OUTPUT pasOutput)
{
    coreList<coreString> asLocalList;

    // scan current folder
    const coreStatus eError = coreData::FolderScan(pcPath, "*", &asLocalList);
    if(eError != CORE_OK) return eError;   // only check here

    FOR_EACH(it, asLocalList)
    {
        if(coreData::FolderExists(it->c_str()))
        {
            // scan deeper folder (recursively)
            FolderScanTree(it->c_str(), pcFilter, pasOutput);
        }
        else if(coreData::StrCmpLike(it->c_str(), pcFilter))
        {
            // add file path
            pasOutput->push_back(std::move(*it));
        }
    }

    return CORE_OK;
}


// ****************************************************************
/* create symbolic link to file or folder */
coreStatus coreData::SymlinkCreate(const coreChar* pcPath, const coreChar* pcTarget)
{
#if defined(_CORE_WINDOWS_)

    const wchar_t* pcWidePath   = coreData::__ToWideChar(pcPath);
    const wchar_t* pcWideTarget = coreData::__ToWideChar(pcTarget);

    // check for directory attribute
    const coreUint32 iFlags = HAS_FLAG(GetFileAttributesW(pcWideTarget), FILE_ATTRIBUTE_DIRECTORY) ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0u;

    // create symbolic link
    if(CreateSymbolicLinkW(pcWidePath, pcWideTarget, iFlags)) return CORE_OK;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    if(!symlink(pcTarget, pcPath)) return CORE_OK;

#endif

    return CORE_ERROR_FILE;
}


// ****************************************************************
/* check for system errors (per thread) */
coreBool coreData::CheckLastError()
{
    if(!Core::Config->GetBool(CORE_CONFIG_BASE_DEBUGMODE) && !DEFINED(_CORE_DEBUG_)) return false;

#if defined(_CORE_WINDOWS_)

    // get last error code
    const coreUint32 iError = GetLastError();
    if(iError != NO_ERROR)
    {
        SetLastError(NO_ERROR);

        // convert error code to readable message
        coreWchar acBuffer[512];
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, iError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), acBuffer, ARRAY_SIZE(acBuffer), NULL);

        // write message to log file
        Core::Log->Warning(CORE_LOG_BOLD("System:") " %s (error code %u)", coreData::__ToAnsiChar(acBuffer), iError);
        return true;
    }

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_EMSCRIPTEN_)

    // get last error code
    const coreInt32 iError = errno;
    if((iError != 0) && (iError != EAGAIN) && (iError != ETIMEDOUT))
    {
        errno = 0;

    #if defined(__USE_GNU)

        // convert error code to readable message (GNU specific)
        coreChar acBuffer[512];
        const coreChar* pcString = strerror_r(iError, acBuffer, ARRAY_SIZE(acBuffer));

    #else

        // convert error code to readable message (XSI compliant)
        coreChar acBuffer[512];
        const coreChar* pcString = (strerror_r(iError, acBuffer, ARRAY_SIZE(acBuffer)), acBuffer);

    #endif

        // write message to log file
        Core::Log->Warning(CORE_LOG_BOLD("System:") " %s (error code %d)", pcString, iError);
        return true;
    }

#endif

    return false;
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
    ASSERT(pcFormat && pTimeMap)

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
#if defined(_CORE_EMSCRIPTEN_)

    return CORE_ERROR_SUPPORT;

#else

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

#endif
}


// ****************************************************************
/* decompress data with Zstandard library */
coreStatus coreData::Decompress(const coreByte* pInput, const coreUint32 iInputSize, coreByte** OUTPUT ppOutput, coreUint32* OUTPUT piOutputSize)
{
#if defined(_CORE_EMSCRIPTEN_)

    return CORE_ERROR_SUPPORT;

#else

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

#endif
}


// ****************************************************************
/* scramble data with 32-bit key */
void coreData::Scramble(coreByte* OUTPUT pData, const coreUintW iSize, const coreUint64 iKey)
{
    ASSERT(pData)

    // create sequence
    coreRand oRand(iKey);

    // combine sequence with original data
    for(coreUintW i = 0u; i < iSize; ++i)
        pData[i] = (oRand.Raw() & 0xFFu) - pData[i];
}

void coreData::Unscramble(coreByte* OUTPUT pData, const coreUintW iSize, const coreUint64 iKey)
{
    ASSERT(pData)

    // scramble again (operation is involutory)
    coreData::Scramble(pData, iSize, iKey);
}


// ****************************************************************
/* safely get first characters of a string */
const coreChar* coreData::StrLeft(const coreChar* pcInput, const coreUintW iNum)
{
    WARN_IF(!pcInput) return "";

    coreChar* pcString = coreData::__NextTempString();

    // calculate string length
    const coreUintW iLen = MIN(iNum, std::strlen(pcInput), CORE_DATA_STRING_LEN - 1u);

    // copy characters into new string
    std::memcpy(pcString, pcInput, iLen);
    pcString[iLen] = '\0';

    return pcString;
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

    // return after last path-delimiter
    for(const coreChar* pcCursor = pcInput + std::strlen(pcInput) - 1u; pcCursor >= pcInput; --pcCursor)
    {
        if(((*pcCursor) == '/') || ((*pcCursor) == '\\'))
            return pcCursor + 1u;
    }

    return pcInput;
}


// ****************************************************************
/* safely get directory from path */
const coreChar* coreData::StrDirectory(const coreChar* pcInput)
{
    WARN_IF(!pcInput) return "";

    coreChar* pcString = coreData::__NextTempString();

    // identify file name
    const coreChar* pcName = coreData::StrFilename(pcInput);
    const coreUintW iLen   = MIN(coreUintW(pcName - pcInput), CORE_DATA_STRING_LEN - 1u);

    // copy remaining path
    std::memcpy(pcString, pcInput, iLen);
    pcString[iLen] = '\0';

    return pcString;
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
coreBool coreData::StrCopy(coreChar* OUTPUT pcOutput, const coreUintW iMaxSize, const coreChar* pcInput)
{
    ASSERT(pcOutput && iMaxSize && pcInput)

    // calculate string length
    const coreUintW iInputLen  = std::strlen(pcInput);
    const coreUintW iOutputLen = MIN(iInputLen, iMaxSize - 1u);

    // copy string with guaranteed null-termination
    std::memcpy(pcOutput, pcInput, iOutputLen);
    pcOutput[iOutputLen] = '\0';

    WARN_IF(iInputLen >= iMaxSize) return false;
    return true;
}


// ****************************************************************
/* prepare path for system directory */
const coreChar* coreData::__PrepareSystemDir(const coreChar* pcPath)
{
    ASSERT(pcPath)

    // get folder name from application name
    static const coreString s_sIdentifier = []()
    {
        return coreString(Core::Application->Settings.Name).replace(" ", "");
    }();

    // create full path
    const coreChar* pcFullPath = PRINT("%s/%s/", pcPath, s_sIdentifier.c_str());

    // create folder hierarchy (and check if path is valid)
    if(coreData::FolderCreate(pcFullPath) != CORE_OK) return NULL;

    return pcFullPath;
}


// ****************************************************************
/* transform 8-bit ANSI to 16-bit Unicode (for Windows API) */
const coreWchar* coreData::__ToWideChar(const coreChar* pcText)
{
#if defined(_CORE_WINDOWS_)

    if(pcText)
    {
        STATIC_ASSERT(sizeof(coreWchar) == sizeof(coreChar) * 2u)

        // use multiple temp-strings as single target
        if(++s_TempString.iCurrent >= CORE_DATA_STRING_NUM - 1u) s_TempString.iCurrent = 0u;       // one less
        coreWchar* pcString = r_cast<coreWchar*>(s_TempString.aacData[s_TempString.iCurrent++]);   // one more

        // convert from UTF-8 string to UTF-16 string
        const coreInt32 iReturn = MultiByteToWideChar(CP_UTF8, 0u, pcText, -1, pcString, CORE_DATA_STRING_LEN);
        ASSERT(iReturn)

        return pcString;
    }

#endif

    return NULL;
}


// ****************************************************************
/* transform 16-bit Unicode to 8-bit ANSI (for Windows API) */
const coreChar* coreData::__ToAnsiChar(const coreWchar* pcText)
{
#if defined(_CORE_WINDOWS_)

    if(pcText)
    {
        coreChar* pcString = coreData::__NextTempString();

        // convert from UTF-16 string to UTF-8 string
        const coreInt32 iReturn = WideCharToMultiByte(CP_UTF8, 0u, pcText, -1, pcString, CORE_DATA_STRING_LEN, NULL, NULL);
        ASSERT(iReturn)

        return pcString;
    }

#endif

    return NULL;
}