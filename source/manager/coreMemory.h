//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MEMORY_H_
#define _CORE_GUARD_MEMORY_H_


// ****************************************************************
// memory definitions
#define CORE_MEMORY_SHARED coreUtils::Print("$%s:%d",    coreUtils::StrRight(__FILE__, 16), __LINE__)
#define CORE_MEMORY_UNIQUE coreUtils::Print("$%s:%d:%p", coreUtils::StrRight(__FILE__, 16), __LINE__, this)


// ****************************************************************
// memory manager
class coreMemoryManager final
{
private:
    std::u_map<std::string, std::weak_ptr<void> > m_apPointer;   //!< list with weak shared memory pointer


private:
    coreMemoryManager()  {}
    ~coreMemoryManager() {m_apPointer.clear();}
    friend class Core;


public:
    //! share memory pointer through specific identifier
    //! @{
    template <typename T> std::shared_ptr<T> Share(const char* pcID);
    //! @}
};


// ****************************************************************
// share memory pointer through specific identifier
// TODO: check if return is optimized
template <typename T> std::shared_ptr<T> coreMemoryManager::Share(const char* pcID)
{
    // check for existing shared memory pointer
    if(m_apPointer.count(pcID))
    {
        if(!m_apPointer[pcID].expired())
            return std::static_pointer_cast<T>(m_apPointer[pcID].lock());
    }

    // create new shared memory pointer
    std::shared_ptr<T> pPointer = std::shared_ptr<T>(new T());
    m_apPointer[pcID] = pPointer;

    return pPointer;
}


#endif // _CORE_GUARD_MEMORY_H_