///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_RESOURCE_H_
#define _CORE_GUARD_RESOURCE_H_

// TODO 3: call OnLoad directly after load instead with delayed function callback ?
// TODO 4: resources exist only within handles, redefine all interfaces
// TODO 5: investigate possible GPU memory fragmentation when streaming in and out lots of resources
// TODO 1: defer resource-unload on ref-count 0 to an explicit call at the end of a frame, and expose explicit unload function (nullify?) -> can also be used for restart
// TODO 3: set textures which are still loaded to default values (default_white.png, default_normal.png) "placeholder", as those do not prevent rendering (like models and shaders), and cause flickering (because textures of previous render-calls are used), but how to handle IsLoaded/IsUsable state for situations where components wait on textures, maybe set manually, or use LOAD option
// TODO 3: set resources which could not be found (or loaded) to fallback values


// ****************************************************************
/* resource definitions */
#define CORE_RESOURCE_INDICES (4096u)   // max number of concurrent resource indices

using coreResourceIndex = coreUint16;   // resource index type

enum coreResourceUpdate : coreBool
{
    CORE_RESOURCE_UPDATE_MANUAL = false,   // updated and managed by the developer
    CORE_RESOURCE_UPDATE_AUTO   = true     // updated automatically by the resource manager
};

enum coreResourceReset : coreBool
{
    CORE_RESOURCE_RESET_EXIT = false,   // invoke shut down routine
    CORE_RESOURCE_RESET_INIT = true     // invoke start up routine
};


// ****************************************************************
/* resource interface */
class INTERFACE coreResource
{
protected:
    coreString m_sName;   // resource name from the resource manager


public:
    coreResource()noexcept : m_sName ("") {}
    virtual ~coreResource() = default;

    ENABLE_COPY(coreResource)

    /* load and unload resource data */
    virtual coreStatus Load(coreFile* pFile) = 0;
    virtual coreStatus Unload()              = 0;

    /* assign resource name */
    inline void AssignName(const coreChar* pcName) {m_sName = pcName;}

    /* get object properties */
    inline const coreChar* GetName()const {return m_sName.c_str();}
};


// ****************************************************************
/* resource dummy class */
class coreResourceDummy final : public coreResource
{
public:
    /* load and unload resource data (without any effect) */
    coreStatus Load(coreFile* pFile)final {return CORE_OK;}
    coreStatus Unload()final              {return CORE_OK;}
};


// ****************************************************************
/* resource handle class */
class coreResourceHandle final
{
private:
    coreResource* m_pResource;             // handled resource object
    coreFile*     m_pFile;                 // pointer to resource file

    coreString m_sName;                    // identifier of this resource handle
    coreBool   m_bAutomatic;               // updated automatically by the resource manager
    coreBool   m_bProxy;                   // resource proxy without own resource

    coreResourceIndex m_iIndex;            // unique resource index

    coreSpinLock           m_UpdateLock;   // spinlock to prevent concurrent resource loading
    coreStatus             m_eStatus;      // current resource status
    coreAtomic<coreUint16> m_iRefCount;    // simple reference-counter


private:
    coreResourceHandle(coreResource* pResource, coreFile* pFile, const coreChar* pcName, const coreBool bAutomatic)noexcept;
    ~coreResourceHandle();


public:
    FRIEND_CLASS(coreResourceManager)
    DISABLE_COPY(coreResourceHandle)

    /* access resource object and status */
    inline coreResource*   GetRawResource()const {return m_pResource;}
    inline const coreBool& IsAutomatic   ()const {return m_bAutomatic;}
    inline       coreBool  IsSuccessful  ()const {return (m_eStatus == CORE_OK);}
    inline       coreBool  IsLoaded      ()const {return (m_eStatus != CORE_BUSY);}
    inline       coreBool  IsLoading     ()const {return (!this->IsLoaded() && m_iRefCount);}

    /* control the reference-counter */
    inline void RefIncrease() {m_iRefCount.FetchAdd(1u); ASSERT(m_iRefCount)}
    inline void RefDecrease() {ASSERT(m_iRefCount) if(!m_iRefCount.SubFetch(1u)) this->Nullify();}

    /* handle resource loading */
    inline coreBool Update () {if(!m_bProxy) {coreSpinLocker oLocker(&m_UpdateLock); if(this->IsLoading() && !m_bAutomatic)      {m_eStatus = m_pResource->Load(m_pFile);                      return true;}} return false;}
    inline coreBool Reload () {if(!m_bProxy) {coreSpinLocker oLocker(&m_UpdateLock); m_pResource->Unload(); if(this->IsLoaded()) {m_eStatus = m_pResource->Load(m_pFile);                      return true;}} return false;}
    inline coreBool Nullify() {if(!m_bProxy) {coreSpinLocker oLocker(&m_UpdateLock); m_pResource->Unload(); if(this->IsLoaded()) {m_eStatus = (m_pFile || m_bAutomatic) ? CORE_BUSY : CORE_OK; return true;}} return false;}

