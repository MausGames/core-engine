///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_RING_H_
#define _CORE_GUARD_RING_H_

// TODO 3: create a dynamic-sized version with coreList ? (replacement in coreParticleSystem)
// TODO 4: rename functions to better and more obvious names


// ****************************************************************
/* ring array class */
template <typename T, coreUintW iSize> class coreRing final : public std::array<T, iSize>
{
private:
    coreUintW m_iCurIndex;   // index of the current item


public:
    constexpr coreRing()noexcept;

    ENABLE_COPY(coreRing)

    /* switch current item */
    constexpr void select(const coreUintW iIndex) {ASSERT(iIndex < iSize)     m_iCurIndex = iIndex;}
    constexpr void next    ()                     {if(++m_iCurIndex >= iSize) m_iCurIndex = 0u;}
    constexpr void previous()                     {if(--m_iCurIndex >= iSize) m_iCurIndex = iSize - 1u;}

    /* access reference to current item */
    constexpr       T&         current()      {return (*this)[m_iCurIndex];}
    constexpr const T&         current()const {return (*this)[m_iCurIndex];}
    constexpr const coreUintW& index  ()const {return m_iCurIndex;}
};


// ****************************************************************
/* constructor */
template <typename T, coreUintW iSize> constexpr coreRing<T, iSize>::coreRing()noexcept
: std::array<T, iSize> ()
, m_iCurIndex          (0u)
{
}


#endif /* _CORE_GUARD_RING_H_ */