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
    #include <fcntl.h>
    #include <sys/sendfile.h>
    #include <gnu/libc-version.h>
#elif defined(_CORE_MACOS_)
    #include <mach/mach.h>
    #include <mach-o/dyld.h>
    #include <sys/sysctl.h>
    #include <copyfile.h>
    #include "additional/macos/cocoa.h"
#elif defined(_CORE_ANDROID_)
    #include <sys/system_properties.h>
#elif defined(_CORE_EMSCRIPTEN_)
    #include <mimalloc.h>
#endif
#if !defined(_CORE_WINDOWS_)
    #include <unistd.h>
    #include <dirent.h>
    #include <pwd.h>
    #include <ftw.h>
    #include <dlfcn.h>
    #include <sys/utsname.h>
    #include <sys/statvfs.h>
    #include <sys/stat.h>
#endif

THREAD_LOCAL coreData::coreTempString coreData::s_TempString         = {};
coreMapStrFull<const coreChar*>       coreData::s_apcCommandLine     = {};
coreString                            coreData::s_sUserFolder        = "";
ZSTD_CCtx*                            coreData::s_pCompressContext   = ZSTD_createCCtx();   // never cleaned up
ZSTD_DCtx*                            coreData::s_pDecompressContext = ZSTD_createDCtx();
coreSpinLock                          coreData::s_CompressLock       = coreSpinLock();
coreSpinLock                          coreData::s_DecompressLock     = coreSpinLock();

extern "C" const coreChar* g_pcUserFolder = "";   // to allow access from C files


// ****************************************************************
/* create formatted string */
coreInt32 coreData::PrintBase(coreChar* OUTPUT pcOutput, const coreInt32 iOutputSize, const coreChar* pcFormat, ...)
{
    ASSERT(pcFormat)

    // prepare variable arguments
    va_list oArgs;
    va_start(oArgs, pcFormat);

    // assemble string
    const coreInt32 iReturn = stbsp_vsnprintf(pcOutput, iOutputSize, pcFormat, oArgs);
    va_end(oArgs);

    return iReturn;
}

