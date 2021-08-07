///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_DATA_H_
#define _CORE_GUARD_DATA_H_

// TODO 3: constexpr strlen (should work without knowing the length ([N], ARRAY_SIZE(x)-1))
// TODO 3: implement constexpr strright
// TODO 3: reuse context on compression and decompression (ZSTD_createCCtx & co), but needs to be thread-safe ?
// TODO 3: --help, --version, --force-x86, --force-x64 (windows, launcher?)
// TODO 3: add ToChars float precision parameter (+ search for 'PRINT("%f' and 'PRINT("%.')
// TODO 3: make !temp and !appdata replace instead


// ****************************************************************
/* data definitions */
#define CORE_DATA_STRING_NUM (32u)    // number of temp-strings
#define CORE_DATA_STRING_LEN (512u)   // length of each temp-string

#if defined(_CORE_WINDOWS_)
    #define CORE_DATA_SLASH "\\"      // default path-delimiter of the operating system (as string)
#else
    #define CORE_DATA_SLASH "/"
#endif

#if defined(_CORE_DEBUG_)
    #define PRINT(...)   ([&]() {if(false) std::printf(__VA_ARGS__); return coreData::Print(__VA_ARGS__);}())   // enable format-specifier checking
#else
    #define PRINT(...)   (coreData::Print(__VA_ARGS__))
#endif

#define TIMEMAP_LOCAL(t) ([](const std::time_t iValue) {return std::localtime(&iValue);}(t))
#define TIMEMAP_GM(t)    ([](const std::time_t iValue) {return std::gmtime   (&iValue);}(t))
#define TIMEMAP_CURRENT  (TIMEMAP_LOCAL(std::time(NULL)))


// ****************************************************************
/* data utility collection */
class INTERFACE coreData final
{
private:
    /* temp-string structure */
    struct coreTempString final
    {
        coreChar* pcPointer = coreMath::CeilAlignPtr(acData, ALIGNMENT_CACHE);             // manually aligned pointer (auto-aligning thread-local data is not supported by the Windows 7 PE-loader)
        coreChar  acData[CORE_DATA_STRING_NUM * CORE_DATA_STRING_LEN + ALIGNMENT_CACHE];   // temp-string buffer
    };


private:
    static thread_local coreTempString s_TempString;       // manually aligned temp-string buffer
    static thread_local coreUintW      s_iCurString;       // current temp-string

    static coreMapStr<const coreChar*> s_apcCommandLine;   // parsed command line arguments
    static coreString                  s_sUserFolder;      // selected user folder


public:
    DISABLE_CONSTRUCTION(coreData)

    /* create formatted string */
    static coreInt32 PrintBase(coreChar* OUTPUT pcOutput, const coreInt32 iMaxLen, const coreChar* pcFormat, ...);
    template <typename... A> static RETURN_RESTRICT const coreChar* Print(const coreChar* pcFormat, A&&... vArgs);
    static constexpr                RETURN_RESTRICT const coreChar* Print(const coreChar* pcFormat) {return pcFormat;}

    /* convert between trivial value and string */
    template <typename T> static const coreChar* ToChars  (const T& tValue);
    template <typename T> static T               FromChars(const coreChar* pcString, const coreUintW iLen);

    /* get application properties */
    static              coreUint64 AppMemory();
    static        const coreChar*  AppPath  ();
    static inline const coreChar*  AppName  () {return coreData::StrFilename (coreData::AppPath());}
    static inline const coreChar*  AppDir   () {return coreData::StrDirectory(coreData::AppPath());}

    /* get operating system properties */
    static       coreBool   SystemMemory     (coreUint64* OUTPUT piAvailable, coreUint64* OUTPUT piTotal);
    static       coreBool   SystemSpace      (coreUint64* OUTPUT piAvailable, coreUint64* OUTPUT piTotal);
    static const coreChar*  SystemName       ();
    static const coreChar*  SystemUserName   ();
    static const coreChar*  SystemDirAppData ();
    static const coreChar*  SystemDirTemp    ();

