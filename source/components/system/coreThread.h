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
// restrained thread interface
// TODO: add interface for multiple executions per frame (+ unshackled)
class coreThread
{
private:
    SDL_Thread* m_pThread;   //!< pointer to thread structure
    std::string m_sName;     //!< name of the thread

    coreUint m_iCurFrame;    //!< current frame
    bool m_bEnd;             //!< end status


public:
    explicit coreThread(const char* pcName)noexcept;
    virtual ~coreThread();

    //! start and kill the thread
    //! @{
    SDL_Thread* StartThread();
    void        KillThread ();
    //! @}


private:
    DISABLE_COPY(coreThread)

    //! execute the thread
    //! @{
    int __Main();
    virtual int  __InitThread() = 0;
    virtual int  __RunThread () = 0;
    virtual void __ExitThread() = 0;
    //! @}

    //! wrapper for thread creation
    //! @{
    friend int coreThreadMain(void* pData)align_func;
    //! @}
};


#endif // _CORE_GUARD_THREAD_H_