///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_VARIANT_H_
#define _CORE_GUARD_VARIANT_H_


// ****************************************************************
/* light variant class */
template <coreUintW iSize> class coreVariant final
{
private:
    coreByte m_aData[iSize];   // buffer containing the stored object

#if defined(_CORE_DEBUG_)
    coreString m_sTypeName;    // current type name (for debug inspection)
    coreUint32 m_iTypeId;      // current type identifier (for run-time checks)
#endif


public:
    coreVariant() = default;
    template <typename T> coreVariant(const T& tValue)noexcept;

    ENABLE_COPY(coreVariant)

    /* access stored object */
    template <typename T> const T& Get()const;
};


// ****************************************************************
/* constructor */
template <coreUintW iSize> template <typename T> coreVariant<iSize>::coreVariant(const T& tValue)noexcept
: m_aData     {}
#if defined(_CORE_DEBUG_)
, m_sTypeName (coreData::TypeName<T>())
, m_iTypeId   (coreData::TypeId  <T>())
#endif
{
    STATIC_ASSERT(sizeof(T) <= sizeof(m_aData))
    STATIC_ASSERT(std::is_trivial_v<T>)

    std::memcpy(m_aData, &tValue, sizeof(T));
}


// ****************************************************************
/* access stored object */
template <coreUintW iSize> template <typename T> const T& coreVariant<iSize>::Get()const
{
#if defined(_CORE_DEBUG_)
    ASSERT(m_iTypeId == coreData::TypeId<T>())
#endif

    return (*r_cast<const T*>(m_aData));
}


// ****************************************************************
/* default variant types */
using coreVariant16 = coreVariant<16>;
using coreVariantW  = coreVariant<sizeof(coreUintW)>;


#endif /* _CORE_GUARD_VARIANT_H_ */