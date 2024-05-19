///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

alignas(ALIGNMENT_PAGE) coreResourceHandle* coreResourceManager::s_apHandleTable  [CORE_RESOURCE_INDICES] = {};   // # pages are only allocated on first access
alignas(ALIGNMENT_PAGE) coreResource*       coreResourceManager::s_apResourceTable[CORE_RESOURCE_INDICES] = {};
coreResourceIndex                           coreResourceManager::s_iTableStart                            = 1u;


// ****************************************************************
/* constructor */
coreResourceHandle::coreResourceHandle(coreResource* pResource, coreFile* pFile, const coreChar* pcName, const coreBool bAutomatic)noexcept
: m_pResource  (pResource)
, m_pFile      (pFile)
, m_sName      (pcName)
, m_bAutomatic (bAutomatic)
, m_bProxy     (false)
, m_iIndex     (0u)
, m_UpdateLock ()
, m_eStatus    ((pFile || bAutomatic) ? CORE_BUSY : CORE_OK)
, m_iRefCount  (0u)
{
    // assign resource name
    if(m_pResource) m_pResource->AssignName(pcName);

    // acquire resource index table space
    coreResourceManager::AllocIndex(this);

    // always load into memory
    if(Core::Config->GetBool(CORE_CONFIG_BASE_PERSISTMODE) || DEFINED(_CORE_SWITCH_))
    {
        if(m_bAutomatic) this->RefIncrease();
    }
}


// ****************************************************************
/* destructor */
coreResourceHandle::~coreResourceHandle()
{
    // unload from memory
    if(Core::Config->GetBool(CORE_CONFIG_BASE_PERSISTMODE) || DEFINED(_CORE_SWITCH_))
    {
        if(m_bAutomatic) this->RefDecrease();
    }

    // release resource index table space
    coreResourceManager::FreeIndex(this);

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
/* reshape with the resource manager */
void coreResourceRelation::__Reshape()
{
    // reset by default
    this->__Reset(CORE_RESOURCE_RESET_EXIT);
    this->__Reset(CORE_RESOURCE_RESET_INIT);
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
    this->SetFrequency(120.0f);

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
void coreResourceManager::UpdateResources(const coreFloat fBudgetSec)
{
    if(m_bActive)
    {
        const coreUint64 iStart = SDL_GetPerformanceCounter();

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

                    // test current budget and stop processing
                    if(coreFloat(coreDouble(SDL_GetPerformanceCounter() - iStart) * Core::System->GetPerfFrequency()) >= fBudgetSec) break;
                }
            }
        }
        m_ResourceLock.Unlock();
    }
}


// ****************************************************************
/* update the resource manager (until loading is finished) */
void coreResourceManager::UpdateWait()
{
    do
    {
        CORE_SPINLOCK_YIELD

        // update both resources and functions
        this->UpdateResources();
        this->UpdateFunctions();
    }
    while(this->IsLoading() && !DEFINED(_CORE_EMSCRIPTEN_));
}


// ****************************************************************
/* retrieve archive */
coreArchive* coreResourceManager::RetrieveArchive(const coreHashString& sPath)
{
    const coreSpinLocker oLocker(&m_FileLock);

    // check for existing archive
    if(m_apArchive.count_bs(sPath)) return m_apArchive.at_bs(sPath);

    // load new archive
    coreArchive* pNewArchive = MANAGED_NEW(coreArchive, sPath.GetString());
    m_apArchive.emplace_bs(sPath, pNewArchive);

    return pNewArchive;
}


