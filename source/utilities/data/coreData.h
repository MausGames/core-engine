//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_DATA_H_
#define _CORE_GUARD_DATA_H_


// ****************************************************************
// data definitions
#define CORE_DATA_STRINGS 16   //!< number of used return-strings

#if defined(_CORE_WINDOWS_)
    #define CORE_DATA_SLASH "\\"
#else
    #define CORE_DATA_SLASH "/"
#endif

#define CORE_DATA_SCAN(s,f,...) {int n = 0; std::sscanf(s, f " %n", ##__VA_ARGS__, &n); coreData::StrSkip(&(s), n);}


// ****************************************************************
// data utility collection
class coreData
{
private:
    static char m_aacString[CORE_DATA_STRINGS][256];   //!< return-string memory
    static coreUint m_iIndex;                          //!< current return-string


public:
    //! create formated string
    //! @{
    static const char* Print(const char* pcMessage, ...);
    //! @}

    //! get application parameters
    //! @{
    static const char* AppName();
    static const char* AppPath();
    //! @}

    //! handle physical files and folders
    //! @{
    static bool FileExists(const char* pcPath);
    static coreError FolderSearch(const char* pcPath, const char* pcFilter, std::vector<std::string>* pasOutput);
    //! @}

    //! retrieve current date and time
    //! @{
    static void DateTime(coreUint* piSec, coreUint* piMin, coreUint* piHou, coreUint* piDay, coreUint* piMon, coreUint* piYea);
    //! @}

    //! operate with string data
    //! @{
    static bool StrCompare(const char* s, const char* t);
    static const char* StrRight(const char* pcInput, const coreUint& iNum);
    static const char* StrExtension(const char* pcInput);
    static float StrVersion(const char* pcInput);
    static void StrSkip(const char** ppcInput, const int &iNum);
    //! @}

    //! open URL with standard web-browser
    //! @{
    static void OpenURL(const char* pcURL);
    //! @}


private:
    //! access next return-string
    //! @{
    static inline char* __NextString() {if(++m_iIndex >= CORE_DATA_STRINGS) m_iIndex = 0; return m_aacString[m_iIndex];}
    //! @}
};


#endif // _CORE_GUARD_DATA_H_