///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_DATA_H_
#define _CORE_GUARD_DATA_H_

// TODO 3: implement constexpr strright
// TODO 3: reuse context on compression and decompression (ZSTD_createCCtx & co), but needs to be thread-safe ?
// TODO 3: --headless
// TODO 3: add ToChars float precision parameter (+ search for 'PRINT("%f' and 'PRINT("%.')
// TODO 3: make !temp and !appdata replace instead
// TODO 3: add compiled/included Windows SDK version to BuildLibrary string
// TODO 3: FolderScanTree should not allocate temporary list and string memory for (non-directory) entries outside the filter-pattern
// TODO 2: implement checks and proper handling for paths exceeding the max length (currently they are truncated and might throw "not enough buffer-space" errors on some APIs)
// TODO 3: manually convert Win32 paths to \\?\ format, only for absolute paths (expand relative paths ? similar to user-folder ?), and requires changing path-delimiter '/' to '\' (what about SDL RWops ?)
// TODO 1: handle distinction between user-folder (read+write) and data-folder (read only), for all platforms, and all file+directory functions
// TODO 3: implement proper UTF8 versions for toupper, tolower, isupper, islower (wchar_t is only 2-bytes on Windows)
// TODO 3: StrVersion should work with integers (sscanf ?)
// TODO 2: implement locked/scoped temp-string return
// TODO 3: localtime_s/_r and gmtime_s/_r
// TODO 3: getpwuid_r and getlogin_r


// ****************************************************************
/* data definitions */
#define CORE_DATA_STRING_NUM (32u)    // number of temp-strings
#define CORE_DATA_STRING_LEN (512u)   // length of each temp-string
#define CORE_DATA_MAX_PATH   (512u)   // maximum supported file-path length

STATIC_ASSERT(CORE_DATA_STRING_LEN >= CORE_DATA_MAX_PATH)

#if defined(_CORE_DEBUG_)
    #define PRINT(...)   ([&]() {if(false) std::printf(__VA_ARGS__); return coreData::Print(__VA_ARGS__);}())   // enable format-specifier checking
#else
    #define PRINT(...)   (coreData::Print(__VA_ARGS__))
#endif

#define TIMEMAP_LOCAL(t) ([](const std::time_t iValue) {return std::localtime(&iValue);}(t))
#define TIMEMAP_GM(t)    ([](const std::time_t iValue) {return std::gmtime   (&iValue);}(t))
#define TIMEMAP_CURRENT  (TIMEMAP_LOCAL(std::time(NULL)))

#define TO_UPPER(c)      (coreChar(std::toupper(coreUint8(c))))
#define TO_LOWER(c)      (coreChar(std::tolower(coreUint8(c))))
#define IS_UPPER(c)      (std::isupper(coreUint8(c)))
#define IS_LOWER(c)      (std::islower(coreUint8(c)))

#if defined(_CORE_MSVC_)
    #define strtok_r strtok_s
#endif

STATIC_ASSERT(sizeof(std::time_t) == 8u)


// ****************************************************************
/* data utility collection */
class INTERFACE coreData final
{
private:
    /* temp-string structure */
    struct alignas(ALIGNMENT_CACHE) coreTempString final
    {
        coreChar  aacData[CORE_DATA_STRING_NUM][CORE_DATA_STRING_LEN];   // aligned temp-string buffer
        coreUintW iCurrent;                                              // current temp-string
    };


private:
    static THREAD_LOCAL coreTempString s_TempString;           // thread-local temp-string container

    static coreMapStrFull<const coreChar*> s_apcCommandLine;   // parsed command line arguments
    static coreString                      s_sUserFolder;      // selected user folder


public:
    DISABLE_CONSTRUCTION(coreData)

