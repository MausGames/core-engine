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

#if defined(_CORE_MSVC_)
    #pragma warning(disable : 4505)   // unreferenced function with internal linkage removed
#endif

#include <xxhash.h>


// ****************************************************************
/* XXH32 hash function (32-bit) */
coreUint32 __coreHashXXH32(const coreByte* pData, const coreUintW iLength, const coreUint32 iSeed)
{
    return XXH32(pData, iLength, iSeed);
}


// ****************************************************************
/* XXH3_64 hash function (64-bit) */
coreUint64 coreHashXXH64(const coreByte* pData, const coreUintW iLength)
{
    return XXH3_64bits(pData, iLength);
}

coreUint64 coreHashXXH64(const coreByte* pData, const coreUintW iLength, const coreUint64 iSeed)
{
    return XXH3_64bits_withSeed(pData, iLength, iSeed);
}

coreUint64 coreHashXXH64(const coreChar* pcString)
{
    return coreHashXXH64(r_cast<const coreByte*>(pcString), std::strlen(pcString) + 1u);
}

coreUint64 coreHashXXH64(const coreChar* pcString, const coreUint64 iSeed)
{
    return coreHashXXH64(r_cast<const coreByte*>(pcString), std::strlen(pcString) + 1u, iSeed);
}


// ****************************************************************
/* XXH3_128 hash function (128-bit) */
coreUint128 coreHashXXH128(const coreByte* pData, const coreUintW iLength)
{
    return std::bit_cast<coreUint128>(XXH3_128bits(pData, iLength));
}

coreUint128 coreHashXXH128(const coreByte* pData, const coreUintW iLength, const coreUint64 iSeed)
{
    return std::bit_cast<coreUint128>(XXH3_128bits_withSeed(pData, iLength, iSeed));
}

coreUint128 coreHashXXH128(const coreChar* pcString)
{
    return coreHashXXH128(r_cast<const coreByte*>(pcString), std::strlen(pcString) + 1u);
}

coreUint128 coreHashXXH128(const coreChar* pcString, const coreUint64 iSeed)
{
    return coreHashXXH128(r_cast<const coreByte*>(pcString), std::strlen(pcString) + 1u, iSeed);
}


// ****************************************************************
/* get xxHash version string */
const coreChar* XXH_versionString()
{
    return STRING(XXH_VERSION_MAJOR) "." STRING(XXH_VERSION_MINOR) "." STRING(XXH_VERSION_RELEASE);
}