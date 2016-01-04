//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_DATA_H_
#define _CORE_GUARD_DATA_H_


// ****************************************************************
/* data definitions */
#define CORE_DATA_STRING_NUM (32u)    //!< number of return-strings
#define CORE_DATA_STRING_LEN (256u)   //!< length of each return-string

#if defined(_CORE_WINDOWS_)
    #define CORE_DATA_SLASH "\\"      //!< default path-delimiter of the operating system (as string)
#else
    #define CORE_DATA_SLASH "/"
#endif

#define PRINT coreData::Print


// ****************************************************************
/* data utility collection */
class INTERFACE coreData final
{
private:
    static thread_local coreChar  m_aacString[CORE_DATA_STRING_NUM][CORE_DATA_STRING_LEN];   //!< pre-allocated return-strings
    static thread_local coreUintW m_iCurString;                                              //!< current return-string


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
    static coreBool   FileExists  (const coreChar* pcPath);
    static coreInt64  FileSize    (const coreChar* pcPath);
    static coreStatus ScanFolder  (const coreChar* pcPath, const coreChar* pcFilter, std::vector<std::string>* OUTPUT pasOutput);
    static void       CreateFolder(const std::string& sPath);
    //! @}

    /*! retrieve current date and time */
    //! @{
    static void            DateTimeValue(coreUint32* OUTPUT piYea, coreUint32* OUTPUT piMon, coreUint32* OUTPUT piDay, coreUint32* OUTPUT piHou, coreUint32* OUTPUT piMin, coreUint32* OUTPUT piSec);
    static const coreChar* DateTimePrint(const coreChar* pcFormat);
    static inline const coreChar* DateString() {return coreData::DateTimePrint("%Y-%m-%d");}
    static inline const coreChar* TimeString() {return coreData::DateTimePrint("%H:%M:%S");}
    //! @}

    /*! operate with string data */
    //! @{
    template <typename F> static const coreChar* StrProcess(const coreChar* pcInput, F&& nFunction);   //!< [](const coreChar cChar) -> coreChar
    static constexpr    coreUintW   StrLenConst (const coreChar* s)                    {return *s ? 1u + StrLenConst(s+1u) : 0u;}
    static constexpr    coreBool    StrCmpConst (const coreChar* s, const coreChar* t) {return *s ? (*s == *t) && StrCmpConst(s+1u, t+1u) : !*t;}
    static inline       coreBool    StrCmpLike  (const coreChar* s, const coreChar* t) {return (*t == '*') ? StrCmpLike(s, t+1u) || (*s && StrCmpLike(s+1u, t)) : *s ? ((*t == '?') || (toupper(*s) == toupper(*t))) && StrCmpLike(s+1u, t+1u) : !*t;}
    static inline const coreChar*   StrUpper    (const coreChar* pcInput)              {return coreData::StrProcess(pcInput, toupper);}
    static inline const coreChar*   StrLower    (const coreChar* pcInput)              {return coreData::StrProcess(pcInput, tolower);}
    static const coreChar*          StrRight    (const coreChar* pcInput, const coreUintW iNum);
    static const coreChar*          StrExtension(const coreChar* pcInput);
    static coreFloat                StrVersion  (const coreChar* pcInput);
    static void                     StrTrim     (std::string* OUTPUT psInput);
    static void                     StrReplace  (std::string* OUTPUT psInput, const coreChar* pcOld, const coreChar* pcNew);
    //! @}


private:
    /*! access next return-string */
    //! @{
    static inline RETURN_RESTRICT coreChar* __NextString() {if(++m_iCurString >= CORE_DATA_STRING_NUM) m_iCurString = 0u; return m_aacString[m_iCurString];}
    //! @}
};


// ****************************************************************
/* create formatted string */
template <typename... A> RETURN_RESTRICT const coreChar* coreData::Print(const coreChar* pcFormat, A&&... vArgs)
{
    coreChar* pcString = coreData::__NextString();

#if defined(_CORE_WINDOWS_)

    // assemble string without guaranteed null-termination
    const coreInt32 iReturn = _snprintf(pcString, CORE_DATA_STRING_LEN - 1u, pcFormat, std::forward<A>(vArgs)...);
    pcString[CORE_DATA_STRING_LEN - 1u] = '\0';

#else

    // assemble string
    const coreInt32 iReturn = snprintf(pcString, CORE_DATA_STRING_LEN, pcFormat, std::forward<A>(vArgs)...);

#endif

    ASSERT(-1 < iReturn && iReturn < coreInt32(CORE_DATA_STRING_LEN))
    return pcString;
}


// ****************************************************************
/* process string with custom sub-function */
template <typename F> const coreChar* coreData::StrProcess(const coreChar* pcInput, F&& nFunction)
{
    coreChar* pcString = coreData::__NextString();
    coreChar* pcCursor = pcString;

    // define max string position
    const coreChar* pcEnd = pcInput + CORE_DATA_STRING_LEN - 1u;
    ASSERT(std::strlen(pcInput) < CORE_DATA_STRING_LEN)

    // process all characters individually
    for(; (*pcInput != '\0') && (pcInput != pcEnd); ++pcCursor, ++pcInput)
        *pcCursor = nFunction(*pcInput);
    *pcCursor = '\0';

    return pcString;
}


#endif /* _CORE_GUARD_DATA_H_ */