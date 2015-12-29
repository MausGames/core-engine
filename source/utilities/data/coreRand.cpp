//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

coreUint32 coreRand::s_iSeed = 0u;


// ****************************************************************
/* constructor */
coreRand::coreRand(const coreUintW iNumRandoms, const coreUint32 iSeed)noexcept
: m_piRandom    (NULL)
, m_iNumRandoms (iNumRandoms)
, m_iCurRandom  (0u)
{
    // init random number generator
    coreRand::Seed(iSeed);

    // pre-generate random numbers
    m_piRandom = new coreInt16[m_iNumRandoms];
    for(coreUintW i = 0u; i < m_iNumRandoms; ++i)
        m_piRandom[i] = coreRand::Rand();

    Core::Log->Info("Random Numbers (%u:%u) generated", iNumRandoms, iSeed);
}

coreRand::coreRand(const coreRand& c)noexcept
: m_piRandom    (NULL)
, m_iNumRandoms (c.m_iNumRandoms)
, m_iCurRandom  (c.m_iCurRandom)
{
    // copy random numbers
    m_piRandom = new coreInt16[m_iNumRandoms];
    std::memcpy(m_piRandom, c.m_piRandom, m_iNumRandoms * sizeof(coreUint16));
}

coreRand::coreRand(coreRand&& m)noexcept
: m_piRandom    (m.m_piRandom)
, m_iNumRandoms (m.m_iNumRandoms)
, m_iCurRandom  (m.m_iCurRandom)
{
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
    std::swap(m_piRandom,    o.m_piRandom);
    std::swap(m_iNumRandoms, o.m_iNumRandoms);
    std::swap(m_iCurRandom,  o.m_iCurRandom);
    return *this;
}