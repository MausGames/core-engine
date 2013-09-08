//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef GUARD_CORE_UTILS_H
#define GUARD_CORE_UTILS_H


// ****************************************************************
// utility definitions
#define CORE_UTILS_STRINGS 16

#if defined(_WIN32)
    #define CORE_UTILS_SLASH "\\"
#else
    #define CORE_UTILS_SLASH "/"
#endif


// ****************************************************************
// data and system utility collection
class coreUtils
{
private:
    static char m_aacString[CORE_UTILS_STRINGS][256];   //!< return-string memory
    static coreUint m_iIndex;                           //!< current return-string


public:
    //! \name get application parameters
    //! @{
    static const char* AppName();
    static const char* AppPath();
    //! @}

    //! \name retrieve current date and time
    //! @{
    static void DateTime(coreUint* piSec, coreUint* piMin, coreUint* piHou, coreUint* piDay, coreUint* piMon, coreUint* piYea);
    //! @}

    //! \name create formated string
    //! @{
    static const char* Print(const char* pcMessage, ...);
    //! @}

    //! \name operate with string data
    //! @{
    static bool StrCmp(const char* s, const char* t);
    static const char* StrRight(const char* pcInput, const coreUint& iNum);
    //! @}

    //! \name open URL with web-browser
    //! @{
    static void OpenURL(const char* pcURL);
    //! @}


private:
    //! \name access next return-string
    //! @{
    static inline char* __NextString() {if(++m_iIndex >= CORE_UTILS_STRINGS) m_iIndex = 0; return m_aacString[m_iIndex];}
    //! @}
};


#endif // GUARD_CORE_UTILS_H