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

// TODO: implement memory management, duh'
// TODO: check if Share return is optimized
// TODO: asynchronous memory defragmentation


// ****************************************************************
// memory definitions
#define CORE_MEMORY_SHARED PRINT("!%s:%d",    coreData::StrRight(__FILE__, 8u), __LINE__)
#define CORE_MEMORY_UNIQUE PRINT("!%s:%d:%p", coreData::StrRight(__FILE__, 8u), __LINE__, this)


// ****************************************************************
// memory manager
class coreMemoryManager final
{
private:
    coreLookupStr<std::weak_ptr<void>> m_apPointer;   //!< list with weak shared memory pointer


private:
    coreMemoryManager()noexcept;
    ~coreMemoryManager();


public:
    FRIEND_CLASS(Core)
    DISABLE_COPY(coreMemoryManager)

    //! share memory pointer through specific identifier
    //! @{
    template <typename T, typename... A> std::shared_ptr<T> Share(const coreChar* pcName, A&&... vArgs);
    //! @}
};


// ****************************************************************
// share memory pointer through specific identifier
template <typename T, typename... A> std::shared_ptr<T> coreMemoryManager::Share(const coreChar* pcName, A&&... vArgs)
{
    // check for existing pointer
    if(m_apPointer.count(pcName))
    {
        const std::weak_ptr<void>& pPointer = m_apPointer.at(pcName);

        // check for valid pointer
        if(!pPointer.expired())
            return std::static_pointer_cast<T>(pPointer.lock());
    }

    // create new pointer
    std::shared_ptr<T> pNewPointer = std::make_shared<T>(std::forward<A>(vArgs)...);
    m_apPointer[pcName] = pNewPointer;

    return pNewPointer;
}


#endif // _CORE_GUARD_MEMORY_H_