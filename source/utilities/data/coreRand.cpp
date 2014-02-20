//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
coreRand::coreRand(const coreUint& iSize, int iSeed)noexcept
: m_iNumRand (iSize)
, m_iCurRand (0)
{
#if defined(_CORE_DEBUG_)

    // always the same random numbers in debug-mode
    if(iSeed < 0) iSeed = 0;

#endif

    // init random number generator
    std::srand((unsigned int)((iSeed < 0) ? std::time(NULL) : iSeed));

    // pre-calculate random numbers
    m_piRand = new int[m_iNumRand];
    for(coreUint i = 0; i < m_iNumRand; ++i)
        m_piRand[i] = std::rand();
}

coreRand::coreRand(const coreRand& c)noexcept
: m_iNumRand (c.m_iNumRand)
, m_iCurRand (c.m_iCurRand)
{
    // copy random numbers
    m_piRand = new int[m_iNumRand];
    std::memcpy(m_piRand, c.m_piRand, m_iNumRand*sizeof(m_piRand[0]));
}

coreRand::coreRand(coreRand&& m)noexcept
: m_piRand   (m.m_piRand)
, m_iNumRand (m.m_iNumRand)
, m_iCurRand (m.m_iCurRand)
{
    m.m_piRand = NULL;
}


// ****************************************************************
// destructor
coreRand::~coreRand()
{
    // delete random numbers
    SAFE_DELETE_ARRAY(m_piRand)
}


// ****************************************************************
// assignment operator
coreRand& coreRand::operator = (coreRand o)noexcept
{
    swap(*this, o);
    return *this;
}

void swap(coreRand& a, coreRand& b)noexcept
{
    using std::swap;
    swap(a.m_piRand,   b.m_piRand);
    swap(a.m_iNumRand, b.m_iNumRand);
    swap(a.m_iCurRand, b.m_iCurRand);
}