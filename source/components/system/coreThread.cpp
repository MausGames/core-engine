///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
coreThread::coreThread(const coreChar* pcName)noexcept
: m_pThread      (NULL)
, m_sName        (pcName)
, m_fFrequency   (0.0f)
, m_bActive      (false)
, m_anFuncNew    {}
, m_anFuncActive {}
, m_LockNew      ()
, m_LockActive   ()
, m_iTokenCount  (0u)
{
}


// ****************************************************************
/* destructor */
coreThread::~coreThread()
{
    // kill the thread
    this->KillThread();

    // clear memory
    m_anFuncNew   .clear();
    m_anFuncActive.clear();
}


// ****************************************************************
/* start the thread */
SDL_Thread* coreThread::StartThread()
{
#if !defined(_CORE_EMSCRIPTEN_)

    if(!m_pThread)
    {
        // reset thread status
        m_bActive = true;

        // create thread object
        m_pThread = SDL_CreateThreadWithStackSize(coreThreadMain, m_sName.c_str(), 1u * 1024u * 1024u, this);
        WARN_IF(!m_pThread)
        {
            Core::Log->Warning("Thread (%s) could not be started (SDL: %s)", m_sName.c_str(), SDL_GetError());
            m_bActive = false;
        }
    }

#endif

    return m_pThread;
}


// ****************************************************************
/* kill the thread */
void coreThread::KillThread()
{
#if !defined(_CORE_EMSCRIPTEN_)

    if(m_pThread)
    {
        // signal thread to shut down
        m_bActive = false;

        // wait for thread to finish
        SDL_WaitThread(m_pThread, NULL);
        m_pThread = NULL;
    }

#endif
}


// ****************************************************************
/* detach custom function */
coreBool coreThread::DetachFunction(const coreUint32 iToken)
{
    if(!m_anFuncNew.empty())
    {
        const coreSpinLocker oLocker(&m_LockNew);

        // search through new custom functions
        FOR_EACH(it, m_anFuncNew)
        {
            if(it->iToken == iToken)
            {
                // remove function from list
                m_anFuncNew.erase(it);
                return true;
            }
        }
    }

    if(!m_anFuncActive.empty())
    {
        const coreSpinLocker oLocker(&m_LockActive);

        // search through active custom functions
        FOR_EACH(it, m_anFuncActive)
        {
            if(it->iToken == iToken)
            {
                // remove function from list
                m_anFuncActive.erase(it);
                return true;
            }
        }
    }

    return false;
}


// ****************************************************************
/* call and manage custom functions */
void coreThread::UpdateFunctions()
{
    const coreSpinLocker oLocker(&m_LockActive);

    if(!m_anFuncNew.empty())
    {
        const coreSpinLocker oLocker2(&m_LockNew);

        // collect new custom functions
        FOR_EACH(it, m_anFuncNew) m_anFuncActive.push_back(std::move(*it));
        m_anFuncNew.clear();
    }

    // loop trough all functions
    FOR_EACH_DYN(it, m_anFuncActive)
    {
        // call function and remove when successful
        if(it->nFunction()) DYN_KEEP  (it, m_anFuncActive)
                       else DYN_REMOVE(it, m_anFuncActive)
    }
}


// ****************************************************************
/* execute the thread */
coreStatus coreThread::__Main()
{
    coreUint64 iBeforeTime = 0u;
    coreUint64 iAfterTime  = 0u;
    coreDouble dWait       = 0.0;

    // call init-routine
    Core::Log->Info("Thread (%s, %04lX) started", m_sName.c_str(), SDL_GetThreadID(m_pThread));
    coreStatus eReturn = this->__InitThread();

    // begin main-loop
    while(eReturn == CORE_OK)
    {
        if(m_fFrequency)
        {
            // handle thread-overhead
            iBeforeTime = SDL_GetPerformanceCounter();
            dWait      -= coreDouble(iBeforeTime - iAfterTime) * Core::System->GetPerfFrequency();

            // wait for next iteration
            dWait = MAX(dWait + (1.0 / coreDouble(m_fFrequency)), 0.0);
            SDL_Delay(F_TO_UI(dWait * 1000.0));

            // handle rounding-errors
            iAfterTime = SDL_GetPerformanceCounter();
            dWait     -= coreDouble(iAfterTime - iBeforeTime) * Core::System->GetPerfFrequency();
        }
        else
        {
            // always yield
            SDL_Delay(0u);
        }

        // check for shut down
        if(!m_bActive) break;

        // call and manage custom functions
        this->UpdateFunctions();

        // call run-routine
        eReturn = this->__RunThread();
    }

    // call exit-routine
    this->__ExitThread();
    Core::Log->Info("Thread (%s, %04lX) finished", m_sName.c_str(), SDL_GetThreadID(m_pThread));

    m_bActive = false;
    return eReturn;
}


// ****************************************************************
/* entry-point function */
coreInt32 SDLCALL coreThreadMain(void* pData)
{
    // retrieve thread object
    coreThread* pThread = s_cast<coreThread*>(pData);

    // set floating-point behavior
    coreMath::EnableExceptions();
    coreMath::EnableRoundToNearest();
    coreMath::DisableDenormals();

    // execute the thread
    return pThread->__Main();
}