    /* attach asynchronous callbacks */
    template <typename F> coreUint32 OnLoadedOnce(F&& nFunction)const;   // [](void) -> void

    /* get object properties */
    inline const coreChar*          GetName    ()const {return m_sName.c_str();}
    inline const coreResourceIndex& GetIndex   ()const {return m_iIndex;}
    inline const coreStatus&        GetStatus  ()const {return m_eStatus;}
    inline       coreUint16         GetRefCount()const {return m_iRefCount;}


private:
    /* handle automatic resource loading */
    inline coreBool __CanAutoUpdate() {if(m_UpdateLock.TryLock()) {if(this->IsLoading() && m_bAutomatic) return true; m_UpdateLock.Unlock();} return false;}
    inline void     __AutoUpdate   () {m_eStatus = m_pResource->Load(m_pFile); m_UpdateLock.Unlock();}
};


// ****************************************************************
/* relation interface */
class INTERFACE coreResourceRelation
{
public:
    coreResourceRelation()noexcept;
    coreResourceRelation(const coreResourceRelation& c)noexcept;
    virtual ~coreResourceRelation();

    FRIEND_CLASS(coreResourceManager)


private:
    /* reset with the resource manager */
    virtual void __Reset(const coreResourceReset eInit) = 0;

    /* reshape with the resource manager */
    virtual void __Reshape();
};


// ****************************************************************
/* resource manager */
class coreResourceManager final : public coreThread
{
private:
    coreMapStr<coreResourceHandle*> m_apHandle;                            // resource handles

    coreMapStr<coreArchive*> m_apArchive;                                  // archives with resource files
    coreMapStr<coreFile*>    m_apDirectFile;                               // direct resource files

    coreMap<coreResourceHandle*, coreResourceHandle*> m_apProxy;           // resource proxies pointing to foreign handles <proxy, foreign>

    coreSet<coreResourceRelation*> m_apRelation;                           // objects to reset with the resource manager

    coreSpinLock m_ResourceLock;                                           // spinlock to prevent invalid resource handle access
    coreSpinLock m_FileLock;                                               // spinlock to prevent invalid resource file access
    coreBool     m_bActive;                                                // current management status

    static coreResourceHandle* s_apHandleTable  [CORE_RESOURCE_INDICES];   // resource handle index table
    static coreResource*       s_apResourceTable[CORE_RESOURCE_INDICES];   // resource object index table (to remove one indirection)
    static coreResourceIndex   s_iTableStart;                              // next index table entry to check


private:
    coreResourceManager()noexcept;
    ~coreResourceManager()final;


public:
    FRIEND_CLASS(Core)
    FRIEND_CLASS(coreResourceRelation)
    DISABLE_COPY(coreResourceManager)

    /* update the resource manager */
    void UpdateResources(const coreFloat fBudgetSec = FLT_MAX);
    void UpdateWait();
    inline coreBool  IsLoading   ()const {return std::any_of  (m_apHandle.begin(), m_apHandle.end(), [](const coreResourceHandle* pHandle) {return pHandle->IsLoading();});}
    inline coreUintW IsLoadingNum()const {return std::count_if(m_apHandle.begin(), m_apHandle.end(), [](const coreResourceHandle* pHandle) {return pHandle->IsLoading();});}

    /* create and delete resource and resource handle */
    template <typename T, typename... A>                 coreResourceHandle* Load     (const coreHashString& sName, const coreResourceUpdate eUpdate, const coreHashString& sPath, A&&... vArgs);
    template <typename T, typename... A> RETURN_RESTRICT coreResourceHandle* LoadNew  (A&&... vArgs)const;
    inline                                               coreResourceHandle* LoadProxy(const coreHashString& sName);
    template <typename T>                                void                Free     (coreResourcePtr<T>* OUTPUT pptResourcePtr);

    /* get existing resource handle */
    template <typename T> inline coreResourceHandle* Get(const coreHashString& sName) {if(!sName) return NULL; ASSERT(m_apHandle.count_bs(sName)) return this->Load<T>(sName, CORE_RESOURCE_UPDATE_AUTO, NULL);}

