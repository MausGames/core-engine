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
coreRand::coreRand(const coreUint& iNumRandoms, const coreUint iSeed)noexcept
: m_iNumRandoms (iNumRandoms)
, m_iCurRandom  (0)
{
    // init random number generator
    coreRand::Seed(iSeed);

    // pre-generate random numbers
    m_piRandom = new int[m_iNumRandoms];
    for(coreUint i = 0; i < m_iNumRandoms; ++i)
        m_piRandom[i] = coreRand::Rand();

    Core::Log->Info("Random Numbers (%u:%u) generated", iNumRandoms, iSeed);
}

coreRand::coreRand(const coreRand& c)noexcept
: m_iNumRandoms (c.m_iNumRandoms)
, m_iCurRandom  (c.m_iCurRandom)
{
    // copy random numbers
    m_piRandom = new int[m_iNumRandoms];
    std::memcpy(m_piRandom, c.m_piRandom, m_iNumRandoms * sizeof(coreUint));
}

coreRand::coreRand(coreRand&& m)noexcept
: m_piRandom    (m.m_piRandom)
, m_iNumRandoms (m.m_iNumRandoms)
, m_iCurRandom  (m.m_iCurRandom)
{
    // reset pointer to moved random numbers
    m.m_piRandom = NULL;
}


// ****************************************************************
/* destructor */
coreRand::~coreRand()
{
    // delete random numbers
    SAFE_DELETE_ARRAY(m_piRandom)
}


// ****************************************************************
/* assignment operations */
coreRand& coreRand::operator = (coreRand o)noexcept
{
    swap(*this, o);
    return *this;
}

void swap(coreRand& a, coreRand& b)noexcept
{
    using std::swap;
    swap(a.m_piRandom,    b.m_piRandom);
    swap(a.m_iNumRandoms, b.m_iNumRandoms);
    swap(a.m_iCurRandom,  b.m_iCurRandom);
}