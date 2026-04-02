///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_ATOMIC_H_
#define _CORE_GUARD_ATOMIC_H_

// TODO 3: use fetch_min and fetch_max with C++26


// ****************************************************************
/* relaxed atomic class */
template <typename T> class coreAtomic final
{
private:
    std::atomic<T> m_tValue;   // atomic variable

    STATIC_ASSERT(std::atomic<T>::is_always_lock_free)


public:
    coreAtomic() = default;
    constexpr coreAtomic(const T tValue)noexcept : m_tValue (tValue) {}

    DISABLE_COPY(coreAtomic)

    /* assignment operations */
    FORCE_INLINE coreAtomic& operator = (const T tValue)noexcept;

    /* access atomic value */
    FORCE_INLINE T Get      ()const;
    FORCE_INLINE operator T ()const;

    /* change atomic value */
    FORCE_INLINE T FetchAdd(const T tValue) {return m_tValue.fetch_add(tValue, std::memory_order::relaxed);}
    FORCE_INLINE T FetchSub(const T tValue) {return m_tValue.fetch_sub(tValue, std::memory_order::relaxed);}
    FORCE_INLINE T FetchAnd(const T tValue) {return m_tValue.fetch_and(tValue, std::memory_order::relaxed);}
    FORCE_INLINE T FetchOr (const T tValue) {return m_tValue.fetch_or (tValue, std::memory_order::relaxed);}
    FORCE_INLINE T FetchXor(const T tValue) {return m_tValue.fetch_xor(tValue, std::memory_order::relaxed);}
    FORCE_INLINE T FetchMin(const T tValue) {T tPrev = m_tValue.load(std::memory_order::relaxed); while((tPrev > tValue) && !m_tValue.compare_exchange_weak(tPrev, tValue, std::memory_order::relaxed)) {} return tPrev;}
    FORCE_INLINE T FetchMax(const T tValue) {T tPrev = m_tValue.load(std::memory_order::relaxed); while((tPrev < tValue) && !m_tValue.compare_exchange_weak(tPrev, tValue, std::memory_order::relaxed)) {} return tPrev;}
    FORCE_INLINE T AddFetch(const T tValue) {return m_tValue.fetch_add(tValue, std::memory_order::relaxed) + tValue;}
    FORCE_INLINE T SubFetch(const T tValue) {return m_tValue.fetch_sub(tValue, std::memory_order::relaxed) - tValue;}
    FORCE_INLINE T AndFetch(const T tValue) {return m_tValue.fetch_and(tValue, std::memory_order::relaxed) & tValue;}
    FORCE_INLINE T OrFetch (const T tValue) {return m_tValue.fetch_or (tValue, std::memory_order::relaxed) | tValue;}
    FORCE_INLINE T XorFetch(const T tValue) {return m_tValue.fetch_xor(tValue, std::memory_order::relaxed) ^ tValue;}
    FORCE_INLINE T MinFetch(const T tValue) {T tPrev = m_tValue.load(std::memory_order::relaxed); while((tPrev > tValue) && !m_tValue.compare_exchange_weak(tPrev, tValue, std::memory_order::relaxed)) {} return MIN(tPrev, tValue);}
    FORCE_INLINE T MaxFetch(const T tValue) {T tPrev = m_tValue.load(std::memory_order::relaxed); while((tPrev < tValue) && !m_tValue.compare_exchange_weak(tPrev, tValue, std::memory_order::relaxed)) {} return MAX(tPrev, tValue);}
    FORCE_INLINE T Exchange(const T tValue) {return m_tValue.exchange (tValue, std::memory_order::relaxed);}

    /* change atomic value with operator */
    FORCE_INLINE void operator += (const T tValue) {this->FetchAdd(tValue);}   // do not return anything
    FORCE_INLINE void operator -= (const T tValue) {this->FetchSub(tValue);}
    FORCE_INLINE void operator &= (const T tValue) {this->FetchAnd(tValue);}
    FORCE_INLINE void operator |= (const T tValue) {this->FetchOr (tValue);}
    FORCE_INLINE void operator ^= (const T tValue) {this->FetchXor(tValue);}
};


// ****************************************************************
/* assignment operations */
template <typename T> FORCE_INLINE coreAtomic<T>& coreAtomic<T>::operator = (const T tValue)noexcept
{
    m_tValue.store(tValue, std::memory_order::relaxed);
    return *this;
}


// ****************************************************************
/* access atomic value */
template <typename T> FORCE_INLINE T coreAtomic<T>::Get()const
{
    return m_tValue.load(std::memory_order::relaxed);
}

template <typename T> FORCE_INLINE coreAtomic<T>::operator T ()const
{
    return m_tValue.load(std::memory_order::relaxed);
}


#endif /* _CORE_GUARD_ATOMIC_H_ */