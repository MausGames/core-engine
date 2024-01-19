///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SPINLOCK_H_
#define _CORE_GUARD_SPINLOCK_H_

// TODO 3: what about read-write-locks, std::shared_mutex


// ****************************************************************
/* spinlock definitions */
#if defined(_CORE_X86_)
    #if defined(_CORE_MSVC_)
        #define CORE_SPINLOCK_YIELD {_mm_pause();}                          // processor level
    #else
        #define CORE_SPINLOCK_YIELD {asm volatile("pause");}                // processor level
    #endif
#elif defined(_CORE_ARM_)
    #if defined(_CORE_MSVC_)
        #define CORE_SPINLOCK_YIELD {__yield();}                            // processor level
    #else
        #define CORE_SPINLOCK_YIELD {asm volatile("yield" ::: "memory");}   // processor level
    #endif
#else
    #if defined(_CORE_WINDOWS_)
        #define CORE_SPINLOCK_YIELD {Sleep(0u);}                            // OS level
    #else
        #define CORE_SPINLOCK_YIELD {sched_yield();}                        // OS level
    #endif
#endif

#if defined(_CORE_EMSCRIPTEN_)
    #define CORE_SPINLOCK_DISABLED   // disable locking (if multi-threading is not supported or required)
#elif defined(_CORE_SWITCH_)
    #define CORE_SPINLOCK_MUTEX      // use regular mutexes (to prevent starvation)
#endif

#if defined(CORE_SPINLOCK_MUTEX)
    #include <mutex>
#endif


// ****************************************************************
/* spinlock class */
class coreSpinLock final
{
private:
#if defined(CORE_SPINLOCK_DISABLED)
    coreBool m_State;             // simple boolean (instead of spinlock)
#elif defined(CORE_SPINLOCK_MUTEX)
    mutable std::mutex m_State;   // regular mutex (instead of spinlock)
#else
    std::atomic_flag m_State;     // atomic lock state
#endif


public:
    constexpr coreSpinLock()noexcept : m_State () {}

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

    DISABLE_COPY(coreAtomic)

    /* assignment operations */
    FORCE_INLINE coreAtomic<T>& operator = (const T& tValue)noexcept;

    /* access atomic value */
    FORCE_INLINE operator T ()const;

    /* change atomic value */
    FORCE_INLINE T FetchAdd(const T& tValue) {return m_tValue.fetch_add(tValue, std::memory_order::relaxed);}
    FORCE_INLINE T FetchSub(const T& tValue) {return m_tValue.fetch_sub(tValue, std::memory_order::relaxed);}
    FORCE_INLINE T FetchAnd(const T& tValue) {return m_tValue.fetch_and(tValue, std::memory_order::relaxed);}
    FORCE_INLINE T FetchOr (const T& tValue) {return m_tValue.fetch_or (tValue, std::memory_order::relaxed);}
    FORCE_INLINE T FetchXor(const T& tValue) {return m_tValue.fetch_xor(tValue, std::memory_order::relaxed);}
    FORCE_INLINE T AddFetch(const T& tValue) {return m_tValue.fetch_add(tValue, std::memory_order::relaxed) + tValue;}
    FORCE_INLINE T SubFetch(const T& tValue) {return m_tValue.fetch_sub(tValue, std::memory_order::relaxed) - tValue;}
    FORCE_INLINE T AndFetch(const T& tValue) {return m_tValue.fetch_and(tValue, std::memory_order::relaxed) & tValue;}
    FORCE_INLINE T OrFetch (const T& tValue) {return m_tValue.fetch_or (tValue, std::memory_order::relaxed) | tValue;}
    FORCE_INLINE T XorFetch(const T& tValue) {return m_tValue.fetch_xor(tValue, std::memory_order::relaxed) ^ tValue;}

    /* change atomic value with operator */
    FORCE_INLINE T operator += (const T& tValue) {return this->AddFetch(tValue);}   // do not return reference
    FORCE_INLINE T operator -= (const T& tValue) {return this->SubFetch(tValue);}
    FORCE_INLINE T operator &= (const T& tValue) {return this->AndFetch(tValue);}
    FORCE_INLINE T operator |= (const T& tValue) {return this->OrFetch (tValue);}
    FORCE_INLINE T operator ^= (const T& tValue) {return this->XorFetch(tValue);}
};


// ****************************************************************
/* acquire the spinlock */
FORCE_INLINE void coreSpinLock::Lock()
{
#if defined(CORE_SPINLOCK_DISABLED)
    m_State = true;
#elif defined(CORE_SPINLOCK_MUTEX)
    m_State.lock();
#else
    while(!this->TryLock()) CORE_SPINLOCK_YIELD
#endif
}


// ****************************************************************
/* release the spinlock */
FORCE_INLINE void coreSpinLock::Unlock()
{
#if defined(CORE_SPINLOCK_DISABLED)
    m_State = false;
#elif defined(CORE_SPINLOCK_MUTEX)
    m_State.unlock();
#else
    m_State.clear(std::memory_order::release);
#endif
}


// ****************************************************************
/* try to acquire the spinlock */
FORCE_INLINE coreBool coreSpinLock::TryLock()
{
#if defined(CORE_SPINLOCK_DISABLED)
    return [this]() {const coreBool A = m_State; m_State = true; return !A;}();
#elif defined(CORE_SPINLOCK_MUTEX)
    return m_State.try_lock();
#else
    return !m_State.test(std::memory_order::relaxed) && !m_State.test_and_set(std::memory_order::acquire);
#endif
}


// ****************************************************************
/* check for current lock state */
FORCE_INLINE coreBool coreSpinLock::IsLocked()const
{
#if defined(CORE_SPINLOCK_DISABLED)
    return m_State;
#elif defined(CORE_SPINLOCK_MUTEX)
    return m_State.try_lock() ? (m_State.unlock(), false) : true;
#else
    return m_State.test(std::memory_order::relaxed);
#endif
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