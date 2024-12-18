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

// TODO 3: implement read-write spinlock (std::shared_mutex)
// TODO 3: get rid of the mutable mutex
// TODO 3: use fetch_min and fetch_max with C++26


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
/* recursive spinlock class */
class coreRecursiveLock final
{
private:
#if defined(CORE_SPINLOCK_DISABLED)
    UNUSED coreBool m_State;             // simple boolean (instead of spinlock)
#elif defined(CORE_SPINLOCK_MUTEX)
    std::recursive_mutex m_State;        // regular recursive mutex (instead of spinlock)
#else
    std::atomic<SDL_threadID> m_State;   // atomic thread-ID of the current owner
#endif

    coreUint8 m_iCount;                  // current levels of ownership


public:
    constexpr coreRecursiveLock()noexcept : m_State (), m_iCount (0u) {}

    DISABLE_COPY(coreRecursiveLock)

    /* acquire and release the recursive spinlock */
    FORCE_INLINE void     Lock();
    FORCE_INLINE void     Unlock();
    FORCE_INLINE coreBool TryLock();

    /* check for current lock state */
    FORCE_INLINE coreBool IsLocked()const;
};


// ****************************************************************
/* spinlock helper class */
template <typename T> class coreSpinLocker final
{
private:
    T* m_pLock;   // associated spinlock


public:
    explicit coreSpinLocker(T* pLock)noexcept : m_pLock (pLock) {m_pLock->Lock();}
    ~coreSpinLocker()                                           {m_pLock->Unlock();}

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
    FORCE_INLINE coreAtomic& operator = (const T& tValue)noexcept;

    /* access atomic value */
    FORCE_INLINE operator T ()const;

    /* change atomic value */
    FORCE_INLINE T FetchAdd(const T& tValue) {return m_tValue.fetch_add(tValue, std::memory_order::relaxed);}
    FORCE_INLINE T FetchSub(const T& tValue) {return m_tValue.fetch_sub(tValue, std::memory_order::relaxed);}
    FORCE_INLINE T FetchAnd(const T& tValue) {return m_tValue.fetch_and(tValue, std::memory_order::relaxed);}
    FORCE_INLINE T FetchOr (const T& tValue) {return m_tValue.fetch_or (tValue, std::memory_order::relaxed);}
    FORCE_INLINE T FetchXor(const T& tValue) {return m_tValue.fetch_xor(tValue, std::memory_order::relaxed);}
    FORCE_INLINE T FetchMin(const T& tValue) {T tPrev = m_tValue.load(std::memory_order::relaxed); while((tPrev > tValue) && !m_tValue.compare_exchange_weak(tPrev, tValue, std::memory_order::relaxed)) {} return tPrev;}
    FORCE_INLINE T FetchMax(const T& tValue) {T tPrev = m_tValue.load(std::memory_order::relaxed); while((tPrev < tValue) && !m_tValue.compare_exchange_weak(tPrev, tValue, std::memory_order::relaxed)) {} return tPrev;}
    FORCE_INLINE T AddFetch(const T& tValue) {return m_tValue.fetch_add(tValue, std::memory_order::relaxed) + tValue;}
    FORCE_INLINE T SubFetch(const T& tValue) {return m_tValue.fetch_sub(tValue, std::memory_order::relaxed) - tValue;}
    FORCE_INLINE T AndFetch(const T& tValue) {return m_tValue.fetch_and(tValue, std::memory_order::relaxed) & tValue;}
    FORCE_INLINE T OrFetch (const T& tValue) {return m_tValue.fetch_or (tValue, std::memory_order::relaxed) | tValue;}
    FORCE_INLINE T XorFetch(const T& tValue) {return m_tValue.fetch_xor(tValue, std::memory_order::relaxed) ^ tValue;}
    FORCE_INLINE T MinFetch(const T& tValue) {T tPrev = m_tValue.load(std::memory_order::relaxed); while((tPrev > tValue) && !m_tValue.compare_exchange_weak(tPrev, tValue, std::memory_order::relaxed)) {} return MIN(tPrev, tValue);}
    FORCE_INLINE T MaxFetch(const T& tValue) {T tPrev = m_tValue.load(std::memory_order::relaxed); while((tPrev < tValue) && !m_tValue.compare_exchange_weak(tPrev, tValue, std::memory_order::relaxed)) {} return MAX(tPrev, tValue);}
    FORCE_INLINE T Exchange(const T& tValue) {return m_tValue.exchange (tValue, std::memory_order::relaxed);}

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
    ASSERT(this->IsLocked())

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
    return m_State ? false : (m_State = true, true);
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
/* acquire the recursive spinlock */
FORCE_INLINE void coreRecursiveLock::Lock()
{
#if defined(CORE_SPINLOCK_DISABLED)
    m_iCount++;
#elif defined(CORE_SPINLOCK_MUTEX)
    m_State.lock(); m_iCount++;
#else
    while(!this->TryLock()) CORE_SPINLOCK_YIELD
#endif
}


// ****************************************************************
/* release the recursive spinlock */
FORCE_INLINE void coreRecursiveLock::Unlock()
{
    ASSERT(this->IsLocked())

#if defined(CORE_SPINLOCK_DISABLED)
    m_iCount--;
#elif defined(CORE_SPINLOCK_MUTEX)
    m_iCount--; m_State.unlock();
#else
    if(!(--m_iCount)) m_State.store(0u, std::memory_order::release);
#endif
}


// ****************************************************************
/* try to acquire the recursive spinlock */
FORCE_INLINE coreBool coreRecursiveLock::TryLock()
{
#if defined(CORE_SPINLOCK_DISABLED)
    return (m_iCount++, true);
#elif defined(CORE_SPINLOCK_MUTEX)
    return m_State.try_lock() ? (m_iCount++, true) : false;
#else

    SDL_threadID       iExpected = 0u;
    const SDL_threadID iDesired  = SDL_ThreadID();

    if(m_State.compare_exchange_weak(iExpected, iDesired, std::memory_order::acquire, std::memory_order::relaxed) || (iExpected == iDesired))
    {
        m_iCount++;
        return true;
    }

    return false;

#endif
}


// ****************************************************************
/* check for current lock state */
FORCE_INLINE coreBool coreRecursiveLock::IsLocked()const
{
    return (m_iCount != 0u);
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