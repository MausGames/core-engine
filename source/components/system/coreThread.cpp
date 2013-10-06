//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

int coreThreadMain(void* pData);


// ****************************************************************
// constructor
coreThread::coreThread()noexcept
: m_pThread   (NULL)
, m_sName     ("")
, m_iCurFrame (0)
, m_bEnd      (true)
{
}


// ****************************************************************
// destructor
coreThread::~coreThread()
{
    this->KillThread();
}


// ****************************************************************
// start the thread
SDL_Thread* coreThread::StartThread(const char* pcName)
{
    if(!m_bEnd) return NULL;
    m_bEnd = false;

    // save name of the thread
    m_sName = pcName;

    // create thread object
    m_pThread = SDL_CreateThread(coreThreadMain, pcName, this);
    if(!m_pThread) Core::Log->Error(1, coreUtils::Print("Could not start thread (%s) (SDL: %s)", pcName, SDL_GetError()));

    return m_pThread;
}


// ****************************************************************
// kill the thread
void coreThread::KillThread()
{
    if(m_bEnd) return;
    m_bEnd = true;

    // set end status and wait
    SDL_WaitThread(m_pThread, NULL);
}


// ****************************************************************
// execute the thread
int coreThread::__Main()
{
    // call init implementation
    Core::Log->Info(coreUtils::Print("Thread (%s:%04lu) started", m_sName.c_str(), SDL_ThreadID()));
    int iReturn = this->__Init();

    m_iCurFrame = 0;
    while(iReturn == 0)
    {
        // wait for next frame
        while(m_iCurFrame >= Core::System->GetCurFrame() && !m_bEnd)
            SDL_Delay(1);
        m_iCurFrame = Core::System->GetCurFrame();

        // check for kill
        if(m_bEnd) break;

        // call run implementation
        iReturn = this->__Run();
    }

    // call exit implementation
    this->__Exit();
    Core::Log->Info(coreUtils::Print("Thread (%s:%04lu) finished", m_sName.c_str(), SDL_ThreadID()));

    m_bEnd = true;
    return iReturn;
}


// ****************************************************************
// wrapper for thread creation
int coreThreadMain(void* pData)
{
    // retrieve thread object
    coreThread* pThread = static_cast<coreThread*>(pData);

    // execute thread
    return pThread->__Main();
}