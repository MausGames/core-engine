//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_RESOURCE_H_
#define _CORE_GUARD_RESOURCE_H_

// TODO: ATI_meminfo and NVX_gpu_memory_info to check for memory usage ? (old, not widely supported)
// TODO: variable templates for handle-list!
// TODO: replace reference-counter with atomic variable ?
// TODO: when and how to load default archive(s) ?
// TODO: assert for path and update-type in load-function
// TODO: check for resource context, with OGL spec 5.1.3: [gets not deleted when] the object is bound to a context bind point in any context
// TODO: extend OnLoad interface with new functions, call directly after load instead of threaded ?
// TODO: resources exist only within handles, redefine all interfaces


// ****************************************************************
/* resource definitions */
enum coreResourceUpdate : coreBool
{
    CORE_RESOURCE_UPDATE_MANUAL = false,   //!< updated and managed by the developer
    CORE_RESOURCE_UPDATE_AUTO   = true     //!< updated automatically by the resource manager
};

enum coreResourceReset : coreBool
{
    CORE_RESOURCE_RESET_EXIT = false,   //!< invoke shut down routine
    CORE_RESOURCE_RESET_INIT = true     //!< invoke start up routine
};


// ****************************************************************
/* resource interface */
class INTERFACE coreResource
{
protected:
    std::string m_sPath;   //!< relative path of the resource file


public:
    coreResource()noexcept : m_sPath ("") {}
    virtual ~coreResource() = default;

    ENABLE_COPY(coreResource)

    /*! load and unload resource data */
    //! @{
    virtual coreStatus Load(coreFile* pFile) = 0;
    virtual coreStatus Unload()              = 0;
    //! @}

    /*! get object properties */
    //! @{
    inline const coreChar* GetPath()const {return m_sPath.c_str();}
    //! @}
};


// ****************************************************************
/* resource handle class */
class coreResourceHandle final
{
private:
    coreResource* m_pResource;   //!< holding resource object
    coreFile* m_pFile;           //!< pointer to resource file

    std::string m_sName;         //!< identifier of this resource handle
    coreBool m_bAutomatic;       //!< updated automatically by the resource manager

    coreInt32  m_iRefCount;      //!< simple reference-counter
    coreStatus m_iStatus;        //!< current resource status


private:
    coreResourceHandle(coreResource* pResource, coreFile* pFile, const coreChar* pcName, const coreBool bAutomatic)noexcept;
    ~coreResourceHandle();


public:
    FRIEND_CLASS(coreResourceManager)
    DISABLE_COPY(coreResourceHandle)

    /*! access resource object and status */
    //! @{
    inline coreResource*   GetResource()const {return m_pResource;}
    inline const coreBool& IsAutomatic()const {return m_bAutomatic;}
    inline       coreBool  IsLoaded   ()const {return (m_iStatus != CORE_BUSY)           ? true : false;}
    inline       coreBool  IsLoading  ()const {return (!this->IsLoaded() && m_iRefCount) ? true : false;}
    //! @}

    /*! control the reference-counter */
    //! @{
    inline void RefIncrease() {++m_iRefCount;}
    inline void RefDecrease() {--m_iRefCount; ASSERT(m_iRefCount >= 0) if(!m_iRefCount) this->Nullify();}
    //! @}

    /*! handle resource loading */
    //! @{
    inline coreBool Update () {if(!this->IsLoaded() && m_iRefCount && !m_bAutomatic)       {m_iStatus = m_pResource->Load(m_pFile);                      return true;} return false;}
    inline coreBool Reload () {if( this->IsLoaded() && m_iRefCount) {m_pResource->Unload(); m_iStatus = m_pResource->Load(m_pFile);                      return true;} return false;}
    inline coreBool Nullify() {if( this->IsLoaded())                {m_pResource->Unload(); m_iStatus = (m_pFile || m_bAutomatic) ? CORE_BUSY : CORE_OK; return true;} return false;}
    //! @}

    /*! insert asynchronous functions */
    //! @{
    template <typename F> void OnLoadOnce(F&& nFunction)const;   //!< [](void) -> void
    //! @}

