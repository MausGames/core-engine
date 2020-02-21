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

// TODO: constexpr strlen -> ARRAY_SIZE(x)-1
// TODO: implement constexpr strright
// TODO: reuse context on compression and decompression (ZSTD_createCCtx & co), but needs to be thread-safe ?


// ****************************************************************
/* data definitions */
#define CORE_DATA_STRING_NUM (32u)    //!< number of temp-strings
#define CORE_DATA_STRING_LEN (256u)   //!< length of each temp-string

#if defined(_CORE_WINDOWS_)
    #define CORE_DATA_SLASH "\\"      //!< default path-delimiter of the operating system (as string)
#else
    #define CORE_DATA_SLASH "/"
#endif

#if defined(_CORE_DEBUG_)
    #define PRINT(...)   ([&]() {if(false) std::printf(__VA_ARGS__); return coreData::Print(__VA_ARGS__);}())   //!< enable format-specifier checking
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
    /*! temp-string structure */
    struct coreTempString final
    {
        coreChar* pcPointer = coreMath::CeilAlignPtr(acData, ALIGNMENT_CACHE);             //!< manually aligned pointer (auto-aligning thread-local data is not supported by the Windows 7 PE-loader)
        coreChar  acData[CORE_DATA_STRING_NUM * CORE_DATA_STRING_LEN + ALIGNMENT_CACHE];   //!< temp-string buffer
    };


private:
    static thread_local coreTempString s_TempString;   //!< manually aligned temp-string buffer
    static thread_local coreUintW      s_iCurString;   //!< current temp-string


public:
    DISABLE_CONSTRUCTION(coreData)

    /*! create formatted string */
    //! @{
    template <typename... A> static RETURN_RESTRICT const coreChar* Print(const coreChar* pcFormat, A&&... vArgs);
    static constexpr                RETURN_RESTRICT const coreChar* Print(const coreChar* pcFormat) {return pcFormat;}
    //! @}

    /*! get application properties */
    //! @{
    static              coreUint64 AppMemory();
    static        const coreChar*  AppPath  ();
    static inline const coreChar*  AppName  () {const coreChar* pcString = coreData::AppPath(); const coreChar* pcSlash = std::strrchr(pcString, CORE_DATA_SLASH[0]); return pcSlash ? pcSlash + 1u : pcString;}
    static inline const coreChar*  AppDir   () {const coreChar* pcString = coreData::AppPath(); const coreChar* pcSlash = std::strrchr(pcString, CORE_DATA_SLASH[0]); if(pcSlash) (*c_cast<coreChar*>(pcSlash + 1u)) = '\0'; return pcString;}
    //! @}

    /* get operating system properties */
    //! @{
    static const coreChar* SystemName();
    static const coreChar* SystemUserName();
    //! @}

    /*! control current working directory */
    //! @{
    static       coreStatus SetCurDir(const coreChar* pcPath);
    static const coreChar*  GetCurDir();
    //! @}

    /*! open URL with default web-browser */
    //! @{
    static coreStatus OpenURL(const coreChar* pcURL);
    //! @}

    /*! handle physical files and folders */
    //! @{
    static coreBool    FileExists   (const coreChar* pcPath);
    static coreInt64   FileSize     (const coreChar* pcPath);
    static std::time_t FileWriteTime(const coreChar* pcPath);
    static coreStatus  FileCopy     (const coreChar* pcFrom, const coreChar* pcTo);
    static coreStatus  FileMove     (const coreChar* pcFrom, const coreChar* pcTo);
    static coreStatus  FileDelete   (const coreChar* pcPath);
    static coreStatus  ScanFolder   (const coreChar* pcPath, const coreChar* pcFilter, std::vector<std::string>* OUTPUT pasOutput);
    static void        CreateFolder (const coreChar* pcPath);
    //! @}

    /*! retrieve date and time */
    //! @{
    static void            DateTimeValue(coreUint16* OUTPUT piYea, coreUint16* OUTPUT piMon, coreUint16* OUTPUT piDay, coreUint16* OUTPUT piHou, coreUint16* OUTPUT piMin, coreUint16* OUTPUT piSec, const std::tm* pTimeMap = TIMEMAP_CURRENT);
    static const coreChar* DateTimePrint(const coreChar* pcFormat, const std::tm* pTimeMap = TIMEMAP_CURRENT);
    static inline const coreChar* DateString(const std::tm* pTimeMap = TIMEMAP_CURRENT) {return coreData::DateTimePrint("%Y-%m-%d", pTimeMap);}
    static inline const coreChar* TimeString(const std::tm* pTimeMap = TIMEMAP_CURRENT) {return coreData::DateTimePrint("%H:%M:%S", pTimeMap);}
    //! @}

    /*! compress and decompress data */
    //! @{
    static coreStatus Compress  (const coreByte* pInput, const coreUint32 iInputSize, coreByte** OUTPUT ppOutput, coreUint32* OUTPUT piOutputSize, const coreInt32 iLevel = ZSTD_CLEVEL_DEFAULT);
    static coreStatus Decompress(const coreByte* pInput, const coreUint32 iInputSize, coreByte** OUTPUT ppOutput, coreUint32* OUTPUT piOutputSize);
    static void       Scramble  (coreByte* OUTPUT pData, const coreUintW iSize, const coreUint32 iKey = 0u);
    static void       Unscramble(coreByte* OUTPUT pData, const coreUintW iSize, const coreUint32 iKey = 0u);
    //! @}

    /*! get compile-time type information */
    //! @{
    template <typename T> static const     coreChar*  TypeName();
    template <typename T> static constexpr coreUint32 TypeId();
    //! @}

    /*! operate with string data */
    //! @{
    template <typename F> static const coreChar* StrProcess     (const coreChar* pcInput,                              F&& nFunction);   //!< [](const coreChar  cChar)   -> coreChar
    template <typename F> static void            StrForEachToken(const coreChar* pcInput, const coreChar* pcDelimiter, F&& nFunction);   //!< [](const coreChar* pcToken) -> void
    static constexpr    coreUintW StrLenConst (const coreChar* s)                    {return *s ? 1u + StrLenConst(s+1u) : 0u;}
    static constexpr    coreBool  StrCmpConst (const coreChar* s, const coreChar* t) {return *s ? (*s == *t) && StrCmpConst(s+1u, t+1u) : !*t;}
    static inline       coreBool  StrCmpLike  (const coreChar* s, const coreChar* t) {return (*t == '*') ? StrCmpLike(s, t+1u) || (*s && StrCmpLike(s+1u, t)) : *s ? ((*t == '?') || (tolower(*s) == tolower(*t))) && StrCmpLike(s+1u, t+1u) : !*t;}
    static inline const coreChar* StrUpper    (const coreChar* pcInput)              {return coreData::StrProcess(pcInput, toupper);}
    static inline const coreChar* StrLower    (const coreChar* pcInput)              {return coreData::StrProcess(pcInput, tolower);}
    static const coreChar*        StrRight    (const coreChar* pcInput, const coreUintW iNum);
    static const coreChar*        StrFilename (const coreChar* pcInput);
    static const coreChar*        StrExtension(const coreChar* pcInput);
    static coreFloat              StrVersion  (const coreChar* pcInput);
    static void                   StrCopy     (const coreChar* pcInput, coreChar* OUTPUT pcOutput, const coreUintW iMaxSize);
    static void                   StrTrim     (std::string* OUTPUT psInput);
    static void                   StrReplace  (std::string* OUTPUT psInput, const coreChar* pcOld, const coreChar* pcNew);
    //! @}

    /*! operate with containers */
    //! @{
    template <typename T> static inline void Shuffle(const T& tBegin, const T& tEnd, const coreUint32 iSeed = std::time(NULL)) {std::shuffle(tBegin, tEnd, std::minstd_rand(iSeed));}
    template <typename T> static typename std::vector<T>::iterator SwapErase(const typename std::vector<T>::iterator& oEntry, std::vector<T>* OUTPUT patContainer);
    //! @}


