///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_RAND_H_
#define _CORE_GUARD_RAND_H_

// TODO 3: replace std::ldexp if possible
// TODO 4: add assertions ?, (min <= max) (though Int violates this when forwarding to Uint) (Float is bi-directional) (actually wrap-around is intended), (0.0f <= chance <= 1.0f)

// NOTE: LCG 32/15 {m_iSeed = 214013u * m_iSeed + 2531011u; return (m_iSeed >> 16u) & 0x7FFFu;}


// ****************************************************************
/* random number definitions */
#define CORE_RAND_MAX         (0xFFFFFFFFu)                                                // max raw random number (32 bit)
#define CORE_RAND_SEED        (DEFINED(_CORE_DEBUG_) ? 0u : SDL_GetPerformanceCounter())   // use current time to seed the generator (but not on debug mode)
#define CORE_RAND_SEED_ALWAYS (SDL_GetPerformanceCounter())                                // always use current time to seed the generator

#define CORE_RAND_RUNTIME     (coreUint32(SDL_GetPerformanceCounter() & 0xFFFFFFFFu))
#define CORE_RAND_COMPILE     (FORCE_COMPILE_TIME(coreHashFNV32(__DATE__ __TIME__)))


// ****************************************************************
/* random number class */
class coreRand final
{
private:
    coreUint64 m_iSeed;   // current seed value (state)


public:
    constexpr explicit coreRand(const coreUint64 iSeed = CORE_RAND_SEED)noexcept : m_iSeed (iSeed) {}

    ENABLE_COPY(coreRand)

    /* generate raw random number */
    coreUint32 Raw();

    /* generate bounded random number */
    inline coreInt32  Int  (const coreInt32  iMax)                        {return coreInt32(this->Uint(iMax));}                        // min = 0, wrap-around on negative input
    inline coreInt32  Int  (const coreInt32  iMin, const coreInt32  iMax) {return coreInt32(this->Uint(iMin, iMax));}
    inline coreUint32 Uint (const coreUint32 iMax)                        {return this->Raw() % (iMax        + 1u);}                   // min = 0u
    inline coreUint32 Uint (const coreUint32 iMin, const coreUint32 iMax) {return this->Raw() % (iMax - iMin + 1u) + iMin;}
    inline coreFloat  Float(const coreFloat  fMax)                        {return coreFloat(std::ldexp(this->Raw(), -32)) * (fMax);}   // min = 0.0f
    inline coreFloat  Float(const coreFloat  fMin, const coreFloat  fMax) {return coreFloat(std::ldexp(this->Raw(), -32)) * (fMax - fMin) + fMin;}
    inline coreBool   Bool ()                                             {return (this->Raw() & 0x01u);}
    inline coreBool   Bool (const coreFloat fChance)                      {return (this->Float(1.0f - CORE_MATH_PRECISION) < fChance);}

    /* set object properties */
    inline void SetSeed(const coreUint64 iSeed) {m_iSeed = iSeed;}

    /* get object properties */
    inline const coreUint64& GetSeed()const {return m_iSeed;}
};


// ****************************************************************
/* generate raw random number (PCG-XSH-RR 64/32) */
inline coreUint32 coreRand::Raw()
{
    const coreUint64 iCurrent = m_iSeed;
    m_iSeed = m_iSeed * 6364136223846793005u + 1442695040888963407u;

    return coreMath::RotateRight32(((iCurrent >> 18u) ^ iCurrent) >> 27u, iCurrent >> 59u);
}


#endif /* _CORE_GUARD_RAND_H_ */