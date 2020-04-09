///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_HASHSTRING_H_
#define _CORE_GUARD_HASHSTRING_H_

// TODO: add debug-logging to catch global hash-collisions (currently only in lookup-container)


// ****************************************************************
/* hash-string wrapper class */
class coreHashString final
{
private:
    coreUint32      m_iHash;      //!< calculated FNV-1a hash-value
    const coreChar* m_pcString;   //!< original string


public:
    constexpr coreHashString(std::nullptr_t  p = NULL)noexcept;
    constexpr coreHashString(const coreChar* pcString)noexcept;

    ENABLE_COPY(coreHashString)

    /*! access hash-value directly */
    //! @{
    constexpr operator coreUint32 ()const {return m_iHash;}
    //! @}

    /*! get object properties */
    //! @{
    constexpr const coreUint32& GetHash  ()const {return m_iHash;}
    constexpr const coreChar*   GetString()const {return m_pcString;}
    //! @}
};


// ****************************************************************
/* constructor */
constexpr coreHashString::coreHashString(std::nullptr_t)noexcept
: m_iHash    (0u)
, m_pcString (NULL)
{
}

constexpr coreHashString::coreHashString(const coreChar* pcString)noexcept
: m_iHash    (pcString ? coreHashFNV1(pcString) : 0u)
, m_pcString (pcString)
{
}


#endif /* _CORE_GUARD_HASHSTRING_H_ */