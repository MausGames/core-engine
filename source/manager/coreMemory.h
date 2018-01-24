//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MEMORY_H_
#define _CORE_GUARD_MEMORY_H_

// TODO: check if Share return is optimized
// TODO: asynchronous memory defragmentation
// TODO: memory-pool: main-page + smaller extension-pages
// TODO: memory-pool: usable with std::vector & co
// TODO: memory-pool: support polymorphism
// TODO: memory-pool: 1 block for multiple objects
// TODO: add interface for reusing big memory-blocks (free when?)
// TODO: __declspec(allocator)
// TODO: consider allocation granularity 64kb


// ****************************************************************
// memory definitions
#define CORE_MEMORY_SHARED  (STRING(__FILE__) ":" STRING(__LINE__))
#define CORE_MEMORY_UNIQUE  (PRINT(CORE_MEMORY_SHARED ":%p", this))

#define MANAGED_NEW(t,...)  (new(Core::Manager::Memory->Allocate(sizeof(t))) t(__VA_ARGS__))
#define MANAGED_DELETE(p)   {CALL_DESTRUCTOR(p) Core::Manager::Memory->Free(sizeof(*(p)), r_cast<void**>(&(p)));}

#define POOLED_NEW(m,t,...) (new((m).Allocate()) t(__VA_ARGS__))
#define POOLED_DELETE(m,p)  {CALL_DESTRUCTOR(p) (m).Free(r_cast<void**>(&(p)));}

#define ALIGNED_NEW(t,c,a)  (r_cast<t*>(_aligned_malloc((c) * sizeof(t), (a))))
#define ALIGNED_DELETE(p)   {_aligned_free(p); (p) = NULL;}

#define STATIC_MEMORY(t,p)  alignas(ALIGNMENT_SSE) static coreByte CONCAT(__m, __LINE__)[sizeof(t)] = {}; t* const p = r_cast<t*>(CONCAT(__m, __LINE__));
#define STATIC_NEW(p,...)   {CALL_CONSTRUCTOR(p, __VA_ARGS__)}
#define STATIC_DELETE(p)    {CALL_DESTRUCTOR(p) std::memset(p, 0, sizeof(*(p)));}

#define ZERO_NEW(t,c)       (r_cast<t*>(std::calloc((c), sizeof(t))))
#define RESIZE_ARRAY(a,c)   {(a) = r_cast<decltype(a)>(std::realloc((a), (c) * sizeof(*(a))));}

#if !defined(_CORE_WINDOWS_)
    #define _aligned_malloc(c,a) std::aligned_alloc(a, c)
    #define _aligned_free(p)     std::free(p)
#endif


// ****************************************************************
// memory-pool class
class coreMemoryPool final
{
private:
    std::vector<coreByte*> m_apPageList;    //!< list with memory-pages containing many memory-blocks
    std::vector<void*>     m_apFreeStack;   //!< stack with pointers to free memory-blocks

    coreUintW m_iBlockSize;                 //!< memory-block size (in bytes)
    coreUintW m_iPageSize;                  //!< memory-page size (in number of containing memory-blocks)


public:
    coreMemoryPool()noexcept;
    coreMemoryPool(const coreUintW iBlockSize, const coreUintW iPageSize)noexcept;
    ~coreMemoryPool();

    ENABLE_COPY(coreMemoryPool)

    //! control state of the memory-pool
    //! @{
    void Configure(const coreUintW iBlockSize, const coreUintW iPageSize);
    void Reset();
    //! @}

    //! create and remove memory-blocks
    //! @{
    RETURN_RESTRICT void* Allocate();
    void Free(void** OUTPUT pptPointer);
    //! @}


private:
    //! add new memory-page to memory-pool
    //! @{
    void __AddPage();
    //! @}
};


// ****************************************************************
// memory manager
class coreMemoryManager final
{
private:
    coreLookupStr<std::weak_ptr<void>>     m_apPointer;     //!< list with weak shared memory pointer
    coreLookup<coreUint16, coreMemoryPool> m_aMemoryPool;   //!< internal memory-pools (each for a different block-size)

    SDL_SpinLock m_iPoolLock;                               //!< spinlock to prevent invalid memory-pool access


private:
    coreMemoryManager()noexcept;
    ~coreMemoryManager();


public:
    FRIEND_CLASS(Core)
    DISABLE_COPY(coreMemoryManager)

    //! share memory pointer through specific identifier
    //! @{
    template <typename T, typename... A> std::shared_ptr<T> Share(const coreHashString& sName, A&&... vArgs);
    //! @}

    //! create and remove memory-blocks through internal memory-pools
    //! @{
    RETURN_RESTRICT void* Allocate(const coreUintW iSize);
    void Free(const coreUintW iSize, void** OUTPUT pptPointer);
    //! @}
};


// ****************************************************************
// share memory pointer through specific identifier
template <typename T, typename... A> std::shared_ptr<T> coreMemoryManager::Share(const coreHashString& sName, A&&... vArgs)
{
    // check for existing pointer
    if(m_apPointer.count(sName))
    {
        const std::weak_ptr<void>& pPointer = m_apPointer.at(sName);

        // check for valid pointer
        if(!pPointer.expired())
            return std::static_pointer_cast<T>(pPointer.lock());
    }

    // create new pointer
    std::shared_ptr<T> pNewPointer = std::make_shared<T>(std::forward<A>(vArgs)...);
    m_apPointer[sName] = pNewPointer;

    return pNewPointer;
}


#endif // _CORE_GUARD_MEMORY_H_