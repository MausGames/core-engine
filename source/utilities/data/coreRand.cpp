//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

coreUint coreRand::s_iSeed = 0;


// ****************************************************************
/* constructor */
coreRand::coreRand(const coreUint& iSize, const coreUint iSeed)noexcept
: m_iNumRand (iSize)
, m_iCurRand (0)
{
    // init random number generator
    coreRand::Srand(iSeed);

    // pre-calculate random numbers
    m_piRand = new int[m_iNumRand];
    for(coreUint i = 0; i < m_iNumRand; ++i)
        m_piRand[i] = coreRand::Rand();
}

coreRand::coreRand(const coreRand& c)noexcept
: m_iNumRand (c.m_iNumRand)
, m_iCurRand (c.m_iCurRand)
{
    // copy random numbers
    m_piRand = new int[m_iNumRand];
    std::memcpy(m_piRand, c.m_piRand, m_iNumRand * sizeof(m_piRand[0]));
}

coreRand::coreRand(coreRand&& m)noexcept
: m_piRand   (m.m_piRand)
, m_iNumRand (m.m_iNumRand)
, m_iCurRand (m.m_iCurRand)
{
    m.m_piRand = NULL;
}


// ****************************************************************
/* destructor */
coreRand::~coreRand()
{
    // delete random numbers
    SAFE_DELETE_ARRAY(m_piRand)
}


// ****************************************************************
/* assignment operator */
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