coreInt32 coreData::PrintBaseV(coreChar* OUTPUT pcOutput, const coreInt32 iOutputSize, const coreChar* pcFormat, va_list oArgs)
{
    ASSERT(pcFormat)

    // assemble string
    return stbsp_vsnprintf(pcOutput, iOutputSize, pcFormat, oArgs);
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
    if(piTotal)     (*piTotal)     = 0u;
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
    if(piTotal)     (*piTotal)     = 0u;
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

    // check for Wine compatibility layer
    const coreChar* pcWineName = "native";
    {
        // get system library
        const HMODULE pLibrary = GetModuleHandleW(L"ntdll.dll");
        if(pLibrary)
        {
            using coreGetString = const coreChar* (__cdecl *)();

            // load Wine specific functions
            const coreGetString nWineGetVersion = r_cast<coreGetString>(GetProcAddress(pLibrary, "wine_get_version"));
            const coreGetString nWineGetBuildId = r_cast<coreGetString>(GetProcAddress(pLibrary, "wine_get_build_id"));

            // get Wine version and build ID
            if(nWineGetVersion && nWineGetBuildId)
            {
                pcWineName = PRINT("Wine %s (%s)", nWineGetVersion(), nWineGetBuildId());
            }
        }
    }

    // return full operating system name
    return PRINT("Windows %s (%u.%u.%u.%u, %s)", pcSubName, iMajor, iMinor, iBuild, iRevision, pcWineName);

#elif defined(_CORE_LINUX_)

    // fetch kernel information
    utsname oInfo;
    if(uname(&oInfo)) return "Linux";

    // return full operating system name
    return PRINT("%s %s (%s, glibc %s %s) 🐧", oInfo.sysname, oInfo.release, oInfo.version, gnu_get_libc_version(), gnu_get_libc_release());

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

    // get real user identifier of calling process
    return getuid();

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
    const passwd* pRecord = getpwuid(getuid());
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
/* get CPU vendor identifier */
const coreCpuType& coreData::SystemCpuType()
{
    static const coreCpuType s_eCpuType = []()
    {
        // retrieve brand string (vendor string is unreliable)
        const coreChar* pcBrand = coreData::StrToLower(coreData::SystemCpuBrand());

        // determine CPU vendor
        if(std::strstr(pcBrand, "amd"))   return CORE_CPU_TYPE_AMD;
        if(std::strstr(pcBrand, "intel")) return CORE_CPU_TYPE_INTEL;
        if(std::strstr(pcBrand, "apple")) return CORE_CPU_TYPE_APPLE;

        WARN_IF(true) {}
        return CORE_CPU_TYPE_UNKNOWN;
    }();

    return s_eCpuType;
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
    const passwd* pRecord = getpwuid(getuid());
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
    const passwd* pRecord = getpwuid(getuid());
    if(pRecord && pRecord->pw_dir && (pcPath = coreData::__PrepareSystemDir(PRINT("%s/Library/Application Support", pRecord->pw_dir))))
        return pcPath;

#elif defined(_CORE_ANDROID_)

    // get internal storage path for this application
    return SDL_GetAndroidInternalStoragePath();

#elif defined(_CORE_EMSCRIPTEN_)

    // use no specific path
    return "";

#elif defined(_CORE_SWITCH_)

    // get default save-game path
    return PRINT("%s", coreAllocScope(SDL_GetPrefPath(NULL, NULL)).Get());

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
    const passwd* pRecord = getpwuid(getuid());
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
    const passwd* pRecord = getpwuid(getuid());
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
/* get C standard library name (and version) */
const coreChar* coreData::BuildLibraryC()
{
#if defined(_CORE_WINDOWS_)

    return "CRT " STRING(_WIN32_WINNT);

#elif defined(_CORE_LINUX_)

    return "glibc " STRING(__GLIBC__) "." STRING(__GLIBC_MINOR__) " (POSIX " STRING(_POSIX_C_SOURCE) ")";

#elif defined(_CORE_MACOS_)

    return "BSD libc " STRING(__MAC_OS_X_VERSION_MIN_REQUIRED) "/" STRING(__MAC_OS_X_VERSION_MAX_ALLOWED);

#elif defined(_CORE_ANDROID_)

    return "Bionic " STRING(__ANDROID_API__) " (NDK " STRING(__NDK_MAJOR__) "." STRING(__NDK_MINOR__) "." STRING(__NDK_BUILD__) ")";

#elif defined(_CORE_IOS_)

    return "BSD libc " STRING(__IPHONE_OS_VERSION_MIN_REQUIRED) "/" STRING(__IPHONE_OS_VERSION_MAX_ALLOWED);

#elif defined(_CORE_EMSCRIPTEN_)

    return "musl";

#endif
}


// ****************************************************************
/* get C++ standard library name and version */
const coreChar* coreData::BuildLibraryCpp()
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
/* detect attached debugger or tracer */
coreBool coreData::DetectDebugger()
{
#if defined(_CORE_WINDOWS_)

    if(IsDebuggerPresent()) return true;

#elif defined(_CORE_LINUX_)

    // open status pseudo-file
    std::FILE* pFile = coreData::FileOpen("/proc/self/status", "rb");
    if(pFile)
    {
        coreChar acBuffer[0x1000];

        // read all data
        const coreUintW iResult = std::fread(acBuffer, 1u, ARRAY_SIZE(acBuffer) - 1u, pFile);
        acBuffer[iResult] = '\0';

        // close file
        std::fclose(pFile);

        // search tracing entry
        const coreChar* pcCursor = std::strstr(acBuffer, "TracerPid:");
        if(pcCursor)
        {
            // check for valid process identifier
            pcCursor += coreStrLenConst("TracerPid:");
            for(; (*pcCursor) != '\0'; ++pcCursor)
            {
                if(!std::isspace(*pcCursor))
                {
                    return ((*pcCursor) >= '1') && ((*pcCursor) <= '9');
                }
            }
        }
    }

#elif defined(_CORE_MACOS_)

    // set management information base
    coreInt32 aiBase[] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, getpid()};

    // retrieve process information
    kinfo_proc oInfo = {};
    coreUintW  iLen  = sizeof(oInfo);
    if(!sysctl(aiBase, ARRAY_SIZE(aiBase), &oInfo, &iLen, NULL, 0u))
    {
        return HAS_FLAG(oInfo.kp_proc.p_flag, P_TRACED);
    }

#endif

    return false;
}


// ****************************************************************
/* detect attached RenderDoc */
coreBool coreData::DetectRenderDoc()
{
#if defined(_CORE_WINDOWS_)

    return coreData::TestLibrary("renderdoc.dll");

#elif defined(_CORE_LINUX_)

    return coreData::TestLibrary("librenderdoc.so");

#else

    return false;

#endif
}


// ****************************************************************
/* detect Steam Deck hardware */
coreBool coreData::DetectSteamDeck()
{
    const coreChar* pcVariable = coreData::GetEnvironment("SteamDeck");
    return (pcVariable && !std::strcmp(pcVariable, "1"));
}


// ****************************************************************
/* detect Gamescope compositor */
coreBool coreData::DetectGamescope()
{
    const coreChar* pcVariable = coreData::GetEnvironment("XDG_CURRENT_DESKTOP");
    return (pcVariable && !std::strcmp(pcVariable, "gamescope"));
}


// ****************************************************************
/* detect Wine or Proton layer */
coreBool coreData::DetectWine()
{
#if defined(_CORE_WINDOWS_)

    return (GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "wine_get_version") != NULL);

#else

    return false;

#endif
}


// ****************************************************************
/* set current working directory */
coreStatus coreData::SetCurDir(const coreChar* pcPath)
{
    ASSERT(pcPath)

#if defined(_CORE_WINDOWS_)

    if(SetCurrentDirectoryW(coreData::__ToWideChar(pcPath))) return CORE_OK;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_SWITCH_)

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

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_SWITCH_)

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

    // return available options
    if(coreData::GetCommandLine("help"))
    {
        std::puts(PRINT("%s %s", CoreApp::Settings::Name, CoreApp::Settings::Version));
        std::puts("Options:");
        std::puts("  --user-folder-path <value>   set path for storing user-specific files (needs write permission)");
        std::puts("  --help                       display available options");
        std::puts("  --version                    display simple version string");
        std::exit(EXIT_SUCCESS);
    }

    // return simple version string
    if(coreData::GetCommandLine("version"))
    {
        std::puts(CoreApp::Settings::Version);
        std::exit(EXIT_SUCCESS);
    }
}


