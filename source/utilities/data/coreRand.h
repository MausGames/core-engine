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


// ****************************************************************
/* random number definitions */
#define CORE_RAND_MAX  (0x7FFF)                                         //!< max raw random number (16 bit)
#define CORE_RAND_TIME (g_bCoreDebug ? 0 : coreUint(std::time(NULL)))   //!< use current time to seed the generator (not on debug mode)


// ****************************************************************
/* random number class */
class coreRand final
{
private:
    int* m_piRand;             //!< precalculated random numbers
    coreUint m_iNumRand;       //!< number of random numbers
    coreUint m_iCurRand;       //!< current random number

    static coreUint s_iSeed;   //!< global generator seed value


public:
    coreRand(const coreUint& iSize, const coreUint iSeed = CORE_RAND_TIME)noexcept;
    coreRand(const coreRand& c)noexcept;
    coreRand(coreRand&& m)noexcept;
    ~coreRand();

    /*! assignment operator */
    //! @{
    coreRand& operator = (coreRand o)noexcept;
    friend void swap(coreRand& a, coreRand& b)noexcept;
    //! @}

    /*! reset the generator */
    //! @{
    inline void Reset() {m_iCurRand = 0;}
    //! @}

    /*! retrieve precalculated random number */
    //! @{
    inline const int& Raw()                                  {if(++m_iCurRand >= m_iNumRand) m_iCurRand = 0; return m_piRand[m_iCurRand];}
    inline int   Int  (const int&   iMax)                    {return             (this->Raw()  % (iMax         +  1));}
    inline int   Int  (const int&   iMin, const int&   iMax) {return iMin +      (this->Raw()  % (iMax - iMin  +  1));}
    inline float Float(const float& fMax)                    {return        float(this->Raw()) *  fMax         * (1.0f / float(CORE_RAND_MAX));}
    inline float Float(const float& fMin, const float& fMax) {return fMin + float(this->Raw()) * (fMax - fMin) * (1.0f / float(CORE_RAND_MAX));}
    //! @}

    /*! control global random number generation */
    //! @{
    static inline void Seed(const coreUint iSeed = CORE_RAND_TIME) {s_iSeed = iSeed;}
    static inline int  Rand()                                      {s_iSeed = 214013 * s_iSeed + 2531011; return (s_iSeed >> 16) & CORE_RAND_MAX;}
    //! @}
};


#endif /* _CORE_GUARD_RAND_H_ */