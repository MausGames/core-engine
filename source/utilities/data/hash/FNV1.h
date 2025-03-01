///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_FNV1_H_
#define _CORE_GUARD_FNV1_H_


// ****************************************************************
/* FNV-1a hash function (32-bit) */
constexpr coreUint32 coreHashFNV32(const coreByte* pData, coreUint32 iLength)
{
    ASSERT(pData && iLength)

    coreUint32 iHash = 2166136261u;

    while(iLength)
    {
        iHash = ((*pData) ^ iHash) * 16777619u;
        ++pData;
        --iLength;
    }

    return iHash;
}

constexpr coreUint32 coreHashFNV32(const coreChar* pcString)
{
    ASSERT(pcString)

    coreUint32 iHash = 2166136261u;

    do
    {
        iHash = ((*pcString) ^ iHash) * 16777619u;
    }
    while(*(pcString++));

    return iHash;
}


// ****************************************************************
/* FNV-1a hash function (64-bit) */
constexpr coreUint64 coreHashFNV64(const coreByte* pData, coreUint64 iLength)
{
    ASSERT(pData && iLength)

    coreUint64 iHash = 14695981039346656037u;

    while(iLength)
    {
        iHash = ((*pData) ^ iHash) * 1099511628211u;
        ++pData;
        --iLength;
    }

    return iHash;
}

constexpr coreUint64 coreHashFNV64(const coreChar* pcString)
{
    ASSERT(pcString)

    coreUint64 iHash = 14695981039346656037u;

    do
    {
        iHash = ((*pcString) ^ iHash) * 1099511628211u;
    }
    while(*(pcString++));

    return iHash;
}


#endif /* _CORE_GUARD_FNV1_H_ */