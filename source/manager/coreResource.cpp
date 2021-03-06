///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
coreResourceHandle::coreResourceHandle(coreResource* pResource, coreFile* pFile, const coreChar* pcName, const coreBool bAutomatic)noexcept
: m_pResource  (pResource)
, m_pFile      (pFile)
, m_sName      (pcName)
, m_bAutomatic (bAutomatic)
, m_eStatus    ((pFile || bAutomatic) ? CORE_BUSY : CORE_OK)
, m_iRefCount  (0u)
, m_UpdateLock ()
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
: coreThread     ("resource_thread")
, m_apHandle     {}
, m_apArchive    {}
, m_apDirectFile {}
, m_apProxy      {}
, m_apRelation   {}
, m_ResourceLock ()
, m_FileLock     ()
, m_bActive      (false)
{
    // configure resource thread
    this->SetFrequency(100.0f);

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
    ASSERT(m_apRelation.empty())

    // shut down the resource manager
    this->Reset(CORE_RESOURCE_RESET_EXIT);

    // reset resource proxies
    FOR_EACH(it, m_apProxy.get_keylist()) this->AssignProxy(*it, NULL);

    // delete resource handles
    FOR_EACH(it, m_apHandle) MANAGED_DELETE(*it)

    // delete resource files
    FOR_EACH(it, m_apArchive)    MANAGED_DELETE(*it)
    FOR_EACH(it, m_apDirectFile) MANAGED_DELETE(*it)

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
    if(m_bActive)
    {
        m_ResourceLock.Lock();
        {
            // loop through all resource handles
            for(coreUintW i = 0u; i < m_apHandle.size(); ++i)   // # size may change
            {
                coreResourceHandle* pCurHandle = m_apHandle[i];

                // check for requirements
                if(pCurHandle->__CanAutoUpdate())
                {
                    m_ResourceLock.Unlock();
                    {
                        // update resource handle
                        pCurHandle->__AutoUpdate();
                    }
                    m_ResourceLock.Lock();
                }
            }
        }
        m_ResourceLock.Unlock();
    }
}


// ****************************************************************
/* retrieve archive */
coreArchive* coreResourceManager::RetrieveArchive(const coreHashString& sPath)
{
    coreSpinLocker oLocker(&m_FileLock);

    // check for existing archive
    if(m_apArchive.count_bs(sPath)) return m_apArchive.at_bs(sPath);

    // load new archive
    coreArchive* pNewArchive = MANAGED_NEW(coreArchive, sPath.GetString());
    m_apArchive.emplace_bs(sPath, pNewArchive);

    ASSERT(pNewArchive->GetNumFiles())
    return pNewArchive;
}


// ****************************************************************
/* retrieve resource file */
coreFile* coreResourceManager::RetrieveFile(const coreHashString& sPath)
{
    coreSpinLocker oLocker(&m_FileLock);

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
    if(m_apDirectFile.count_bs(sPath)) return m_apDirectFile.at_bs(sPath);

    // load new direct resource file
    coreFile* pNewFile = MANAGED_NEW(coreFile, sPath.GetString());
    m_apDirectFile.emplace_bs(sPath, pNewFile);

    return pNewFile;
}


// ****************************************************************
/* point resource proxy to foreign handle */
void coreResourceManager::AssignProxy(coreResourceHandle* OUTPUT pProxy, coreResourceHandle* OUTPUT pForeign)
{
    ASSERT(m_apProxy.count(&*pProxy) && (pProxy != pForeign))

    if(pProxy->m_pResource)
    {
        // decrease old reference-counter
        m_apProxy.at(&*pProxy)->RefDecrease();
    }

    // point resource proxy to foreign handle
    pProxy->m_pResource = pForeign ? pForeign->m_pResource : NULL;

    if(pProxy->m_pResource)
    {
        // increase new reference-counter
        pForeign->RefIncrease();

        // forward status of the foreign handle
        pProxy->m_eStatus = CORE_BUSY;
        pForeign->OnLoadedOnce([=]() {pProxy->m_eStatus = pForeign->m_eStatus;});
    }

    // save new foreign handle
    m_apProxy.at(&*pProxy) = pForeign;
}


// ****************************************************************
/* reset all resources and relation-objects */
void coreResourceManager::Reset(const coreResourceReset eInit)
{
    const coreBool bActive = eInit ? true : false;

    // check and set current status
    if(m_bActive == bActive) return;
    m_bActive = bActive;

    // copy list with relation-objects (container may change)
    const coreSet<coreResourceRelation*> apRelationCopy = m_apRelation;

    if(m_bActive)
    {
        // reload resource proxies
        FOR_EACH(it, m_apProxy)
            this->AssignProxy((*m_apProxy.get_key(it)), (*it));

        // start up relation-objects
        FOR_EACH(it, apRelationCopy)
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
        FOR_EACH(it, apRelationCopy)
            if(m_apRelation.count_bs(*it)) (*it)->__Reset(CORE_RESOURCE_RESET_EXIT);

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

    // setup texturing and packing
    if(CORE_GL_SUPPORT(V2_compatibility) || DEFINED(_CORE_GLES_)) glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
    glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);
    glPixelStorei(GL_PACK_ALIGNMENT,   4);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    // disable parallel shader compilation
    if(CORE_GL_SUPPORT(ARB_parallel_shader_compile))
        glMaxShaderCompilerThreadsARB(0u);

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
/* load all relevant default resources */
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

    d_cast<coreProgram*>(this->Load<coreProgram>("default_2d_program", CORE_RESOURCE_UPDATE_AUTO, NULL)->GetRawResource())
        ->AttachShader  (this->Get <coreShader> ("default_2d.vert"))
        ->AttachShader  (this->Get <coreShader> ("default_2d.frag"))
        ->Finish();

    d_cast<coreProgram*>(this->Load<coreProgram>("default_label_sharp_program", CORE_RESOURCE_UPDATE_AUTO, NULL)->GetRawResource())
        ->AttachShader  (this->Get <coreShader> ("default_label.vert"))
        ->AttachShader  (this->Get <coreShader> ("default_label_sharp.frag"))
        ->Finish();

    d_cast<coreProgram*>(this->Load<coreProgram>("default_label_smooth_program", CORE_RESOURCE_UPDATE_AUTO, NULL)->GetRawResource())
        ->AttachShader  (this->Get <coreShader> ("default_label.vert"))
        ->AttachShader  (this->Get <coreShader> ("default_label_smooth.frag"))
        ->Finish();

    Core::Log->Info("Default resources loaded");
}