    /* control current working directory */
    static       coreStatus SetCurDir(const coreChar* pcPath);
    static const coreChar*  GetCurDir();

    /* control command line arguments */
    static        void            SetCommandLine(const coreInt32 iArgc, coreChar** ppcArgv);
    static inline const coreChar* GetCommandLine(const coreHashString& sArgument) {return s_apcCommandLine.count(sArgument) ? s_apcCommandLine.at(sArgument) : NULL;}

    /* control user folder */
    static        void            InitUserFolder();
    static inline const coreChar* UserFolder(const coreChar* pcPath) {ASSERT(pcPath) return PRINT("%s%s", s_sUserFolder.c_str(), pcPath);}

    /* open URL with default web-browser */
    static coreStatus OpenURL(const coreChar* pcURL);

    /* handle dynamic libraries */
    static void*      OpenLibrary (const coreChar* pcName);
    static void*      GetAddress  (void* pLibrary, const coreChar* pcName);
    static coreStatus CloseLibrary(void* pLibrary);

    /* handle physical files and folders */
    static coreBool    FileExists   (const coreChar* pcPath);
    static coreInt64   FileSize     (const coreChar* pcPath);
    static std::time_t FileWriteTime(const coreChar* pcPath);
    static coreStatus  FileCopy     (const coreChar* pcFrom, const coreChar* pcTo);
    static coreStatus  FileMove     (const coreChar* pcFrom, const coreChar* pcTo);
    static coreStatus  FileDelete   (const coreChar* pcPath);
    static coreStatus  ScanFolder   (const coreChar* pcPath, const coreChar* pcFilter, coreList<coreString>* OUTPUT pasOutput);
    static coreStatus  CreateFolder (const coreChar* pcPath);

    /* retrieve date and time */
    static void            DateTimeValue(coreUint16* OUTPUT piYea, coreUint16* OUTPUT piMon, coreUint16* OUTPUT piDay, coreUint16* OUTPUT piHou, coreUint16* OUTPUT piMin, coreUint16* OUTPUT piSec, const std::tm* pTimeMap = TIMEMAP_CURRENT);
    static const coreChar* DateTimePrint(const coreChar* pcFormat, const std::tm* pTimeMap = TIMEMAP_CURRENT);
    static inline const coreChar* DateString(const std::tm* pTimeMap = TIMEMAP_CURRENT) {return coreData::DateTimePrint("%Y-%m-%d", pTimeMap);}
    static inline const coreChar* TimeString(const std::tm* pTimeMap = TIMEMAP_CURRENT) {return coreData::DateTimePrint("%H:%M:%S", pTimeMap);}

    /* compress and decompress data */
    static coreStatus Compress  (const coreByte* pInput, const coreUint32 iInputSize, coreByte** OUTPUT ppOutput, coreUint32* OUTPUT piOutputSize, const coreInt32 iLevel = ZSTD_CLEVEL_DEFAULT);
    static coreStatus Decompress(const coreByte* pInput, const coreUint32 iInputSize, coreByte** OUTPUT ppOutput, coreUint32* OUTPUT piOutputSize);
    static void       Scramble  (coreByte* OUTPUT pData, const coreUintW iSize, const coreUint32 iKey = 0u);
    static void       Unscramble(coreByte* OUTPUT pData, const coreUintW iSize, const coreUint32 iKey = 0u);

    /* get compile-time type information */
    template <typename T> static const     coreChar*  TypeName();
    template <typename T> static constexpr coreUint32 TypeId();

