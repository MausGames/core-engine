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
/* relaxed atomic class */
template <typename T> class coreAtomic final
{
private:
    std::atomic<T> m_tValue;   // atomic variable

    STATIC_ASSERT(std::atomic<T>::is_always_lock_free)


public:
    coreAtomic() = default;
    constexpr coreAtomic(const T& tValue)noexcept : m_tValue (tValue) {}

    DISABLE_COPY(coreAtomic<T>)

    /* assignment operations */
    FORCE_INLINE coreAtomic<T>& operator = (const T& tValue)noexcept;

    /* access atomic value */
    FORCE_INLINE operator T ()const;

    /* change atomic value */
    FORCE_INLINE T FetchAdd(const T& tValue) {return m_tValue.fetch_add(tValue, std::memory_order::relaxed);}
    FORCE_INLINE T FetchSub(const T& tValue) {return m_tValue.fetch_sub(tValue, std::memory_order::relaxed);}
    FORCE_INLINE T AddFetch(const T& tValue) {return m_tValue.fetch_add(tValue, std::memory_order::relaxed) + tValue;}
    FORCE_INLINE T SubFetch(const T& tValue) {return m_tValue.fetch_sub(tValue, std::memory_order::relaxed) - tValue;}
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
    m_bState.clear(std::memory_order::release);
}


// ****************************************************************
/* try to acquire the spinlock */
FORCE_INLINE coreBool coreSpinLock::TryLock()
{
    return !m_bState.test(std::memory_order::relaxed) && !m_bState.test_and_set(std::memory_order::acquire);
}


// ****************************************************************
/* check for current lock state */
FORCE_INLINE coreBool coreSpinLock::IsLocked()const
{
    return m_bState.test(std::memory_order::relaxed);
}


// ****************************************************************
/* assignment operations */
template <typename T> FORCE_INLINE coreAtomic<T>& coreAtomic<T>::operator = (const T& tValue)noexcept
{
    m_tValue.store(tValue, std::memory_order::relaxed);
    return *this;
}


// ****************************************************************
/* access atomic value */
template <typename T> FORCE_INLINE coreAtomic<T>::operator T ()const
{
    return m_tValue.load(std::memory_order::relaxed);
}


#endif /* _CORE_GUARD_SPINLOCK_H_ */