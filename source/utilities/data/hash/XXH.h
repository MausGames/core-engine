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
template <typename T> constexpr coreUint32 coreHashXXH32(const T* ptData, const coreUintW iLength, const coreUint32 iSeed = 0u)
{
    ASSERT(ptData && iLength)

    STATIC_ASSERT(sizeof(T) == 1u)

    if(std::is_constant_evaluated())
    {
        constexpr coreUint32 c1 = 0x9E3779B1u;
        constexpr coreUint32 c2 = 0x85EBCA77u;
        constexpr coreUint32 c3 = 0xC2B2AE3Du;
        constexpr coreUint32 c4 = 0x27D4EB2Fu;
        constexpr coreUint32 c5 = 0x165667B1u;

        constexpr auto nShuffle = [](const coreUint32 A, const coreUint32 B)
        {
            return coreMath::RotateLeft32(A + B * c2, 13u) * c1;
        };

        constexpr auto nRead8 = [](const T* ptData, const coreUintW iPos)
        {
            return coreUint8(ptData[iPos]);
        };

        constexpr auto nRead32 = [](const T* ptData, const coreUintW iPos)
        {
            const coreUint32 b1 = coreUint8(ptData[iPos + 0u]);
            const coreUint32 b2 = coreUint8(ptData[iPos + 1u]);
            const coreUint32 b3 = coreUint8(ptData[iPos + 2u]);
            const coreUint32 b4 = coreUint8(ptData[iPos + 3u]);

            return b1 | (b2 << 8u) | (b3 << 16u) | (b4 << 24u);
        };

        coreUint32 iHash = coreUint32(iLength);
        coreUintW  iPos  = 0u;

        if(iLength >= 16u)
        {
            coreUint32 v1 = iSeed + c1 + c2;
            coreUint32 v2 = iSeed + c2;
            coreUint32 v3 = iSeed;
            coreUint32 v4 = iSeed - c1;

            while(iLength - iPos >= 16u)
            {
                v1 = nShuffle(v1, nRead32(ptData, iPos +  0u));
                v2 = nShuffle(v2, nRead32(ptData, iPos +  4u));
                v3 = nShuffle(v3, nRead32(ptData, iPos +  8u));
                v4 = nShuffle(v4, nRead32(ptData, iPos + 12u));
                iPos += 16u;
            }

            iHash += coreMath::RotateLeft32(v1, 1u) + coreMath::RotateLeft32(v2, 7u) + coreMath::RotateLeft32(v3, 12u) + coreMath::RotateLeft32(v4, 18u);
        }
        else
        {
            iHash += iSeed + c5;
        }

        while(iLength - iPos >= 4u)
        {
            iHash = coreMath::RotateLeft32(iHash + nRead32(ptData, iPos) * c3, 17u) * c4;
            iPos += 4u;
        }

        while(iLength - iPos >= 1u)
        {
            iHash = coreMath::RotateLeft32(iHash + nRead8(ptData, iPos) * c5, 11u) * c1;
            iPos += 1u;
        }

        iHash ^= iHash >> 15u;
        iHash *= c2;
        iHash ^= iHash >> 13u;
        iHash *= c3;
        iHash ^= iHash >> 16u;

        return iHash;
    }
    else
    {
        extern coreUint32 __coreHashXXH32(const coreByte* pData, const coreUintW iLength, const coreUint32 iSeed);
        return __coreHashXXH32(r_cast<const coreByte*>(ptData), iLength, iSeed);
    }
}

constexpr coreUint32 coreHashXXH32(const coreChar* pcString, const coreUint32 iSeed = 0u)
{
    return coreHashXXH32(pcString, coreStrLenConst(pcString) + 1u, iSeed);
}


// ****************************************************************
/* XXH3_64 hash function (64-bit) */
extern coreUint64 coreHashXXH64(const coreByte* pData, const coreUintW iLength);
extern coreUint64 coreHashXXH64(const coreByte* pData, const coreUintW iLength, const coreUint64 iSeed);
extern coreUint64 coreHashXXH64(const coreChar* pcString);
extern coreUint64 coreHashXXH64(const coreChar* pcString, const coreUint64 iSeed);


// ****************************************************************
/* XXH3_128 hash function (128-bit) */
extern coreUint128 coreHashXXH128(const coreByte* pData, const coreUintW iLength);
extern coreUint128 coreHashXXH128(const coreByte* pData, const coreUintW iLength, const coreUint64 iSeed);
extern coreUint128 coreHashXXH128(const coreChar* pcString);
extern coreUint128 coreHashXXH128(const coreChar* pcString, const coreUint64 iSeed);


// ****************************************************************
/* get xxHash version string */
extern const coreChar* XXH_versionString();


#endif /* _CORE_GUARD_XXH_H_ */