// ****************************************************************
/* retrieve resource file */
coreFile* coreResourceManager::RetrieveFile(const coreHashString& sPath)
{
    const coreSpinLocker oLocker(&m_FileLock);

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
/* retrieve relative paths of all available resource files */
void coreResourceManager::FolderScan(const coreChar* pcPath, const coreChar* pcFilter, coreList<coreString>* OUTPUT pasOutput)
{
    ASSERT(pcPath && pcFilter && pasOutput)

    // find direct resource files
    coreData::FolderScan(pcPath, pcFilter, pasOutput);

    // find resources files in archives
    const coreChar* pcPattern = PRINT("%s/%s", pcPath, pcFilter);
    FOR_EACH(it, m_apArchive)
    {
        for(coreUintW i = 0u, ie = (*it)->GetNumFiles(); i < ie; ++i)
        {
            // check and add file path
            const coreChar* pcFile = (*it)->GetFile(i)->GetPath();
            if(coreData::StrCmpLike(pcFile, pcPattern))
            {
                pasOutput->push_back(pcFile);
            }
        }
    }

    // sort and remove duplicates
    std::sort(pasOutput->begin(), pasOutput->end());
    pasOutput->erase(std::unique(pasOutput->begin(), pasOutput->end()), pasOutput->end());
}


// ****************************************************************
/* point resource proxy to foreign handle */
void coreResourceManager::AssignProxy(coreResourceHandle* pProxy, coreResourceHandle* pForeign)
{
    ASSERT(m_apProxy.count(pProxy) && !m_apProxy.count(pForeign) && (pProxy != pForeign))

    // check current foreign handle (to prevent reloading)
    if(m_apProxy.at(pProxy) == pForeign) return;

    if(pProxy->m_pResource)
    {
        // decrease old reference-counter
        m_apProxy.at(pProxy)->RefDecrease();
    }

    // point resource proxy to foreign handle
    pProxy->m_pResource = pForeign ? pForeign->m_pResource : NULL;
    pProxy->m_eStatus   = pForeign ? CORE_BUSY             : CORE_OK;

    if(pProxy->m_pResource)
    {
        // increase new reference-counter
        pForeign->RefIncrease();

        // forward status of the foreign handle
        pForeign->OnLoadedOnce([=]() {pProxy->m_eStatus = pForeign->m_eStatus;});
    }

    // update resource index table
    s_apResourceTable[pProxy->m_iIndex] = pProxy->m_pResource;

    // save new foreign handle
    m_apProxy.at(pProxy) = pForeign;
}


// ****************************************************************
/* refresh resource proxy */
void coreResourceManager::RefreshProxy(coreResourceHandle* pProxy)
{
    // retrieve foreign handle
    coreResourceHandle* pForeign = m_apProxy.at(pProxy);
    if(pForeign)
    {
        // reset and forward status again
        pProxy->m_eStatus = CORE_BUSY;
        pForeign->OnLoadedOnce([=]() {pProxy->m_eStatus = pForeign->m_eStatus;});
    }
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
        // refresh resource proxies
        FOR_EACH(it, m_apProxy)
            this->RefreshProxy(*m_apProxy.get_key(it));

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

        // clear global shader code
        coreShader::ClearGlobalCode();
    }
}

void coreResourceManager::Reset()
{
    ASSERT(m_bActive)

    // reset without separation
    this->Reset(CORE_RESOURCE_RESET_EXIT);
    this->Reset(CORE_RESOURCE_RESET_INIT);
}


// ****************************************************************
/* reshape all resources and relation-objects */
void coreResourceManager::Reshape()
{
    // copy list with relation-objects (container may change)
    const coreSet<coreResourceRelation*> apRelationCopy = m_apRelation;

    // reshape relation-objects
    FOR_EACH(it, apRelationCopy)
        if(m_apRelation.count_bs(*it)) (*it)->__Reshape();
}


// ****************************************************************
/* acquire resource index table space */
void coreResourceManager::AllocIndex(coreResourceHandle* OUTPUT pHandle)
{
    ASSERT(pHandle && !pHandle->m_iIndex && (SDL_ThreadID() == Core::System->GetMainThread()))

    for(coreUintW i = s_iTableStart; i < CORE_RESOURCE_INDICES; ++i)
    {
        if(!s_apHandleTable[i])
        {
            // fill resource index table
            s_apHandleTable  [i] = pHandle;
            s_apResourceTable[i] = pHandle->m_pResource;

            // advance table start
            s_iTableStart = i + 1u;

            // assign resource index
            pHandle->m_iIndex = i;
            return;
        }
    }

    // no free resource index table space available
    WARN_IF(true) {}
}


// ****************************************************************
/* release resource index table space */
void coreResourceManager::FreeIndex(coreResourceHandle* OUTPUT pHandle)
{
    ASSERT(pHandle && pHandle->m_iIndex && (SDL_ThreadID() == Core::System->GetMainThread()))

    const coreResourceIndex iIndex = pHandle->m_iIndex;
    ASSERT(pHandle == s_apHandleTable[iIndex])

    // clear resource index table
    s_apHandleTable  [iIndex] = NULL;
    s_apResourceTable[iIndex] = NULL;

    // rewind table start
    s_iTableStart = MIN(s_iTableStart, iIndex);

    // reset resource index
    pHandle->m_iIndex = 0u;
}


// ****************************************************************
/* init resource thread */
coreStatus coreResourceManager::__InitThread()
{
    const SDL_GLContext& pContext = Core::Graphics->GetResourceContext();

    // assign resource context to resource thread
    if((SDL_GL_MakeCurrent(Core::System->GetWindow(), pContext) && SDL_GL_MakeCurrent(NULL, pContext)) || (SDL_GL_GetCurrentContext() != pContext))
    {
        Core::Log->Warning("Resource context could not be assigned to resource thread (SDL: %s)", SDL_GetError());
        return CORE_ERROR_SYSTEM;
    }
    Core::Log->Info("Resource context assigned to resource thread");

    // enable OpenGL debug output
    Core::Graphics->DebugOpenGL();

    // setup texturing and packing
    if(CORE_GL_SUPPORT(V2_compatibility) || DEFINED(_CORE_GLES_)) glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
    glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);
    glPixelStorei(GL_PACK_ALIGNMENT,   4);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    // enable parallel shader compilation
    if(CORE_GL_SUPPORT(ARB_parallel_shader_compile))
        glMaxShaderCompilerThreadsARB(0xFFFFFFFFu);

    return CORE_OK;
}


// ****************************************************************
/* run resource thread */
coreStatus coreResourceManager::__RunThread()
{
    // update the resource manager
    this->UpdateResources();

    // check for system errors
    coreData::CheckLastError();

    // check for OpenGL errors
    Core::Graphics->CheckOpenGL();

    return CORE_OK;
}


