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

// TODO 4: rename functions to better and more obvious names


// ****************************************************************
/* generic ring container class */
template <typename T, typename S, std::unsigned_integral U> class coreRingGen final : public S
{
private:
    U m_iCurIndex;   // index of the current item


public:
    constexpr coreRingGen()noexcept;

    ENABLE_COPY(coreRingGen)

    /* switch current item */
    constexpr void select  (const U iIndex) {ASSERT(iIndex < this->size())     m_iCurIndex = iIndex;}
    constexpr void next    ()               {if(++m_iCurIndex >= this->size()) m_iCurIndex = 0u;}
    constexpr void previous()               {if(--m_iCurIndex >= this->size()) m_iCurIndex = this->size() - 1u;}

    /* access reference to current item */
    constexpr       T& current()      {return (*this)[m_iCurIndex];}
    constexpr const T& current()const {return (*this)[m_iCurIndex];}
    constexpr const U& index  ()const {return m_iCurIndex;}
};


// ****************************************************************
/* constructor */
template <typename T, typename S, std::unsigned_integral U> constexpr coreRingGen<T, S, U>::coreRingGen()noexcept
: S           ()
, m_iCurIndex (0u)
{
}


// ****************************************************************
/* default ring container types */
template <typename T, coreUintW iSize> using coreRing    = coreRingGen<T, std::array<T, iSize>, TIGHTEST_UINT_TYPE(iSize)>;
template <typename T>                  using coreRingDyn = coreRingGen<T, coreList<T>,          coreUintW>;


#endif /* _CORE_GUARD_RING_H_ */