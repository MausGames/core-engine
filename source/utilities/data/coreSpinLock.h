///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SPINLOCK_H_
#define _CORE_GUARD_SPINLOCK_H_


// ****************************************************************
/* spinlock class */
class coreSpinLock final
{
private:
    std::atomic_flag m_bState;   // atomic lock state


public:
    constexpr coreSpinLock()noexcept : m_bState () {}

    DISABLE_COPY(coreSpinLock)

    /* acquire and release the spinlock */
    FORCE_INLINE void     Lock();
    FORCE_INLINE void     Unlock();
    FORCE_INLINE coreBool TryLock();

    /* check for current lock state */
    FORCE_INLINE coreBool IsLocked()const;
};


// ****************************************************************
/* spinlock helper class */
class coreSpinLocker final
{
private:
    coreSpinLock* m_pLock;   // associated spinlock


public:
    explicit coreSpinLocker(coreSpinLock* pLock)noexcept : m_pLock (pLock) {m_pLock->Lock();}
    ~coreSpinLocker()                                                      {m_pLock->Unlock();}

    DISABLE_COPY(coreSpinLocker)
    DISABLE_HEAP
};


// ****************************************************************
/* acquire the spinlock */
FORCE_INLINE void coreSpinLock::Lock()
{
    while(!this->TryLock())
    {
    #if defined(_CORE_SSE_)
        _mm_pause();     // processor level spinning
    #else
        SDL_Delay(0u);   // OS level spinning
    #endif
    }
}


// ****************************************************************
/* release the spinlock */
FORCE_INLINE void coreSpinLock::Unlock()
{
    m_bState.clear(std::memory_order_release);
}


// ****************************************************************
/* try to acquire the spinlock */
FORCE_INLINE coreBool coreSpinLock::TryLock()
{
    return !m_bState.test(std::memory_order_relaxed) && !m_bState.test_and_set(std::memory_order_acquire);
}


// ****************************************************************
/* check for current lock state */
FORCE_INLINE coreBool coreSpinLock::IsLocked()const
{
    return m_bState.test(std::memory_order_relaxed);
}


#endif /* _CORE_GUARD_SPINLOCK_H_ */