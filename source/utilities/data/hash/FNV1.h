//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_FNV1_H_
#define _CORE_GUARD_FNV1_H_


// ****************************************************************
/* FNV-1a hash function */
constexpr FUNC_PURE FUNC_NOALIAS coreUint32 coreHashFNV1(const coreByte* pData, coreUint32 iLength)
{
    coreUint32 iHash = 2166136261u;

    while(iLength)
    {
        iHash = ((*pData) ^ iHash) * 16777619u;
        ++pData;
        --iLength;
    }

    return iHash;
}

constexpr FUNC_PURE FUNC_NOALIAS coreUint32 coreHashFNV1(const coreChar* pcString)
{
    coreUint32 iHash = 2166136261u;

    while(*pcString)
    {
        iHash = ((*pcString) ^ iHash) * 16777619u;
        ++pcString;
    }

    return iHash;
}


#endif /* _CORE_GUARD_FNV1_H_ */