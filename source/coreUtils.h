#pragma once


// ****************************************************************
// string definition
#define CORE_UTILS_STRINGS 16


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

    // check if file exists
    static bool FileExists(const char* pcPath);

    // get file data and size (data is allocated into ppDataOut)
    static void FileDataSize(FILE* pFile, coreByte** ppDataOut, coreUint* piSizeOut);

    // retrieve current date and time
    static void DateTime(coreUint* piSec, coreUint* piMin, coreUint* piHou, coreUint* piDay, coreUint* piMon, coreUint* piYea);

    // create printed string
    static const char* Print(const char* pcMessage, ...);

    // open URL with webbrowser
    static void OpenURL(const char* pcURL);


private:
    // access next return-string
    static inline char* __NextString() {if(++m_iIndex >= CORE_UTILS_STRINGS) m_iIndex = 0; return m_aacString[m_iIndex];}
};