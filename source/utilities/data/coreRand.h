///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_RAND_H_
#define _CORE_GUARD_RAND_H_


// ****************************************************************
/* random number definitions */
#define CORE_RAND_MAX         (0x7FFFu)                                                    // max raw random number (15 bit)
#define CORE_RAND_SEED        (DEFINED(_CORE_DEBUG_) ? 0u : coreUint32(std::time(NULL)))   // use current time to seed the generator (but not on debug mode)
#define CORE_RAND_SEED_ALWAYS (coreUint32(std::time(NULL)))                                // always use current time to seed the generator

#define CORE_RAND_RUNTIME     (coreUint32(SDL_GetPerformanceCounter() & 0xFFFFFFFFu))
#define CORE_RAND_COMPILE     (FORCE_COMPILE_TIME(coreHashFNV1(__DATE__ __TIME__)))


// ****************************************************************
/* random number class */
class coreRand final
{
private:
    coreUint32 m_iSeed;   // current seed value


public:
    constexpr explicit coreRand(const coreUint32 iSeed = CORE_RAND_SEED)noexcept : m_iSeed (iSeed) {}

    ENABLE_COPY(coreRand)

    /* generate random numbers */
    inline coreInt16 Raw  ()                                           {m_iSeed = 214013u * m_iSeed + 2531011u; return (m_iSeed >> 16u) & CORE_RAND_MAX;}
    inline coreInt16 Int  (const coreInt16 iMax)                       {return              (this->Raw()  % (iMax         +  1));}
    inline coreInt16 Int  (const coreInt16 iMin, const coreInt16 iMax) {return iMin +       (this->Raw()  % (iMax - iMin  +  1));}
    inline coreFloat Float(const coreFloat fMax)                       {return        I_TO_F(this->Raw()) *  fMax         * (1.0f / I_TO_F(CORE_RAND_MAX));}
    inline coreFloat Float(const coreFloat fMin, const coreFloat fMax) {return fMin + I_TO_F(this->Raw()) * (fMax - fMin) * (1.0f / I_TO_F(CORE_RAND_MAX));}
    inline coreBool  Bool ()                                           {return (this->Raw() & 0x01)                                ? true : false;}
    inline coreBool  Bool (const coreFloat fChance)                    {return (this->Float(1.0f - CORE_MATH_PRECISION) < fChance) ? true : false;}

    /* set object properties */
    inline void SetSeed(const coreUint32 iSeed) {m_iSeed = iSeed;}

    /* get object properties */
    inline const coreUint32& GetSeed()const {return m_iSeed;}
};


#endif /* _CORE_GUARD_RAND_H_ */