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
coreResourceHandle::coreResourceHandle(coreResource* pResource, coreResource* pDefault, coreFile* pFile)noexcept
: m_pResource (pResource)
, m_pDefault  (pDefault)
, m_pFile     (pFile)
, m_pCur      (pFile ? pDefault : pResource)
, m_iRef      (0)
, m_bManaged  (pFile ? true : false)
{
}


// ****************************************************************
// destructor
coreResourceHandle::~coreResourceHandle()
{
    // forgot to delete a resource access, a resource-using object,
    // a shared memory object or used global variables
    SDL_assert(!m_iRef);

    // delete associated resource object
    SAFE_DELETE(m_pResource)
}


// ****************************************************************
// control resource loading
void coreResourceHandle::__Update()
{
    if(!m_bManaged) return;

    if(m_iRef != 0 && !this->IsLoaded())
    {
        // load associated resource
        const coreError iStatus = m_pResource->Load(m_pFile);
        if(iStatus == CORE_OK)
        {
            // successfully loaded
            m_pCur = m_pResource;
        }
        else if(iStatus < 0)
        {
            // stop managing the resource handle
            m_bManaged = false;
        }
    }
}


// ****************************************************************
// constructor
coreReset::coreReset()noexcept
{
    // add object to resource manager
    Core::Manager::Resource->AddReset(this);
}


// ****************************************************************
// destructor
coreReset::~coreReset()
{
    // remove object from resource manager
    Core::Manager::Resource->RemoveReset(this);
}


// ****************************************************************
// constructor
coreResourceManager::coreResourceManager()noexcept
: m_bActive (false)
{
    // start up the resource manager
    this->Reset(true);

    Core::Log->Info("Resource Manager created");
}


// ****************************************************************
// destructor
coreResourceManager::~coreResourceManager()
{
    SDL_assert(!m_apReset.size());

    // shut down the resource manager
    this->Reset(false);

    // delete resource handles and default resources
    FOR_EACH(it, m_apHandle)  SAFE_DELETE(it->second)
    FOR_EACH(it, m_apDefault) SAFE_DELETE(it->second)

    // delete resource files
    FOR_EACH(it, m_apArchive)    SAFE_DELETE(it->second)
    FOR_EACH(it, m_apDirectFile) SAFE_DELETE(it->second)

    // clear memory
    m_apHandle.clear();
    m_apDefault.clear();
    m_apArchive.clear();
    m_apDirectFile.clear();
    m_apReset.clear();

    Core::Log->Info("Resource Manager destroyed");
}


// ****************************************************************
// retrieve archive
coreArchive* coreResourceManager::RetrieveArchive(const char* pcPath)
{
    if(m_apArchive.count(pcPath)) return m_apArchive[pcPath];

    // load new archive
    coreArchive* pNewArchive = new coreArchive(pcPath);
    m_apArchive[pcPath] = pNewArchive;

    SDL_assert(pNewArchive->GetSize());
    return pNewArchive;
}


// ****************************************************************
// retrieve resource file
// TODO MAJOR: when and how load default archive(s) ?
coreFile* coreResourceManager::RetrieveFile(const char* pcPath)
{
     // check for direct resource file
    if(!coreData::FileExists(pcPath))
    {
        // check archives
        FOR_EACH(it, m_apArchive)
        {
            coreFile* pFile = it->second->GetFile(pcPath);
            if(pFile) return pFile;
        }

        // resource file not found
        SDL_assert(false);
    }

    if(m_apDirectFile.count(pcPath)) return m_apDirectFile[pcPath];

    // load new direct resource file
    coreFile* pNewFile = new coreFile(pcPath);
    m_apDirectFile[pcPath] = pNewFile;

    return pNewFile;
}


// ****************************************************************
// reset all resources and reset-objects
void coreResourceManager::Reset(const bool& bInit)
{
    // check and set current status
    if(m_bActive == bInit) return;
    m_bActive = bInit;

    if(bInit)
    {
        // load default resources
        FOR_EACH(it, m_apDefault)
            it->second->Load(this->RetrieveFile(it->first.c_str()));

        // init reset-objects
        FOR_EACH(it, m_apReset)
            (*it)->__Reset(true);

        // start resource thread
        if(Core::Graphics->GetResourceContext())
            this->StartThread("resource_thread");
    }
    else
    {
        // kill resource thread
        if(Core::Graphics->GetResourceContext())
            this->KillThread();

        // shut down reset-objects
        FOR_EACH(it, m_apReset)
            (*it)->__Reset(false);

        // unload all resources
        FOR_EACH(it, m_apHandle)  it->second->__Nullify();
        FOR_EACH(it, m_apDefault) it->second->Unload();
    }
}


// ****************************************************************
// init resource thread
int coreResourceManager::__Init()
{
    // assign secondary OpenGL context to resource thread
    if(SDL_GL_MakeCurrent(Core::System->GetWindow(), Core::Graphics->GetResourceContext()))
        Core::Log->Error(1, coreData::Print("Secondary OpenGL context could not be assigned to resource thread (SDL: %s)", SDL_GetError()));
    else Core::Log->Info("Secondary OpenGL context assigned to resource thread");

    // init GLEW on secondary OpenGL context
    const GLenum iError = glewInit();
    if(iError != GLEW_OK) Core::Log->Error(1, coreData::Print("GLEW could not be initialized on secondary OpenGL context (GLEW: %s)", glewGetErrorString(iError)));
    else Core::Log->Info(coreData::Print("GLEW initialized on secondary OpenGL context (%s)", glewGetString(GLEW_VERSION)));

    return 0;
}


// ****************************************************************
// run resource thread
int coreResourceManager::__Run()
{
    if(m_bActive)
    {
        // update resource handles
        for(coreUint i = 0; i < m_apHandle.size(); ++i)
            m_apHandle[i]->__Update();
    }
    return 0;
}


// ****************************************************************
// exit resource thread
void coreResourceManager::__Exit()
{
    // dissociate secondary OpenGL context from resource thread
    SDL_GL_MakeCurrent(Core::System->GetWindow(), NULL);
}