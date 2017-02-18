//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#if defined(_WIN32)

#define _WIN32_WINNT _WIN32_WINNT_WINXP
#define  WINVER      _WIN32_WINNT_WINXP
#define  WIN32_LEAN_AND_MEAN

#pragma warning(disable : 4100)

#include <SDL2/SDL_main.h>
#include <windows.h>
#include <crtdbg.h>


// ****************************************************************
/* always use the discrete high-performance graphics device */
__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001u;
__declspec(dllexport) DWORD NvOptimusEnablement                  = 0x00000001u;


// ****************************************************************
/* improve hardware utilization on NUMA systems */
static void ImproveNUMA(void)
{
    // get highest NUMA node number
    ULONG iNumaNode;
    if(GetNumaHighestNodeNumber(&iNumaNode))
    {
        // get all processors of the NUMA node
        ULONGLONG iProcessMask;
        if(GetNumaNodeProcessorMask((UCHAR)iNumaNode, &iProcessMask))
        {
            // change processor affinity mask
            if(iProcessMask) SetProcessAffinityMask(GetCurrentProcess(), (DWORD_PTR)iProcessMask);
        }
    }
}


// ****************************************************************
/* start up the application */
extern int WINAPI WinMain(_In_ HINSTANCE pInstance, _In_opt_ HINSTANCE pPrevInstance, _In_ LPSTR pcCmdLine, _In_ int iCmdShow)
{
#if defined(_DEBUG)

    // activate memory debugging in MSVC
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetBreakAlloc(0);

#endif

    // improve hardware utilization on NUMA systems
    ImproveNUMA();

    // initialize the SDL library
    SDL_SetMainReady();

    // run the application
    char* argv[] = {"CoreApp", NULL};
    return SDL_main(sizeof(argv) / sizeof(argv[0]) - 1, argv);
}


#endif /* _WIN32 */