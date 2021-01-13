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

// TODO: use atomic_flag with C++20 ?


// ****************************************************************
/* spinlock class */
class coreSpinLock final
{
private:
    std::atomic<coreBool> m_bState;   // atomic lock state


public:
    constexpr coreSpinLock()noexcept : m_bState (false) {}

    DISABLE_COPY(coreSpinLock)

    /* acquire and release the spinlock */
    FORCE_INLINE void     Lock();
    FORCE_INLINE void     Unlock();
    FORCE_INLINE coreBool TryLock();
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
    m_bState.store(false, std::memory_order_release);
}


// ****************************************************************
/* try to acquire the spinlock */
FORCE_INLINE coreBool coreSpinLock::TryLock()
{
    return !m_bState.load(std::memory_order_relaxed) && !m_bState.exchange(true, std::memory_order_acquire);
}


#endif /* _CORE_GUARD_SPINLOCK_H_ */