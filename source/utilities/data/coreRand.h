//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_RAND_H_
#define _CORE_GUARD_RAND_H_


// ****************************************************************
// random number generator class
// TODO: implement Mersenne Twister
class coreRand final
{
private:
    int* m_piRand;         //!< precalculated random numbers
    coreUint m_iNumRand;   //!< number of random numbers
    coreUint m_iCurRand;   //!< current random number


public:
    coreRand(const coreUint& iSize, int iSeed = -1)noexcept;
    coreRand(const coreRand& c)noexcept;
    coreRand(coreRand&& m)noexcept;
    ~coreRand();

    //! assignment operator
    //! @{
    coreRand& operator = (coreRand o)noexcept;
    friend void swap(coreRand& a, coreRand& b)noexcept;
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
    inline float Float(const float& fMin, const float& fMax) {constexpr_var float fFactor = 1.0f/float(RAND_MAX); return fMin + (fMax - fMin) * float(this->GetRaw()) * fFactor;}
    //! @}
};


#endif // _CORE_GUARD_RAND_H_