// ****************************************************************
/* log all current environment variables */
void coreData::LogEnvironment()
{
#if defined(_CORE_MACOS_)
    extern coreChar** environ;
#endif

    WARN_IF(!environ) return;

    Core::Log->ListStartInfo("Environment Variables");
    {
        for(coreChar** ppcEnviron = environ; (*ppcEnviron); ++ppcEnviron)
        {
            const coreChar* pcAssign = std::strchr((*ppcEnviron), '=');
            if(pcAssign)
            {
                Core::Log->ListAdd("%.*s = %s", coreInt32(pcAssign - (*ppcEnviron)), (*ppcEnviron), pcAssign + 1u);
            }
        }
    }
    Core::Log->ListEnd();
}


// ****************************************************************
/* set environment variable (of current process) */
coreStatus coreData::SetEnvironment(const coreChar* pcName, const coreChar* pcValue)
{
    ASSERT(pcName)

#if defined(_CORE_WINDOWS_)

    // create, replace, or remove environment variable
    if(!_wputenv_s(coreData::__ToWideChar(pcName), pcValue ? coreData::__ToWideChar(pcValue) : L"")) return CORE_OK;   // SetEnvironmentVariable is unreliable

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_SWITCH_)

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

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_SWITCH_)

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
#elif defined(_CORE_SWITCH_)
    coreData::SetCurDir(SDL_GetBasePath());
#else
    coreData::SetCurDir(PRINT("%s/../..", coreData::ProcessDir()));
#endif

    // get command line argument
    const coreChar* pcPath = coreData::GetCommandLine("user-folder-path");

    // use specific user folder
    if(pcPath && pcPath[0])
    {
        pcPath = std::strcmp(pcPath, "!appdata") ? PRINT("%s/", pcPath) : coreData::SystemDirAppData();
    }

    // use default user folder (and create directory hierarchy)
    if(!pcPath || !pcPath[0] || (coreData::DirectoryCreate(pcPath) != CORE_OK) || !coreData::DirectoryWritable(pcPath))
    {
        #if defined(_CORE_WINDOWS_) || defined(_CORE_LINUX_)
            pcPath = CoreApp::Settings::UserManagement ? coreData::SystemDirAppData() : "user/";
        #else
            pcPath = coreData::SystemDirAppData();
        #endif

        coreData::DirectoryCreate(pcPath);
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
        const coreChar* pcUserID = Core::Platform->GetUserID();
        return PRINT("%s%s%s", s_sUserFolder.c_str(), pcUserID ? PRINT("user_%s/", pcUserID) : "default/", pcPath);
    }
    else
    {
        // forward to user-shared location
        return coreData::UserFolderShared(pcPath);
    }
}


// ****************************************************************
/* test if dynamic library is already loaded */
coreBool coreData::TestLibrary(const coreChar* pcName)
{
    ASSERT(pcName)

#if defined(_CORE_WINDOWS_)

    return GetModuleHandleW(coreData::__ToWideChar(pcName));

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_)

    return dlopen(pcName, RTLD_LAZY | RTLD_NOLOAD);

#else

    return false;

#endif
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
    if(!pLibrary) pLibrary = dlopen(pcLocal, RTLD_LAZY);   // independent of library paths (DT_RPATH, LD_LIBRARY_PATH, etc.)
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

#elif defined(_CORE_EMSCRIPTEN_)

    return mi_heap_new();

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

#elif defined(_CORE_EMSCRIPTEN_)

    mi_heap_destroy(s_cast<mi_heap_t*>(pHeap));

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

#elif defined(_CORE_EMSCRIPTEN_)

    return mi_heap_malloc(s_cast<mi_heap_t*>(pHeap), iSize);

#else

    return std::malloc(iSize);

#endif
}

// ****************************************************************
/* allocate zero-initialized memory from private heap */
RETURN_RESTRICT void* coreData::HeapCalloc(void* pHeap, const coreUintW iNum, const coreUintW iSize)
{
    ASSERT(pHeap && iSize)

#if defined(_CORE_WINDOWS_)

    return ::HeapAlloc(pHeap, HEAP_ZERO_MEMORY, iNum * iSize);

#elif defined(_CORE_EMSCRIPTEN_)

    return mi_heap_calloc(s_cast<mi_heap_t*>(pHeap), iNum, iSize);

#else

    return std::calloc(iNum, iSize);

#endif
}

// ****************************************************************
/* allocate dynamic-sized memory from private heap */
void coreData::HeapRealloc(void* pHeap, void** OUTPUT ppPointer, const coreUintW iSize)
{
    ASSERT(pHeap && ppPointer && iSize)

#if defined(_CORE_WINDOWS_)

    (*ppPointer) = (*ppPointer) ? ::HeapReAlloc(pHeap, 0u, *ppPointer, iSize) : ::HeapAlloc(pHeap, 0u, iSize);

#elif defined(_CORE_EMSCRIPTEN_)

    (*ppPointer) = mi_heap_realloc(s_cast<mi_heap_t*>(pHeap), *ppPointer, iSize);

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

#elif defined(_CORE_EMSCRIPTEN_)

    mi_free(*ppPointer);

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

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_EMSCRIPTEN_) || defined(_CORE_SWITCH_)

    struct stat oBuffer;

    // quick POSIX check
    if(!stat(pcPath, &oBuffer) && S_ISREG(oBuffer.st_mode)) return true;

#else

    // open file
    SDL_IOStream* pFile = SDL_IOFromFile(pcPath, "rb");
    if(pFile)
    {
        // file exists
        SDL_CloseIO(pFile);
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

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_EMSCRIPTEN_) || defined(_CORE_SWITCH_)

    struct stat oBuffer;

    // get POSIX file info
    if(!stat(pcPath, &oBuffer))
    {
        // return file size
        return oBuffer.st_size;
    }

