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

// TODO: convert return-strings into one single big memory-buffer ?
// TODO: check if FindFirstFile really needs full path


// ****************************************************************
/* data definitions */
#define CORE_DATA_STRING_NUM (32u)    //!< number of return-strings
#define CORE_DATA_STRING_LEN (256u)   //!< length of each return-string

#if defined(_CORE_WINDOWS_)
    #define CORE_DATA_SLASH "\\"      //!< default path-delimiter of the operating system
#else
    #define CORE_DATA_SLASH "/"
#endif

#define PRINT(x,...) coreData::Print(x, ##__VA_ARGS__)


// ****************************************************************
/* data utility collection */
class coreData final
{
private:
    static thread_local char     m_aacString[CORE_DATA_STRING_NUM][CORE_DATA_STRING_LEN];   //!< pre-allocated return-strings
    static thread_local coreUint m_iCurString;                                              //!< current return-string


public:
    /*! create formatted string */
    //! @{
    template <typename... A> static const char* Print(const char* pcFormat, A&&... vArgs);
    static inline                   const char* Print(const char* pcFormat) {return pcFormat;}
    //! @}

    /*! get application properties */
    //! @{
    static        const char* AppPath();
    static inline const char* AppName() {const char* pcString = coreData::AppPath(); const char* pcSlash = std::strrchr(pcString, CORE_DATA_SLASH[0]); return pcSlash ? pcSlash+1 : pcString;}
    static inline const char* AppDir () {const char* pcString = coreData::AppPath(); const char* pcSlash = std::strrchr(pcString, CORE_DATA_SLASH[0]); if(pcSlash) (*c_cast<char*>(pcSlash+1)) = '\0'; return pcString;}
    //! @}

    /*! control current working directory */
    //! @{
    static coreError   SetCurDir(const char* pcPath);
    static const char* GetCurDir();
    //! @}

    /*! open URL with default web-browser */
    //! @{
    static coreError OpenURL(const char* pcURL);
    //! @}

    /*! handle physical files and folders */
    //! @{
    static bool      FileExists  (const char* pcPath);
    static coreError ScanFolder  (const char* pcPath, const char* pcFilter, std::vector<std::string>* pasOutput);
    static void      CreateFolder(const std::string& sPath);
    //! @}

    /*! retrieve current date and time */
    //! @{
    static void        DateTimeValue(coreUint* piYea, coreUint* piMon, coreUint* piDay, coreUint* piHou, coreUint* piMin, coreUint* piSec);
    static const char* DateTimePrint(const char* pcFormat);
    static inline const char* DateString() {return coreData::DateTimePrint("%Y-%m-%d");}
    static inline const char* TimeString() {return coreData::DateTimePrint("%H:%M:%S");}
    //! @}

    /*! operate with string data */
    //! @{
    template <typename F> static const char* StrProcess(const char* pcInput, F&& pFunction);
    static constexpr_func bool StrCmpConst (const char*  s, const char* t) {return *s ? (*s == *t) && StrCmpConst(s+1, t+1) : !*t;}
    static inline         bool StrCmpLike  (const char*  s, const char* t) {return (*t == '*') ? StrCmpLike(s, t+1) || (*s && StrCmpLike(s+1, t)) : *s ? ((*t == '?') || (toupper(*s) == toupper(*t))) && StrCmpLike(s+1, t+1) : !*t;}
    static inline const char*  StrUpper    (const char*  pcInput)          {return coreData::StrProcess(pcInput, toupper);}
    static inline const char*  StrLower    (const char*  pcInput)          {return coreData::StrProcess(pcInput, tolower);}
    static const char*         StrRight    (const char*  pcInput, const coreUint& iNum);
    static const char*         StrExtension(const char*  pcInput);
    static float               StrVersion  (const char*  pcInput);
    static void                StrTrim     (std::string* psInput);
    //! @}

private:
    DISABLE_TORS(coreData)

    /*! access next return-string */
    //! @{
    static inline char* __NextString() {if(++m_iCurString >= CORE_DATA_STRING_NUM) m_iCurString = 0; return m_aacString[m_iCurString];}
    //! @}
};


// ****************************************************************
/* create formatted string */
template <typename... A> const char* coreData::Print(const char* pcFormat, A&&... vArgs)
{
    char* pcString = coreData::__NextString();

#if defined(_CORE_WINDOWS_)

    // assemble string without guaranteed null-termination
    const int iReturn = _snprintf(pcString, CORE_DATA_STRING_LEN - 1, pcFormat, std::forward<A>(vArgs)...);
    pcString[CORE_DATA_STRING_LEN - 1] = '\0';

#else

    // assemble string
    const int iReturn = snprintf(pcString, CORE_DATA_STRING_LEN, pcFormat, std::forward<A>(vArgs)...);

#endif

    ASSERT(-1 < iReturn && iReturn < CORE_DATA_STRING_LEN)
    return pcString;
}


// ****************************************************************
/* process string with custom sub-function */
template <typename F> const char* coreData::StrProcess(const char* pcInput, F&& pFunction)
{
    char* pcString = coreData::__NextString();
    char* pcCursor = pcString;

    // define max string position
    const char* pcEnd = pcInput + CORE_DATA_STRING_LEN - 1;
    ASSERT(std::strlen(pcInput) < CORE_DATA_STRING_LEN)

    // process all characters individually
    for(; (*pcInput != '\0') && (pcInput != pcEnd); ++pcCursor, ++pcInput)
        *pcCursor = pFunction(*pcInput);
    *pcCursor = '\0';

    return pcString;
}


#endif /* _CORE_GUARD_DATA_H_ */