    /* retrieve archives and resource files */
    coreArchive* RetrieveArchive(const coreHashString& sPath);
    coreFile*    RetrieveFile   (const coreHashString& sPath);
    void         FolderScan     (const coreChar* pcPath, const coreChar* pcFilter, coreList<coreString>* OUTPUT pasOutput);

    /* point resource proxy to foreign handle */
    void        AssignProxy(coreResourceHandle*   pProxy, coreResourceHandle*   pForeign);
    inline void AssignProxy(coreResourceHandle*   pProxy, const coreHashString& sForeign) {this->AssignProxy(pProxy,                               this->Get<coreResourceDummy>(sForeign));}
    inline void AssignProxy(const coreHashString& sProxy, coreResourceHandle*   pForeign) {this->AssignProxy(this->Get<coreResourceDummy>(sProxy), pForeign);}
    inline void AssignProxy(const coreHashString& sProxy, const coreHashString& sForeign) {this->AssignProxy(this->Get<coreResourceDummy>(sProxy), this->Get<coreResourceDummy>(sForeign));}

    /* reset all resources and relation-objects */
    void Reset(const coreResourceReset eInit);
    void Reset();

    /* reshape all resources and relation-objects */
    void Reshape();

    /* manage resource index table */
    static void AllocIndex(coreResourceHandle* OUTPUT pHandle);
    static void FreeIndex (coreResourceHandle* OUTPUT pHandle);
    static inline coreResourceHandle* FetchHandle  (const coreResourceIndex iIndex) {ASSERT(iIndex < CORE_RESOURCE_INDICES) return s_apHandleTable  [iIndex];}
    static inline coreResource*       FetchResource(const coreResourceIndex iIndex) {ASSERT(iIndex < CORE_RESOURCE_INDICES) return s_apResourceTable[iIndex];}


private:
    /* resource thread implementations */
    coreStatus __InitThread()final;
    coreStatus __RunThread ()final;
    void       __ExitThread()final;

    /* load all relevant default resources */
    void __LoadDefault();

    /* bind and unbind relation-objects */
    inline void __BindRelation  (coreResourceRelation* pRelation) {ASSERT(!m_apRelation.count_bs(pRelation)) m_apRelation.insert_bs(pRelation);}
    inline void __UnbindRelation(coreResourceRelation* pRelation) {ASSERT( m_apRelation.count_bs(pRelation)) m_apRelation.erase_bs (pRelation);}
};


// ****************************************************************
/* resource access class */
template <typename T> class coreResourcePtr final
{
private:
    coreResourceIndex m_iIndex;   // resource index


public:
    constexpr coreResourcePtr(std::nullptr_t p = NULL)noexcept;
    coreResourcePtr(coreResourceHandle* pHandle)noexcept;
    coreResourcePtr(const coreResourcePtr<T>& c)noexcept;
    coreResourcePtr(coreResourcePtr<T>&&      m)noexcept;
    ~coreResourcePtr();

    DISABLE_HEAP

    /* assignment operations */
    coreResourcePtr<T>& operator = (coreResourcePtr<T> o)noexcept;

    /* access resource object and resource handle */
    inline T*                  GetResource()const {return d_cast<T*>(coreResourceManager::FetchResource(m_iIndex));}
    inline coreResourceHandle* GetHandle  ()const {return coreResourceManager::FetchHandle(m_iIndex);}
    inline explicit operator coreBool     ()const {return m_iIndex;}
    inline T*       operator ->           ()const {return  this->GetResource();}
    inline T&       operator *            ()const {return *this->GetResource();}

    /* check for usable resource object */
    inline coreBool IsUsable()const {return (m_iIndex && this->GetHandle()->IsSuccessful());}

    /* attach asynchronous callbacks */
    template <typename F> coreUint32 OnUsableOnce(F&& nFunction)const {ASSERT(m_iIndex) coreResourceHandle* pHandle = this->GetHandle(); return pHandle->OnLoadedOnce([=, nFunction = std::forward<F>(nFunction)]() {if(pHandle->IsSuccessful()) nFunction();});}   // [](void) -> void
};


// ****************************************************************
/* dummy resource access type */
using coreDummyPtr = coreResourcePtr<coreResourceDummy>;


// ****************************************************************
/* attach asynchronous callbacks */
template <typename F> coreUint32 coreResourceHandle::OnLoadedOnce(F&& nFunction)const
{
    if(this->IsLoaded())
    {
        // call function immediately
        nFunction();
        return 0u;
    }
    else
    {
        // attach wrapper to the resource thread
        return Core::Manager::Resource->AttachFunction([this, nFunction = std::forward<F>(nFunction)]()
        {
            if(this->IsLoaded())
            {
                // call and remove function when loaded
                nFunction();
                return CORE_OK;
            }
            if(!this->IsLoading())
            {
                // remove function when not loading anymore
                return CORE_OK;
            }
            return CORE_BUSY;
        });
    }
}