#else

    // open file
    SDL_IOStream* pFile = SDL_IOFromFile(pcPath, "rb");
    if(pFile)
    {
        // get size from stream
        const coreInt64 iSize = SDL_GetIOSize(pFile);
        SDL_CloseIO(pFile);

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

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_EMSCRIPTEN_) || defined(_CORE_SWITCH_)

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

    // do not copy to same file (unreliable check)
    WARN_IF(!std::strcmp(pcFrom, pcTo)) return CORE_INVALID_INPUT;

#if defined(_CORE_WINDOWS_)

    // copy directly (with attributes)
    if(CopyFileW(coreData::__ToWideChar(pcFrom), coreData::__ToWideChar(pcTo), false)) return CORE_OK;

#elif defined(_CORE_LINUX_)

    // open source file
    const coreInt32 iFileFrom = open(pcFrom, O_RDONLY);
    if(iFileFrom != -1)
    {
        struct stat oBuffer;

        // get POSIX file info
        if(!fstat(iFileFrom, &oBuffer))
        {
            // open destination file
            const coreInt32 iFileTo = open(pcTo, O_WRONLY | O_CREAT | O_TRUNC, oBuffer.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));
            if(iFileTo != -1)
            {
                coreIntW iRet;
                coreIntW iLen = oBuffer.st_size ? oBuffer.st_size : UINT32_MAX;

                // copy directly in kernel space
                if((iRet = copy_file_range(iFileFrom, NULL, iFileTo, NULL, iLen, 0u)) > 0)
                {
                    iLen -= iRet;

                    do
                    {
                        // use fast function
                        iLen -= iRet = copy_file_range(iFileFrom, NULL, iFileTo, NULL, iLen, 0u);
                    }
                    while((iLen > 0) && (iRet > 0));
                }
                else
                {
                    do
                    {
                        // use reliable function
                        iLen -= iRet = sendfile(iFileTo, iFileFrom, NULL, iLen);
                    }
                    while((iLen > 0) && (iRet > 0));
                }

                // close both files
                close(iFileTo);
                close(iFileFrom);

                return (iRet > -1) ? CORE_OK : CORE_ERROR_FILE;
            }
        }

        // close source file
        close(iFileFrom);
    }

#elif defined(_CORE_MACOS_)

    // copy directly (with attributes)
    if(!copyfile(pcFrom, pcTo, NULL, COPYFILE_ALL)) return CORE_OK;

#else

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

    // do not move to same file (unreliable check)
    WARN_IF(!std::strcmp(pcFrom, pcTo)) return CORE_INVALID_INPUT;

#if defined(_CORE_WINDOWS_)

    if(MoveFileExW(coreData::__ToWideChar(pcFrom), coreData::__ToWideChar(pcTo), MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED)) return CORE_OK;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_EMSCRIPTEN_)

    if(!std::rename(pcFrom, pcTo)) return CORE_OK;

#elif defined(_CORE_SWITCH_)

    if(coreData::FileExists(pcFrom)) {coreData::FileDelete(pcTo); if(!std::rename(pcFrom, pcTo)) return CORE_OK;}

#endif

    // try to copy on error (# but never delete source file)
    return coreData::FileCopy(pcFrom, pcTo);
}


// ****************************************************************
/* delete existing file */
coreStatus coreData::FileDelete(const coreChar* pcPath)
{
    ASSERT(pcPath)

#if defined(_CORE_WINDOWS_)

    if(DeleteFileW(coreData::__ToWideChar(pcPath))) return CORE_OK;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_EMSCRIPTEN_) || defined(_CORE_SWITCH_)

    if(!unlink(pcPath)) return CORE_OK;

#endif

    return CORE_ERROR_FILE;
}


// ****************************************************************
/* check if directory exists */
coreBool coreData::DirectoryExists(const coreChar* pcPath)
{
    ASSERT(pcPath)

    // current working directory always exists
    if(!pcPath[0]) return true;

#if defined(_CORE_WINDOWS_)

    const coreUint32 iAttributes = GetFileAttributesW(coreData::__ToWideChar(pcPath));

    // quick Windows check
    if((iAttributes != INVALID_FILE_ATTRIBUTES) && HAS_FLAG(iAttributes, FILE_ATTRIBUTE_DIRECTORY)) return true;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_EMSCRIPTEN_) || defined(_CORE_SWITCH_)

    struct stat oBuffer;

    // quick POSIX check
    if(!stat(pcPath, &oBuffer) && S_ISDIR(oBuffer.st_mode)) return true;

#else

    // try to write into presumed directory
    if(coreData::DirectoryWritable(pcPath)) return true;

#endif

    return false;
}


// ****************************************************************
/* check if directory is writable */
coreBool coreData::DirectoryWritable(const coreChar* pcPath)
{
    ASSERT(pcPath)

    // get temporary file name
    const coreChar* pcTemp = PRINT("%s/check_%u", pcPath, coreData::ProcessID());

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
    SDL_IOStream* pFile = SDL_IOFromFile(pcTemp, "wb");
    if(pFile)
    {
        // close and delete file again
        SDL_CloseIO(pFile);
        coreData::FileDelete(pcTemp);

        return true;
    }

#endif

    return false;
}


