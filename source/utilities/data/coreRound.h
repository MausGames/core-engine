//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_ARRAY_H_
#define _CORE_GUARD_ARRAY_H_

// TODO: create a dynamic-sized version with std::vector ? (replacement in coreRand and coreParticleSystem)


// ****************************************************************
/* round-robin array class */
template <typename T, coreUintW iSize> class coreRound final
{
private:
    std::array<T, iSize> m_atItem;   //!< static item container
    coreUintW m_iCurIndex;           //!< index of the current item


public:
    coreRound()noexcept;
    coreRound(const coreRound<T, iSize>& c)noexcept;
    coreRound(coreRound<T, iSize>&&      m)noexcept;

    /*! assignment operations */
    //! @{
    coreRound<T, iSize>& operator = (coreRound<T, iSize> o)noexcept;
    static void swap(coreRound<T, iSize>& a, coreRound<T, iSize>& b)noexcept;
    //! @}

    /*! switch current item */
    //! @{
    inline void Select(const coreUintW& iIndex) {ASSERT(iIndex < iSize)     m_iCurIndex = CLAMP(iIndex, 0u, iSize - 1u);}
    inline void Next    ()                      {if(++m_iCurIndex >= iSize) m_iCurIndex = 0u;}
    inline void Previous()                      {if(--m_iCurIndex >= iSize) m_iCurIndex = iSize - 1u;}
    //! @}

    /*! access reference to current item */
    //! @{
    inline       T&        Current()      {return m_atItem[m_iCurIndex];}
    inline const T&        Current()const {return m_atItem[m_iCurIndex];}
    inline const coreUintW Index  ()const {return m_iCurIndex;}
    //! @}

    /*! modify the item container */
    //! @{
    inline void Fill(const T& tValue) {m_atItem.fill(tValue);}
    //! @}

    /*! access item container directly */
    //! @{
    inline operator       T* ()                    {return &m_atItem.front();}
    inline operator const T* ()const               {return &m_atItem.front();}
    inline       std::array<T, iSize>* List()      {return &m_atItem;}
    inline const std::array<T, iSize>* List()const {return &m_atItem;}
    //! @}
};


// ****************************************************************
/* constructor */
template <typename T, coreUintW iSize> coreRound<T, iSize>::coreRound()noexcept
: m_iCurIndex (0u)
{
}

template <typename T, coreUintW iSize> coreRound<T, iSize>::coreRound(const coreRound<T, iSize>& c)noexcept
: m_atItem    (c.m_atItem)
, m_iCurIndex (c.m_iCurIndex)
{
}

template <typename T, coreUintW iSize> coreRound<T, iSize>::coreRound(coreRound<T, iSize>&& m)noexcept
: m_atItem    (std::move(m.m_atItem))
, m_iCurIndex (m.m_iCurIndex)
{
}


// ****************************************************************
/* assignment operations */
template <typename T, coreUintW iSize> coreRound<T, iSize>& coreRound<T, iSize>::operator = (coreRound<T, iSize> o)noexcept
{
    swap(*this, o);
    return *this;
}

template <typename T, coreUintW iSize> void coreRound<T, iSize>::swap(coreRound<T, iSize>& a, coreRound<T, iSize>& b)noexcept
{
    std::swap(a.m_atItem,    b.m_atItem);
    std::swap(a.m_iCurIndex, b.m_iCurIndex);
}


// ****************************************************************
/* swap specialization */
namespace std {template<typename T, coreUintW iSize> inline void swap(coreRound<T, iSize>& a, coreRound<T, iSize>& b) {coreRound<T, iSize>::swap(a, b);}}


#endif /* _CORE_GUARD_ARRAY_H_ */