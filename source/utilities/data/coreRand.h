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
// TODO: make size a template parameter ?
// https://software.intel.com/en-us/articles/fast-random-number-generator-on-the-intel-pentiumr-4-processor/


// ****************************************************************
/* random number definitions */
#define CORE_RAND_MAX  (0x7FFF)                                                   //!< max raw random number (16 bit)
#define CORE_RAND_TIME (DEFINED(_CORE_DEBUG_) ? 0u : coreUint(std::time(NULL)))   //!< use current time to seed the generator (not on debug mode)


// ****************************************************************
/* random number class */
class coreRand final
{
private:
    int* m_piRandom;           //!< pre-calculated random numbers
    coreUint m_iNumRandoms;    //!< number of random numbers
    coreUint m_iCurRandom;     //!< current random number

    static coreUint s_iSeed;   //!< global generator seed value


public:
    coreRand(const coreUint& iNumRandoms, const coreUint iSeed = CORE_RAND_TIME)noexcept;
    coreRand(const coreRand& c)noexcept;
    coreRand(coreRand&&      m)noexcept;
    ~coreRand();

    /*! assignment operations */
    //! @{
    coreRand& operator = (coreRand o)noexcept;
    friend void swap(coreRand& a, coreRand& b)noexcept;
    //! @}

    /*! reset current random number */
    //! @{
    inline void Reset() {m_iCurRandom = 0;}
    //! @}

    /*! retrieve pre-calculated random number */
    //! @{
    inline const int& Raw()                                  {if(++m_iCurRandom >= m_iNumRandoms) m_iCurRandom = 0; return m_piRandom[m_iCurRandom];}
    inline int   Int  (const int&   iMax)                    {return              (this->Raw()  % (iMax         +  1));}
    inline int   Int  (const int&   iMin, const int&   iMax) {return iMin +       (this->Raw()  % (iMax - iMin  +  1));}
    inline float Float(const float& fMax)                    {return        I_TO_F(this->Raw()) *  fMax         * (1.0f / I_TO_F(CORE_RAND_MAX));}
    inline float Float(const float& fMin, const float& fMax) {return fMin + I_TO_F(this->Raw()) * (fMax - fMin) * (1.0f / I_TO_F(CORE_RAND_MAX));}
    //! @}

    /*! get object properties */
    //! @{
    inline const coreUint& GetNumRandoms()const {return m_iNumRandoms;}
    //! @}

    /*! control global random number generation */
    //! @{
    static inline void Seed(const coreUint iSeed = CORE_RAND_TIME) {s_iSeed = iSeed;}
    static inline int  Rand()                                      {s_iSeed = 214013 * s_iSeed + 2531011; return (s_iSeed >> 16) & CORE_RAND_MAX;}
    //! @}
};


#endif /* _CORE_GUARD_RAND_H_ */