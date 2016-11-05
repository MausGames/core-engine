//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_RAND_H_
#define _CORE_GUARD_RAND_H_

// TODO: move global random number object here ?
// TODO: do not pre-calculate global random numbers, memory access more expensive then direct calculation (only seed access)
// TODO: make size a template parameter ?
// TODO: check out Xorshift (or Xorshift+) algorithm to increase period


// ****************************************************************
/* random number definitions */
#define CORE_RAND_MAX     (0x7FFFu)                                                    //!< max raw random number (15 bit)
#define CORE_RAND_TIME    (DEFINED(_CORE_DEBUG_) ? 0u : coreUint32(std::time(NULL)))   //!< use current time to seed the generator (not on debug mode)

#define CORE_RAND_COMPILE (coreHashFNV1(__DATE__ __TIME__))


// ****************************************************************
/* random number class */
class coreRand final
{
private:
    coreInt16* m_piRandom;       //!< pre-calculated random numbers
    coreUintW  m_iNumRandoms;    //!< number of random numbers
    coreUintW  m_iCurRandom;     //!< current random number

    static coreUint32 s_iSeed;   //!< global generator seed value


public:
    explicit coreRand(const coreUintW iNumRandoms, const coreUint32 iSeed = CORE_RAND_TIME)noexcept;
    coreRand(const coreRand& c)noexcept;
    coreRand(coreRand&&      m)noexcept;
    ~coreRand();

    /*! assignment operations */
    //! @{
    coreRand& operator = (coreRand o)noexcept;
    //! @}

    /*! reset current random number */
    //! @{
    inline void Reset() {m_iCurRandom = 0u;}
    //! @}

    /*! retrieve pre-calculated random number */
    //! @{
    inline const coreInt16& Raw()                                      {if(++m_iCurRandom >= m_iNumRandoms) m_iCurRandom = 0u; return m_piRandom[m_iCurRandom];}
    inline coreInt16 Int  (const coreInt16 iMax)                       {return              (this->Raw()  % (iMax         +  1));}
    inline coreInt16 Int  (const coreInt16 iMin, const coreInt16 iMax) {return iMin +       (this->Raw()  % (iMax - iMin  +  1));}
    inline coreFloat Float(const coreFloat fMax)                       {return        I_TO_F(this->Raw()) *  fMax         * (1.0f / I_TO_F(CORE_RAND_MAX));}
    inline coreFloat Float(const coreFloat fMin, const coreFloat fMax) {return fMin + I_TO_F(this->Raw()) * (fMax - fMin) * (1.0f / I_TO_F(CORE_RAND_MAX));}
    inline coreBool  Bool ()                                           {return              (this->Raw()  & 0x0001) ? true : false;}
    //! @}

    /*! get object properties */
    //! @{
    inline const coreUintW& GetNumRandoms()const {return m_iNumRandoms;}
    //! @}

    /*! control global random number generation */
    //! @{
    static inline void      Seed(const coreUint32 iSeed = CORE_RAND_TIME) {s_iSeed = iSeed;}
    static inline coreInt16 Rand()                                        {s_iSeed = 214013u * s_iSeed + 2531011u; return (s_iSeed >> 16u) & CORE_RAND_MAX;}
    //! @}
};


#endif /* _CORE_GUARD_RAND_H_ */