// ****************************************************************
/* exit resource thread */
void coreResourceManager::__ExitThread()
{
    // dissociate resource context from resource thread
    if(SDL_GL_MakeCurrent(Core::System->GetWindow(), NULL)) SDL_GL_MakeCurrent(NULL, NULL);
}


// ****************************************************************
/* load all relevant default resources */
void coreResourceManager::__LoadDefault()
{
    coreList<coreString> asArchiveList;

    coreData::FolderScan("data/archives", "*.cfa", &asArchiveList);
    FOR_EACH(it, asArchiveList) this->RetrieveArchive(it->c_str());

    this->Load<coreTexture>("default_black.webp",             CORE_RESOURCE_UPDATE_AUTO,   "data/textures/default_black.webp",  CORE_TEXTURE_LOAD_NO_COMPRESS | CORE_TEXTURE_LOAD_NO_FILTER | CORE_TEXTURE_LOAD_NEAREST);
    this->Load<coreTexture>("default_normal.webp",            CORE_RESOURCE_UPDATE_AUTO,   "data/textures/default_normal.webp", CORE_TEXTURE_LOAD_NO_COMPRESS | CORE_TEXTURE_LOAD_NO_FILTER | CORE_TEXTURE_LOAD_NEAREST);
    this->Load<coreTexture>("default_white.webp",             CORE_RESOURCE_UPDATE_AUTO,   "data/textures/default_white.webp",  CORE_TEXTURE_LOAD_NO_COMPRESS | CORE_TEXTURE_LOAD_NO_FILTER | CORE_TEXTURE_LOAD_NEAREST);
    this->Load<coreShader> ("default_2d.vert",                CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_2d.vert");
    this->Load<coreShader> ("default_2d.frag",                CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_2d.frag");
    this->Load<coreShader> ("default_3d_low.vert",            CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_3d_low.vert");
    this->Load<coreShader> ("default_3d_raw.vert",            CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_3d_raw.vert");
    this->Load<coreShader> ("default_3d.frag",                CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_3d.frag");
    this->Load<coreShader> ("default_label.vert",             CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_label.vert");
    this->Load<coreShader> ("default_label_sharp.frag",       CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_label_sharp.frag");
    this->Load<coreShader> ("default_label_smooth.frag",      CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_label_smooth.frag");
    this->Load<coreShader> ("default_particle.vert",          CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_particle.vert", CORE_SHADER_OPTION_INSTANCING);
    this->Load<coreShader> ("default_particle.frag",          CORE_RESOURCE_UPDATE_MANUAL, "data/shaders/default_particle.frag", CORE_SHADER_OPTION_INSTANCING);
    this->Load<coreFont>   ("default.ttf",                    CORE_RESOURCE_UPDATE_AUTO,   "data/fonts/default.ttf");

    d_cast<coreProgram*>(this->Load<coreProgram>("default_2d_program", CORE_RESOURCE_UPDATE_AUTO, NULL)->GetRawResource())
        ->AttachShader  (this->Get <coreShader> ("default_2d.vert"))
        ->AttachShader  (this->Get <coreShader> ("default_2d.frag"))
        ->Finish();

    d_cast<coreProgram*>(this->Load<coreProgram>("default_3d_low_program", CORE_RESOURCE_UPDATE_AUTO, NULL)->GetRawResource())
        ->AttachShader  (this->Get <coreShader> ("default_3d_low.vert"))
        ->AttachShader  (this->Get <coreShader> ("default_3d.frag"))
        ->Finish();

    d_cast<coreProgram*>(this->Load<coreProgram>("default_3d_raw_program", CORE_RESOURCE_UPDATE_AUTO, NULL)->GetRawResource())
        ->AttachShader  (this->Get <coreShader> ("default_3d_raw.vert"))
        ->AttachShader  (this->Get <coreShader> ("default_3d.frag"))
        ->Finish();

    d_cast<coreProgram*>(this->Load<coreProgram>("default_label_sharp_program", CORE_RESOURCE_UPDATE_AUTO, NULL)->GetRawResource())
        ->AttachShader  (this->Get <coreShader> ("default_label.vert"))
        ->AttachShader  (this->Get <coreShader> ("default_label_sharp.frag"))
        ->Finish();

    d_cast<coreProgram*>(this->Load<coreProgram>("default_label_smooth_program", CORE_RESOURCE_UPDATE_AUTO, NULL)->GetRawResource())
        ->AttachShader  (this->Get <coreShader> ("default_label.vert"))
        ->AttachShader  (this->Get <coreShader> ("default_label_smooth.frag"))
        ->Finish();

    d_cast<coreProgram*>(this->Load<coreProgram>("default_particle_program", CORE_RESOURCE_UPDATE_AUTO, NULL)->GetRawResource())
        ->AttachShader  (this->Get <coreShader> ("default_particle.vert"))
        ->AttachShader  (this->Get <coreShader> ("default_particle.frag"))
        ->Finish();

    Core::Log->Info("Default resources loaded");
}