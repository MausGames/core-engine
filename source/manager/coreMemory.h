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


// ****************************************************************
// memory definitions
#define CORE_MEMORY_SHARED PRINT("!%s:%d",    coreData::StrRight(__FILE__, 8), __LINE__)
#define CORE_MEMORY_UNIQUE PRINT("!%s:%d:%p", coreData::StrRight(__FILE__, 8), __LINE__, this)


// ****************************************************************
// memory manager
class coreMemoryManager final
{
friend class Core;

private:
    coreLookupStr<std::weak_ptr<void> > m_apPointer;   //!< list with weak shared memory pointer


private:
    coreMemoryManager()noexcept;
    ~coreMemoryManager();


public:
    DISABLE_COPY(coreMemoryManager)

    //! share memory pointer through specific identifier
    //! @{
    template <typename T, typename... A> std::shared_ptr<T> Share(const char* pcName, A&&... vArgs);
    //! @}
};


// ****************************************************************
// share memory pointer through specific identifier
// TODO: check if return is optimized
template <typename T, typename... A> std::shared_ptr<T> coreMemoryManager::Share(const char* pcName, A&&... vArgs)
{
    // check for existing pointer
    if(m_apPointer.count(pcName))
    {
        std::weak_ptr<void>& pPointer = m_apPointer[pcName];

        // check for valid pointer
        if(!pPointer.expired())
            return std::static_pointer_cast<T>(pPointer.lock());
    }

    // create new pointer
    std::shared_ptr<T> pNewPointer = std::shared_ptr<T>(new T(std::forward<A>(vArgs)...));
    m_apPointer[pcName] = pNewPointer;

    return pNewPointer;
}


#endif // _CORE_GUARD_MEMORY_H_