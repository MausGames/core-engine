///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#if defined(_WIN32)

#pragma warning(disable : 4091)   // missing variable declaration
#pragma warning(disable : 4100)   // unreferenced formal parameter
#pragma warning(disable : 4255)   // function prototype missing
#pragma warning(disable : 4668)   // preprocessor macro not defined
#pragma warning(disable : 4820)   // padding after data member

#include "additional/windows/header.h"
#include <DbgHelp.h>
#include <crtdbg.h>
#include <stdlib.h>

typedef BOOL (WINAPI *uMiniDumpWriteDump) (HANDLE, DWORD, HANDLE, MINIDUMP_TYPE, PMINIDUMP_EXCEPTION_INFORMATION, PMINIDUMP_USER_STREAM_INFORMATION, PMINIDUMP_CALLBACK_INFORMATION);

extern int __cdecl coreMain(int argc, char** argv);
extern const char* g_pcUserFolder;


// ****************************************************************
/* always use the discrete high-performance graphics device */
__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001u;
__declspec(dllexport) DWORD NvOptimusEnablement                  = 0x00000001u;


// ****************************************************************
/* handle pure virtual function calls and invalid arguments */
static void __cdecl HandlePurecall(void)
{
    DebugBreak();
}

static void __cdecl HandleInvalidParameter(const wchar_t* pcExpression, const wchar_t* pcFunction, const wchar_t* pcFile, const unsigned int iLine, const uintptr_t iReserved)
{
    DebugBreak();
}


// ****************************************************************
/* create crash dump on exception */
static long WINAPI CreateCrashDump(EXCEPTION_POINTERS* pPointers)
{
    // try to recover several times
    static int s_iRecover = 10;
    if(s_iRecover--) return EXCEPTION_CONTINUE_EXECUTION;

    // switch to user folder
    SetCurrentDirectoryA(g_pcUserFolder);

    // load debug library
    const HMODULE pLibrary = LoadLibraryA("dbghelp.dll");
    if(pLibrary)
    {
        // get function pointer from debug library
        const uMiniDumpWriteDump nMiniDumpWriteDump = (uMiniDumpWriteDump)GetProcAddress(pLibrary, "MiniDumpWriteDump");
        if(nMiniDumpWriteDump)
        {
            char* pcBuffer = NULL;
            DWORD iSize    = 0u;

            // copy log file (because it may still be open for writing)
            CopyFileA("log.html", "crash.dmp", FALSE);

            // open log file for reading
            const HANDLE pLog = CreateFileA("crash.dmp", GENERIC_READ, 0u, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
            if(pLog != INVALID_HANDLE_VALUE)
            {
                // get log file size
                iSize = GetFileSize(pLog, NULL);
                if(iSize != INVALID_FILE_SIZE)
                {
                    // read log data
                    pcBuffer = (char*)calloc(iSize + 1u, sizeof(char));
                    ReadFile(pLog, pcBuffer, iSize, &iSize, NULL);
                }

                // close log file
                CloseHandle(pLog);
            }

            // open crash dump for writing
            const HANDLE pDump = CreateFileA("crash.dmp", GENERIC_WRITE, 0u, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if(pDump != INVALID_HANDLE_VALUE)
            {
                // prepare crash data
                MINIDUMP_USER_STREAM oComment;
                oComment.Type       = CommentStreamA;
                oComment.BufferSize = iSize + 1u;
                oComment.Buffer     = pcBuffer;

                MINIDUMP_USER_STREAM_INFORMATION oUserStream;
                oUserStream.UserStreamCount = 1u;
                oUserStream.UserStreamArray = &oComment;

                MINIDUMP_EXCEPTION_INFORMATION oException;
                oException.ThreadId          = GetCurrentThreadId();
                oException.ExceptionPointers = pPointers;
                oException.ClientPointers    = TRUE;

                // write crash data
                nMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), pDump, MiniDumpNormal, &oException, pcBuffer ? &oUserStream : NULL, NULL);

                // close crash dump
                CloseHandle(pDump);
            }

            // delete log data
            free(pcBuffer);
        }

        // unload debug library
        FreeLibrary(pLibrary);
    }

    // define user-friendly error messages
    const char* const pcMessageSuccess =
        "I'm really sorry, but the game just crashed. :(\n"
        "A crash-dump was created in the user-folder, containing:\n"
        "- the current process-state\n"
        "- the exact location of the crash\n"
        "- the log-file with your hardware-configuration";

    const char* const pcMessageFail =
        "I'm really sorry, but the game just crashed. :(\n"
        "Unfortunately it was not possible to create a crash-dump.";

    // show message box and exit application
    const BOOL bDumpExists = (GetFileAttributesA("crash.dmp") != INVALID_FILE_ATTRIBUTES);
    MessageBoxA(NULL, bDumpExists ? pcMessageSuccess : pcMessageFail, NULL, MB_OK | MB_ICONERROR);

    return EXCEPTION_EXECUTE_HANDLER;
}


// ****************************************************************
/* improve hardware utilization on NUMA systems */
static void ImproveNuma(void)
{
    // get highest NUMA node number
    ULONG iNumaNode;
    if(GetNumaHighestNodeNumber(&iNumaNode) && iNumaNode)
    {
        // get all processors of the NUMA node
        ULONGLONG iProcessMask;
        if(GetNumaNodeProcessorMask((UCHAR)iNumaNode, &iProcessMask) && iProcessMask)
        {
            // change processor affinity mask
            SetProcessAffinityMask(GetCurrentProcess(), (DWORD_PTR)iProcessMask);
        }
    }
}


// ****************************************************************
/* start up the application */
extern int WINAPI WinMain(_In_ HINSTANCE pInstance, _In_opt_ HINSTANCE pPrevInstance, _In_ LPSTR pcCmdLine, _In_ int iCmdShow)
{
#if defined(_DEBUG)

    // activate memory debugging in MSVC
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_EVERY_1024_DF);
    _CrtSetBreakAlloc(0);

    // handle pure virtual function calls and invalid arguments
    _set_purecall_handler(HandlePurecall);
    _set_invalid_parameter_handler(HandleInvalidParameter);

#else

    // create crash dump on exception
    SetUnhandledExceptionFilter(CreateCrashDump);

#endif

    // improve hardware utilization on NUMA systems
    ImproveNuma();

    // run the application
    return coreMain(__argc, __argv);
}


#endif /* _WIN32 */
