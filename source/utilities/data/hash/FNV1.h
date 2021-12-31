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
/* FNV-1a hash function */
constexpr FUNC_LOCAL coreUint32 coreHashFNV1(const coreByte* pData, coreUint32 iLength)
{
    coreUint32 iHash = 2166136261u;

    while(iLength)
    {
        iHash = coreUint32((coreUint64((*pData) ^ iHash) * 16777619u) & 0xFFFFFFFFu);
        ++pData;
        --iLength;
    }

    return iHash;
}

constexpr FUNC_LOCAL coreUint32 coreHashFNV1(const coreChar* pcString)
{
    coreUint32 iHash = 2166136261u;

    while(*pcString)
    {
        iHash = coreUint32((coreUint64((*pcString) ^ iHash) * 16777619u) & 0xFFFFFFFFu);
        ++pcString;
    }

    return iHash;
}


#endif /* _CORE_GUARD_FNV1_H_ */