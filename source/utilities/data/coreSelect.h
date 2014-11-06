//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SELECT_H_
#define _CORE_GUARD_SELECT_H_

// TODO: create a dynamic-sized version with std::vector ? (replacement in coreRand and coreParticleSystem)
// TODO: copy+move ?


// ****************************************************************
/* extended array class */
template <typename T, coreUint iSize> class coreSelect final
{
private:
    std::array<T, iSize> m_aItem;   //!< static item container
    coreUint m_iCurIndex;           //!< index of the current item


public:
    coreSelect()noexcept                        : m_iCurIndex (0) {}
    explicit coreSelect(const T& Value)noexcept : m_iCurIndex (0) {m_aItem.fill(Value);}

    /*! switch current item */
    //! @{
    inline void Select(const coreUint& iIndex) {ASSERT(iIndex < iSize)     m_iCurIndex = CLAMP(iIndex, 0u, iSize - 1);}
    inline void Next    ()                     {if(++m_iCurIndex >= iSize) m_iCurIndex = 0;}
    inline void Previous()                     {if(--m_iCurIndex >= iSize) m_iCurIndex = iSize - 1;}
    //! @}

    /*! access reference to current item */
    //! @{
    inline       T& GetCur()      {return m_aItem[m_iCurIndex];}
    inline const T& GetCur()const {return m_aItem[m_iCurIndex];}
    //! @}

    /*! access item container directly */
    //! @{
    inline operator       T* ()                    {return &m_aItem.front();}
    inline operator const T* ()const               {return &m_aItem.front();}
    inline       std::array<T, iSize>* List()      {return &m_aItem;}
    inline const std::array<T, iSize>* List()const {return &m_aItem;}
    //! @}


private:
    DISABLE_COPY(coreSelect)
};


#endif /* _CORE_GUARD_SELECT_H_ */