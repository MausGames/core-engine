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
    virtual ~coreResource()               {}

    DISABLE_COPY(coreResource)

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
    coreResourceHandle(coreResource* pResource, coreFile* pFile, const coreChar* pcName, const coreBool& bAutomatic)noexcept;
    ~coreResourceHandle();


public:
    FRIEND_CLASS(coreResourceManager)
    DISABLE_COPY(coreResourceHandle)

    /*! access resource object and status */
    //! @{
    inline coreResource*   GetResource()const {return m_pResource;}
    inline const coreBool& IsAutomatic()const {return m_bAutomatic;}
    inline       coreBool  IsLoaded   ()const {return (m_iStatus != CORE_BUSY) ? true : false;}
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
    template <typename F> void OnLoadOnce(F&& nFunction)const;
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
    coreBool m_bActive;              //!< status value to overload the reference-counter


public:
    constexpr_func coreResourcePtr()noexcept;
    coreResourcePtr(coreResourceHandle* pHandle)noexcept;
    coreResourcePtr(const coreResourcePtr<T>& c)noexcept;
    coreResourcePtr(coreResourcePtr<T>&&      m)noexcept;
    ~coreResourcePtr();

    DISABLE_NEW

    /*! assignment operator */
    //! @{
    coreResourcePtr<T>& operator = (coreResourcePtr<T> o)noexcept;
    template <typename S> friend void swap(coreResourcePtr<S>& a, coreResourcePtr<S>& b)noexcept;
    //! @}

    /*! access resource object and resource handle */
    //! @{
    inline T* operator ->       ()const         {ASSERT(m_pHandle) return   s_cast<T*>(m_pHandle->GetResource());}
    inline T& operator *        ()const         {ASSERT(m_pHandle) return *(s_cast<T*>(m_pHandle->GetResource()));}
    inline    operator coreBool ()const         {return m_pHandle ? true : false;}
    inline coreResourceHandle* GetHandle()const {return m_pHandle;}
    //! @}

    /*! overload the reference-counter */
    //! @{
    void SetActive(const coreBool& bStatus);
    inline coreBool IsActive()const {return (m_pHandle && m_bActive)                          ? true : false;}
    inline coreBool IsUsable()const {return (m_pHandle && m_bActive && m_pHandle->IsLoaded()) ? true : false;}
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
    virtual void __Reset(const coreResourceReset& bInit) = 0;
    //! @}
};


// ****************************************************************
/* resource manager */
class coreResourceManager final : public coreThread
{
private:
    coreLookupStr<coreResourceHandle*> m_apHandle;    //!< resource handles

    coreLookupStr<coreArchive*> m_apArchive;          //!< archives with resource files
    coreLookupStr<coreFile*> m_apDirectFile;          //!< direct resource files

    std::u_set<coreResourceRelation*> m_apRelation;   //!< objects to reset with the resource manager

    SDL_SpinLock m_iLock;                             //!< spinlock to prevent invalid resource handle access
    coreBool m_bActive;                               //!< current management status


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
    inline coreBool IsLoading()const {FOR_EACH(it, m_apHandle) {if(!(*it)->IsLoaded() && (*it)->GetRefCount()) return true;} return false;}
    //! @}

    /*! create and delete resource and resource handle */
    //! @{
    template <typename T, typename... A>        coreResourceHandle* Load   (const coreChar* pcName, const coreResourceUpdate& bUpdate, const coreChar* pcPath, A&&... vArgs);
    template <typename T, typename... A> inline coreResourceHandle* LoadNew(A&&... vArgs)const {return new coreResourceHandle(new T(std::forward<A>(vArgs)...), NULL, "", false);}
    template <typename T> void Free(coreResourcePtr<T>* pptResourcePtr);
    //! @}

    /*! get existing resource handle */
    //! @{
    template <typename T> inline coreResourceHandle* Get(const coreChar* pcName) {if(!pcName) return NULL; ASSERT(m_apHandle.count(pcName)) return this->Load<T>(pcName, CORE_RESOURCE_UPDATE_AUTO, NULL);}
    //! @}

