///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_THREAD_H_
#define _CORE_GUARD_THREAD_H_

// TODO 5: maintain separate sorted token-table to make dependency-lookup faster (or other way to accelerate dependency-lookup)


// ****************************************************************
/* thread interface */
class INTERFACE coreThread
{
private:
    /* custom function structure */
    struct coreCustomFunc final
    {
        std::function<coreStatus()> nFunction;     // actual function to execute
        coreUint32                  iToken;        // unique token as identifier
        coreUint32                  iDependency;   // dependency token
    };


private:
    SDL_Thread* m_pThread;                     // pointer to thread structure
    coreString  m_sName;                       // name of the thread

    coreAtomic<coreFloat> m_fFrequency;        // average number of iterations per second (0.0f = ignore)
    coreAtomic<coreBool>  m_bActive;           // currently active and not forced to shut down

    coreList<coreCustomFunc> m_anFuncNew;      // new custom functions (separate, to allow attaching and executing at the same time)
    coreList<coreCustomFunc> m_anFuncActive;   // active custom functions
    coreAtomic<coreUint16>   m_iFuncNum;       // total number of custom functions (new and active)

    coreSpinLock m_LockNew;                    // spinlock for collecting new functions
    coreSpinLock m_LockActive;                 // spinlock for executing active functions

    coreUint32 m_iTokenCount;                  // number of assigned function tokens


public:
    explicit coreThread(const coreChar* pcName)noexcept;
    virtual ~coreThread();

    DISABLE_COPY(coreThread)

    /* start and kill the thread */
    SDL_Thread* StartThread();
    void        KillThread ();

    /* run custom functions within the thread */
    template <typename F> coreUint32 AttachFunction(F&& nFunction, const coreUint32 iDependency = 0u);   // [](void) -> coreStatus (CORE_OK, CORE_BUSY)
    coreBool DetachFunction(const coreUint32 iToken);
    void     UpdateFunctions();

    /* set object properties */
    inline void SetFrequency(const coreFloat fFrequency) {m_fFrequency = fFrequency;}

    /* get object properties */
    inline const coreChar*  GetName        ()const {return m_sName.c_str();}
    inline       coreFloat  GetFrequency   ()const {return m_fFrequency;}
    inline       coreBool   GetActive      ()const {return m_bActive;}
    inline       coreUint16 GetNumFunctions()const {return m_iFuncNum;}


private:
    /* execute the thread */
    coreStatus __Main();
    virtual coreStatus __InitThread() = 0;
    virtual coreStatus __RunThread () = 0;
    virtual void       __ExitThread() = 0;

    /* track number of custom functions */
    inline void __FuncIncrease() {m_iFuncNum.FetchAdd(1u); ASSERT(m_iFuncNum)}
    inline void __FuncDecrease() {ASSERT(m_iFuncNum) m_iFuncNum.FetchSub(1u);}

    /* entry-point function */
    friend coreInt32 SDLCALL coreThreadMain(void* pData);
};


// ****************************************************************
/* attach custom function */
template <typename F> coreUint32 coreThread::AttachFunction(F&& nFunction, const coreUint32 iDependency)
{
    const coreSpinLocker oLocker(&m_LockNew);

    // get unique token
    const coreUint32 iToken = (++m_iTokenCount);
    ASSERT(iToken > iDependency)

    // create new custom function
    coreCustomFunc oFunc;
    oFunc.nFunction   = std::forward<F>(nFunction);
    oFunc.iToken      = iToken;
    oFunc.iDependency = iDependency;

    // add function to list
    m_anFuncNew.push_back(std::move(oFunc));
    this->__FuncIncrease();

    return iToken;
}


#endif /* _CORE_GUARD_THREAD_H_ */