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
coreResourceHandle::coreResourceHandle(coreResource* pResource, coreFile* pFile, const coreChar* pcName, const coreBool bAutomatic)noexcept
: m_pResource  (pResource)
, m_pFile      (pFile)
, m_sName      (pcName)
, m_bAutomatic (bAutomatic)
, m_iStatus    ((pFile || bAutomatic) ? CORE_BUSY : CORE_OK)
, m_iRefCount  (0)
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
: coreThread      ("resource_thread")
, m_apHandle      {}
, m_apArchive     {}
, m_apDirectFile  {}
, m_apProxy       {}
, m_apRelation    {}
, m_iResourceLock (0)
, m_iFileLock     (0)
, m_bActive       (false)
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

    // reset resource proxies
    FOR_EACH(it, m_apProxy.get_keylist()) this->AssignProxy(*it, NULL);

    // delete resource handles
    FOR_EACH(it, m_apHandle) MANAGED_DELETE(coreResourceHandle, *it)

    // delete resource files
    FOR_EACH(it, m_apArchive)    MANAGED_DELETE(coreArchive, *it)
    FOR_EACH(it, m_apDirectFile) MANAGED_DELETE(coreFile,    *it)

    // clear memory
    m_apHandle    .clear();
    m_apArchive   .clear();
    m_apDirectFile.clear();
    m_apProxy     .clear();
    m_apRelation  .clear();

    Core::Log->Info(CORE_LOG_BOLD("Resource Manager destroyed"));
}


// ****************************************************************
/* update the resource manager */
void coreResourceManager::UpdateResources()
{
    // check for current status
    if(m_bActive)
    {
        coreAtomicLock(&m_iResourceLock);
        {
            for(coreUintW i = 0u; i < m_apHandle.size(); ++i)   // # size may change
            {
                // update resource handle
                if(m_apHandle[i]->__AutoUpdate())
                {
                    // allow changes during iteration
                    coreAtomicUnlock(&m_iResourceLock);
                    coreAtomicLock  (&m_iResourceLock);
                }
            }
        }
        coreAtomicUnlock(&m_iResourceLock);
    }
}


// ****************************************************************
/* retrieve archive */
coreArchive* coreResourceManager::RetrieveArchive(const coreHashString& sPath)
{
    coreLockRelease oRelease(m_iFileLock);

    // check for existing archive
    if(m_apArchive.count(sPath)) return m_apArchive.at(sPath);

    // load new archive
    coreArchive* pNewArchive = MANAGED_NEW(coreArchive, sPath.GetString());
    m_apArchive.emplace(sPath, pNewArchive);

    ASSERT(pNewArchive->GetNumFiles())
    return pNewArchive;
}


// ****************************************************************
/* retrieve resource file */
coreFile* coreResourceManager::RetrieveFile(const coreHashString& sPath)
{
    coreLockRelease oRelease(m_iFileLock);

    // try to open direct resource file first
    if(!coreData::FileExists(sPath.GetString()))
    {
        // check archives
        FOR_EACH(it, m_apArchive)
        {
            coreFile* pFile = (*it)->GetFile(sPath.GetString());
            if(pFile) return pFile;
        }

        // resource file not found
        WARN_IF(true) {}
    }

    // check for existing direct resource file
    if(m_apDirectFile.count(sPath)) return m_apDirectFile.at(sPath);

    // load new direct resource file
    coreFile* pNewFile = MANAGED_NEW(coreFile, sPath.GetString());
    m_apDirectFile.emplace(sPath, pNewFile);

    return pNewFile;
}


// ****************************************************************
/* point resource proxy to foreign handle */
void coreResourceManager::AssignProxy(coreResourceHandle* OUTPUT pProxy, coreResourceHandle* OUTPUT pForeign)
{
    ASSERT(m_apProxy.count(pProxy) && (pProxy != pForeign))

    if(pProxy->m_pResource)
    {
        // decrease old reference-counter
        m_apProxy.at(pProxy)->RefDecrease();
    }

    // point resource proxy to foreign handle
    pProxy->m_pResource = pForeign ? pForeign->m_pResource : NULL;

    if(pProxy->m_pResource)
    {
        // increase new reference-counter
        pForeign->RefIncrease();

        // forward status of the foreign handle
        pProxy->m_iStatus = CORE_BUSY;
        pForeign->OnLoadOnce([=]() {pProxy->m_iStatus = pForeign->m_iStatus;});
    }

    // save new foreign handle
    m_apProxy.at(pProxy) = pForeign;
}