    /*! get object properties */
    //! @{
    inline const coreChar*   GetName    ()const {return m_sName.c_str();}
    inline const coreInt32&  GetRefCount()const {return m_iRefCount;}
    inline const coreStatus& GetStatus  ()const {return m_iStatus;}
    //! @}


private:
    /*! handle automatic resource loading */
    //! @{
    inline coreBool __AutoUpdate() {if(!this->IsLoaded() && m_iRefCount && m_bAutomatic) {m_iStatus = m_pResource->Load(m_pFile); return true;} return false;}
    //! @}
};


// ****************************************************************
/* resource access class */
template <typename T> class coreResourcePtr final
{
private:
    coreResourceHandle* m_pHandle;   //!< resource handle


public:
    constexpr coreResourcePtr(std::nullptr_t p = NULL)noexcept;
    coreResourcePtr(coreResourceHandle* pHandle)noexcept;
    coreResourcePtr(const coreResourcePtr<T>& c)noexcept;
    coreResourcePtr(coreResourcePtr<T>&&      m)noexcept;
    ~coreResourcePtr();

    DISABLE_NEW

    /*! assignment operator */
    //! @{
    coreResourcePtr<T>& operator = (coreResourcePtr<T> o)noexcept;
    //! @}

    /*! access resource object and resource handle */
    //! @{
    inline T* operator ->       ()const         {ASSERT(m_pHandle) return   s_cast<T*>(m_pHandle->GetResource());}
    inline T& operator *        ()const         {ASSERT(m_pHandle) return *(s_cast<T*>(m_pHandle->GetResource()));}
    inline    operator coreBool ()const         {return m_pHandle ? true : false;}
    inline coreResourceHandle* GetHandle()const {return m_pHandle;}
    //! @}

    /*! check for usable resource object */
    //! @{
    inline coreBool IsUsable()const {return (m_pHandle && m_pHandle->IsLoaded()) ? true : false;}
    //! @}
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
    /*! reset with the resource manager */
    //! @{
    virtual void __Reset(const coreResourceReset bInit) = 0;
    //! @}
};


// ****************************************************************
/* resource manager */
class coreResourceManager final : public coreThread
{
private:
    coreLookupStr<coreResourceHandle*> m_apHandle;   //!< resource handles

    coreLookupStr<coreArchive*> m_apArchive;         //!< archives with resource files
    coreLookupStr<coreFile*> m_apDirectFile;         //!< direct resource files

    coreSet<coreResourceRelation*> m_apRelation;     //!< objects to reset with the resource manager

    SDL_SpinLock m_iResourceLock;                    //!< spinlock to prevent invalid resource handle access
    SDL_SpinLock m_iFileLock;                        //!< spinlock to prevent invalid resource file access
    coreBool m_bActive;                              //!< current management status


private:
    coreResourceManager()noexcept;
    ~coreResourceManager();


public:
    FRIEND_CLASS(Core)
    FRIEND_CLASS(coreResourceRelation)
    DISABLE_COPY(coreResourceManager)

    /*! update the resource manager */
    //! @{
    void UpdateResources();
    inline coreBool  IsLoading   ()const {return std::any_of  (m_apHandle.begin(), m_apHandle.end(), [](const coreResourceHandle* pHandle) {return pHandle->IsLoading();});}
    inline coreUintW IsLoadingNum()const {return std::count_if(m_apHandle.begin(), m_apHandle.end(), [](const coreResourceHandle* pHandle) {return pHandle->IsLoading();});}
    //! @}

    /*! create and delete resource and resource handle */
    //! @{
    template <typename T, typename... A>        coreResourceHandle* Load   (const coreHashString& sName, const coreResourceUpdate bUpdate, const coreHashString& sPath, A&&... vArgs);
    template <typename T, typename... A> inline coreResourceHandle* LoadNew(A&&... vArgs)const {return new coreResourceHandle(new T(std::forward<A>(vArgs)...), NULL, "", false);}
    template <typename T> void Free(coreResourcePtr<T>* OUTPUT pptResourcePtr);
    //! @}

    /*! get existing resource handle */
    //! @{
    template <typename T> inline coreResourceHandle* Get(const coreHashString& sName) {if(!sName) return NULL; ASSERT(m_apHandle.count(sName)) return this->Load<T>(sName, CORE_RESOURCE_UPDATE_AUTO, NULL);}
    //! @}