// ****************************************************************
/* create resource and resource handle */
template <typename T, typename... A> coreResourceHandle* coreResourceManager::Load(const coreHashString& sName, const coreResourceUpdate eUpdate, const coreHashString& sPath, A&&... vArgs)
{
    // check for existing resource handle
    if(m_apHandle.count_bs(sName)) return m_apHandle.at_bs(sName);

    // create new resource handle
    coreResourceHandle* pNewHandle = MANAGED_NEW(coreResourceHandle, std::is_same<T, coreResourceDummy>::value ? NULL : new T(std::forward<A>(vArgs)...), sPath ? this->RetrieveFile(sPath) : NULL, sName.GetString(), eUpdate ? true : false);

    m_ResourceLock.Lock();
    {
        // add resource handle to manager
        m_apHandle.emplace_bs(sName, pNewHandle);
    }
    m_ResourceLock.Unlock();

    return pNewHandle;
}

template <typename T, typename... A> RETURN_RESTRICT coreResourceHandle* coreResourceManager::LoadNew(A&&... vArgs)const
{
    // create unique unmanaged resource handle
    return MANAGED_NEW(coreResourceHandle, new T(std::forward<A>(vArgs)...), NULL, "", false);
}

inline coreResourceHandle* coreResourceManager::LoadProxy(const coreHashString& sName)
{
    // check for existing resource proxy
    if(m_apHandle.count_bs(sName))
    {
        ASSERT(m_apProxy.count(m_apHandle.at_bs(sName)))
        return m_apHandle.at_bs(sName);
    }

    // create new resource proxy without own resource
    coreResourceHandle* pNewProxy = this->Load<coreResourceDummy>(sName, CORE_RESOURCE_UPDATE_MANUAL, NULL);

    // mark as resource proxy
    pNewProxy->m_bProxy = true;

    // add resource proxy to manager
    m_apProxy.emplace(pNewProxy, NULL);

    return pNewProxy;
}


// ****************************************************************
/* delete resource and resource handle */
template <typename T> void coreResourceManager::Free(coreResourcePtr<T>* OUTPUT pptResourcePtr)
{
    ASSERT(pptResourcePtr)

    // retrieve resource handle
    coreResourceHandle* pHandle = pptResourcePtr->GetHandle();
    if(pHandle)
    {
        if(!pHandle->m_sName.empty())
        {
            m_ResourceLock.Lock();
            {
                // remove resource handle from manager
                m_apHandle.erase_bs(pHandle->m_sName.c_str());
            }
            m_ResourceLock.Unlock();
        }

        // delete possible resource proxy
        m_apProxy.erase(pHandle);

        // wait on possible resource loading
        while(pHandle->m_UpdateLock.IsLocked()) CORE_SPINLOCK_YIELD   // # locked again in destructor

        // delete resource handle
        (*pptResourcePtr) = NULL;
        MANAGED_DELETE(pHandle)
    }
}


// ****************************************************************
/* constructor */
template <typename T> constexpr coreResourcePtr<T>::coreResourcePtr(std::nullptr_t)noexcept
: m_iIndex (0u)
{
}

template <typename T> coreResourcePtr<T>::coreResourcePtr(coreResourceHandle* pHandle)noexcept
: m_iIndex (pHandle ? pHandle->GetIndex() : 0u)
{
    if(m_iIndex) pHandle->RefIncrease();
}

template <typename T> coreResourcePtr<T>::coreResourcePtr(const coreResourcePtr<T>& c)noexcept
: m_iIndex (c.m_iIndex)
{
    if(m_iIndex) this->GetHandle()->RefIncrease();
}

template <typename T> coreResourcePtr<T>::coreResourcePtr(coreResourcePtr<T>&& m)noexcept
: m_iIndex (m.m_iIndex)
{
    m.m_iIndex = 0u;
}


// ****************************************************************
/* destructor */
template <typename T> coreResourcePtr<T>::~coreResourcePtr()
{
    if(m_iIndex) this->GetHandle()->RefDecrease();
}


// ****************************************************************
/* assignment operations */
template <typename T> coreResourcePtr<T>& coreResourcePtr<T>::operator = (coreResourcePtr<T> o)noexcept
{
    // swap properties
    std::swap(m_iIndex, o.m_iIndex);

    return *this;
}


#endif /* _CORE_GUARD_RESOURCE_H_ */