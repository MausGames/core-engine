//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

ENTRY_POINT coreInt32 SDLCALL coreThreadMain(void* pData);


// ****************************************************************
/* constructor */
coreThread::coreThread(const coreChar* pcName)noexcept
: m_pThread     (NULL)
, m_sName       (pcName)
, m_iExecutions (1u)
, m_bActive     (false)
, m_anFunction  {}
, m_iLock       (0)
{
}


// ****************************************************************
/* destructor */
coreThread::~coreThread()
{
    // kill the thread
    this->KillThread();

    // clear memory
    m_anFunction.clear();
}


// ****************************************************************
/* start the thread */
SDL_Thread* coreThread::StartThread()
{
    if(!m_pThread)
    {
        // reset thread status
        m_bActive = true;

        // create thread object
        m_pThread = SDL_CreateThread(coreThreadMain, m_sName.c_str(), this);
        if(!m_pThread)
        {
            Core::Log->Warning("Could not start thread (%s) (SDL: %s)", m_sName.c_str(), SDL_GetError());
            m_bActive = false;
        }
    }

    return m_pThread;
}


// ****************************************************************
/* kill the thread */
void coreThread::KillThread()
{
    // signal thread to shut down
    m_bActive = false;

    // wait for thread to finish
    SDL_WaitThread(m_pThread, NULL);
    m_pThread = NULL;
}


// ****************************************************************
/* call and manage custom functions */
void coreThread::UpdateFunctions()
{
    if(m_anFunction.empty()) return;

    SDL_AtomicLock(&m_iLock);
    {
        // loop trough all functions
        FOR_EACH_DYN(it, m_anFunction)
        {
            // call function and remove when successful
            if((*it)()) DYN_KEEP  (it)
                   else DYN_REMOVE(it, m_anFunction)
        }
    }
    SDL_AtomicUnlock(&m_iLock);
}


// ****************************************************************
/* execute the thread */
coreStatus coreThread::__Main()
{
    coreUint32 iCurFrame     = 0u;
    coreUint8  iCurExecution = 0u;

    // call init-routine
    Core::Log->Info("Thread (%s:%04lu) started", m_sName.c_str(), SDL_GetThreadID(m_pThread));
    coreStatus iReturn = this->__InitThread();

    // begin main-loop
    while(iReturn == CORE_OK)
    {
        do
        {
            // wait for next frame or execution
            SDL_Delay(1u);
        }
        while((iCurFrame >= Core::System->GetCurFrame()) && (iCurExecution == m_iExecutions) && m_bActive);

        // check for external shut down
        if(!m_bActive) break;

        // process execution number
        if(iCurExecution >= m_iExecutions)
        {
            // save latest frame number
            iCurFrame     = Core::System->GetCurFrame();
            iCurExecution = 0u;
        }
        ++iCurExecution;

        // call and manage custom functions
        this->UpdateFunctions();

        // call run-routine
        iReturn = this->__RunThread();
    }

    // call exit-routine
    this->__ExitThread();
    Core::Log->Info("Thread (%s:%04lu) finished", m_sName.c_str(), SDL_GetThreadID(m_pThread));

    m_bActive = false;
    return iReturn;
}


// ****************************************************************
/* entry-point function */
ENTRY_POINT coreInt32 SDLCALL coreThreadMain(void* pData)
{
    // retrieve thread object
    coreThread* pThread = s_cast<coreThread*>(pData);

    // disable denormals
    coreMath::DisableDenormals();

    // execute the thread
    return pThread->__Main();
}