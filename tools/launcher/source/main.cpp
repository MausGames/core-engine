//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#define _HAS_EXCEPTIONS (0)
#define WIN32_LEAN_AND_MEAN

#pragma warning(disable : 4100)

#include <windows.h>
#include <shellapi.h>

#include <string>
#include <vector>

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);


// ****************************************************************
int ScanFolder(const char* pcPath, std::vector<std::string>* pasOutput)
{
    HANDLE pFolder;
    WIN32_FIND_DATA oFile;

    // open folder
    pFolder = FindFirstFile(pcPath, &oFile);
    if(pFolder == INVALID_HANDLE_VALUE) return 1;

    do
    {
        // check and add file path
        if(oFile.cFileName[0] != '.')
            pasOutput->push_back(oFile.cFileName);
    }
    while(FindNextFile(pFolder, &oFile));

    // close folder
    FindClose(pFolder);
    return 0;
}


// ****************************************************************
bool IsWindowsVistaOrHigher()
{
    OSVERSIONINFO iOS;
    std::memset(&iOS, 0, sizeof(iOS));
    iOS.dwOSVersionInfoSize = sizeof(iOS);

    // get operating system version
    GetVersionEx(&iOS);

    // check for Windows Vista or higher
    return (iOS.dwMajorVersion >= 6) ? true : false;
}


// ****************************************************************
bool IsWow64()
{
    int iStatus = 0;

    // get function pointer from kernel library
    LPFN_ISWOW64PROCESS nIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle("kernel32"), "IsWow64Process");
    if(nIsWow64Process)
    {
        // check for 64-bit operating system
        if(!nIsWow64Process(GetCurrentProcess(), &iStatus))
            iStatus = 0;
    }

    return iStatus ? true : false;
}


// ****************************************************************
int WINAPI WinMain(HINSTANCE pInstance, HINSTANCE pPrevInstance, LPSTR pcCmdLine, int iCmdShow)
{
    // change working directory
    if(!SetCurrentDirectory((IsWow64() && IsWindowsVistaOrHigher()) ? "bin\\windows\\x64\\" : "bin\\windows\\x86\\")) return -2;

    // get executable name
    std::vector<std::string> asFile;
    ScanFolder("*_msvc.exe", &asFile);

    // start real application
    return asFile.empty() ? -1 : (int)ShellExecute(NULL, "open", asFile[0].c_str(), NULL, NULL, SW_SHOWNORMAL);
}