// ****************************************************************
/* copy directory from source to destination (and merge) */
coreStatus coreData::DirectoryCopy(const coreChar* pcFrom, const coreChar* pcTo)
{
    ASSERT(pcFrom && pcTo)

    // do not copy to same directory (unreliable check)
    WARN_IF(!std::strcmp(pcFrom, pcTo)) return CORE_INVALID_INPUT;

    // check if source directory even exists
    if(!coreData::DirectoryExists(pcFrom)) return CORE_ERROR_FILE;

    // create destination directory
    coreData::DirectoryCreate(pcTo);

    // copy all children (recursively)
    return coreData::DirectoryEnum(pcFrom, "*", CORE_ENUM_TYPE_DEFAULT, c_cast<coreChar*>(pcTo), [](const coreChar* pcPath, const coreFileStats& oStats, void* pData)
    {
        const coreChar* pcNewPath = PRINT("%s/%s", s_cast<coreChar*>(pData), coreData::StrFilename(pcPath));

        if(oStats.bDirectory) coreData::DirectoryCopy(pcPath, pcNewPath);
                         else coreData::FileCopy     (pcPath, pcNewPath);
    });
}


// ****************************************************************
/* move directory from source to destination (and merge) */
coreStatus coreData::DirectoryMove(const coreChar* pcFrom, const coreChar* pcTo)
{
    ASSERT(pcFrom && pcTo)

    // do not move to same directory (unreliable check)
    WARN_IF(!std::strcmp(pcFrom, pcTo)) return CORE_INVALID_INPUT;

    // check if source directory even exists
    if(!coreData::DirectoryExists(pcFrom)) return CORE_ERROR_FILE;

    // move directory fast (but only works if destination directory does not exist and is on the same volume)
#if defined(_CORE_WINDOWS_)
    if(MoveFileExW(coreData::__ToWideChar(pcFrom), coreData::__ToWideChar(pcTo), 0u)) return CORE_OK;
#else
    if(!std::rename(pcFrom, pcTo)) return CORE_OK;
#endif

    // create destination directory
    coreData::DirectoryCreate(pcTo);

    // move all children (recursively)
    const coreStatus eStatus = coreData::DirectoryEnum(pcFrom, "*", CORE_ENUM_TYPE_DEFAULT, c_cast<coreChar*>(pcTo), [](const coreChar* pcPath, const coreFileStats& oStats, void* pData)
    {
        const coreChar* pcNewPath = PRINT("%s/%s", s_cast<coreChar*>(pData), coreData::StrFilename(pcPath));

        if(oStats.bDirectory) coreData::DirectoryMove(pcPath, pcNewPath);
                         else coreData::FileMove     (pcPath, pcNewPath);
    });

    // delete source directory (if empty)
    coreData::DirectoryDelete(pcFrom, false);

    return eStatus;
}


// ****************************************************************
/* delete existing directory */
coreStatus coreData::DirectoryDelete(const coreChar* pcPath, const coreBool bWithContent)
{
    ASSERT(pcPath)

#if defined(_CORE_WINDOWS_)

    if(bWithContent)
    {
        // delete all children first (recursively)
        coreData::DirectoryEnum(pcPath, "*", CORE_ENUM_TYPE_TREE, NULL, [](const coreChar* pcPath, const coreFileStats& oStats, void* pData)
        {
            if(oStats.bDirectory) RemoveDirectoryW(coreData::__ToWideChar(pcPath));
                             else DeleteFileW     (coreData::__ToWideChar(pcPath));
        });
    }

    // delete base directory (if empty)
    if(RemoveDirectoryW(coreData::__ToWideChar(pcPath))) return CORE_OK;

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_EMSCRIPTEN_) || defined(_CORE_SWITCH_)

    if(bWithContent)
    {
        // delete everything (recursively)
        const auto nWalkFunc = [](const coreChar* pcPath, const struct stat* pBuffer, const coreInt32 iFlags, struct FTW* pInfo)
        {
            if(S_ISDIR(pBuffer->st_mode)) return rmdir (pcPath);
                                     else return unlink(pcPath);
        };

        // walk through the whole directory tree (children first)
        if(!nftw(pcPath, nWalkFunc, 20, FTW_DEPTH | FTW_PHYS)) return CORE_OK;
    }
    else
    {
        // delete base directory (if empty)
        if(!rmdir(pcPath)) return CORE_OK;
    }

#endif

    return CORE_ERROR_FILE;
}


// ****************************************************************
/* create directory hierarchy */
coreStatus coreData::DirectoryCreate(const coreChar* pcPath)
{
    ASSERT(pcPath)

    // check if directory already exists
    if(coreData::DirectoryExists(pcPath)) return CORE_OK;

    // handle parent directory first (recursively)
    coreData::DirectoryCreate(coreData::StrDirectory(pcPath));   // ignore errors

    // create base directory
#if defined(_CORE_WINDOWS_)
    if(CreateDirectoryW(coreData::__ToWideChar(pcPath), NULL)) return CORE_OK;
#else
    if(!mkdir(pcPath, S_IRWXU)) return CORE_OK;
#endif

    return CORE_ERROR_FILE;
}


// ****************************************************************
/* retrieve relative paths of all files from a directory */
coreStatus coreData::DirectoryScan(const coreChar* pcPath, const coreChar* pcFilter, coreList<coreString>* OUTPUT pasOutput)
{
    ASSERT(pcPath && pcFilter && pasOutput)

    return coreData::DirectoryEnum(pcPath, pcFilter, CORE_ENUM_TYPE_DEFAULT, pasOutput, [](const coreChar* pcPath, const coreFileStats& oStats, void* pData)
    {
        if(!oStats.bDirectory) s_cast<coreList<coreString>*>(pData)->push_back(pcPath);
    });
}


