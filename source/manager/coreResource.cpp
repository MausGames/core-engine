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
/* constructor */
coreResourceHandle::coreResourceHandle(coreResource* pResource, coreFile* pFile, const char* pcName, const bool& bAutomatic)noexcept
: m_pResource  (pResource)
, m_pFile      (pFile)
, m_sName      (pcName)
, m_bAutomatic (bAutomatic)
, m_iRefCount  (0)
, m_iStatus    ((pFile || bAutomatic) ? CORE_BUSY : CORE_OK)
{
}


// ****************************************************************
/* destructor */
coreResourceHandle::~coreResourceHandle()
{
    ASSERT(!m_iRefCount)

    // delete resource object
    SAFE_DELETE(m_pResource)
}


// ****************************************************************
/* constructor */
coreResourceRelation::coreResourceRelation()noexcept
{
    // bind object to resource manager
    Core::Manager::Resource->__BindRelation(this);
}

coreResourceRelation::coreResourceRelation(const coreResourceRelation& c)noexcept
{
    // bind object to resource manager
    Core::Manager::Resource->__BindRelation(this);
}


// ****************************************************************
/* destructor */
coreResourceRelation::~coreResourceRelation()
{
    // unbind object from resource manager
    Core::Manager::Resource->__UnbindRelation(this);
}


// ****************************************************************
/* constructor */
coreResourceManager::coreResourceManager()noexcept
: coreThread ("resource_thread")
, m_iLock    (0)
, m_bActive  (false)
{
    // start up the resource manager
    this->Reset(CORE_RESOURCE_RESET_INIT);

    // load all relevant default resources
    this->__LoadDefault();

    Core::Log->Info(CORE_LOG_BOLD("Resource Manager created"));
}


// ****************************************************************
/* destructor */
coreResourceManager::~coreResourceManager()
{
    ASSERT(!m_apRelation.size())

    // shut down the resource manager
    this->Reset(CORE_RESOURCE_RESET_EXIT);

    // delete resource handles
    FOR_EACH(it, m_apHandle) SAFE_DELETE(it->second)

    // delete resource files
    FOR_EACH(it, m_apArchive)    SAFE_DELETE(it->second)
    FOR_EACH(it, m_apDirectFile) SAFE_DELETE(it->second)

    // clear memory
    m_apHandle    .clear();
    m_apArchive   .clear();
    m_apDirectFile.clear();
    m_apRelation  .clear();

    Core::Log->Info("Resource Manager destroyed");
}


// ****************************************************************
/* update the resource manager */
void coreResourceManager::UpdateResources()
{
    // check for current status
    if(m_bActive)
    {
        SDL_AtomicLock(&m_iLock);
        {
            for(coreUint i = 0; i < m_apHandle.size(); ++i)
            {
                // update resource handle
                if(m_apHandle[i]->__AutoUpdate())
                {
                    // allow changes during iteration
                    SDL_AtomicUnlock(&m_iLock);
                    SDL_AtomicLock(&m_iLock);
                }
            }
        }
        SDL_AtomicUnlock(&m_iLock);
    }
}


// ****************************************************************
/* retrieve archive */
coreArchive* coreResourceManager::RetrieveArchive(const char* pcPath)
{
    // check for existing archive
    if(m_apArchive.count(pcPath)) return m_apArchive[pcPath];

    // load new archive
    coreArchive* pNewArchive = new coreArchive(pcPath);
    m_apArchive[pcPath] = pNewArchive;

    ASSERT(pNewArchive->GetNumFiles())
    return pNewArchive;
}


// ****************************************************************
/* retrieve resource file */
coreFile* coreResourceManager::RetrieveFile(const char* pcPath)
{
    // try to open direct resource file first
    if(!coreData::FileExists(pcPath))
    {
        // check archives
        FOR_EACH(it, m_apArchive)
        {
            coreFile* pFile = it->second->GetFile(pcPath);
            if(pFile) return pFile;
        }

        // resource file not found
        WARN_IF(true) {}
    }

    // check for existing direct resource file
    if(m_apDirectFile.count(pcPath)) return m_apDirectFile[pcPath];

    // load new direct resource file
    coreFile* pNewFile = new coreFile(pcPath);
    m_apDirectFile[pcPath] = pNewFile;

    return pNewFile;
}


