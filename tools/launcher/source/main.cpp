//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#if defined(_WIN32)

#pragma warning(disable : 4100)   // unreferenced formal parameter
#pragma warning(disable : 4127)   // constant conditional expression

#define _WIN32_WINNT _WIN32_WINNT_WINXP
#define WINVER       _WIN32_WINNT_WINXP
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>

using uIsWow64Process = BOOL (WINAPI *) (HANDLE, PBOOL);


// ****************************************************************
static bool ScanFolder(const wchar_t* pcPath, std::vector<std::wstring>* __restrict pasOutput)
{
    HANDLE pFolder;
    WIN32_FIND_DATAW oFile;

    // open folder
    pFolder = FindFirstFileW(pcPath, &oFile);
    if(pFolder == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    do
    {
        // check and add file path
        if(oFile.cFileName[0] != '.')
        {
            pasOutput->push_back(oFile.cFileName);
        }
    }
    while(FindNextFileW(pFolder, &oFile));

    // close folder
    FindClose(pFolder);
    return true;
}


// ****************************************************************
static bool IsWow64()
{
    int iStatus = 0;

    // check for pointer-size (compile-time)
    if(sizeof(void*) == 8u)
    {
        return true;
    }

    // get function pointer from kernel library
    const uIsWow64Process nIsWow64Process = reinterpret_cast<uIsWow64Process>(GetProcAddress(GetModuleHandleW(L"kernel32"), "IsWow64Process"));
    if(nIsWow64Process)
    {
        // check for 64-bit operating system
        if(!nIsWow64Process(GetCurrentProcess(), &iStatus))
        {
            iStatus = 0;
        }
    }

    return iStatus ? true : false;
}


// ****************************************************************
static bool IsWindowsVistaOrHigher()
{
    OSVERSIONINFOW iOS = {};
    iOS.dwOSVersionInfoSize = sizeof(iOS);

    // get operating system version
    GetVersionExW(&iOS);

    // check for Windows Vista or higher
    return (iOS.dwMajorVersion >= 6u);
}


// ****************************************************************
extern int WINAPI wWinMain(_In_ HINSTANCE pInstance, _In_opt_ HINSTANCE pPrevInstance, _In_ LPWSTR pcCmdLine, _In_ int iCmdShow)
{
    // set working directory
    const wchar_t* pcDirectory = (IsWow64() && IsWindowsVistaOrHigher()) ? L"bin\\windows\\x64\\" : L"bin\\windows\\x86\\";
    if(!SetCurrentDirectoryW(pcDirectory))
    {
        MessageBoxW(NULL, L"Could not set working directory!", NULL, MB_OK | MB_ICONERROR);
        return -2;
    }

    // find executable name
    std::vector<std::wstring> asFile;
    if(!ScanFolder(L"*_msvc.exe", &asFile) || asFile.empty())
    {
        MessageBoxW(NULL, L"Could not find executable!", NULL, MB_OK | MB_ICONERROR);
        return -1;
    }

    // start real application
    return (int)ShellExecuteW(NULL, L"open", asFile[0].c_str(), pcCmdLine, NULL, SW_SHOWNORMAL);
}


#endif /* _WIN32 */