    /* create formatted string */
    static coreInt32 PrintBase (coreChar* OUTPUT pcOutput, const coreInt32 iMaxLen, SDL_PRINTF_FORMAT_STRING const coreChar* pcFormat, ...) SDL_PRINTF_VARARG_FUNC(3);
    static coreInt32 PrintBaseV(coreChar* OUTPUT pcOutput, const coreInt32 iMaxLen, const coreChar* pcFormat, va_list oArgs);
    template <typename... A> static RETURN_RESTRICT const coreChar* Print(const coreChar* pcFormat, A&&... vArgs);
    static constexpr                RETURN_RESTRICT const coreChar* Print(const coreChar* pcFormat) {return pcFormat;}

    /* convert between trivial value and string */
    template <typename T>                static const coreChar* ToChars  (const T& tValue);
    template <typename T, typename... A> static T               FromChars(const coreChar* pcString, const coreUintW iLen, A&&... vArgs);

    /* get application properties */
    static              coreUint32 ProcessID    ();
    static              coreUint64 ProcessMemory();
    static        const coreChar*  ProcessPath  ();
    static inline const coreChar*  ProcessName  () {return coreData::StrFilename (coreData::ProcessPath());}
    static inline const coreChar*  ProcessDir   () {return coreData::StrDirectory(coreData::ProcessPath());}

    /* get operating system properties */
    static       coreBool   SystemMemory    (coreUint64* OUTPUT piAvailable, coreUint64* OUTPUT piTotal);
    static       coreBool   SystemSpace     (coreUint64* OUTPUT piAvailable, coreUint64* OUTPUT piTotal);
    static const coreChar*  SystemOsName    ();
    static       coreUint32 SystemUserID    ();
    static const coreChar*  SystemUserName  ();
    static const coreChar*  SystemCpuVendor ();
    static const coreChar*  SystemCpuBrand  ();
    static const coreChar*  SystemDirAppData();
    static const coreChar*  SystemDirTemp   ();

    /* get build properties */
    static const coreChar* BuildCompiler();
    static const coreChar* BuildLibrary ();

    /* control current working directory */
    static       coreStatus SetCurDir(const coreChar* pcPath);
    static const coreChar*  GetCurDir();

    /* control command line arguments */
    static        void            LogCommandLine();
    static        void            SetCommandLine(const coreInt32 iArgc, const coreChar* const* ppcArgv);
    static inline const coreChar* GetCommandLine(const coreHashString& sArgument) {ASSERT(coreData::StrIsLower(sArgument.GetString())) return s_apcCommandLine.count(sArgument) ? s_apcCommandLine.at(sArgument) : NULL;}

    /* control environment variables */
    static void            LogEnvironment();
    static coreStatus      SetEnvironment(const coreChar* pcName, const coreChar* pcValue);
    static const coreChar* GetEnvironment(const coreChar* pcName);

    /* control default folders */
    static void            InitDefaultFolders();
    static const coreChar* UserFolderShared (const coreChar* pcPath);
    static const coreChar* UserFolderPrivate(const coreChar* pcPath);

    /* handle dynamic libraries */
    static void*      OpenLibrary (const coreChar* pcName);
    static void*      GetAddress  (void* pLibrary, const coreChar* pcName);
    static coreStatus CloseLibrary(void* pLibrary);

    /* handle dynamic memory */
    static                 void* HeapCreate (const coreBool bThreadSafe);
    static                 void  HeapDestroy(void* pHeap);
    static RETURN_RESTRICT void* HeapMalloc (void* pHeap, const coreUintW iSize);
    static RETURN_RESTRICT void* HeapCalloc (void* pHeap, const coreUintW iNum, const coreUintW iSize);
    static                 void  HeapRealloc(void* pHeap, void** OUTPUT ppPointer, const coreUintW iSize);
    static                 void  HeapFree   (void* pHeap, void** OUTPUT ppPointer);

