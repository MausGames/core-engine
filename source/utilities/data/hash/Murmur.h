///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MURMUR_H_
#define _CORE_GUARD_MURMUR_H_


// ****************************************************************
/* Murmur3A hash function (x86, 32-bit) */
inline FUNC_LOCAL coreUint32 coreHashMurmur32(const coreByte* pData, const coreUint32 iLength, const coreUint32 iSeed = 0u)
{
    ASSERT(pData && iLength)

    constexpr coreUint32 c1 = 0xCC9E2D51u;
    constexpr coreUint32 c2 = 0x1B873593u;
    constexpr coreUint32 c3 = 0x85EBCA6Bu;
    constexpr coreUint32 c4 = 0xC2B2AE35u;

    const coreUintW iBlocks = iLength / 4u;

    coreUint32 h1 = iSeed;

    for(coreUintW i = 0u; i < iBlocks; ++i)
    {
        coreUint32 k1 = r_cast<const coreUint32*>(pData)[i];

        k1 *= c1;
        k1  = coreMath::RotateLeft32(k1, 15u);
        k1 *= c2;

        h1 ^= k1;
        h1  = coreMath::RotateLeft32(h1, 13u);
        h1  = h1 * 5u + 0xE6546B64u;
    }

    const coreUint8* pTail = pData + iBlocks * 4u;

    coreUint32 k1 = 0u;

    switch(iLength & 0x03u)
    {
    case 3u: k1 ^= coreUint32(pTail[2]) << 16u; FALLTHROUGH
    case 2u: k1 ^= coreUint32(pTail[1]) << 8u;  FALLTHROUGH
    case 1u: k1 ^= coreUint32(pTail[0]);
             k1 *= c1;
             k1  = coreMath::RotateLeft32(k1, 15u);
             k1 *= c2;
             h1 ^= k1;
    }

    h1 ^= iLength;

    h1 ^= h1 >> 16u;
    h1 *= c3;
    h1 ^= h1 >> 13u;
    h1 *= c4;
    h1 ^= h1 >> 16u;

    return h1;
}

inline FUNC_LOCAL coreUint32 coreHashMurmur32(const coreChar* pcString, const coreUint32 iSeed = 0u)
{
    return coreHashMurmur32(r_cast<const coreByte*>(pcString), std::strlen(pcString) + 1u, iSeed);
}


// ****************************************************************
/* Murmur2B hash function (x64, 64-bit) */
inline FUNC_LOCAL coreUint64 coreHashMurmur64(const coreByte* pData, coreUint64 iLength, const coreUint64 iSeed = 0u)
{
    ASSERT(pData && iLength)

    constexpr coreUint64 m = 0xC6A4A7935BD1E995u;
    constexpr coreUint64 r = 47u;

    coreUint64 h = iSeed ^ (iLength * m);

    while(iLength >= 8u)
    {
        coreUint64 k = (*r_cast<const coreUint64*>(pData));

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;

        pData   += 8u;
        iLength -= 8u;
    }

    switch(iLength)
    {
    case 7u: h ^= coreUint64(pData[6]) << 48u; FALLTHROUGH
    case 6u: h ^= coreUint64(pData[5]) << 40u; FALLTHROUGH
    case 5u: h ^= coreUint64(pData[4]) << 32u; FALLTHROUGH
    case 4u: h ^= coreUint64(pData[3]) << 24u; FALLTHROUGH
    case 3u: h ^= coreUint64(pData[2]) << 16u; FALLTHROUGH
    case 2u: h ^= coreUint64(pData[1]) << 8u;  FALLTHROUGH
    case 1u: h ^= coreUint64(pData[0]);
             h *= m;
    }

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}

inline FUNC_LOCAL coreUint64 coreHashMurmur64(const coreChar* pcString, const coreUint64 iSeed = 0u)
{
    return coreHashMurmur64(r_cast<const coreByte*>(pcString), std::strlen(pcString) + 1u, iSeed);
}