private:
    /*! access next temp-string */
    //! @{
    static inline RETURN_RESTRICT coreChar* __NextTempString() {if(++s_iCurString >= CORE_DATA_STRING_NUM) s_iCurString = 0u; return &s_TempString.pcPointer[s_iCurString * CORE_DATA_STRING_LEN];}
    //! @}
};


// ****************************************************************
/* create formatted string */
template <typename... A> RETURN_RESTRICT const coreChar* coreData::Print(const coreChar* pcFormat, A&&... vArgs)
{
    coreChar* pcString = coreData::__NextTempString();

    // read arguments and assemble string
    const coreInt32 iReturn = std::snprintf(pcString, CORE_DATA_STRING_LEN, pcFormat, std::forward<A>(vArgs)...);
    ASSERT((iReturn > -1) && (iReturn < coreInt32(CORE_DATA_STRING_LEN)))

    return pcString;
}


// ****************************************************************
/* get compile-time type name (with run-time extraction) */
template <typename T> const coreChar* coreData::TypeName()
{
    coreChar* pcString = coreData::__NextTempString();

#if defined(_CORE_MSVC_)

    // analyze function signature (const char* __cdecl coreData::TypeName<int>(void))
    static const coreChar* pcBase = __FUNCSIG__;
    static const coreChar* pcFrom = std::strchr (pcBase, '<') + 1u; if(pcFrom - 1u == NULL) return "";
    static const coreChar* pcTo   = std::strrchr(pcFrom, '>');      if(pcTo        == NULL) return "";

#elif defined(_CORE_GCC_) || defined(_CORE_CLANG_)

    // analyze function signature (const char* coreData::TypeName() [with T = int])
    static const coreChar* pcBase = __PRETTY_FUNCTION__;
    static const coreChar* pcFrom = std::strchr (pcBase, '=') + 2u; if(pcFrom - 2u == NULL) return "";
    static const coreChar* pcTo   = std::strrchr(pcFrom, ']');      if(pcTo        == NULL) return "";

#endif

    // calculate name length
    static const coreUintW iLen = MIN(P_TO_UI(pcTo - pcFrom), CORE_DATA_STRING_LEN - 1u);

    // extract name from the function signature
    std::memcpy(pcString, pcFrom, iLen);
    pcString[iLen] = '\0';

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
    coreData::StrCopy(pcInput, pcString, CORE_DATA_STRING_LEN);

    // tokenize string and forward to function
    const coreChar* pcToken = std::strtok(pcString, pcDelimiter);
    while(pcToken != NULL)
    {
        nFunction(pcToken);
        pcToken = std::strtok(NULL, pcDelimiter);
    }
}


// ****************************************************************
/* delete vector entry without compaction */
template <typename T> typename std::vector<T>::iterator coreData::SwapErase(const typename std::vector<T>::iterator& oEntry, std::vector<T>* OUTPUT patContainer)
{
    ASSERT(!patContainer->empty())

    // remember current index
    const coreUintW iIndex = oEntry - patContainer->begin();

    // swap and delete target entry (but do not preserve ordering)
    std::swap(oEntry, patContainer->end() - 1u);
    patContainer->pop_back();

    return patContainer->begin() + iIndex;
}


#endif /* _CORE_GUARD_DATA_H_ */