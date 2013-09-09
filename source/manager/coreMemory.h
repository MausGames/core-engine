//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef GUARD_CORE_MEMORY_H
#define GUARD_CORE_MEMORY_H


// ****************************************************************
// memory definitions
#define CORE_MEMORY_SHARED coreUtils::Print("$%s:%d",    coreUtils::StrRight(__FILE__, 16), __LINE__)
#define CORE_MEMORY_UNIQUE coreUtils::Print("$%s:%d:%p", coreUtils::StrRight(__FILE__, 16), __LINE__, this)


// ****************************************************************
// memory manager
class coreMemoryManager final
{
private:


private:
    coreMemoryManager()  {}
    ~coreMemoryManager() {}
    friend class Core;


public:


};


#endif // GUARD_CORE_MEMORY_H