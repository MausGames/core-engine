//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#pragma warning(disable : 4100)

#include <SDL2/SDL_main.h>
#include <windows.h>


// ****************************************************************
/* improve hardware utilization on NUMA systems */
void ImproveNUMA(void)
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
int WINAPI WinMain(HINSTANCE pInstance, HINSTANCE pPrevInstance, LPSTR pcCmdLine, int iCmdShow)
{
    // initialize the SDL library
    SDL_SetMainReady();

    // improve hardware utilization on NUMA systems
    ImproveNUMA();

    // run the application
    char* argv[2] = {"CoreApp", NULL};
    SDL_main(1, argv);

    return 0;
}


#endif /* _WIN32 */