///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_XXH_H_
#define _CORE_GUARD_XXH_H_


// ****************************************************************
/* XXH32 hash function (32-bit) */
FUNC_LOCAL coreUint32 coreHashXXH32(const coreByte* pData, const coreUintW iLength, const coreUint32 iSeed = 0u);


// ****************************************************************
/* XXH3_64 hash function (64-bit) */
FUNC_LOCAL coreUint64 coreHashXXH64(const coreByte* pData, const coreUintW iLength);
FUNC_LOCAL coreUint64 coreHashXXH64(const coreByte* pData, const coreUintW iLength, const coreUint64 iSeed);


// ****************************************************************
/* XXH3_128 hash function (128-bit) */
FUNC_LOCAL coreUint128 coreHashXXH128(const coreByte* pData, const coreUintW iLength);
FUNC_LOCAL coreUint128 coreHashXXH128(const coreByte* pData, const coreUintW iLength, const coreUint64 iSeed);


// ****************************************************************
/* get xxHash version string */
const coreChar* XXH_versionString();


#endif /* _CORE_GUARD_XXH_H_ */