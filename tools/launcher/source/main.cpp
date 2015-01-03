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

#include <Windows.h>
#include <shellapi.h>

#include <string>
#include <vector>

typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);


// ****************************************************************
int ScanFolder(const char* pcPath, std::vector<std::string>* pasOutput)
{
    HANDLE hFolder;
    WIN32_FIND_DATA hFile;

    // open folder
    hFolder = FindFirstFile(pcPath, &hFile);
    if(hFolder == INVALID_HANDLE_VALUE) return 1;

    do
    {
        // check and add file path
        if(hFile.cFileName[0] != '.')
            pasOutput->push_back(hFile.cFileName);
    }
    while(FindNextFile(hFolder, &hFile));

    // close folder
    FindClose(hFolder);
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
    return iOS.dwMajorVersion >= 6;
}


// ****************************************************************
bool IsWow64()
{
    int iStatus = 0;

    if(IsWindowsVistaOrHigher())
    {
        // get function pointer from kernel library
        LPFN_ISWOW64PROCESS nIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle("kernel32"), "IsWow64Process");
        if(nIsWow64Process)
        {
            // check for 64-bit operating system
            if(!nIsWow64Process(GetCurrentProcess(), &iStatus))
                iStatus = 0;
        }
    }

    return iStatus ? true : false;
}


// ****************************************************************
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int iCmdShow)
{
    // change working directory
    if(!SetCurrentDirectory(IsWow64() ? "bin\\windows\\x64\\" : "bin\\windows\\x86\\")) return -2;

    // get executable name
    std::vector<std::string> asFile;
    ScanFolder(IsWindowsVistaOrHigher() ? "*_msvc.exe" : "*_mingw.exe", &asFile);

    // start real application
    return asFile.empty() ? -1 : (int)ShellExecute(NULL, "open", asFile[0].c_str(), NULL, NULL, SW_SHOWNORMAL);
}