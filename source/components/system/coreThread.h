//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_THREAD_H_
#define _CORE_GUARD_THREAD_H_


// ****************************************************************
/* thread interface */
class INTERFACE coreThread
{
private:
    SDL_Thread* m_pThread;                                   //!< pointer to thread structure
    std::string m_sName;                                     //!< name of the thread

    coreUint8 m_iExecutions;                                 //!< max number of executions per frame (0 = unshackled)
    coreBool  m_bActive;                                     //!< active and currently not forced to shut down

    SDL_SpinLock m_iFuncLock;                                //!< spinlock to prevent invalid function access
    std::vector<std::function<coreStatus()>> m_anFunction;   //!< custom functions being executed by the thread


public:
    explicit coreThread(const coreChar* pcName)noexcept;
    virtual ~coreThread();

    DISABLE_COPY(coreThread)

    /*! start and kill the thread */
    //! @{
    SDL_Thread* StartThread();
    void        KillThread ();
    inline const coreBool& IsActive()const {return m_bActive;}
    //! @}

    /*! run custom functions within the thread */
    //! @{
    void UpdateFunctions();
    template <typename F> inline void AttachFunction(F&& nFunction) {ASSERT(m_anFunction.size() < 60u) SDL_AtomicLock(&m_iFuncLock); m_anFunction.push_back(nFunction); SDL_AtomicUnlock(&m_iFuncLock);}   //!< [](void) -> coreError (CORE_OK, CORE_BUSY)
    //! @}

    /*! set object properties */
    //! @{
    inline void SetExecutions(const coreUint8 iExecutions) {m_iExecutions = iExecutions;}
    //! @}

    /*! get object properties */
    //! @{
    inline const coreChar*  GetName      ()const {return m_sName.c_str();}
    inline const coreUint8& GetExecutions()const {return m_iExecutions;}
    //! @}


private:
    /*! execute the thread */
    //! @{
    coreStatus __Main();
    virtual coreStatus __InitThread() = 0;
    virtual coreStatus __RunThread () = 0;
    virtual void       __ExitThread() = 0;
    //! @}

    /*! entry-point function */
    //! @{
    friend ENTRY_POINT coreInt32 SDLCALL coreThreadMain(void* pData);
    //! @}
};


#endif /* _CORE_GUARD_THREAD_H_ */