    /* handle physical files and folders */
    static std::FILE*  FileOpen      (const coreChar* pcPath, const coreChar* pcMode);
    static coreBool    FileExists    (const coreChar* pcPath);
    static coreInt64   FileSize      (const coreChar* pcPath);
    static std::time_t FileWriteTime (const coreChar* pcPath);
    static coreStatus  FileCopy      (const coreChar* pcFrom, const coreChar* pcTo);
    static coreStatus  FileMove      (const coreChar* pcFrom, const coreChar* pcTo);
    static coreStatus  FileDelete    (const coreChar* pcPath);
    static coreBool    FolderExists  (const coreChar* pcPath);
    static coreBool    FolderWritable(const coreChar* pcPath);
    static coreStatus  FolderCreate  (const coreChar* pcPath);
    static coreStatus  FolderScan    (const coreChar* pcPath, const coreChar* pcFilter, coreList<coreString>* OUTPUT pasOutput);
    static coreStatus  FolderScanTree(const coreChar* pcPath, const coreChar* pcFilter, coreList<coreString>* OUTPUT pasOutput);
    static coreStatus  SymlinkCreate (const coreChar* pcPath, const coreChar* pcTarget);

    /* check for system errors */
    static coreBool CheckLastError();

    /* retrieve date and time */
    static void            DateTimeValue(coreUint16* OUTPUT piYea, coreUint16* OUTPUT piMon, coreUint16* OUTPUT piDay, coreUint16* OUTPUT piHou, coreUint16* OUTPUT piMin, coreUint16* OUTPUT piSec, const std::tm* pTimeMap = TIMEMAP_CURRENT);
    static const coreChar* DateTimePrint(const coreChar* pcFormat, const std::tm* pTimeMap = TIMEMAP_CURRENT);
    static inline const coreChar* DateString(const std::tm* pTimeMap = TIMEMAP_CURRENT) {return coreData::DateTimePrint("%Y-%m-%d", pTimeMap);}
    static inline const coreChar* TimeString(const std::tm* pTimeMap = TIMEMAP_CURRENT) {return coreData::DateTimePrint("%H:%M:%S", pTimeMap);}

    /* compress and decompress data */
    static coreStatus Compress  (const coreByte* pInput, const coreUint32 iInputSize, coreByte** OUTPUT ppOutput, coreUint32* OUTPUT piOutputSize, const coreInt32  iLevel = ZSTD_CLEVEL_DEFAULT);
    static coreStatus Compress  (const coreByte* pInput, const coreUint32 iInputSize, coreByte*  OUTPUT pOutput,  coreUint32* OUTPUT piOutputSize, const coreInt32  iLevel = ZSTD_CLEVEL_DEFAULT);
    static coreStatus Decompress(const coreByte* pInput, const coreUint32 iInputSize, coreByte** OUTPUT ppOutput, coreUint32* OUTPUT piOutputSize, const coreUint32 iLimit = UINT32_MAX);
    static coreStatus Decompress(const coreByte* pInput, const coreUint32 iInputSize, coreByte*  OUTPUT pOutput,  coreUint32* OUTPUT piOutputSize);
    static void       Scramble  (coreByte* OUTPUT pData, const coreUintW iSize, const coreUint64 iKey = 0u);
    static void       Unscramble(coreByte* OUTPUT pData, const coreUintW iSize, const coreUint64 iKey = 0u);

    /* get compile-time type information */
    template <typename T> static const     coreChar*  TypeName();
    template <typename T> static constexpr coreUint32 TypeId();

