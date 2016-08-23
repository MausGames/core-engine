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
/* compile-time FNV-1a hash function */
constexpr FUNC_NOALIAS coreUint32 coreHashCompileFNV1(const coreByte* pData, const coreUint32 iLength, const coreUint32 iCode)
{
    return iLength ? coreHashCompileFNV1(pData + 1u, iLength - 1u, ((*pData) ^ iCode) * 16777619u) : iCode;
}

constexpr FUNC_NOALIAS coreUint32 coreHashCompileFNV1(const coreByte* pData, const coreUint32 iLength)
{
    return coreHashCompileFNV1(pData, iLength, 2166136261u);
}

constexpr FUNC_NOALIAS coreUint32 coreHashCompileFNV1(const coreChar* pcString, const coreUint32 iCode)
{
    return *pcString ? coreHashCompileFNV1(pcString + 1u, ((*pcString) ^ iCode) * 16777619u) : iCode;
}

constexpr FUNC_NOALIAS coreUint32 coreHashCompileFNV1(const coreChar* pcString)
{
    return coreHashCompileFNV1(pcString, 2166136261u);
}


// ****************************************************************
/* run-time FNV-1a hash function */
inline FUNC_NOALIAS coreUint32 coreHashRunFNV1(const coreByte* pData, coreUint32 iLength)
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

inline FUNC_NOALIAS coreUint32 coreHashRunFNV1(const coreChar* pcString)
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