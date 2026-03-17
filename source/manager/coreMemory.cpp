///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

alignas(ALIGNMENT_PAGE) static coreByte s_aaTempBuffer [CORE_MEMORY_TEMP_NUM][CORE_MEMORY_TEMP_SIZE] = {};
static coreUintW                        s_aiTempPointer[CORE_MEMORY_TEMP_NUM]                        = {};
static THREAD_LOCAL coreUintW           s_iTempIndex                                                 = SIZE_MAX;


// ****************************************************************
/* initialize temp-storage */
void coreTempStorageInit(const coreUintW iIndex)
{
    ASSERT(iIndex < CORE_MEMORY_TEMP_NUM)
    s_iTempIndex = iIndex;
}


// ****************************************************************
/* allocate memory from the temp-storage */
void* coreTempStorageAllocate(const coreUintW iSize, const coreUintW iAlign)
{
    ASSERT(s_iTempIndex != SIZE_MAX)
    ASSERT(iSize && iAlign)

    const coreUintW I = s_iTempIndex;

    // select assigned temp-storage
    coreByte*  B = s_aaTempBuffer [I];
    coreUintW& P = s_aiTempPointer[I];

    // calculate address range
    const coreUintW iStart = coreMath::CeilAlign(P, iAlign);
    const coreUintW iEnd   = iStart + iSize;

    // check if allocation fits into the temp-storage
    if(iEnd <= CORE_MEMORY_TEMP_SIZE)
    {
        // adjust temp-storage pointer
        P = iEnd;
        return B + iStart;
    }

    return NULL;
}


// ****************************************************************
/* free memory from the temp-storage */
coreBool coreTempStorageFree(void** OUTPUT pPointer)
{
    ASSERT(s_iTempIndex != SIZE_MAX)
    ASSERT(pPointer && (*pPointer))

    const coreUintW I = s_iTempIndex;

    // select assigned temp-storage
    coreByte*  B = s_aaTempBuffer [I];
    coreUintW& P = s_aiTempPointer[I];

    // calculate address range
    const coreUintW iStart = P_TO_UI(*pPointer) - P_TO_UI(B);

    // check if allocation is from the temp-storage
    if(iStart <= CORE_MEMORY_TEMP_SIZE)
    {
        // adjust temp-storage pointer
        P = MIN(P, iStart);
        (*pPointer) = NULL;

        return true;
    }

    return false;
}


#if defined(_CORE_WINDOWS_) || defined(_CORE_EMSCRIPTEN_) || defined(_CORE_SWITCH_)

// ****************************************************************
/* assign private heap to SDL library */
static void* s_pHeap = NULL;   // private heap object

static void* SDLCALL coreMalloc (const coreUintW iSize)                       {return coreData::HeapMalloc(s_pHeap, iSize);}
static void* SDLCALL coreCalloc (const coreUintW iNum, const coreUintW iSize) {return coreData::HeapCalloc(s_pHeap, iNum, iSize);}
static void* SDLCALL coreRealloc(void* pPointer, const coreUintW iSize)       {coreData::HeapRealloc(s_pHeap, &pPointer, iSize); return pPointer;}
static void  SDLCALL coreFree   (void* pPointer)                              {coreData::HeapFree   (s_pHeap, &pPointer);}

static struct coreInit final
{
    coreInit()noexcept
    {
        // create private heap object
        s_pHeap = coreData::HeapCreate(true);

        // register allocation functions
        SDL_SetMemoryFunctions(coreMalloc, coreCalloc, coreRealloc, coreFree);
    }

    ~coreInit()
    {
        // destroy private heap object
        coreData::HeapDestroy(s_pHeap);
    }
}
s_Init;

#endif


// ****************************************************************
/* constructor */
coreMemoryPool::coreMemoryPool()noexcept
: m_apPageList  {}
, m_apFreeStack {}
, m_iBlockSize  (0u)
, m_iPageSize   (0u)
, m_iAlign      (0u)
, m_bValid      (true)
, m_pHeap       (NULL)
{
}

coreMemoryPool::coreMemoryPool(const coreUint16 iBlockSize, const coreUint16 iPageSize, const coreUint8 iAlign)noexcept
: coreMemoryPool ()
{
    this->Configure(iBlockSize, iPageSize, iAlign);
}

coreMemoryPool::coreMemoryPool(coreMemoryPool&& m)noexcept
: m_apPageList  (std::move(m.m_apPageList))
, m_apFreeStack (std::move(m.m_apFreeStack))
, m_iBlockSize  (m.m_iBlockSize)
, m_iPageSize   (m.m_iPageSize)
, m_iAlign      (m.m_iAlign)
, m_bValid      (m.m_bValid)
, m_pHeap       (std::exchange(m.m_pHeap, NULL))
{
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
    std::swap(m_iAlign,      m.m_iAlign);
    std::swap(m_bValid,      m.m_bValid);
    std::swap(m_pHeap,       m.m_pHeap);

    return *this;
}


