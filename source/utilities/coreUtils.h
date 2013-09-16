//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_UTILS_H_
#define _CORE_GUARD_UTILS_H_


// ****************************************************************
// utility definitions
#define CORE_UTILS_STRINGS 16

#if defined(_CORE_WINDOWS_)
    #define CORE_UTILS_SLASH "\\"
#else
    #define CORE_UTILS_SLASH "/"
#endif

#define SCAN_DATA(s,f,...) {int n = 0; sscanf(s, f " %n", ##__VA_ARGS__, &n); coreUtils::StrSkip(&(s), n);}


// ****************************************************************
// data and system utility collection
class coreUtils
{
private:
    static char m_aacString[CORE_UTILS_STRINGS][256];   //!< return-string memory
    static coreUint m_iIndex;                           //!< current return-string


public:
    //! get application parameters
    //! @{
    static const char* AppName();
    static const char* AppPath();
    //! @}

    //! retrieve current date and time
    //! @{
    static void DateTime(coreUint* piSec, coreUint* piMin, coreUint* piHou, coreUint* piDay, coreUint* piMon, coreUint* piYea);
    //! @}

    //! create formated string
    //! @{
    static const char* Print(const char* pcMessage, ...);
    //! @}

    //! operate with string data
    //! @{
    static bool StrCmp(const char* s, const char* t);
    static const char* StrRight(const char* pcInput, const coreUint& iNum);
    static const char* StrExt(const char* pcInput);
    static void StrSkip(const char** ppcInput, const int &iNum);
    //! @}

    //! open URL with the web-browser
    //! @{
    static void OpenURL(const char* pcURL);
    //! @}


private:
    //! access next return-string
    //! @{
    static inline char* __NextString() {if(++m_iIndex >= CORE_UTILS_STRINGS) m_iIndex = 0; return m_aacString[m_iIndex];}
    //! @}
};


#endif // _CORE_GUARD_UTILS_H_