//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
coreResourceHandle::coreResourceHandle(coreFile* pFile, coreResource* pResource, coreResource* pNull)
: m_pFile     (pFile)
, m_pResource (pResource)
, m_pNull     (pNull ? pNull : pResource)
, m_pCur      (pNull ? pNull : pResource)
, m_iRef      (0)
, m_iStatus   (CORE_RESOURCE_NOT_LOADED)
{
}


// ****************************************************************
// destructor
coreResourceHandle::~coreResourceHandle()
{
    // forgot to delete a resource access, a resource-using object or used global variables
    SDL_assert(!m_iRef);

    // delete associated resource object
    SAFE_DELETE(m_pResource)
}


// ****************************************************************
// control resource loading
void coreResourceHandle::Update()
{
    if(m_iStatus == CORE_RESOURCE_UNMANAGED) return;

    if(m_iRef != 0 && m_iStatus == CORE_RESOURCE_NOT_LOADED)
    {
        // load associated resource
        const coreError iError = m_pResource->Load(m_pFile);
        if(iError == CORE_OK)
        {
            m_pCur    = m_pResource;
            m_iStatus = CORE_RESOURCE_LOADED;
            if(m_pFile) m_pFile->UnloadData();
        }
        else if(iError == CORE_FILE_ERROR)
            m_iStatus = CORE_RESOURCE_UNMANAGED;
    }
    else if(m_iRef == 0 && m_iStatus == CORE_RESOURCE_LOADED)
    {
        // unload associated resource
        this->Nullify();
    }
}


// ****************************************************************
// constructor
coreReset::coreReset()
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
coreResourceManager::coreResourceManager()
{
    // reserve memory for archives
    m_apArchive.reserve(32);

    // start up the resource manager
    this->Reset(true);

    Core::Log->Info("Resource Manager created");
}


// ****************************************************************
// destructor
coreResourceManager::~coreResourceManager()
{
    // shut down the resource manager
    this->Reset(false);

    // delete resource handles and NULL resources
    for(auto it = m_apHandle.begin(); it != m_apHandle.end(); ++it) SAFE_DELETE(it->second)
    for(auto it = m_apNull.begin();   it != m_apNull.end();   ++it) SAFE_DELETE(it->second)

    // delete resource files
    for(auto it = m_apArchive.begin();    it != m_apArchive.end();    ++it) SAFE_DELETE(*it)
    for(auto it = m_apDirectFile.begin(); it != m_apDirectFile.end(); ++it) SAFE_DELETE(it->second)

    // clear memory
    m_apHandle.clear();
    m_apNull.clear();
    m_apArchive.clear();
    m_apDirectFile.clear();
    m_apReset.clear();

    Core::Log->Info("Resource Manager destroyed");
}


// ****************************************************************
// add archive with resource files
coreError coreResourceManager::AddArchive(const char* pcPath)
{
    // load and check archive
    coreArchive* pArchive = new coreArchive(pcPath);
    if(!pArchive->GetSize())
    {
        SAFE_DELETE(pArchive)
        return CORE_FILE_ERROR;
    }

    // add archive
    m_apArchive.push_back(pArchive);
    return CORE_OK;
}


// ****************************************************************
// retrieve resource file
coreFile* coreResourceManager::RetrieveResourceFile(const char* pcPath)
{
    // check for direct resource file
    if(coreFile::FileExists(pcPath))
    {
        if(m_apDirectFile.count(pcPath)) return m_apDirectFile[pcPath];

        // load new direct resource file
        coreFile* pFile = new coreFile(pcPath);
        m_apDirectFile[pcPath] = pFile;

        return pFile;
    }

    // check archives
    for(auto it = m_apArchive.begin(); it != m_apArchive.end(); ++it)
    {
        coreFile* pFile = (*it)->GetFile(pcPath);
        if(pFile) return pFile;
    }

    // resource file not found
    SDL_assert(false);
    if(!m_apDirectFile.count(pcPath))
        m_apDirectFile[pcPath] = new coreFile(pcPath);

    return m_apDirectFile[pcPath];
}


// ****************************************************************
// reset resource manager
void coreResourceManager::Reset(const bool& bInit)
{
    if(bInit)
    {
        // re-load NULL resources
        for(auto it = m_apNull.begin(); it != m_apNull.end(); ++it)
            it->second->Load(this->RetrieveResourceFile(it->first.c_str()));

        // re-init reset-objects
        for(auto it = m_apReset.begin(); it != m_apReset.end(); ++it)
            (*it)->Reset(true);

        // start resource thread
        this->StartThread("resource_thread");
    }
    else
    {
        // kill resource thread
        this->KillThread();

        // shut down reset-objects
        for(auto it = m_apReset.begin(); it != m_apReset.end(); ++it)
            (*it)->Reset(false);

        // unload resources
        for(auto it = m_apHandle.begin(); it != m_apHandle.end(); ++it) it->second->Nullify();
        for(auto it = m_apNull.begin();   it != m_apNull.end();   ++it) it->second->Unload();
    }
}


// ****************************************************************
// init resource thread
int coreResourceManager::__Init()
{
    // assign secondary OpenGL context to resource thread
    if(SDL_GL_MakeCurrent(Core::System->GetWindow(), Core::Graphics->GetResourceContext()))
        Core::Log->Error(1, coreUtils::Print("Secondary OpenGL context could not be assigned to resource thread (SDL: %s)", SDL_GetError()));
    else Core::Log->Info("Secondary OpenGL context assigned to resource thread");

    // init GLEW on secondary OpenGL context
    const GLenum iError = glewInit();
    if(iError != GLEW_OK) Core::Log->Error(1, coreUtils::Print("GLEW could not be initialized on secondary OpenGL context (GLEW: %s)", glewGetErrorString(iError)));
    else Core::Log->Info("GLEW initialized on secondary OpenGL context");

    return 0;
}


// ****************************************************************
// run resource thread
int coreResourceManager::__Run()
{
    // update resource handles
    for(auto it = m_apHandle.begin(); it != m_apHandle.end(); ++it)
        it->second->Update();

    return 0;
}


// ****************************************************************
// exit resource thread
void coreResourceManager::__Exit()
{
    // dissociate secondary OpenGL context from resource thread
    SDL_GL_MakeCurrent(Core::System->GetWindow(), NULL);
}