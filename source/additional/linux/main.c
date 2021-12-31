///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#if defined(__linux__)

#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <unistd.h>
#include <signal.h>
#include <execinfo.h>
#include <malloc.h>
#include <stdio.h>

extern int coreMain(int argc, char** argv);
extern const char* g_pcUserFolder;


// ****************************************************************
/* execute custom signal handler */
static void SignalHandler(int iSignal, siginfo_t* pInfo, void* pContext)
{
    // switch to user folder
    chdir(g_pcUserFolder);

    // create custom crash dump
    FILE* pFile = fopen("crash.dmp", "wb");
    if(pFile)
    {
        void* apAddress[64];

        // write backtrace
        const int iNum = backtrace(apAddress, sizeof(apAddress) / sizeof(apAddress[0]));
        if(iNum) backtrace_symbols_fd(apAddress, iNum, fileno(pFile));

        // write allocator stats
        malloc_info(0, pFile);

        // write signal information
        fwrite(pInfo, sizeof(siginfo_t), 1u, pFile);

        // close file
        fclose(pFile);
    }
}


// ****************************************************************
/* install custom signal handler */
static void InstallSignalHandler(void)
{
    // create signal action with custom signal handler
    struct sigaction oAction = {0};
    oAction.sa_sigaction = &SignalHandler;
    oAction.sa_flags     = SA_SIGINFO | SA_RESETHAND;

    // set signal action
    sigaction(SIGILL,  &oAction, NULL);
    sigaction(SIGABRT, &oAction, NULL);
    sigaction(SIGFPE,  &oAction, NULL);
    sigaction(SIGBUS,  &oAction, NULL);
    sigaction(SIGSEGV, &oAction, NULL);
    sigaction(SIGSYS,  &oAction, NULL);
    sigaction(SIGPIPE, &oAction, NULL);
}


// ****************************************************************
/* start up the application */
int main(int argc, char** argv)
{
#if defined(__OPTIMIZE__)

    // install custom signal handler
    InstallSignalHandler();

#else

    // improve memory debugging with glibc
    mallopt(M_CHECK_ACTION, 3);
    mallopt(M_PERTURB,      1);

#endif

    // run the application
    return coreMain(argc, argv);
}


#endif /* __linux__ */