// ****************************************************************
/* reset all resources and relation-objects */
void coreResourceManager::Reset(const coreResourceReset& bInit)
{
    const bool bActive = bInit ? true : false;

    // check and set current status
    if(m_bActive == bActive) return;
    m_bActive = bActive;

    if(m_bActive)
    {
        // start up relation-objects
        FOR_EACH(it, m_apRelation)
            (*it)->__Reset(CORE_RESOURCE_RESET_INIT);

        // start resource thread
        if(Core::Graphics->GetResourceContext())
            this->StartThread();
    }
    else
    {
        // kill resource thread
        if(Core::Graphics->GetResourceContext())
            this->KillThread();

        // shut down relation-objects
        FOR_EACH(it, m_apRelation)
            (*it)->__Reset(CORE_RESOURCE_RESET_EXIT);

        // unload all resources
        FOR_EACH(it, m_apHandle)
            it->second->Nullify();
    }
}


// ****************************************************************
/* init resource thread */
int coreResourceManager::__InitThread()
{
    // assign resource context to resource thread
    if(SDL_GL_MakeCurrent(Core::System->GetWindow(), Core::Graphics->GetResourceContext()))
        Core::Log->Error("Resource context could not be assigned to resource thread (SDL: %s)", SDL_GetError());
    else Core::Log->Info("Resource context assigned to resource thread");

    // init OpenGL
    coreInitOpenGL();

    // enable OpenGL debug output
    Core::Log->DebugOpenGL();

    // setup texturing
    glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
    glPixelStorei(GL_PACK_ALIGNMENT,   4);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    return 0;
}


// ****************************************************************
/* run resource thread */
int coreResourceManager::__RunThread()
{
    // update the resource manager
    this->UpdateResources();

    return 0;
}


// ****************************************************************
/* exit resource thread */
void coreResourceManager::__ExitThread()
{
    // dissociate resource context from resource thread
    SDL_GL_MakeCurrent(Core::System->GetWindow(), NULL);
}


// ****************************************************************
/* load all relevant default resource */
void coreResourceManager::__LoadDefault()
{
    this->Load<coreTexture>("default_black.png",        CORE_RESOURCE_UPDATE_AUTO,   "data/textures/default_black.png");
    this->Load<coreTexture>("default_white.png",        CORE_RESOURCE_UPDATE_AUTO,   "data/textures/default_white.png");
    this->Load<coreShader> ("default_2d.vert",          CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_2d.vert");
    this->Load<coreShader> ("default_2d.frag",          CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_2d.frag");
    this->Load<coreShader> ("default_label.vert",       CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_label.vert");
    this->Load<coreShader> ("default_label_sharp.frag", CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_label_sharp.frag");
    this->Load<coreFont>   ("default.ttf",              CORE_RESOURCE_UPDATE_AUTO,   "data/fonts/default.ttf");

    ((coreProgram*)    this->Load<coreProgram>("default_2d_program", CORE_RESOURCE_UPDATE_AUTO, NULL)->GetResource())
        ->AttachShader(this->Get <coreShader> ("default_2d.vert"))
        ->AttachShader(this->Get <coreShader> ("default_2d.frag"))
        ->Finish();

    ((coreProgram*)    this->Load<coreProgram>("default_label_sharp_program", CORE_RESOURCE_UPDATE_AUTO, NULL)->GetResource())
        ->AttachShader(this->Get <coreShader> ("default_label.vert"))
        ->AttachShader(this->Get <coreShader> ("default_label_sharp.frag"))
        ->Finish();

    Core::Log->Info("Default resources loaded");
}