    /* operate with string data */
    template <typename F> static const coreChar* StrProcess     (const coreChar* pcInput,                              F&& nFunction);   // [](const coreChar  cChar)   -> coreChar
    template <typename F> static void            StrForEachToken(const coreChar* pcInput, const coreChar* pcDelimiter, F&& nFunction);   // [](const coreChar* pcToken) -> void
    static inline       coreBool  StrCmpLike  (const coreChar* s, const coreChar* t) {return ((*t) == '*') ? StrCmpLike(s, t+1u) || ((*s) && StrCmpLike(s+1u, t)) : (*s) ? (((*t) == '?') || (TO_LOWER(*s) == TO_LOWER(*t))) && StrCmpLike(s+1u, t+1u) : !(*t);}
    static inline const coreChar* StrToUpper  (const coreChar* pcInput)              {return coreData::StrProcess(pcInput, [](const coreChar c) {return TO_UPPER(c);});}
    static inline const coreChar* StrToLower  (const coreChar* pcInput)              {return coreData::StrProcess(pcInput, [](const coreChar c) {return TO_LOWER(c);});}
    static inline       coreBool  StrIsUpper  (const coreChar* pcInput)              {while(*pcInput) if(IS_LOWER(*(pcInput++))) return false; return true;}
    static inline       coreBool  StrIsLower  (const coreChar* pcInput)              {while(*pcInput) if(IS_UPPER(*(pcInput++))) return false; return true;}
    static const coreChar*        StrLeft     (const coreChar* pcInput, const coreUintW iNum);
    static const coreChar*        StrRight    (const coreChar* pcInput, const coreUintW iNum);
    static const coreChar*        StrFilename (const coreChar* pcInput);
    static const coreChar*        StrDirectory(const coreChar* pcInput);
    static const coreChar*        StrExtension(const coreChar* pcInput);
    static coreFloat              StrVersion  (const coreChar* pcInput);
    static coreBool               StrCopy     (coreChar* OUTPUT pcOutput, const coreUintW iMaxLen, const coreChar* pcInput);

    /* operate with containers */
    template <typename T> static inline void Shuffle(const T& tBegin, const T& tEnd, const coreUint32 iSeed = std::time(NULL)) {std::shuffle(tBegin, tEnd, std::minstd_rand(iSeed));}


private:
    /* access next temp-string */
    static inline RETURN_RESTRICT coreChar* __NextTempString() {coreTempString& A = s_TempString; if(++A.iCurrent >= CORE_DATA_STRING_NUM) A.iCurrent = 0u; return A.aacData[A.iCurrent];}

    /* prepare path for system directory */
    static const coreChar* __PrepareSystemDir(const coreChar* pcPath);

    /* transform between 8-bit ANSI and 16-bit Unicode (for Windows API) */
    static const coreWchar* __ToWideChar(const coreChar*  pcText);
    static const coreChar*  __ToAnsiChar(const coreWchar* pcText);
};


// ****************************************************************
/* create formatted string */
template <typename... A> RETURN_RESTRICT const coreChar* coreData::Print(const coreChar* pcFormat, A&&... vArgs)
{
    coreChar* pcString = coreData::__NextTempString();

    // forward arguments and assemble string
    const coreUint32 iReturn = coreData::PrintBase(pcString, CORE_DATA_STRING_LEN, pcFormat, std::forward<A>(vArgs)...);
    ASSERT(iReturn < CORE_DATA_STRING_LEN)

    return pcString;
}


// ****************************************************************
/* convert trivial value to string */
template <typename T> const coreChar* coreData::ToChars(const T& tValue)
{
    coreChar* pcString = coreData::__NextTempString();

    // use high-performance conversion
    const std::to_chars_result oResult = std::to_chars(pcString, pcString + CORE_DATA_STRING_LEN - 1u, tValue);
    WARN_IF(oResult.ec != std::errc()) return "";

    // always null-terminate
    (*oResult.ptr) = '\0';

    return pcString;
}

#if defined(_CORE_MACOS_) && (MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_15)
    template <> inline const coreChar* coreData::ToChars(const coreInt16&  tValue) {return PRINT("%d",  tValue);}
    template <> inline const coreChar* coreData::ToChars(const coreInt32&  tValue) {return PRINT("%d",  tValue);}
    template <> inline const coreChar* coreData::ToChars(const coreUint16& tValue) {return PRINT("%u",  tValue);}
    template <> inline const coreChar* coreData::ToChars(const coreUint32& tValue) {return PRINT("%u",  tValue);}
    template <> inline const coreChar* coreData::ToChars(const coreUintW&  tValue) {return PRINT("%zu", tValue);}
    template <> inline const coreChar* coreData::ToChars(const coreFloat&  tValue) {return PRINT("%f",  tValue);}
#endif


