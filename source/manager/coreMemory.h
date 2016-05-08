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


// ****************************************************************
// memory definitions
#define CORE_MEMORY_SHARED PRINT("!%s:%d",    coreData::StrRight(__FILE__, 8u), __LINE__)
#define CORE_MEMORY_UNIQUE PRINT("!%s:%d:%p", coreData::StrRight(__FILE__, 8u), __LINE__, this)


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
    void Clear();
    //! @}

    //! construct and destruct object
    //! @{
    template <typename T, typename... A> RETURN_RESTRICT T* New(A&&... vArgs);
    template <typename T> void Delete(T** OUTPUT pptPointer);
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

    //! construct and destruct object in internal memory-pool
    //! @{
    template <typename T, typename... A> RETURN_RESTRICT T* New(A&&... vArgs);
    template <typename T> void Delete(T** OUTPUT pptPointer);
    //! @}
};


// ****************************************************************
// construct object
template <typename T, typename... A> RETURN_RESTRICT T* coreMemoryPool::New(A&&... vArgs)
{
    ASSERT(sizeof(T) <= m_iBlockSize)

    // check for free memory-block
    if(m_apFreeStack.empty()) this->__AddPage();
    ASSERT(!m_apFreeStack.empty())

    // get object from the free-stack
    void* pMemory = m_apFreeStack.back();
    m_apFreeStack.pop_back();

    // call constructor
    return new(pMemory) T(std::forward<A>(vArgs)...);
}


// ****************************************************************
// destruct and return object
template <typename T> void coreMemoryPool::Delete(T** OUTPUT pptPointer)
{
    ASSERT(std::any_of(m_apPageList.begin(), m_apPageList.end(), [&](const coreByte* pPage){return (P_TO_UI(*pptPointer) - P_TO_UI(pPage)) < (m_iBlockSize * m_iPageSize);}))

    // return object to the free-stack
    m_apFreeStack.push_back(*pptPointer);

    // call destructor and remove pointer
    (*pptPointer)->~T();
    (*pptPointer) = NULL;
}


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


// ****************************************************************
// construct object from internal memory-pool
template <typename T, typename... A> RETURN_RESTRICT T* coreMemoryManager::New(A&&... vArgs)
{
    // check and create memory-pool
    if(!m_aMemoryPool.count(sizeof(T))) m_aMemoryPool.emplace(sizeof(T), sizeof(T), 128u);

    // forward request to internal memory-pool
    return m_aMemoryPool.at(sizeof(T)).New<T>(std::forward<A>(vArgs)...);
}


// ****************************************************************
// destruct and return object to internal memory-pool
template <typename T> void coreMemoryManager::Delete(T** OUTPUT pptPointer)
{
    // forward request to internal memory-pool
    m_aMemoryPool.at(sizeof(T)).Delete<T>(pptPointer);
}


#endif // _CORE_GUARD_MEMORY_H_