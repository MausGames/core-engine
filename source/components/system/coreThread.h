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
class coreThread
{
private:
    SDL_Thread* m_pThread;                             //!< pointer to thread structure
    std::string m_sName;                               //!< name of the thread
                                                       
    coreByte m_iExecutions;                            //!< max number of executions per frame (0 = unshackled)
    bool     m_bActive;                                //!< active and currently not forced to shut down

    std::vector<std::function<int()> > m_apFunction;   //!< custom functions being executed by the thread
    SDL_SpinLock m_iLock;                              //!< spinlock to prevent invalid function access


public:
    explicit coreThread(const char* pcName)noexcept;
    virtual ~coreThread();

    /*! start and kill the thread */
    //! @{
    SDL_Thread* StartThread();
    void        KillThread ();
    inline const bool& IsActive()const {return m_bActive;}
    //! @}

    /*! run custom functions within the thread */
    //! @{
    void UpdateFunctions();
    template <typename F> inline void AttachFunction(F&& pFunction) {ASSERT(this->IsActive()) SDL_AtomicLock(&m_iLock); m_apFunction.push_back(pFunction); SDL_AtomicUnlock(&m_iLock);}
    //! @}

    /*! set object properties */
    //! @{
    inline void SetExecutions(const coreByte& iExecutions) {m_iExecutions = iExecutions;}
    //! @}

    /*! get object properties */
    //! @{
    inline const char*     GetName      ()const {return m_sName.c_str();}
    inline const coreByte& GetExecutions()const {return m_iExecutions;}
    //! @}


private:
    DISABLE_COPY(coreThread)

    /*! execute the thread */
    //! @{
    int __Main();
    virtual int  __InitThread() = 0;
    virtual int  __RunThread () = 0;
    virtual void __ExitThread() = 0;
    //! @}

    /*! entry-point function */
    //! @{
    friend int coreThreadMain(void* pData)align_func;
    //! @}
};


#endif /* _CORE_GUARD_THREAD_H_ */