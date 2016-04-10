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
coreMemoryManager::coreMemoryManager()noexcept
: m_apPointer {}
{
    Core::Log->Info(CORE_LOG_BOLD("Memory Manager created"));
}


// ****************************************************************
// destructor
coreMemoryManager::~coreMemoryManager()
{
    // clear memory
    m_apPointer.clear();

    Core::Log->Info(CORE_LOG_BOLD("Memory Manager destroyed"));
}


// ****************************************************************
// constructor
coreMemoryPool::coreMemoryPool()noexcept
: m_apPageList  {}
, m_apFreeStack {}
, m_iBlockSize  (0u)
, m_iPageSize   (0u)
{
}

coreMemoryPool::coreMemoryPool(const coreUintW iBlockSize, const coreUintW iPageSize)noexcept
: m_apPageList  {}
, m_apFreeStack {}
, m_iBlockSize  (iBlockSize)
, m_iPageSize   (iPageSize)
{
    // reserve memory for the free-stack
    m_apFreeStack.reserve(iPageSize);
}


// ****************************************************************
// destructor
coreMemoryPool::~coreMemoryPool()
{
    this->Clear();
}


// ****************************************************************
// set required memory-pool properties
void coreMemoryPool::Configure(const coreUintW iBlockSize, const coreUintW iPageSize)
{
    ASSERT(m_apPageList.empty())

    // save memory-block and memory-page size
    m_iBlockSize = iBlockSize;
    m_iPageSize  = iPageSize;

    // reserve memory for the free-stack
    m_apFreeStack.reserve(iPageSize);
}


// ****************************************************************
// reset memory-pool to its initial state
void coreMemoryPool::Clear()
{
    ASSERT(m_apFreeStack.size() == (m_apPageList.size() * m_iPageSize))

    // delete all memory-pages
    FOR_EACH(it, m_apPageList)
        SAFE_DELETE_ARRAY(*it)

    // clear memory
    m_apPageList .clear();
    m_apFreeStack.clear();
}


// ****************************************************************
// add new memory-page to memory-pool
void coreMemoryPool::__AddPage()
{
    // create new memory-page
    coreByte* pNewPage = new coreByte[m_iBlockSize * m_iPageSize];
    m_apPageList.push_back(pNewPage);

    // add all containing memory-blocks to the free-stack
    for(coreUintW i = m_iPageSize; i--; )
        m_apFreeStack.push_back(pNewPage + (m_iBlockSize * i));
}
