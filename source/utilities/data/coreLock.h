///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_LOCK_H_
#define _CORE_GUARD_LOCK_H_

// TODO 3: get rid of the mutable mutexes (due to IsLocked)


// ****************************************************************
/* lock definitions */
#if defined(_CORE_X86_)
    #if defined(_CORE_MSVC_)
        #define CORE_LOCK_YIELD {_mm_pause();}                          // processor level
    #else
        #define CORE_LOCK_YIELD {asm volatile("pause");}                // processor level
    #endif
#elif defined(_CORE_ARM_)
    #if defined(_CORE_MSVC_)
        #define CORE_LOCK_YIELD {__yield();}                            // processor level
    #else
        #define CORE_LOCK_YIELD {asm volatile("yield" ::: "memory");}   // processor level
    #endif
#else
    #if defined(_CORE_WINDOWS_)
        #define CORE_LOCK_YIELD {Sleep(0u);}                            // OS level
    #else
        #define CORE_LOCK_YIELD {sched_yield();}                        // OS level
    #endif
#endif

#if defined(_CORE_EMSCRIPTEN_)
    #define CORE_LOCK_DISABLED   // disable locking (if multi-threading is not supported or required)
#elif defined(_CORE_SWITCH_)
    #define CORE_LOCK_MUTEX      // use regular mutexes (to prevent starvation)
#endif

#if defined(CORE_LOCK_MUTEX)
    #include <mutex>
    #include <shared_mutex>
#endif


// ****************************************************************
/* lock class */
class coreLock final
{
private:
#if defined(CORE_LOCK_DISABLED)
    coreBool m_State;             // simple boolean (instead of spinlock)
#elif defined(CORE_LOCK_MUTEX)
    mutable std::mutex m_State;   // regular mutex (instead of spinlock)
#else
    std::atomic_flag m_State;     // atomic lock state
#endif


public:
    constexpr coreLock()noexcept : m_State () {}

    DISABLE_COPY(coreLock)

    /* acquire and release the lock */
    FORCE_INLINE void     Lock();
    FORCE_INLINE void     Unlock();
    FORCE_INLINE coreBool TryLock();

    /* check for current lock state */
    FORCE_INLINE coreBool IsLocked()const;
};


// ****************************************************************
/* recursive lock class */
class coreRecursiveLock final
{
private:
#if defined(CORE_LOCK_DISABLED)
    UNUSED coreBool m_State;             // simple boolean (instead of spinlock)
#elif defined(CORE_LOCK_MUTEX)
    std::recursive_mutex m_State;        // regular recursive mutex (instead of spinlock)
#else
    std::atomic<SDL_ThreadID> m_State;   // atomic thread-ID of the current owner
#endif

    coreUint8 m_iCount;                  // current levels of ownership


public:
    coreRecursiveLock()noexcept : m_State (), m_iCount (0u) {}

    DISABLE_COPY(coreRecursiveLock)

    /* acquire and release the recursive lock */
    FORCE_INLINE void     Lock();
    FORCE_INLINE void     Unlock();
    FORCE_INLINE coreBool TryLock();

    /* check for current lock state */
    FORCE_INLINE coreBool IsLocked()const;
};


// ****************************************************************
/* shared lock class */
class coreSharedLock final
{
private:
#if defined(CORE_LOCK_DISABLED)
    coreBool m_State;                    // simple boolean (instead of spinlock)
#elif defined(CORE_LOCK_MUTEX)
    mutable std::shared_mutex m_State;   // regular shared mutex (instead of spinlock)
#else
    std::atomic<coreUint8> m_State;      // atomic read count (255 = write)
#endif


public:
    coreSharedLock()noexcept : m_State () {}

    DISABLE_COPY(coreSharedLock)

    /* acquire and release the shared lock */
    FORCE_INLINE void     LockWrite();
    FORCE_INLINE void     LockRead();
    FORCE_INLINE void     UnlockWrite();
    FORCE_INLINE void     UnlockRead();
    FORCE_INLINE coreBool TryLockWrite();
    FORCE_INLINE coreBool TryLockRead();

    /* check for current lock state */
    FORCE_INLINE coreBool IsLocked()const;
};


// ****************************************************************
/* lock helper class */
template <typename T> class coreLocker final
{
private:
    T* m_pLock;   // associated lock


public:
    FORCE_INLINE explicit coreLocker(T* pLock)noexcept : m_pLock (pLock) {m_pLock->Lock();}
    FORCE_INLINE ~coreLocker()                                           {m_pLock->Unlock();}

    DISABLE_COPY(coreLocker)
    DISABLE_HEAP
};


// ****************************************************************
/* acquire the lock */
FORCE_INLINE void coreLock::Lock()
{
#if defined(CORE_LOCK_DISABLED)
    m_State = true;
#elif defined(CORE_LOCK_MUTEX)
    m_State.lock();
#else
    while(!this->TryLock()) CORE_LOCK_YIELD
#endif
}


// ****************************************************************
/* release the lock */
FORCE_INLINE void coreLock::Unlock()
{
    ASSERT(this->IsLocked())

#if defined(CORE_LOCK_DISABLED)
    m_State = false;
#elif defined(CORE_LOCK_MUTEX)
    m_State.unlock();
#else
    m_State.clear(std::memory_order::release);
#endif
}


