//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
coreMemoryManager::coreMemoryManager()noexcept
{
    Core::Log->Info("Memory Manager created");
}


// ****************************************************************
// destructor
coreMemoryManager::~coreMemoryManager()
{
    // clear memory
    m_apPointer.clear();

    Core::Log->Info("Memory Manager destroyed");
}