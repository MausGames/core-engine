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
#define CORE_MEMORY_SHARED coreData::Print("$%s:%d",    coreData::StrRight(__FILE__, 16), __LINE__)
#define CORE_MEMORY_UNIQUE coreData::Print("$%s:%d:%p", coreData::StrRight(__FILE__, 16), __LINE__, this)


// ****************************************************************
// memory manager
class coreMemoryManager final
{
private:
    std::u_map<std::string, std::weak_ptr<void> > m_apPointer;   //!< list with weak shared memory pointer


private:
    coreMemoryManager()noexcept {}
    ~coreMemoryManager()        {m_apPointer.clear();}
    friend class Core;


public:
    //! share memory pointer through specific identifier
    //! @{
    template <typename T> std::shared_ptr<T> Share(const char* pcName);
    //! @}
};


// ****************************************************************
// share memory pointer through specific identifier
// TODO: check if return is optimized
template <typename T> std::shared_ptr<T> coreMemoryManager::Share(const char* pcName)
{
    // check for existing shared memory pointer
    if(m_apPointer.count(pcName))
    {
        if(!m_apPointer[pcName].expired())
            return std::static_pointer_cast<T>(m_apPointer[pcName].lock());
    }

    // create new shared memory pointer
    std::shared_ptr<T> pPointer = std::shared_ptr<T>(new T());
    m_apPointer[pcName] = pPointer;

    return pPointer;
}


#endif // _CORE_GUARD_MEMORY_H_