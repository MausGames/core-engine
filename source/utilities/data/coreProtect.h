///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_PROTECT_H_
#define _CORE_GUARD_PROTECT_H_

// TODO: put OUTPUT on the back ? only when fresh filled not edited
// TODO: function protection
// TODO: add second dummy location which also changes, but different, random selector, if this changes break


#if defined(_CORE_DEBUG_) || defined(_CORE_MOBILE_) || 1


// ****************************************************************
/* protection disabled */
template <typename T> using coreProtect = T;


#else


// ****************************************************************
/*  */ 
template <typename T> class coreProtect
{
private:
    /*  */ 
    struct coreStorage final
    {
        union
        {
            T        m_tValue;            // 
            coreByte m_aValueMemory[1];   // 
        };
        union
        {
            coreUint32* m_piChecksum;           // 
            coreByte    m_aChecksumMemory[1];   // 
        };
    };

    static constexpr coreUintW CHECKSUM_SIZE = MAX(sizeof(coreUint32*), alignof(T));                   // 
    static constexpr coreUintW VALUE_SIZE    = sizeof(coreStorage) - CHECKSUM_SIZE;   // with padding 


private:
    mutable coreStorage m_Storage;   // protected data storage (should never contain readable data)


public:
    FORCE_INLINE coreProtect(const T& tValue = T())noexcept;
    FORCE_INLINE coreProtect(const coreProtect<T>& c)noexcept;
    FORCE_INLINE coreProtect(coreProtect<T>&& m)noexcept;
    FORCE_INLINE ~coreProtect();

    /* assignment operations */
    FORCE_INLINE coreProtect<T>& operator = (coreProtect<T> o)noexcept;

    FORCE_INLINE operator T () const;

    FORCE_INLINE T operator  = (const T& tValue);
    FORCE_INLINE T operator += (const T& tValue);
    FORCE_INLINE T operator -= (const T& tValue);
    FORCE_INLINE T operator *= (const T& tValue);
    FORCE_INLINE T operator /= (const T& tValue);


private:
    FORCE_INLINE void Protect  (coreStorage* OUTPUT pStorage);
    FORCE_INLINE void Unprotect(coreStorage* OUTPUT pStorage) const;

    static FORCE_INLINE coreUint32 CalculateChecksum(const coreStorage* pStorage);
    static FORCE_INLINE coreBool   IsChecksumValid  (const coreStorage* pStorage);

    static FORCE_INLINE FUNC_NOALIAS void ScrambleMemory  (coreByte* OUTPUT pMemory, const coreUintW iSize, const coreByte oKey);
    static FORCE_INLINE FUNC_NOALIAS void UnscrambleMemory(coreByte* OUTPUT pMemory, const coreUintW iSize, const coreByte oKey);
};


// ****************************************************************
/*  */ 
template <typename T> FORCE_INLINE coreProtect<T>::coreProtect(const T& tValue)noexcept
: m_Storage {}
{
    coreStorage tTemp;
    tTemp.m_tValue = tValue;
    tTemp.m_piChecksum = new coreUint32();

    this->Protect(&tTemp);
}

template <typename T> FORCE_INLINE coreProtect<T>::coreProtect(const coreProtect<T>& c)noexcept
: coreProtect ((T)c)
{
}

template <typename T> FORCE_INLINE coreProtect<T>::coreProtect(coreProtect<T>&& m)noexcept
: m_Storage (m.m_Storage)
{
    std::memset(&m.m_Storage, 0, sizeof(m.m_Storage));
}


// ****************************************************************
/*  */ 
template <typename T> FORCE_INLINE coreProtect<T>::~coreProtect()
{
    if(m_Storage.m_piChecksum)
    {
        coreStorage tTemp;
        this->Unprotect(&tTemp);

        SAFE_DELETE(tTemp.m_piChecksum);
    }
}


// ****************************************************************
/*  */ 
template <typename T> FORCE_INLINE coreProtect<T>::operator T () const
{
    coreStorage tTemp;
    this->Unprotect(&tTemp);

    return tTemp.m_tValue;
}


// ****************************************************************
/*  */ 
template <typename T> FORCE_INLINE coreProtect<T>& coreProtect<T>::operator = (coreProtect<T> o)noexcept
{
    std::swap(m_Storage, o.m_Storage);
    return *this;
}