// ****************************************************************
/* retrieve relative paths of all files from a directory tree */
coreStatus coreData::DirectoryScanTree(const coreChar* pcPath, const coreChar* pcFilter, coreList<coreString>* OUTPUT pasOutput)
{
    ASSERT(pcPath && pcFilter && pasOutput)

    return coreData::DirectoryEnum(pcPath, pcFilter, CORE_ENUM_TYPE_TREE, pasOutput, [](const coreChar* pcPath, const coreFileStats& oStats, void* pData)
    {
        if(!oStats.bDirectory) s_cast<coreList<coreString>*>(pData)->push_back(pcPath);
    });
}


// ****************************************************************
/* enumerate relative paths of all children from a directory */
coreStatus coreData::DirectoryEnum(const coreChar* pcPath, const coreChar* pcFilter, const coreEnumType eEnumType, void* pEnumData, const coreEnumFunc nEnumFunc)
{
    ASSERT(pcPath && pcFilter)

    coreChar acString[CORE_DATA_MAX_PATH];

    // make local copy (to handle printed paths)
    coreData::StrCopy(acString, ARRAY_SIZE(acString), pcPath);

#if defined(_CORE_WINDOWS_)

    WIN32_FIND_DATAW oFile;

    // open folder
    const HANDLE pFolder = FindFirstFileExW(coreData::__ToWideChar(PRINT(HAS_FLAG(eEnumType, CORE_ENUM_TYPE_TREE) ? "%s/*" : "%s/%s", acString, pcFilter)), FindExInfoBasic, &oFile, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
    if(pFolder == INVALID_HANDLE_VALUE)
    {
        Core::Log->Warning("Folder (%s/%s) could not be opened", acString, pcFilter);
        return CORE_ERROR_FILE;
    }

    // loop through all children
    do
    {
        if(oFile.cFileName[0] != L'.')
        {
            // enter child directory (recursively)
            if(HAS_FLAG(eEnumType, CORE_ENUM_TYPE_TREE) && HAS_FLAG(oFile.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
            {
                coreData::DirectoryEnum(PRINT("%s/%s", acString, coreData::__ToAnsiChar(oFile.cFileName)), pcFilter, eEnumType, pEnumData, nEnumFunc);
            }

            // check filter pattern
            if(!HAS_FLAG(eEnumType, CORE_ENUM_TYPE_TREE) || coreData::StrCmpLike(pFile->d_name, pcFilter))
            {
                const coreChar* pcFullPath = PRINT("%s/%s", acString, coreData::__ToAnsiChar(oFile.cFileName));

                // prepare file info
                coreFileStats oStats = {};
                oStats.bDirectory = HAS_FLAG(oFile.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY);

                // query additional file info
                if(HAS_FLAG(eEnumType, CORE_ENUM_TYPE_STATS))
                {
                    oStats.iSize      = (coreInt64(oFile.nFileSizeHigh) << 32u) | (coreInt64(oFile.nFileSizeLow));
                    oStats.iWriteTime = r_cast<coreUint64&>(oFile.ftLastWriteTime) / 10000000ull - 11644473600ull;
                }

                // enumerate child
                nEnumFunc(pcFullPath, oStats, pEnumData);
            }
        }
    }
    while(FindNextFileW(pFolder, &oFile));

    // close folder
    FindClose(pFolder);

#else

    dirent* pFile;

    // open folder
    DIR* pFolder = opendir(acString);
    if(!pFolder)
    {
        Core::Log->Warning("Folder (%s/%s) could not be opened", acString, pcFilter);
        return CORE_ERROR_FILE;
    }

    // loop through all children
    while((pFile = readdir(pFolder)))
    {
        if(pFile->d_name[0] != '.')
        {
            // enter child directory (recursively)
            if(HAS_FLAG(eEnumType, CORE_ENUM_TYPE_TREE) && (pFile->d_type == DT_DIR))
            {
                coreData::DirectoryEnum(PRINT("%s/%s", acString, pFile->d_name), pcFilter, eEnumType, pEnumData, nEnumFunc);
            }

            // check filter pattern
            if(coreData::StrCmpLike(pFile->d_name, pcFilter))
            {
                const coreChar* pcFullPath = PRINT("%s/%s", acString, pFile->d_name);

                // prepare file info
                coreFileStats oStats = {};
                oStats.bDirectory = (pFile->d_type == DT_DIR);

                // query additional file info
                if(HAS_FLAG(eEnumType, CORE_ENUM_TYPE_STATS))
                {
                    struct stat oBuffer;

                    // get POSIX file info
                    if(!stat(pcFullPath, &oBuffer))
                    {
                        oStats.iSize      = oBuffer.st_size;
                        oStats.iWriteTime = oBuffer.st_mtime;
                    }
                }

                // enumerate child
                nEnumFunc(pcFullPath, oStats, pEnumData);
            }
        }
    }

    // close folder
    closedir(pFolder);

#endif

    return CORE_OK;
}


// ****************************************************************
/* create symbolic link to file or directory */
coreStatus coreData::SymlinkCreate(const coreChar* pcPath, const coreChar* pcTarget)
{
#if defined(_CORE_WINDOWS_)

    const coreWchar* pcWidePath   = coreData::__ToWideChar(pcPath);
    const coreWchar* pcWideTarget = coreData::__ToWideChar(pcTarget);

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
    if(!Core::Debug->IsEnabled()) return false;

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

#elif defined(_CORE_LINUX_) || defined(_CORE_MACOS_) || defined(_CORE_EMSCRIPTEN_) || defined(_CORE_SWITCH_)

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
        const coreChar* pcString = strerror_r(iError, acBuffer, ARRAY_SIZE(acBuffer)) ? "unknown error" : acBuffer;

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
    ASSERT(pInput && iInputSize && ppOutput && piOutputSize && (iLevel >= ZSTD_minCLevel()) && (iLevel <= ZSTD_maxCLevel()))

    // retrieve required output size
    const coreUintW iBound  = ZSTD_compressBound(iInputSize);
    coreByte*       pBuffer = new coreByte[iBound + sizeof(coreUint32)];

    const coreSpinLocker oLocker(&s_CompressLock);

    // compress data
    const coreUintW iWritten = ZSTD_compressCCtx(s_pCompressContext, pBuffer + sizeof(coreUint32), iBound, pInput, iInputSize, iLevel);
    if(ZSTD_isError(iWritten))
    {
        SAFE_DELETE_ARRAY(pBuffer)

        Core::Log->Warning("Error compressing data (ZSTD: %s)", ZSTD_getErrorName(iWritten));
        return CORE_INVALID_INPUT;
    }

    // store original size
    (*r_cast<coreUint32*>(pBuffer)) = iInputSize;

    // return compressed data and size
    (*ppOutput)     = pBuffer;
    (*piOutputSize) = iWritten + sizeof(coreUint32);

    return CORE_OK;
}

coreStatus coreData::Compress(const coreByte* pInput, const coreUint32 iInputSize, coreByte* OUTPUT pOutput, coreUint32* OUTPUT piOutputSize, const coreInt32 iLevel)
{
    ASSERT(pInput && iInputSize && piOutputSize && (iLevel >= ZSTD_minCLevel()) && (iLevel <= ZSTD_maxCLevel()))

    // retrieve required output size
    const coreUintW iBound = ZSTD_compressBound(iInputSize);

    // check for target buffer space
    if(((*piOutputSize) < iBound + sizeof(coreUint32)) || !pOutput)
    {
        (*piOutputSize) = iBound + sizeof(coreUint32);
        return CORE_INVALID_DATA;
    }

    const coreSpinLocker oLocker(&s_CompressLock);

    // compress data
    const coreUintW iWritten = ZSTD_compressCCtx(s_pCompressContext, pOutput + sizeof(coreUint32), iBound, pInput, iInputSize, iLevel);
    if(ZSTD_isError(iWritten))
    {
        Core::Log->Warning("Error compressing data (ZSTD: %s)", ZSTD_getErrorName(iWritten));
        return CORE_INVALID_INPUT;
    }

    // store original size
    (*r_cast<coreUint32*>(pOutput)) = iInputSize;

    // return compressed size
    (*piOutputSize) = iWritten + sizeof(coreUint32);

    return CORE_OK;
}


// ****************************************************************
/* decompress data with Zstandard library */
coreStatus coreData::Decompress(const coreByte* pInput, const coreUint32 iInputSize, coreByte** OUTPUT ppOutput, coreUint32* OUTPUT piOutputSize, const coreUint32 iLimit)
{
    ASSERT(pInput && iInputSize && ppOutput && piOutputSize && iLimit)

    // check data integrity
    const coreUint64 iContentSize = ZSTD_getFrameContentSize(pInput + sizeof(coreUint32), iInputSize - sizeof(coreUint32));
    if((iContentSize == ZSTD_CONTENTSIZE_ERROR) || ((iContentSize > iLimit) && (iContentSize != ZSTD_CONTENTSIZE_UNKNOWN)))
    {
        Core::Log->Warning("Error checking data integrity (size: %llu)", iContentSize);
        return CORE_INVALID_DATA;
    }

    // retrieve original size
    const coreUint32 iBound  = MIN(*r_cast<const coreUint32*>(pInput), iLimit);
    coreByte*        pBuffer = new coreByte[iBound];

    const coreSpinLocker oLocker(&s_DecompressLock);

    // decompress data
    const coreUintW iWritten = ZSTD_decompressDCtx(s_pDecompressContext, pBuffer, iBound, pInput + sizeof(coreUint32), iInputSize - sizeof(coreUint32));
    if(ZSTD_isError(iWritten) || (iWritten != iBound))
    {
        SAFE_DELETE_ARRAY(pBuffer)

        Core::Log->Warning("Error decompressing data (ZSTD: %s)", ZSTD_getErrorName(iWritten));
        return CORE_INVALID_INPUT;
    }

    // return decompressed data and size
    (*ppOutput)     = pBuffer;
    (*piOutputSize) = iBound;

    return CORE_OK;
}

coreStatus coreData::Decompress(const coreByte* pInput, const coreUint32 iInputSize, coreByte* OUTPUT pOutput, coreUint32* OUTPUT piOutputSize)
{
    ASSERT(pInput && iInputSize && piOutputSize)

    // check data integrity
    const coreUint64 iContentSize = ZSTD_getFrameContentSize(pInput + sizeof(coreUint32), iInputSize - sizeof(coreUint32));
    if(iContentSize == ZSTD_CONTENTSIZE_ERROR)
    {
        Core::Log->Warning("Error checking data integrity (size: %llu)", iContentSize);
        return CORE_INVALID_DATA;
    }

    // retrieve original size
    const coreUint32 iBound = (*r_cast<const coreUint32*>(pInput));

    // check for target buffer space
    if(((*piOutputSize) < iBound) || !pOutput)
    {
        (*piOutputSize) = iBound;
        return CORE_INVALID_DATA;
    }

    const coreSpinLocker oLocker(&s_DecompressLock);

    // decompress data
    const coreUintW iWritten = ZSTD_decompressDCtx(s_pDecompressContext, pOutput, iBound, pInput + sizeof(coreUint32), iInputSize - sizeof(coreUint32));
    if(ZSTD_isError(iWritten) || (iWritten != iBound))
    {
        Core::Log->Warning("Error decompressing data (ZSTD: %s)", ZSTD_getErrorName(iWritten));
        return CORE_INVALID_INPUT;
    }

    // return decompressed size
    (*piOutputSize) = iBound;

    return CORE_OK;
}


// ****************************************************************
/* scramble data with 64-bit key */
void coreData::Scramble(coreByte* OUTPUT pData, const coreUintW iSize, const coreUint64 iKey)
{
    ASSERT(pData)

    // create sequence
    coreRand oRand(iKey);

    // combine sequence with original data
    for(coreUintW i = 0u; i < iSize; ++i)
    {
        pData[i] = (oRand.Raw() & 0xFFu) - pData[i];
    }
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
const coreChar* coreData::StrFilename(const coreChar* pcInput, const coreBool bExtension)
{
    WARN_IF(!pcInput) return "";

    if(bExtension)
    {
        // return after last path-delimiter
        for(const coreChar* pcCursor = pcInput + std::strlen(pcInput) - 2u; pcCursor >= pcInput; --pcCursor)
        {
            if(((*pcCursor) == '/') || ((*pcCursor) == '\\'))
                return pcCursor + 1u;
        }

        return pcInput;
    }
    else
    {
        // get file name without extension
        const coreChar* pcFull = coreData::StrFilename(pcInput);
        const coreChar* pcDot  = std::strrchr(pcFull, '.');
        return pcDot ? coreData::StrLeft(pcFull, pcDot - pcFull) : pcFull;
    }
}


// ****************************************************************
/* safely get directory from path */
const coreChar* coreData::StrDirectory(const coreChar* pcInput)
{
    WARN_IF(!pcInput) return "";

    const coreChar* pcName = coreData::StrFilename(pcInput);
    return coreData::StrLeft(pcInput, pcName - pcInput);
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
/* safely get version number (with single digits) */
corePoint2U8 coreData::StrVersion(const coreChar* pcInput)
{
    WARN_IF(!pcInput) return corePoint2U8(0u, 0u);

    const coreChar* pcDot = std::strchr(pcInput, '.');
    return (P_TO_UI(pcDot) > P_TO_UI(pcInput)) ? corePoint2U8((pcDot - 1u)[0] - '0', (pcDot + 1u)[0] - '0') : corePoint2U8(0u, 0u);
}


// ****************************************************************
/* copy string into another buffer */
coreUintW coreData::StrCopy(coreChar* OUTPUT pcOutput, const coreUintW iOutputSize, const coreChar* pcInput, const coreUintW iNum)
{
    ASSERT(pcOutput && iOutputSize && pcInput && (iNum <= std::strlen(pcInput)))

    // calculate string length
    const coreUintW iInputLen  = iNum ? iNum : std::strlen(pcInput);
    const coreUintW iOutputLen = MIN(iInputLen, iOutputSize - 1u);

    // copy string with guaranteed null-termination
    std::memcpy(pcOutput, pcInput, iOutputLen);
    pcOutput[iOutputLen] = '\0';

    WARN_IF(iInputLen >= iOutputSize) return 0u;
    return iOutputLen;
}


// ****************************************************************
/* prepare path for system directory */
const coreChar* coreData::__PrepareSystemDir(const coreChar* pcPath)
{
    ASSERT(pcPath)

    // get folder name from application name
    static const coreString s_sIdentifier = []()
    {
        return coreString(CoreApp::Settings::Name).replace(" ", "");
    }();

    // create full path
    const coreChar* pcFullPath = PRINT("%s/%s/", pcPath, s_sIdentifier.c_str());

    // create directory hierarchy (and check if path is valid)
    if(coreData::DirectoryCreate(pcFullPath) != CORE_OK) return NULL;

    return pcFullPath;
}


// ****************************************************************
/* transform 8-bit ANSI to 16-bit Unicode (for Windows API) */
const coreWchar* coreData::__ToWideChar(const coreChar* pcText)
{
#if defined(_CORE_WINDOWS_)

    if(pcText)
    {
        // wide temp-string structure (defined here for easier handling)
        struct alignas(ALIGNMENT_CACHE) coreTempStringW final
        {
            coreWchar aacData[4][CORE_DATA_STRING_LEN];
            coreUintW iCurrent;
        };
        static THREAD_LOCAL coreTempStringW s_TempStringW;
        coreTempStringW& A = s_TempStringW;

        // access next wide temp-string
        if(++A.iCurrent >= ARRAY_SIZE(A.aacData)) A.iCurrent = 0u;
        coreWchar* pcString = A.aacData[A.iCurrent];

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