    /*! retrieve archives and resource files */
    //! @{
    coreArchive* RetrieveArchive(const coreChar* pcPath);
    coreFile*    RetrieveFile   (const coreChar* pcPath);
    //! @}

    /*! reset all resources and relation-objects */
    //! @{
    void Reset(const coreResourceReset& bInit);
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
template <typename T> constexpr_func coreResourcePtr<T>::coreResourcePtr()noexcept
: m_pHandle (NULL)
, m_bActive (true)
{
}

template <typename T> coreResourcePtr<T>::coreResourcePtr(coreResourceHandle* pHandle)noexcept
: m_pHandle (pHandle)
, m_bActive (true)
{
    if(this->IsActive()) m_pHandle->RefIncrease();
}

template <typename T> coreResourcePtr<T>::coreResourcePtr(const coreResourcePtr<T>& c)noexcept
: m_pHandle (c.m_pHandle)
, m_bActive (c.m_bActive)
{
    if(this->IsActive()) m_pHandle->RefIncrease();
}

template <typename T> coreResourcePtr<T>::coreResourcePtr(coreResourcePtr<T>&& m)noexcept
: m_pHandle (m.m_pHandle)
, m_bActive (m.m_bActive)
{
    m.m_pHandle = NULL;
}


// ****************************************************************
/* destructor */
template <typename T> coreResourcePtr<T>::~coreResourcePtr()
{
    if(this->IsActive()) m_pHandle->RefDecrease();
}


// ****************************************************************
/* assignment operator */
template <typename T> coreResourcePtr<T>& coreResourcePtr<T>::operator = (coreResourcePtr<T> o)noexcept
{
    std::swap(*this, o);
    return *this;
}

template <typename S> void swap(coreResourcePtr<S>& a, coreResourcePtr<S>& b)noexcept
{
    std::swap(a.m_pHandle, b.m_pHandle);
    std::swap(a.m_bActive, b.m_bActive);
}


// ****************************************************************
/* swap specialization */
namespace std {template<typename S> inline void swap(coreResourcePtr<S>& a, coreResourcePtr<S>& b) {::swap(a, b);}}


// ****************************************************************
/* overload the reference-counter */
template <typename T> void coreResourcePtr<T>::SetActive(const coreBool& bStatus)
{
    if(m_bActive && !bStatus)
    {
        // set resource access inactive
        if(m_pHandle) m_pHandle->RefDecrease();
        m_bActive = false;
    }
    else if(!m_bActive && bStatus)
    {
        // set resource access active
        if(m_pHandle) m_pHandle->RefIncrease();
        m_bActive = true;
    }
}


// ****************************************************************
/* create resource and resource handle */
template <typename T, typename... A> coreResourceHandle* coreResourceManager::Load(const coreChar* pcName, const coreResourceUpdate& bUpdate, const coreChar* pcPath, A&&... vArgs)
{
    // check for existing resource handle
    if(m_apHandle.count(pcName)) return m_apHandle.at(pcName);

    // create new resource handle
    coreResourceHandle* pNewHandle = new coreResourceHandle(new T(std::forward<A>(vArgs)...), pcPath ? this->RetrieveFile(pcPath) : NULL, pcName, bUpdate ? true : false);

    // add resource handle to manager
    SDL_AtomicLock  (&m_iLock);
    m_apHandle[pcName] = pNewHandle;
    SDL_AtomicUnlock(&m_iLock);

    return pNewHandle;
}


// ****************************************************************
/* delete resource and resource handle */
template <typename T> void coreResourceManager::Free(coreResourcePtr<T>* pptResourcePtr)
{
    ASSERT(pptResourcePtr)

    // retrieve resource handle
    coreResourceHandle* pHandle = pptResourcePtr->GetHandle();
    if(pHandle)
    {
        SDL_AtomicLock(&m_iLock);
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
        SDL_AtomicUnlock(&m_iLock);

        // delete resource handle
        *pptResourcePtr = NULL;
        SAFE_DELETE(pHandle)
    }
}


#endif /* _CORE_GUARD_RESOURCE_H_ */