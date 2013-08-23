//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
// TODO: implement mersenne twister


// ****************************************************************
// random number generator class
class coreRand
{
private:
    int* m_piRand;                     // precalculated random numbers
    coreUint m_NumRand;                // number of random numbers
    coreUint m_CurRand;                // current random number

    static const float s_fPrecision;   // floating point precision


public:
    coreRand(const coreUint& iSize, int iInit = -1);
    coreRand(const coreRand& c);
    coreRand(coreRand&& c);
    ~coreRand();

    // reset the generator
    inline void Reset() {m_CurRand = 0;}

    // get raw random number
    inline const int& GetRaw() {if(++m_CurRand >= m_NumRand) m_CurRand = 0; return m_piRand[m_CurRand];}

    // calculate constrained random number
    inline int Int(const int& iMin, const int& iMax)         {return iMin + (this->GetRaw() % (iMax - iMin + 1));}
    inline float Float(const float& fMin, const float& fMax) {return fMin + (fMax - fMin) * float(this->GetRaw()) * s_fPrecision;}
};