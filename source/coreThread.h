#pragma once


// ****************************************************************
// synchronous thread interface class
class coreThread
{
private:
    SDL_Thread* m_pThread;   // pointer to thread struct
    std::string m_sName;     // name of the thread

    coreUint m_iCurFrame;    // current frame
    bool m_bEnd;             // end status


public:
    coreThread();
    virtual ~coreThread();

    // control thread
    SDL_Thread* StartThread(const char* pcName);
    void KillThread();


private:
    // execute thread
    int __Main();
    virtual int __Init() {return 0;}
    virtual int __Run() = 0;
    virtual void __Exit() {}

    // wrapper for thread creation
    friend int coreThreadMain(void* pData);
};