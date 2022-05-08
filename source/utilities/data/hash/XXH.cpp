///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

#define XXH_INLINE_ALL
#define XXH_NO_STREAM
#define XXH_SIZE_OPT 1
#if defined(_CORE_DEBUG_)
    #define XXH_DEBUGLEVEL 1
#endif

#include <xxhash.h>


// ****************************************************************
/* XXH32 hash function (32-bit) */
FUNC_LOCAL coreUint32 coreHashXXH32(const coreByte* pData, const coreUintW iLength, const coreUint32 iSeed)
{
    return XXH32(pData, iLength, iSeed);
}


// ****************************************************************
/* XXH3_64 hash function (64-bit) */
FUNC_LOCAL coreUint64 coreHashXXH64(const coreByte* pData, const coreUintW iLength)
{
    return XXH3_64bits(pData, iLength);
}

FUNC_LOCAL coreUint64 coreHashXXH64(const coreByte* pData, const coreUintW iLength, const coreUint64 iSeed)
{
    return XXH3_64bits_withSeed(pData, iLength, iSeed);
}


// ****************************************************************
/* XXH3_128 hash function (128-bit) */
FUNC_LOCAL coreUint128 coreHashXXH128(const coreByte* pData, const coreUintW iLength)
{
    return std::bit_cast<coreUint128>(XXH3_128bits(pData, iLength));
}

FUNC_LOCAL coreUint128 coreHashXXH128(const coreByte* pData, const coreUintW iLength, const coreUint64 iSeed)
{
    return std::bit_cast<coreUint128>(XXH3_128bits_withSeed(pData, iLength, iSeed));
}


// ****************************************************************
/* get xxHash version string */
const coreChar* XXH_versionString()
{
    return STRING(XXH_VERSION_MAJOR) "." STRING(XXH_VERSION_MINOR) "." STRING(XXH_VERSION_RELEASE);
}