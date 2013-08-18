#include "Core.h"

const float coreRand::s_fPrecision = 1.0f/float(RAND_MAX);


// ****************************************************************
// constructor
coreRand::coreRand(const coreUint& iSize, int iInit)
: m_NumRand (iSize)
, m_CurRand (0)
{
#if defined (_DEBUG)
    // always the same random numbers in debug-mode
    if(iInit < 0) iInit = 0;
#endif

    // init random number generator
    srand((unsigned int)((iInit < 0) ? time(NULL) : iInit));

    // precalculate random numbers
    m_piRand = new int[m_NumRand];
    for(coreUint i = 0; i < m_NumRand; ++i)
        m_piRand[i] = rand();
}

coreRand::coreRand(const coreRand& c)
: m_piRand  (c.m_piRand)
, m_NumRand (c.m_NumRand)
, m_CurRand (c.m_CurRand)
{
}


// ****************************************************************
// destructor
coreRand::~coreRand()
{
    // delete numbers
    SAFE_DELETE_ARRAY(m_piRand)
}