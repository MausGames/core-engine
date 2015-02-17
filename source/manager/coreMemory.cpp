//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
coreMemoryManager::coreMemoryManager()noexcept
{
    Core::Log->Info(CORE_LOG_BOLD("Memory Manager created"));
}


// ****************************************************************
// destructor
coreMemoryManager::~coreMemoryManager()
{
    // clear memory
    m_apPointer.clear();

    Core::Log->Info(CORE_LOG_BOLD("Memory Manager destroyed"));
}