///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_HASHSTRING_H_
#define _CORE_GUARD_HASHSTRING_H_

// TODO 3: add debug-logging to catch global hash-collisions (currently only in map-container)
// TODO 3: get constexpr evaluation in implicit contructors working on MSVC (and GCC) (>90% of all hash-strings benefit from compile-time hashing)


// ****************************************************************
/* hash-string definitions */
#define CORE_HASHSTRING_FUNC coreHashXXH32


// ****************************************************************
/* hash-string class */
class coreHashString final
{
private:
    coreUint32      m_iHash;      // calculated hash-value
    const coreChar* m_pcString;   // original string


public:
    constexpr coreHashString(const coreChar* pcString = NULL)noexcept;

    ENABLE_COPY(coreHashString)

    /* access hash-value directly */
    constexpr operator coreUint32 ()const {return m_iHash;}

    /* get object properties */
    constexpr const coreUint32& GetHash  ()const {return m_iHash;}
    constexpr const coreChar*   GetString()const {return m_pcString;}
};


// ****************************************************************
/* constructor */
constexpr coreHashString::coreHashString(const coreChar* pcString)noexcept
: m_iHash    (pcString ? CORE_HASHSTRING_FUNC(pcString) : 0u)
, m_pcString (pcString)
{
}


#endif /* _CORE_GUARD_HASHSTRING_H_ */