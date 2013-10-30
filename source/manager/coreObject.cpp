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
// define texture through resource file
const coreTexturePtr& coreObject::DefineTextureFile(const coreByte& iUnit, const char* pcPath)
{
    SDL_assert(iUnit < CORE_TEXTURE_UNITS);

    // set and return the texture resource pointer
    m_apTexture[iUnit] = Core::Manager::Resource->LoadFile<coreTexture>(pcPath);
    return m_apTexture[iUnit];
}


// ****************************************************************
// define texture through linked resource
const coreTexturePtr& coreObject::DefineTextureLink(const coreByte& iUnit, const char* pcName)
{
    SDL_assert(iUnit < CORE_TEXTURE_UNITS);

    // set and return the texture resource pointer
    m_apTexture[iUnit] = Core::Manager::Resource->LoadLink<coreTexture>(pcName);
    return m_apTexture[iUnit];
}


// ****************************************************************
// define shader-program through shared memory
const coreProgramShr& coreObject::DefineProgramShare(const char* pcName)
{
    // set and return the shader-program memory pointer
    m_pProgram = Core::Manager::Memory->Share<coreProgram>(pcName);
    return m_pProgram;
}


// ****************************************************************
// constructor
coreObjectManager::coreObjectManager()noexcept
{
    Core::Log->Info("Object Manager created");
}


// ****************************************************************
// destructor
coreObjectManager::~coreObjectManager()
{
    Core::Log->Info("Object Manager destroyed");
}