// ****************************************************************
/* set required memory-pool properties */
void coreMemoryPool::Configure(const coreUint16 iBlockSize, const coreUint16 iPageSize, const coreUint8 iAlign)
{
    ASSERT(iBlockSize && iPageSize && iAlign && coreMath::IsPot(iAlign) && coreMath::IsAligned(iBlockSize, iAlign))
    ASSERT(m_apPageList.empty())

    // save properties
    m_iBlockSize = iBlockSize;
    m_iPageSize  = iPageSize;
    m_iAlign     = (iAlign <= ALIGNMENT_NEW) ? 1u : iAlign;

    // create private heap object
    if(!m_pHeap) m_pHeap = coreData::HeapCreate(false);
}


// ****************************************************************
/* reset memory-pool to its initial state */
void coreMemoryPool::Reset()
{
    ASSERT((m_apFreeStack.size() == (m_apPageList.size() * m_iPageSize)) || !m_bValid)

    // delete all memory-pages
    FOR_EACH(it, m_apPageList)
    {
        coreData::HeapFree(m_pHeap, r_cast<void**>(&(*it)));
    }

    // clear memory
    m_apPageList .clear();
    m_apFreeStack.clear();

    // revalidate memory-pool
    m_bValid = true;
}


// ****************************************************************
/* shutdown memory-pool (for faster cleanup) */
void coreMemoryPool::Shutdown()
{
    // invalidate memory-pool
    m_bValid = false;
}


// ****************************************************************
/* create memory-block */
RETURN_RESTRICT void* coreMemoryPool::Allocate()
{
    ASSERT(m_bValid)

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

    // only return pointer if memory-pool is still valid
    if(m_bValid)
    {
        // find target location with binary search
        ASSERT(std::is_sorted(m_apFreeStack.begin(), m_apFreeStack.end(), std::greater()))
        const auto it = std::lower_bound(m_apFreeStack.begin(), m_apFreeStack.end(), *ppPointer, std::greater());

        // return pointer to the free-stack
        m_apFreeStack.insert(it, *ppPointer);
    }

    // remove reference
    (*ppPointer) = NULL;
}


// ****************************************************************
/* check if pointer belongs to the memory-pool */
coreBool coreMemoryPool::Contains(const void* pPointer)const
{
    const coreUintW iTotalSize = m_iBlockSize * m_iPageSize + m_iAlign - 1u;
    return std::any_of(m_apPageList.begin(), m_apPageList.end(), [&](const coreByte* pPage) {return (P_TO_UI(pPointer) - P_TO_UI(pPage) < iTotalSize);});
}


// ****************************************************************
/* add new memory-page to memory-pool */
void coreMemoryPool::__AddPage()
{
    ASSERT(m_iBlockSize && m_iPageSize && m_iAlign && m_pHeap)

    // create new memory-page
    coreByte* pNewPage = s_cast<coreByte*>(coreData::HeapMalloc(m_pHeap, m_iBlockSize * m_iPageSize + m_iAlign - 1u));
    m_apPageList.push_back(pNewPage);

    // align base address
    pNewPage = coreMath::CeilAlignPtr(pNewPage, m_iAlign);

    // reserve memory for the free-stack
    m_apFreeStack.reserve(m_apFreeStack.size() + m_iPageSize);

    // add all containing memory-blocks to the free-stack
    for(coreUintW i = m_iPageSize; i--; )
    {
        m_apFreeStack.push_back_unsafe(pNewPage + (m_iBlockSize * i));
    }
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
RETURN_RESTRICT void* coreMemoryManager::Allocate(const coreUint16 iSize, const coreUint8 iAlign)
{
    const coreLocker oLocker(&m_PoolLock);

    // assemble lookup key
    const coreUint32 iKey = (coreUint32(iSize) << 16u) | (coreUint32(iAlign));

    // check and create memory-pool
    if(!m_aMemoryPool.count(iKey)) m_aMemoryPool.emplace(iKey, iSize, 0x1000u / iSize, iAlign);

    // forward request to internal memory-pool
    return m_aMemoryPool.at(iKey).Allocate();
}


// ****************************************************************
/* remove memory-block to internal memory-pool */
void coreMemoryManager::Free(const coreUint16 iSize, const coreUint8 iAlign, void** OUTPUT ppPointer)
{
    const coreLocker oLocker(&m_PoolLock);

    // assemble lookup key
    const coreUint32 iKey = (coreUint32(iSize) << 16u) | (coreUint32(iAlign));

    // forward request to internal memory-pool
    m_aMemoryPool.at(iKey).Free(ppPointer);
}