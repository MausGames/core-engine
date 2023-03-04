///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
coreMemoryPool::coreMemoryPool()noexcept
: m_apPageList  {}
, m_apFreeStack {}
, m_iBlockSize  (0u)
, m_iPageSize   (0u)
, m_pHeap       (NULL)
{
}

coreMemoryPool::coreMemoryPool(const coreUintW iBlockSize, const coreUintW iPageSize)noexcept
: coreMemoryPool ()
{
    this->Configure(iBlockSize, iPageSize);
}

coreMemoryPool::coreMemoryPool(coreMemoryPool&& m)noexcept
: m_apPageList  (std::move(m.m_apPageList))
, m_apFreeStack (std::move(m.m_apFreeStack))
, m_iBlockSize  (m.m_iBlockSize)
, m_iPageSize   (m.m_iPageSize)
, m_pHeap       (m.m_pHeap)
{
    m.m_pHeap = NULL;
}


// ****************************************************************
/* destructor */
coreMemoryPool::~coreMemoryPool()
{
    // reset memory-pool
    this->Reset();

    // destroy private heap object
    if(m_pHeap) coreData::HeapDestroy(m_pHeap);
}


// ****************************************************************
/* assignment operations */
coreMemoryPool& coreMemoryPool::operator = (coreMemoryPool&& m)noexcept
{
    // swap properties
    std::swap(m_apPageList,  m.m_apPageList);
    std::swap(m_apFreeStack, m.m_apFreeStack);
    std::swap(m_iBlockSize,  m.m_iBlockSize);
    std::swap(m_iPageSize,   m.m_iPageSize);
    std::swap(m_pHeap,       m.m_pHeap);

    return *this;
}


// ****************************************************************
/* set required memory-pool properties */
void coreMemoryPool::Configure(const coreUintW iBlockSize, const coreUintW iPageSize)
{
    ASSERT(iBlockSize && iPageSize)
    ASSERT(m_apPageList.empty())

    // save memory-block and memory-page size
    m_iBlockSize = iBlockSize;
    m_iPageSize  = iPageSize;

    // reserve memory for the free-stack
    m_apFreeStack.reserve(iPageSize);

    // create private heap object
    if(!m_pHeap) m_pHeap = coreData::HeapCreate(false);
}


// ****************************************************************
/* reset memory-pool to its initial state */
void coreMemoryPool::Reset()
{
    ASSERT(m_apFreeStack.size() == (m_apPageList.size() * m_iPageSize))

    // delete all memory-pages
    FOR_EACH(it, m_apPageList)
        coreData::HeapFree(m_pHeap, r_cast<void**>(&(*it)));

    // clear memory
    m_apPageList .clear();
    m_apFreeStack.clear();
}


// ****************************************************************
/* create memory-block */
RETURN_RESTRICT void* coreMemoryPool::Allocate()
{
    // check for free memory-block
    if(m_apFreeStack.empty()) this->__AddPage();
    ASSERT(!m_apFreeStack.empty())

    // get pointer from the free-stack
    void* pMemory = m_apFreeStack.back();
    m_apFreeStack.pop_back();

    return pMemory;
}


// ****************************************************************
/* remove memory-block */
void coreMemoryPool::Free(void** OUTPUT ppPointer)
{
    ASSERT(this->Contains(*ppPointer))

    // find target location with binary search
    ASSERT(std::is_sorted(m_apFreeStack.begin(), m_apFreeStack.end(), std::greater()))
    const auto it = std::lower_bound(m_apFreeStack.begin(), m_apFreeStack.end(), *ppPointer, std::greater());

    // return pointer to the free-stack
    m_apFreeStack.insert(it, *ppPointer);

    // remove reference
    (*ppPointer) = NULL;
}


// ****************************************************************
/* check if pointer belongs to the memory-pool */
coreBool coreMemoryPool::Contains(const void* pPointer)const
{
    const coreUintW iTotalSize = m_iBlockSize * m_iPageSize;
    return std::any_of(m_apPageList.begin(), m_apPageList.end(), [&](const coreByte* pPage) {return (P_TO_UI(pPointer) - P_TO_UI(pPage) < iTotalSize);});
}


// ****************************************************************
/* add new memory-page to memory-pool */
void coreMemoryPool::__AddPage()
{
    ASSERT(m_iBlockSize && m_iPageSize && m_pHeap)

    // create new memory-page
    coreByte* pNewPage = s_cast<coreByte*>(coreData::HeapMalloc(m_pHeap, m_iBlockSize * m_iPageSize));
    m_apPageList.push_back(pNewPage);

    // add all containing memory-blocks to the free-stack
    for(coreUintW i = m_iPageSize; i--; )
        m_apFreeStack.push_back(pNewPage + (m_iBlockSize * i));
}


// ****************************************************************
/* constructor */
coreMemoryManager::coreMemoryManager()noexcept
: m_apPointer   {}
, m_aMemoryPool {}
, m_PoolLock    ()
{
    Core::Log->Info(CORE_LOG_BOLD("Memory Manager created"));
}


// ****************************************************************
/* destructor */
coreMemoryManager::~coreMemoryManager()
{
    // clear memory
    m_apPointer  .clear();
    m_aMemoryPool.clear();

    Core::Log->Info(CORE_LOG_BOLD("Memory Manager destroyed"));
}


// ****************************************************************
/* create memory-block from internal memory-pool */
RETURN_RESTRICT void* coreMemoryManager::Allocate(const coreUintW iSize)
{
    coreSpinLocker oLocker(&m_PoolLock);

    // check and create memory-pool
    if(!m_aMemoryPool.count(iSize)) m_aMemoryPool.emplace(iSize, iSize, 128u);

    // forward request to internal memory-pool
    return m_aMemoryPool.at(iSize).Allocate();
}


// ****************************************************************
/* remove memory-block to internal memory-pool */
void coreMemoryManager::Free(const coreUintW iSize, void** OUTPUT ppPointer)
{
    coreSpinLocker oLocker(&m_PoolLock);

    // forward request to internal memory-pool
    m_aMemoryPool.at(iSize).Free(ppPointer);
}