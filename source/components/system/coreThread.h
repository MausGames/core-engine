//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef GUARD_CORE_THREAD_H
#define GUARD_CORE_THREAD_H


// ****************************************************************
// restrained thread interface class
class coreThread
{
private:
    SDL_Thread* m_pThread;   //!< pointer to thread structure
    std::string m_sName;     //!< name of the thread

    coreUint m_iCurFrame;    //!< current frame
    bool m_bEnd;             //!< end status


public:
    coreThread();
    virtual ~coreThread();

    //! control thread
    //! @{
    SDL_Thread* StartThread(const char* pcName);
    void KillThread();
    //! @}


private:
    //! disable copy
    //! @{
    coreThread(const coreThread& c) deletefunc;
    coreThread& operator = (const coreThread& c) deletefunc;
    //! @}

    //! execute thread
    //! @{
    int __Main();
    virtual int __Init() {return 0;}
    virtual int __Run() = 0;
    virtual void __Exit() {}
    //! @}

    //! wrapper for thread creation
    //! @{
    friend int coreThreadMain(void* pData);
    //! @}
};


#endif // GUARD_CORE_THREAD_H