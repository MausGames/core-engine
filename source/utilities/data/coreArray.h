///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_ARRAY_H_
#define _CORE_GUARD_ARRAY_H_

// TODO: create a dynamic-sized version with std::vector ? (replacement in coreParticleSystem)
// TODO: rename class and functions to better and more obvious names


// ****************************************************************
/* extended array class */
template <typename T, coreUintW iSize> class coreArray final : public std::array<T, iSize>
{
private:
    coreUintW m_iCurIndex;   //!< index of the current item


public:
    constexpr coreArray()noexcept;

    ENABLE_COPY(coreArray)

    /*! switch current item */
    //! @{
    inline void select(const coreUintW iIndex) {ASSERT(iIndex < iSize)     m_iCurIndex = iIndex;}
    inline void next    ()                     {if(++m_iCurIndex >= iSize) m_iCurIndex = 0u;}
    inline void previous()                     {if(--m_iCurIndex >= iSize) m_iCurIndex = iSize - 1u;}
    //! @}

    /*! access reference to current item */
    //! @{
    inline       T&         current()      {return (*this)[m_iCurIndex];}
    inline const T&         current()const {return (*this)[m_iCurIndex];}
    inline const coreUintW& index  ()const {return m_iCurIndex;}
    //! @}
};


// ****************************************************************
/* constructor */
template <typename T, coreUintW iSize> constexpr coreArray<T, iSize>::coreArray()noexcept
: std::array<T, iSize> ()
, m_iCurIndex          (0u)
{
}


#endif /* _CORE_GUARD_ARRAY_H_ */