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

    ENABLE_COPY(coreRound)

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
: m_atItem    {}
, m_iCurIndex (0u)
{
}


#endif /* _CORE_GUARD_ARRAY_H_ */