// ****************************************************************
/* convert string to trivial value */
template <typename T, typename... A> T coreData::FromChars(const coreChar* pcString, const coreUintW iLen, A&&... vArgs)
{
    ASSERT(pcString)

    T tValue;

    // use high-performance conversion
    const std::from_chars_result oResult = std::from_chars(pcString, pcString + iLen, tValue, std::forward<A>(vArgs)...);
    WARN_IF(oResult.ec != std::errc()) return T(0);

    return tValue;
}

#if defined(_CORE_LIBCPP_)
    template <> inline coreFloat coreData::FromChars(const coreChar* pcString, const coreUintW iLen) {ASSERT(pcString) return coreFloat(std::atof(pcString));}
#endif


// ****************************************************************
/* get compile-time type name (with run-time extraction) */
template <typename T> const coreChar* coreData::TypeName()
{
    coreChar* pcString = coreData::__NextTempString();

#if defined(_CORE_MSVC_)

    // analyze function signature (const char* __cdecl coreData::TypeName<int>(void))
    static const coreChar* s_pcBase = __FUNCSIG__;
    static const coreChar* s_pcFrom = std::strchr (s_pcBase, '<') + 1u; if(s_pcFrom - 1u == NULL) return "";
    static const coreChar* s_pcTo   = std::strrchr(s_pcFrom, '>');      if(s_pcTo        == NULL) return "";

#elif defined(_CORE_GCC_) || defined(_CORE_CLANG_)

    // analyze function signature (const char* coreData::TypeName() [with T = int])
    static const coreChar* s_pcBase = __PRETTY_FUNCTION__;
    static const coreChar* s_pcFrom = std::strchr (s_pcBase, '=') + 2u; if(s_pcFrom - 2u == NULL) return "";
    static const coreChar* s_pcTo   = std::strrchr(s_pcFrom, ']');      if(s_pcTo        == NULL) return "";

#endif

    // calculate name length
    static const coreUintW s_iLen = MIN(coreUintW(s_pcTo - s_pcFrom), CORE_DATA_STRING_LEN - 1u);

    // extract name from the function signature
    std::memcpy(pcString, s_pcFrom, s_iLen);
    pcString[s_iLen] = '\0';

    return pcString;
}


// ****************************************************************
/* get compile-time type identifier (not deterministic) */
template <typename T> constexpr coreUint32 coreData::TypeId()
{
#if defined(_CORE_MSVC_)

    return FORCE_COMPILE_TIME(coreHashXXH32(__FUNCDNAME__));

#elif defined(_CORE_GCC_) || defined(_CORE_CLANG_)

    return FORCE_COMPILE_TIME(coreHashXXH32(__PRETTY_FUNCTION__));

#endif
}


// ****************************************************************
/* process string with custom sub-function */
template <typename F> const coreChar* coreData::StrProcess(const coreChar* pcInput, F&& nFunction)
{
    ASSERT(pcInput)

    coreChar* pcString = coreData::__NextTempString();
    coreChar* pcCursor = pcString;

    // define max string position
    const coreChar* pcEnd = pcInput + CORE_DATA_STRING_LEN - 1u;
    ASSERT(std::strlen(pcInput) < CORE_DATA_STRING_LEN)

    // process all characters individually
    for(; ((*pcInput) != '\0') && (pcInput != pcEnd); ++pcCursor, ++pcInput)
        (*pcCursor) = nFunction(*pcInput);
    (*pcCursor) = '\0';

    return pcString;
}


// ****************************************************************
/* call function for each valid string token */
template <typename F> void coreData::StrForEachToken(const coreChar* pcInput, const coreChar* pcDelimiter, F&& nFunction)
{
    ASSERT(pcInput && pcDelimiter)

    coreChar  acString[1024];
    coreChar* pcContext = NULL;

    // make local copy
    coreData::StrCopy(acString, ARRAY_SIZE(acString), pcInput);

    // tokenize string and forward to function
    const coreChar* pcToken = strtok_r(acString, pcDelimiter, &pcContext);
    while(pcToken != NULL)
    {
        nFunction(pcToken);
        pcToken = strtok_r(NULL, pcDelimiter, &pcContext);
    }
}


#endif /* _CORE_GUARD_DATA_H_ */