///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MEMORY_H_
#define _CORE_GUARD_MEMORY_H_

// TODO 5: memory-pool: main-page + smaller extension-pages
// TODO 5: memory-pool: usable with coreList & co
// TODO 5: memory-pool: support polymorphism
// TODO 5: memory-pool: 1 block for multiple objects
// TODO 5: add interface for reusing big memory-blocks (free when?)
// TODO 5: __declspec(allocator)
// TODO 3: pointer to next free block inside unused block (linked list) instead of free-stack (consider sorting)
// TODO 5: <old comment style>
// TODO 3: implement quick free and quick reset to memory pool (but not to manager), without sorting (and without insertion?), for regular destruction
// TODO 3: #define STATIC_EXTERN(t,p) extern t* const p; needs to be before Core class
// TODO 2: add ASSERT when accessing static-memory without initialization


// ****************************************************************
/* memory definitions */
#define CORE_MEMORY_SHARED  (STRING(__FILE__) ":" STRING(__LINE__))
#define CORE_MEMORY_UNIQUE  (PRINT(CORE_MEMORY_SHARED ":%p", this))

#define __CHECK_ALLOC(t,c)  ([](const coreUintW iSize) {STATIC_ASSERT(std::is_trivial<typename std::decay<t>::type>::value) ASSERT(iSize)}(c))

#define MANAGED_NEW(t,...)  (ASSUME_ALIGNED(new(Core::Manager::Memory->Allocate(sizeof(t))) t(__VA_ARGS__), alignof(t)))
#define MANAGED_DELETE(p)   {if(p) {CALL_DESTRUCTOR(p) Core::Manager::Memory->Free(sizeof(*(p)), r_cast<void**>(&(p)));}}

#define POOLED_NEW(m,t,...) (ASSUME_ALIGNED(new((m).Allocate()) t(__VA_ARGS__), alignof(t)))
#define POOLED_DELETE(m,p)  {if(p) {CALL_DESTRUCTOR(p) (m).Free(r_cast<void**>(&(p)));}}

#define ALIGNED_NEW(t,c,a)  (__CHECK_ALLOC(t, c), ASSUME_ALIGNED(s_cast<t*>(_aligned_malloc((c) * sizeof(t), (a))), (a)))
#define ALIGNED_DELETE(p)   {_aligned_free(p); (p) = NULL;}

#define ZERO_NEW(t,c)       (__CHECK_ALLOC(t, c), ASSUME_ALIGNED(s_cast<t*>(std::calloc((c), sizeof(t))), ALIGNMENT_NEW))
#define ZERO_DELETE(p)      {std::free(p); (p) = NULL;}

#define DYNAMIC_RESIZE(p,c) {__CHECK_ALLOC(decltype(*(p)), c); (p) = ASSUME_ALIGNED(s_cast<decltype(p)>(std::realloc((p), (c) * sizeof(*(p)))), ALIGNMENT_NEW);}
#define DYNAMIC_DELETE(p)   {std::free(p); (p) = NULL;}

#define STATIC_MEMORY(t,p)  alignas(alignof(t)) static coreByte CONCAT(__m, __LINE__)[sizeof(t) + sizeof(coreBool)] = {}; t* const p = r_cast<t*>(CONCAT(__m, __LINE__));
#define STATIC_ISVALID(p)   (*(r_cast<coreBool*>((p) + 1u)))
#define STATIC_NEW(p,...)   {ASSERT(!STATIC_ISVALID(p)) CALL_CONSTRUCTOR(p, __VA_ARGS__) STATIC_ISVALID(p) = true;}
#define STATIC_DELETE(p)    {if(STATIC_ISVALID(p))      CALL_DESTRUCTOR (p)              STATIC_ISVALID(p) = false;}

#if !defined(_CORE_WINDOWS_)
    #define _aligned_malloc(c,a) std::aligned_alloc(a, coreMath::CeilAlign(c, a))
    #define _aligned_free(p)     std::free(p)
#endif

#if defined(_CORE_MACOS_) && (MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_15)
    namespace std {FORCE_INLINE RETURN_RESTRICT void* aligned_alloc_macos(const coreUintW iAlign, const coreUintW iSize) {void* P; posix_memalign(&P, iAlign, iSize); return P;}}
    #define aligned_alloc aligned_alloc_macos
#endif


// ****************************************************************
/* memory-pool class */
class coreMemoryPool final
{
private:
    coreList<coreByte*> m_apPageList;    // list with memory-pages containing many memory-blocks
    coreList<void*>     m_apFreeStack;   // stack with pointers to free memory-blocks

    coreUintW m_iBlockSize;              // memory-block size (in bytes)
    coreUintW m_iPageSize;               // memory-page size (in number of containing memory-blocks)

    void* m_pHeap;                       // private heap object


public:
    coreMemoryPool()noexcept;
    coreMemoryPool(const coreUintW iBlockSize, const coreUintW iPageSize)noexcept;
    coreMemoryPool(coreMemoryPool&& m)noexcept;
    ~coreMemoryPool();

    /* assignment operations */
    coreMemoryPool& operator = (coreMemoryPool&& m)noexcept;

    /* control state of the memory-pool */
    void Configure(const coreUintW iBlockSize, const coreUintW iPageSize);
    void Reset();

    /* create and remove memory-blocks */
    RETURN_RESTRICT void* Allocate();
    void Free(void** OUTPUT ppPointer);

    /* check if pointer belongs to the memory-pool */
    coreBool Contains(const void* pPointer)const;


private:
    /* add new memory-page to memory-pool */
    void __AddPage();
};


// ****************************************************************
/* memory manager */
class coreMemoryManager final
{
private:
    coreMapStr<std::weak_ptr<void>>     m_apPointer;     // list with weak shared memory pointer
    coreMap<coreUint16, coreMemoryPool> m_aMemoryPool;   // internal memory-pools (each for a different block-size)

    coreSpinLock m_PoolLock;                             // spinlock to prevent invalid memory-pool access


private:
    coreMemoryManager()noexcept;
    ~coreMemoryManager();


public:
    FRIEND_CLASS(Core)
    DISABLE_COPY(coreMemoryManager)

    /* share memory pointer through specific identifier */
    template <typename T, typename... A> std::shared_ptr<T> Share(const coreHashString& sName, A&&... vArgs);

    /* create and remove memory-blocks through internal memory-pools */
    RETURN_RESTRICT void* Allocate(const coreUintW iSize);
    void Free(const coreUintW iSize, void** OUTPUT ppPointer);
};


// ****************************************************************
/* share memory pointer through specific identifier */
template <typename T, typename... A> std::shared_ptr<T> coreMemoryManager::Share(const coreHashString& sName, A&&... vArgs)
{
    // check for existing pointer
    if(m_apPointer.count(sName))
    {
        const std::weak_ptr<void>& pPointer = m_apPointer.at(sName);

        // check for valid pointer
        if(!pPointer.expired()) return std::static_pointer_cast<T>(pPointer.lock());
    }

    // create new pointer
    std::shared_ptr<T> pNewPointer = std::make_shared<T>(std::forward<A>(vArgs)...);
    m_apPointer[sName] = pNewPointer;

    return pNewPointer;
}


#endif /* _CORE_GUARD_MEMORY_H_ */