// ****************************************************************
/* try to acquire the lock */
FORCE_INLINE coreBool coreLock::TryLock()
{
#if defined(CORE_LOCK_DISABLED)
    return m_State ? false : (m_State = true, true);
#elif defined(CORE_LOCK_MUTEX)
    return m_State.try_lock();
#else
    return !m_State.test(std::memory_order::relaxed) && !m_State.test_and_set(std::memory_order::acquire);
#endif
}


// ****************************************************************
/* check for current lock state */
FORCE_INLINE coreBool coreLock::IsLocked()const
{
#if defined(CORE_LOCK_DISABLED)
    return m_State;
#elif defined(CORE_LOCK_MUTEX)
    return m_State.try_lock() ? (m_State.unlock(), false) : true;
#else
    return m_State.test(std::memory_order::relaxed);
#endif
}


// ****************************************************************
/* acquire the recursive lock */
FORCE_INLINE void coreRecursiveLock::Lock()
{
#if defined(CORE_LOCK_DISABLED)
    m_iCount++;
#elif defined(CORE_LOCK_MUTEX)
    m_State.lock(); m_iCount++;
#else
    while(!this->TryLock()) CORE_LOCK_YIELD
#endif
}


// ****************************************************************
/* release the recursive lock */
FORCE_INLINE void coreRecursiveLock::Unlock()
{
    ASSERT(this->IsLocked())

#if defined(CORE_LOCK_DISABLED)
    m_iCount--;
#elif defined(CORE_LOCK_MUTEX)
    m_iCount--; m_State.unlock();
#else
    if(!(--m_iCount)) m_State.store(0u, std::memory_order::release);
#endif
}


// ****************************************************************
/* try to acquire the recursive lock */
FORCE_INLINE coreBool coreRecursiveLock::TryLock()
{
#if defined(CORE_LOCK_DISABLED)
    return (m_iCount++, true);
#elif defined(CORE_LOCK_MUTEX)
    return m_State.try_lock() ? (m_iCount++, true) : false;
#else

    SDL_ThreadID       iExpected = 0u;
    const SDL_ThreadID iDesired  = SDL_GetCurrentThreadID();

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
/* acquire the shared lock (exclusive) */
FORCE_INLINE void coreSharedLock::LockWrite()
{
#if defined(CORE_LOCK_DISABLED)
    m_State = true;
#elif defined(CORE_LOCK_MUTEX)
    m_State.lock();
#else
    while(!this->TryLockWrite()) CORE_LOCK_YIELD
#endif
}


// ****************************************************************
/* acquire the shared lock (shared) */
FORCE_INLINE void coreSharedLock::LockRead()
{
#if defined(CORE_LOCK_DISABLED)
    m_State = true;
#elif defined(CORE_LOCK_MUTEX)
    m_State.lock_shared();
#else
    while(!this->TryLockRead()) CORE_LOCK_YIELD
#endif
}


// ****************************************************************
/* release the shared lock (exclusive) */
FORCE_INLINE void coreSharedLock::UnlockWrite()
{
    ASSERT(this->IsLocked())

#if defined(CORE_LOCK_DISABLED)
    m_State = false;
#elif defined(CORE_LOCK_MUTEX)
    m_State.unlock();
#else
    m_State.store(0u, std::memory_order::release);
#endif
}


// ****************************************************************
/* release the shared lock (shared) */
FORCE_INLINE void coreSharedLock::UnlockRead()
{
    ASSERT(this->IsLocked())

#if defined(CORE_LOCK_DISABLED)
    m_State = false;
#elif defined(CORE_LOCK_MUTEX)
    m_State.unlock_shared();
#else
    m_State.fetch_sub(1u, std::memory_order::release);
#endif
}


// ****************************************************************
/* try to acquire the shared lock (exclusive) */
FORCE_INLINE coreBool coreSharedLock::TryLockWrite()
{
#if defined(CORE_LOCK_DISABLED)
    return m_State ? false : (m_State = true, true);
#elif defined(CORE_LOCK_MUTEX)
    return m_State.try_lock();
#else

    coreUint8 iCurrent = 0u;

    if(m_State.compare_exchange_weak(iCurrent, 255u, std::memory_order::acquire, std::memory_order::relaxed))
    {
        return true;
    }

    return false;

#endif
}


// ****************************************************************
/* try to acquire the shared lock (shared) */
FORCE_INLINE coreBool coreSharedLock::TryLockRead()
{
#if defined(CORE_LOCK_DISABLED)
    return m_State ? false : (m_State = true, true);
#elif defined(CORE_LOCK_MUTEX)
    return m_State.try_lock_shared();
#else

    coreUint8 iCurrent = m_State.load(std::memory_order::relaxed);

    while(true)
    {
        if(iCurrent == 255u)
        {
            return false;
        }

        if(m_State.compare_exchange_weak(iCurrent, iCurrent + 1u, std::memory_order::acquire, std::memory_order::relaxed))
        {
            return true;
        }
    }

#endif
}


// ****************************************************************
/* check for current lock state */
FORCE_INLINE coreBool coreSharedLock::IsLocked()const
{
#if defined(CORE_LOCK_DISABLED)
    return m_State;
#elif defined(CORE_LOCK_MUTEX)
    return m_State.try_lock() ? (m_State.unlock(), false) : true;
#else
    return m_State.load(std::memory_order::relaxed);
#endif
}


// ****************************************************************
/* user-defined deduction guides */
template<typename T> coreLocker(T*) -> coreLocker<T>;


#endif /* _CORE_GUARD_LOCK_H_ */