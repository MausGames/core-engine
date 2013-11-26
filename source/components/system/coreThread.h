//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_THREAD_H_
#define _CORE_GUARD_THREAD_H_


// ****************************************************************
// restrained thread interface
class coreThread
{
private:
    SDL_Thread* m_pThread;   //!< pointer to thread structure
    std::string m_sName;     //!< name of the thread

    coreUint m_iCurFrame;    //!< current frame
    bool m_bEnd;             //!< end status


public:
    coreThread()noexcept;
    virtual ~coreThread();

    //! start and kill the thread
    //! @{
    SDL_Thread* StartThread(const char* pcName);
    void KillThread();
    //! @}


private:
    DISABLE_COPY(coreThread)

    //! execute the thread
    //! @{
    int __Main();
    virtual int __Init()  {return 0;}
    virtual int __Run() = 0;
    virtual void __Exit() {}
    //! @}

    //! wrapper for thread creation
    //! @{
    friend int coreThreadMain(void* pData) align_func;
    //! @}
};


#endif // _CORE_GUARD_THREAD_H_