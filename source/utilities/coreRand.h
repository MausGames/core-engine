//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_RAND_H_
#define _CORE_GUARD_RAND_H_


// ****************************************************************
// random number generator class
// \todo implement Mersenne Twister
class coreRand final
{
private:
    int* m_piRand;                     //!< precalculated random numbers
    coreUint m_iNumRand;               //!< number of random numbers
    coreUint m_iCurRand;               //!< current random number

    static const float c_fPrecision;   //!< floating point precision


public:
    coreRand(const coreUint& iSize, int iInit = -1);
    coreRand(const coreRand& c);
    coreRand(coreRand&& m);
    ~coreRand();

    //! assignment operators
    //! @{
    coreRand& operator = (const coreRand& c);
    coreRand& operator = (coreRand&& m);
    //! @}

    //! reset the generator
    //! @{
    inline void Reset() {m_iCurRand = 0;}
    //! @}

    //! get raw random number
    //! @{
    inline const int& GetRaw() {if(++m_iCurRand >= m_iNumRand) m_iCurRand = 0; return m_piRand[m_iCurRand];}
    //! @}

    //! calculate constrained random number
    //! @{
    inline int Int(const int& iMin, const int& iMax)         {return iMin + (this->GetRaw() % (iMax - iMin + 1));}
    inline float Float(const float& fMin, const float& fMax) {return fMin + (fMax - fMin) * float(this->GetRaw()) * c_fPrecision;}
    //! @}
};


#endif // _CORE_GUARD_RAND_H_