    /* operate with string data */
    template <typename F> static const coreChar* StrProcess     (const coreChar* pcInput,                              F&& nFunction);   // [](const coreChar  cChar)   -> coreChar
    template <typename F> static void            StrForEachToken(const coreChar* pcInput, const coreChar* pcDelimiter, F&& nFunction);   // [](const coreChar* pcToken) -> void
    static constexpr    coreUintW StrLenConst (const coreChar* s)                    {return *s ? 1u + StrLenConst(s+1u) : 0u;}
    static constexpr    coreBool  StrCmpConst (const coreChar* s, const coreChar* t) {return *s ? (*s == *t) && StrCmpConst(s+1u, t+1u) : !*t;}
    static inline       coreBool  StrCmpLike  (const coreChar* s, const coreChar* t) {return (*t == '*') ? StrCmpLike(s, t+1u) || (*s && StrCmpLike(s+1u, t)) : *s ? ((*t == '?') || (tolower(*s) == tolower(*t))) && StrCmpLike(s+1u, t+1u) : !*t;}
    static inline const coreChar* StrUpper    (const coreChar* pcInput)              {return coreData::StrProcess(pcInput, toupper);}
    static inline const coreChar* StrLower    (const coreChar* pcInput)              {return coreData::StrProcess(pcInput, tolower);}
    static const coreChar*        StrRight    (const coreChar* pcInput, const coreUintW iNum);
    static const coreChar*        StrFilename (const coreChar* pcInput);
    static const coreChar*        StrDirectory(const coreChar* pcInput);
    static const coreChar*        StrExtension(const coreChar* pcInput);
    static coreFloat              StrVersion  (const coreChar* pcInput);
    static void                   StrCopy     (coreChar* OUTPUT pcOutput, const coreUintW iMaxLen, const coreChar* pcInput);

    /* operate with containers */
    template <typename T> static inline void Shuffle(const T& tBegin, const T& tEnd, const coreUint32 iSeed = std::time(NULL)) {std::shuffle(tBegin, tEnd, std::minstd_rand(iSeed));}


private:
    /* access next temp-string */
    static inline RETURN_RESTRICT coreChar* __NextTempString() {if(++s_iCurString >= CORE_DATA_STRING_NUM) s_iCurString = 0u; return &s_TempString.pcPointer[s_iCurString * CORE_DATA_STRING_LEN];}

    /* prepare path for system directory */
    static const coreChar* __PrepareSystemDir(const coreChar* pcPath);
};


// ****************************************************************
/* create formatted string */
template <typename... A> RETURN_RESTRICT const coreChar* coreData::Print(const coreChar* pcFormat, A&&... vArgs)
{
    coreChar* pcString = coreData::__NextTempString();

    // forward arguments and assemble string
    const coreInt32 iReturn = coreData::PrintBase(pcString, CORE_DATA_STRING_LEN, pcFormat, std::forward<A>(vArgs)...);
    ASSERT((iReturn > -1) && (iReturn < coreInt32(CORE_DATA_STRING_LEN)))

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

#if defined(_CORE_LIBCPP_)
    template <> inline const coreChar* coreData::ToChars(const coreFloat& tValue) {return PRINT("%f", tValue);}
#endif


// ****************************************************************
/* convert string to trivial value */
template <typename T> T coreData::FromChars(const coreChar* pcString, const coreUintW iLen)
{
    T tValue;

    // use high-performance conversion
    const std::from_chars_result oResult = std::from_chars(pcString, pcString + iLen, tValue);
    WARN_IF(oResult.ec != std::errc()) return T(0);

    return tValue;
}

#if defined(_CORE_LIBCPP_)
    template <> inline coreFloat coreData::FromChars(const coreChar* pcString, const coreUintW iLen) {return coreFloat(std::atof(pcString));}
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

    return FORCE_COMPILE_TIME(coreHashFNV1(__FUNCDNAME__));

#elif defined(_CORE_GCC_) || defined(_CORE_CLANG_)

    return FORCE_COMPILE_TIME(coreHashFNV1(__PRETTY_FUNCTION__));

#endif
}


// ****************************************************************
/* process string with custom sub-function */
template <typename F> const coreChar* coreData::StrProcess(const coreChar* pcInput, F&& nFunction)
{
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
    coreChar* pcString = coreData::__NextTempString();

    // make local copy
    coreData::StrCopy(pcString, CORE_DATA_STRING_LEN, pcInput);

    // tokenize string and forward to function
    const coreChar* pcToken = std::strtok(pcString, pcDelimiter);
    while(pcToken != NULL)
    {
        nFunction(pcToken);
        pcToken = std::strtok(NULL, pcDelimiter);
    }
}


#endif /* _CORE_GUARD_DATA_H_ */