// ****************************************************************
/*  */ 
#define __PROTECTED_OPERATOR(f)           \
{                                         \
    coreStorage tTemp;                    \
    this->Unprotect(&tTemp);              \
                                          \
    const T tReturn = (tTemp.m_tValue f); \
                                          \
    this->Protect(&tTemp);                \
    return tReturn;                       \
}

template <typename T> FORCE_INLINE T coreProtect<T>::operator = (const T& tValue)
{
    __PROTECTED_OPERATOR(= tValue)
}

template <typename T> FORCE_INLINE T coreProtect<T>::operator += (const T& tValue)
{
    __PROTECTED_OPERATOR(+= tValue)
}

template <typename T> FORCE_INLINE T coreProtect<T>::operator -= (const T& tValue)
{
    __PROTECTED_OPERATOR(-= tValue)
}

template <typename T> FORCE_INLINE T coreProtect<T>::operator *= (const T& tValue)
{
    __PROTECTED_OPERATOR(*= tValue)
}

template <typename T> FORCE_INLINE T coreProtect<T>::operator /= (const T& tValue)
{
    __PROTECTED_OPERATOR(/= tValue)
}

#undef __PROTECTED_OPERATOR


// ****************************************************************
/*  */ 
template <typename T> FORCE_INLINE void coreProtect<T>::Protect(coreStorage* OUTPUT pStorage)
{
    (*pStorage->m_piChecksum) = CalculateChecksum(pStorage);

    ScrambleMemory(pStorage->m_aValueMemory,    VALUE_SIZE,    (*pStorage->m_piChecksum)      & 0xFFu);
    ScrambleMemory(pStorage->m_aChecksumMemory, CHECKSUM_SIZE, ( pStorage->m_aValueMemory[0]) & 0xFFu);

    m_Storage = (*pStorage);
}


// ****************************************************************
/*  */ 
template <typename T> FORCE_INLINE void coreProtect<T>::Unprotect(coreStorage* OUTPUT pStorage) const
{
    (*pStorage) = m_Storage;

    UnscrambleMemory(pStorage->m_aChecksumMemory, CHECKSUM_SIZE, ( pStorage->m_aValueMemory[0]) & 0xFFu);
    UnscrambleMemory(pStorage->m_aValueMemory,    VALUE_SIZE,    (*pStorage->m_piChecksum)      & 0xFFu);

    if(!IsChecksumValid(pStorage))
        std::abort();

#if defined(_CORE_WINDOWS_)

    #if 0

        if(IsDebuggerPresent())
            std::abort();

    #else

        #pragma message("Warning: Protection disabled!")

    #endif

#endif
}


// ****************************************************************
/*  */ 
template <typename T> FORCE_INLINE coreUint32 coreProtect<T>::CalculateChecksum(const coreStorage* pStorage)
{
    return coreHashFNV1(pStorage->m_aValueMemory, VALUE_SIZE);
}


// ****************************************************************
/*  */ 
template <typename T> FORCE_INLINE coreBool coreProtect<T>::IsChecksumValid(const coreStorage* pStorage)
{
    return (*pStorage->m_piChecksum) == CalculateChecksum(pStorage);
}


// ****************************************************************
/*  */ 
template <typename T> FORCE_INLINE FUNC_NOALIAS void coreProtect<T>::ScrambleMemory(coreByte* OUTPUT pMemory, const coreUintW iSize, const coreByte oKey)
{
    ASSERT(pMemory)

    pMemory[iSize - 1u] ^= oKey;
    for(coreUintW i = iSize - 1u; i > 0u; --i)
    {
        pMemory[i - 1u] ^= (pMemory[i] + oKey) & 0xFFu;
    }
}


// ****************************************************************
/*  */ 
template <typename T> FORCE_INLINE FUNC_NOALIAS void coreProtect<T>::UnscrambleMemory(coreByte* OUTPUT pMemory, const coreUintW iSize, const coreByte oKey)
{
    ASSERT(pMemory)

    for(coreUintW i = 1u; i < iSize; ++i)
    {
        pMemory[i - 1u] ^= (pMemory[i] + oKey) & 0xFFu;
    }
    pMemory[iSize - 1u] ^= oKey;
}


#endif


#endif /* _CORE_GUARD_PROTECT_H_ */