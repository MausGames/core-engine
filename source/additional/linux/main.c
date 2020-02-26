///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#if defined(__linux__)

#pragma GCC diagnostic ignored "-Wunused-parameter"

#include <signal.h>
#include <execinfo.h>
#include <stdlib.h>
#include <stdio.h>

extern int coreMain(int argc, char** argv);


// ****************************************************************
/* execute custom signal handler */
static void SignalHandler(int iSignal, siginfo_t* pInfo, void* pContext)
{
    // create custom crash dump
    FILE* pFile = fopen("crash.dmp", "wb");
    if(pFile)
    {
        void* apAddress[64];

        // write backtrace
        const int iNum = backtrace(apAddress, sizeof(apAddress) / sizeof(apAddress[0]));
        if(iNum) backtrace_symbols_fd(apAddress, iNum, fileno(pFile));

        // write signal information
        fwrite(pInfo, sizeof(siginfo_t), 1u, pFile);

        // close file
        fclose(pFile);
    }

    // try to create regular crash dump
    abort();
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
    // install custom signal handler
    InstallSignalHandler();

    // run the application
    return coreMain(argc, argv);
}


#endif /* __linux__ */