    /*! retrieve archives and resource files */
    //! @{
    coreArchive* RetrieveArchive(const coreHashString& sPath);
    coreFile*    RetrieveFile   (const coreHashString& sPath);
    //! @}

    /*! reset all resources and relation-objects */
    //! @{
    void Reset(const coreResourceReset bInit);
    //! @}


private:
    /*! resource thread implementations */
    //! @{
    coreStatus __InitThread()override;
    coreStatus __RunThread ()override;
    void       __ExitThread()override;
    //! @}

    /*! load all relevant default resource */
    //! @{
    void __LoadDefault();
    //! @}

    /*! bind and unbind relation-objects */
    //! @{
    inline void __BindRelation  (coreResourceRelation* pRelation) {ASSERT(!m_apRelation.count(pRelation)) m_apRelation.insert(pRelation);}
    inline void __UnbindRelation(coreResourceRelation* pRelation) {ASSERT( m_apRelation.count(pRelation)) m_apRelation.erase (pRelation);}
    //! @}
};


// ****************************************************************
/* insert asynchronous functions */
template <typename F> void coreResourceHandle::OnLoadOnce(F&& nFunction)const
{
    // call function immediately
    if(this->IsLoaded()) nFunction();
    else
    {
        // attach wrapper to the resource thread
        Core::Manager::Resource->AttachFunction([=]()
        {
            if(this->IsLoaded())
            {
                // call and remove function when loaded
                nFunction();
                return CORE_OK;
            }
            return CORE_BUSY;
        });
    }
}


// ****************************************************************
/* constructor */
template <typename T> constexpr coreResourcePtr<T>::coreResourcePtr(std::nullptr_t)noexcept
: m_pHandle (NULL)
{
}

template <typename T> coreResourcePtr<T>::coreResourcePtr(coreResourceHandle* pHandle)noexcept
: m_pHandle (pHandle)
{
    if(m_pHandle) m_pHandle->RefIncrease();
}

template <typename T> coreResourcePtr<T>::coreResourcePtr(const coreResourcePtr<T>& c)noexcept
: m_pHandle (c.m_pHandle)
{
    if(m_pHandle) m_pHandle->RefIncrease();
}

template <typename T> coreResourcePtr<T>::coreResourcePtr(coreResourcePtr<T>&& m)noexcept
: m_pHandle (m.m_pHandle)
{
    m.m_pHandle = NULL;
}


// ****************************************************************
/* destructor */
template <typename T> coreResourcePtr<T>::~coreResourcePtr()
{
    if(m_pHandle) m_pHandle->RefDecrease();
}


// ****************************************************************
/* assignment operator */
template <typename T> coreResourcePtr<T>& coreResourcePtr<T>::operator = (coreResourcePtr<T> o)noexcept
{
    std::swap(m_pHandle, o.m_pHandle);
    return *this;
}


// ****************************************************************
/* create resource and resource handle */
template <typename T, typename... A> coreResourceHandle* coreResourceManager::Load(const coreHashString& sName, const coreResourceUpdate bUpdate, const coreHashString& sPath, A&&... vArgs)
{
    // check for existing resource handle
    if(m_apHandle.count(sName)) return m_apHandle.at(sName);

    // create new resource handle
    coreResourceHandle* pNewHandle = new coreResourceHandle(new T(std::forward<A>(vArgs)...), sPath ? this->RetrieveFile(sPath) : NULL, sName.GetString(), bUpdate ? true : false);

    SDL_AtomicLock(&m_iResourceLock);
    {
        // add resource handle to manager
        m_apHandle[sName] = pNewHandle;
    }
    SDL_AtomicUnlock(&m_iResourceLock);

    return pNewHandle;
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
        SDL_AtomicLock(&m_iResourceLock);
        {
            // remove resource handle from manager
            FOR_EACH(it, m_apHandle)
            {
                if((*it) == pHandle)
                {
                    m_apHandle.erase(it);
                    break;
                }
            }
        }
        SDL_AtomicUnlock(&m_iResourceLock);

        // delete resource handle
        *pptResourcePtr = NULL;
        SAFE_DELETE(pHandle)
    }
}


#endif /* _CORE_GUARD_RESOURCE_H_ */