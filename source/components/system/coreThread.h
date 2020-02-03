///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_THREAD_H_
#define _CORE_GUARD_THREAD_H_


// ****************************************************************
/* thread interface */
class INTERFACE coreThread
{
private:
    SDL_Thread* m_pThread;                                     //!< pointer to thread structure
    std::string m_sName;                                       //!< name of the thread

    coreFloat m_fFrequency;                                    //!< average number of iterations per second (0.0f = ignore)
    coreBool  m_bActive;                                       //!< currently active and not forced to shut down

    std::vector<std::function<coreStatus()>> m_anFuncNew;      //!< new custom functions (separate, to allow attaching and executing at the same time)
    std::vector<std::function<coreStatus()>> m_anFuncActive;   //!< active custom functions
    SDL_SpinLock m_iLockNew;                                   //!< spinlock for collecting new functions
    SDL_SpinLock m_iLockActive;                                //!< spinlock for executing active functions


public:
    explicit coreThread(const coreChar* pcName)noexcept;
    virtual ~coreThread();

    DISABLE_COPY(coreThread)

    /*! start and kill the thread */
    //! @{
    SDL_Thread* StartThread();
    void        KillThread ();
    //! @}

    /*! run custom functions within the thread */
    //! @{
    void UpdateFunctions();
    template <typename F> inline void AttachFunction(F&& nFunction) {coreAtomicLock(&m_iLockNew); m_anFuncNew.push_back(nFunction); coreAtomicUnlock(&m_iLockNew);}   //!< [](void) -> coreStatus (CORE_OK, CORE_BUSY)
    //! @}

    /*! set object properties */
    //! @{
    inline void SetFrequency(const coreFloat fFrequency) {m_fFrequency = fFrequency;}
    //! @}

    /*! get object properties */
    //! @{
    inline const coreChar*  GetName     ()const {return m_sName.c_str();}
    inline const coreFloat& GetFrequency()const {return m_fFrequency;}
    inline const coreBool&  GetActive   ()const {return m_bActive;}
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
    friend coreInt32 coreThreadMain(void* pData);
    //! @}
};


#endif /* _CORE_GUARD_THREAD_H_ */