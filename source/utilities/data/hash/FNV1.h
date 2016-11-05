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
constexpr FUNC_PURE FUNC_NOALIAS coreUint32 coreHashFNV1(const coreByte* pData, const coreUint32 iLength, const coreUint32 iCode)
{
    return iLength ? coreHashFNV1(pData + 1u, iLength - 1u, ((*pData) ^ iCode) * 16777619u) : iCode;
}

constexpr FUNC_PURE FUNC_NOALIAS coreUint32 coreHashFNV1(const coreByte* pData, const coreUint32 iLength)
{
    return coreHashFNV1(pData, iLength, 2166136261u);
}

constexpr FUNC_PURE FUNC_NOALIAS coreUint32 coreHashFNV1(const coreChar* pcString, const coreUint32 iCode)
{
    return (*pcString) ? coreHashFNV1(pcString + 1u, ((*pcString) ^ iCode) * 16777619u) : iCode;
}

constexpr FUNC_PURE FUNC_NOALIAS coreUint32 coreHashFNV1(const coreChar* pcString)
{
    return coreHashFNV1(pcString, 2166136261u);
}


#endif /* _CORE_GUARD_FNV1_H_ */