///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_ALAW_H_
#define _CORE_GUARD_ALAW_H_


// ****************************************************************
/* A-law encoding lookup table */
alignas(ALIGNMENT_CACHE) static constexpr coreUint8 g_aiTableEncodeALAW[] =
{
    0u, 1u, 2u, 2u, 3u, 3u, 3u, 3u,
    4u, 4u, 4u, 4u, 4u, 4u, 4u, 4u,
    5u, 5u, 5u, 5u, 5u, 5u, 5u, 5u,
    5u, 5u, 5u, 5u, 5u, 5u, 5u, 5u,
    6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
    6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
    6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
    6u, 6u, 6u, 6u, 6u, 6u, 6u, 6u,
    7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
    7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
    7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
    7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
    7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
    7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
    7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u,
    7u, 7u, 7u, 7u, 7u, 7u, 7u, 7u
};


// ****************************************************************
/* A-law decoding lookup table */
alignas(ALIGNMENT_CACHE) static constexpr coreInt16 g_aiTableDecodeALAW[] =
{
     -5504,  -5248,  -6016,  -5760,  -4480,  -4224,  -4992,  -4736,
     -7552,  -7296,  -8064,  -7808,  -6528,  -6272,  -7040,  -6784,
     -2752,  -2624,  -3008,  -2880,  -2240,  -2112,  -2496,  -2368,
     -3776,  -3648,  -4032,  -3904,  -3264,  -3136,  -3520,  -3392,
    -22016, -20992, -24064, -23040, -17920, -16896, -19968, -18944,
    -30208, -29184, -32256, -31232, -26112, -25088, -28160, -27136,
    -11008, -10496, -12032, -11520,  -8960,  -8448,  -9984,  -9472,
    -15104, -14592, -16128, -15616, -13056, -12544, -14080, -13568,
      -344,   -328,   -376,   -360,   -280,   -264,   -312,   -296,
      -472,   -456,   -504,   -488,   -408,   -392,   -440,   -424,
       -88,    -72,   -120,   -104,    -24,     -8,    -56,    -40,
      -216,   -200,   -248,   -232,   -152,   -136,   -184,   -168,
     -1376,  -1312,  -1504,  -1440,  -1120,  -1056,  -1248,  -1184,
     -1888,  -1824,  -2016,  -1952,  -1632,  -1568,  -1760,  -1696,
      -688,   -656,   -752,   -720,   -560,   -528,   -624,   -592,
      -944,   -912,  -1008,   -976,   -816,   -784,   -880,   -848,
      5504,   5248,   6016,   5760,   4480,   4224,   4992,   4736,
      7552,   7296,   8064,   7808,   6528,   6272,   7040,   6784,
      2752,   2624,   3008,   2880,   2240,   2112,   2496,   2368,
      3776,   3648,   4032,   3904,   3264,   3136,   3520,   3392,
     22016,  20992,  24064,  23040,  17920,  16896,  19968,  18944,
     30208,  29184,  32256,  31232,  26112,  25088,  28160,  27136,
     11008,  10496,  12032,  11520,   8960,   8448,   9984,   9472,
     15104,  14592,  16128,  15616,  13056,  12544,  14080,  13568,
       344,    328,    376,    360,    280,    264,    312,    296,
       472,    456,    504,    488,    408,    392,    440,    424,
        88,     72,    120,    104,     24,      8,     56,     40,
       216,    200,    248,    232,    152,    136,    184,    168,
      1376,   1312,   1504,   1440,   1120,   1056,   1248,   1184,
      1888,   1824,   2016,   1952,   1632,   1568,   1760,   1696,
       688,    656,    752,    720,    560,    528,    624,    592,
       944,    912,   1008,    976,    816,    784,    880,    848
};


// ****************************************************************
/* A-law sample encoding function */
constexpr coreUint8 coreEncodeSampleALAW(const coreInt16 iSample)
{
    const coreUint16 iValue    = MIN(ABS(iSample), 0x7F7B);
    const coreUint8  iSign     = HAS_BIT(iSample,  15u);
    const coreUint8  iExponent = g_aiTableEncodeALAW[iValue >> 8u];
    const coreUint8  iMantissa = (iValue >> MAX(iExponent + 3u, 4u)) & 0x0Fu;

    return ((iSign << 7u) | (iExponent << 4u) | iMantissa) ^ 0xD5u;
}


// ****************************************************************
/* A-law encoding function */
inline void coreEncodeALAW(const coreByte* pInput, const coreUint32 iInputSize, coreByte** OUTPUT ppOutput, coreUint32* OUTPUT piOutputSize)
{
    const coreUint32 iCount   = iInputSize / sizeof(coreInt16);
    coreUint8*       piBuffer = new coreUint8[iCount];

    for(coreUintW i = 0u, ie = iCount; i < ie; ++i)
    {
        piBuffer[i] = coreEncodeSampleALAW(r_cast<const coreInt16*>(pInput)[i]);
    }

    (*ppOutput)     = r_cast<coreByte*>(piBuffer);
    (*piOutputSize) = iCount * sizeof(coreUint8);
}


// ****************************************************************
/* A-law decoding function */
inline void coreDecodeALAW(const coreByte* pInput, const coreUint32 iInputSize, coreByte** OUTPUT ppOutput, coreUint32* OUTPUT piOutputSize)
{
    const coreUint32 iCount   = iInputSize / sizeof(coreUint8);
    coreInt16*       piBuffer = new coreInt16[iCount];

    for(coreUintW i = 0u, ie = iCount; i < ie; ++i)
    {
        piBuffer[i] = g_aiTableDecodeALAW[pInput[i]];
    }

    (*ppOutput)     = r_cast<coreByte*>(piBuffer);
    (*piOutputSize) = iCount * sizeof(coreInt16);
}


#endif /* _CORE_GUARD_ALAW_H_ */