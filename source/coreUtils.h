//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once


// ****************************************************************
// utility definition
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
    static char m_aacString[CORE_UTILS_STRINGS][256];   // return-string memory
    static coreUint m_iIndex;                           // current return-string


public:
    // get application parameters
    static const char* AppName();
    static const char* AppPath();

    // retrieve current date and time
    static void DateTime(coreUint* piSec, coreUint* piMin, coreUint* piHou, coreUint* piDay, coreUint* piMon, coreUint* piYea);

    // create printed string
    static const char* Print(const char* pcMessage, ...);

    // compare strings with wildcards
    static bool WildCmp(const char* s, const char* t);

    // open URL with webbrowser
    static void OpenURL(const char* pcURL);


private:
    // access next return-string
    static inline char* __NextString() {if(++m_iIndex >= CORE_UTILS_STRINGS) m_iIndex = 0; return m_aacString[m_iIndex];}
};