// ****************************************************************
/* reset all resources and relation-objects */
void coreResourceManager::Reset(const coreResourceReset bInit)
{
    const coreBool bActive = bInit ? true : false;

    // check and set current status
    if(m_bActive == bActive) return;
    m_bActive = bActive;

    if(m_bActive)
    {
        // reload resource proxies
        FOR_EACH(it, m_apProxy)
            this->AssignProxy((*m_apProxy.get_key(it)), (*it));

        // start up relation-objects
        for(coreUintW i = 0u; i < m_apRelation.size(); ++i)
            m_apRelation[i]->__Reset(CORE_RESOURCE_RESET_INIT);

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
        for(coreUintW i = 0u; i < m_apRelation.size(); ++i)
            m_apRelation[i]->__Reset(CORE_RESOURCE_RESET_EXIT);

        // unload all resources
        FOR_EACH(it, m_apHandle)
            (*it)->Nullify();
    }
}


// ****************************************************************
/* init resource thread */
coreStatus coreResourceManager::__InitThread()
{
    // assign resource context to resource thread
    if(SDL_GL_MakeCurrent(Core::System->GetWindow(), Core::Graphics->GetResourceContext()))
        Core::Log->Error("Resource context could not be assigned to resource thread (SDL: %s)", SDL_GetError());
    else Core::Log->Info("Resource context assigned to resource thread");

    // enable OpenGL debug output
    Core::Log->DebugOpenGL();

    // setup texturing
    if(DEFINED(_CORE_GLES_)) glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
    glPixelStorei(GL_PACK_ALIGNMENT,   4);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    return CORE_OK;
}


// ****************************************************************
/* run resource thread */
coreStatus coreResourceManager::__RunThread()
{
    // update the resource manager
    this->UpdateResources();

    return CORE_OK;
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
    this->Load<coreTexture>("default_black.png",         CORE_RESOURCE_UPDATE_AUTO,   "data/textures/default_black.png");
    this->Load<coreTexture>("default_white.png",         CORE_RESOURCE_UPDATE_AUTO,   "data/textures/default_white.png");
    this->Load<coreShader> ("default_2d.vert",           CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_2d.vert");
    this->Load<coreShader> ("default_2d.frag",           CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_2d.frag");
    this->Load<coreShader> ("default_label.vert",        CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_label.vert");
    this->Load<coreShader> ("default_label_sharp.frag",  CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_label_sharp.frag");
    this->Load<coreShader> ("default_label_smooth.frag", CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_label_smooth.frag");
    this->Load<coreFont>   ("default.ttf",               CORE_RESOURCE_UPDATE_AUTO,   "data/fonts/default.ttf");

    s_cast<coreProgram*>(this->Load<coreProgram>("default_2d_program", CORE_RESOURCE_UPDATE_AUTO, NULL)->GetResource())
        ->AttachShader  (this->Get <coreShader> ("default_2d.vert"))
        ->AttachShader  (this->Get <coreShader> ("default_2d.frag"))
        ->Finish();

    s_cast<coreProgram*>(this->Load<coreProgram>("default_label_sharp_program", CORE_RESOURCE_UPDATE_AUTO, NULL)->GetResource())
        ->AttachShader  (this->Get <coreShader> ("default_label.vert"))
        ->AttachShader  (this->Get <coreShader> ("default_label_sharp.frag"))
        ->Finish();

    s_cast<coreProgram*>(this->Load<coreProgram>("default_label_smooth_program", CORE_RESOURCE_UPDATE_AUTO, NULL)->GetResource())
        ->AttachShader  (this->Get <coreShader> ("default_label.vert"))
        ->AttachShader  (this->Get <coreShader> ("default_label_smooth.frag"))
        ->Finish();

    Core::Log->Info("Default resources loaded");
}