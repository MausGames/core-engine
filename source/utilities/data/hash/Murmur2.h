//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
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
    case 3u: iHash ^= pData[2] << 16u;
    case 2u: iHash ^= pData[1] << 8u;
    case 1u: iHash ^= pData[0];
             iHash *= m;
    };

    iHash ^= iHash >> 13u;
    iHash *= m;
    iHash ^= iHash >> 15u;

    return iHash;
}


#endif /* _CORE_GUARD_MURMUR2_H_ */