// ****************************************************************
/* Murmur3F hash function (x64, 128-bit) */
inline FUNC_LOCAL coreUint128 coreHashMurmur128(const coreByte* pData, const coreUint64 iLength, const coreUint32 iSeed = 0u)
{
    ASSERT(pData && iLength)

    constexpr coreUint64 c1 = 0x87C37B91114253D5u;
    constexpr coreUint64 c2 = 0x4CF5AD432745937Fu;
    constexpr coreUint64 c3 = 0xFF51AFD7ED558CCDu;
    constexpr coreUint64 c4 = 0xC4CEB9FE1A85EC53u;

    const coreUintW iBlocks = iLength / 16u;

    coreUint64 h1 = iSeed;
    coreUint64 h2 = iSeed;

    for(coreUintW i = 0u; i < iBlocks; ++i)
    {
        coreUint64 k1 = r_cast<const coreUint64*>(pData)[i * 2u];
        coreUint64 k2 = r_cast<const coreUint64*>(pData)[i * 2u + 1u];

        k1 *= c1;
        k1  = coreMath::RotateLeft64(k1, 31u);
        k1 *= c2;

        h1 ^= k1;
        h1  = coreMath::RotateLeft64(h1, 27u);
        h1 += h2;
        h1  = h1 * 5u + 0x52DCE729u;

        k2 *= c2;
        k2  = coreMath::RotateLeft64(k2, 33u);
        k2 *= c1;

        h2 ^= k2;
        h2  = coreMath::RotateLeft64(h2, 31u);
        h2 += h1;
        h2  = h2 * 5u + 0x38495AB5u;
    }

    const coreUint8* pTail = pData + iBlocks * 16u;

    coreUint64 k1 = 0u;
    coreUint64 k2 = 0u;

    switch(iLength & 0x0Fu)
    {
    case 15u: k2 ^= coreUint64(pTail[14]) << 48u; FALLTHROUGH
    case 14u: k2 ^= coreUint64(pTail[13]) << 40u; FALLTHROUGH
    case 13u: k2 ^= coreUint64(pTail[12]) << 32u; FALLTHROUGH
    case 12u: k2 ^= coreUint64(pTail[11]) << 24u; FALLTHROUGH
    case 11u: k2 ^= coreUint64(pTail[10]) << 16u; FALLTHROUGH
    case 10u: k2 ^= coreUint64(pTail[9])  << 8u;  FALLTHROUGH
    case 9u:  k2 ^= coreUint64(pTail[8]);
              k2 *= c2;
              k2  = coreMath::RotateLeft64(k2, 33u);
              k2 *= c1;
              h2 ^= k2;
              FALLTHROUGH

    case 8u:  k1 ^= coreUint64(pTail[7])  << 56u; FALLTHROUGH
    case 7u:  k1 ^= coreUint64(pTail[6])  << 48u; FALLTHROUGH
    case 6u:  k1 ^= coreUint64(pTail[5])  << 40u; FALLTHROUGH
    case 5u:  k1 ^= coreUint64(pTail[4])  << 32u; FALLTHROUGH
    case 4u:  k1 ^= coreUint64(pTail[3])  << 24u; FALLTHROUGH
    case 3u:  k1 ^= coreUint64(pTail[2])  << 16u; FALLTHROUGH
    case 2u:  k1 ^= coreUint64(pTail[1])  << 8u;  FALLTHROUGH
    case 1u:  k1 ^= coreUint64(pTail[0]);
              k1 *= c1;
              k1  = coreMath::RotateLeft64(k1, 31u);
              k1 *= c2;
              h1 ^= k1;
    }

    h1 ^= iLength;
    h2 ^= iLength;

    h1 += h2;
    h2 += h1;

    h1 ^= h1 >> 33u;
    h1 *= c3;
    h1 ^= h1 >> 33u;
    h1 *= c4;
    h1 ^= h1 >> 33u;

    h2 ^= h2 >> 33u;
    h2 *= c3;
    h2 ^= h2 >> 33u;
    h2 *= c4;
    h2 ^= h2 >> 33u;

    h1 += h2;
    h2 += h1;

    return coreUint128(h1, h2);
}

inline FUNC_LOCAL coreUint128 coreHashMurmur128(const coreChar* pcString, const coreUint32 iSeed = 0u)
{
    return coreHashMurmur128(r_cast<const coreByte*>(pcString), std::strlen(pcString) + 1u, iSeed);
}


#endif /* _CORE_GUARD_MURMUR_H_ */