///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MURMUR2_H_
#define _CORE_GUARD_MURMUR2_H_


// ****************************************************************
/* Murmur2 hash function */
constexpr FUNC_LOCAL coreUint32 coreHashMurmur2(const coreByte* pData, coreUint32 iLength)
{
    constexpr coreUint32 s = 0xC58F1A7Bu;
    constexpr coreUint32 m = 0x5BD1E995u;
    constexpr coreUint32 r = 24u;

    coreUint32 iHash = s ^ iLength;

    while(iLength >= 4u)
    {
      coreUint32 k = (*r_cast<const coreUint32*>(pData));

      k *= m;
      k ^= k >> r;
      k *= m;

      iHash *= m;
      iHash ^= k;

      pData   += 4u;
      iLength -= 4u;
    }

    switch(iLength)
    {
    case 3u: iHash ^= coreUint32(pData[2]) << 16u; FALLTHROUGH
    case 2u: iHash ^= coreUint32(pData[1]) << 8u;  FALLTHROUGH
    case 1u: iHash ^= coreUint32(pData[0]);
             iHash *= m;
    };

    iHash ^= iHash >> 13u;
    iHash *= m;
    iHash ^= iHash >> 15u;

    return iHash;
}


// ****************************************************************
/* Murmur2 64-bit hash function */
constexpr FUNC_LOCAL coreUint64 coreHashMurmur64A(const coreByte* pData, coreUint64 iLength)
{
    constexpr coreUint64 s = 0xC58F1A7BC58F1A7Bu;
    constexpr coreUint64 m = 0xC6A4A7935BD1E995u;
    constexpr coreUint64 r = 47u;

    coreUint64 iHash = s ^ (iLength * m);

    while(iLength >= 8u)
    {
      coreUint64 k = (*r_cast<const coreUint64*>(pData));

      k *= m;
      k ^= k >> r;
      k *= m;

      iHash ^= k;
      iHash *= m;

      pData   += 8u;
      iLength -= 8u;
    }

    switch(iLength)
    {
    case 7u: iHash ^= coreUint64(pData[6]) << 48u; FALLTHROUGH
    case 6u: iHash ^= coreUint64(pData[5]) << 40u; FALLTHROUGH
    case 5u: iHash ^= coreUint64(pData[4]) << 32u; FALLTHROUGH
    case 4u: iHash ^= coreUint64(pData[3]) << 24u; FALLTHROUGH
    case 3u: iHash ^= coreUint64(pData[2]) << 16u; FALLTHROUGH
    case 2u: iHash ^= coreUint64(pData[1]) << 8u;  FALLTHROUGH
    case 1u: iHash ^= coreUint64(pData[0]);
             iHash *= m;
    };

    iHash ^= iHash >> r;
    iHash *= m;
    iHash ^= iHash >> r;

    return iHash;
}


#endif /* _CORE_GUARD_MURMUR2_H_ */