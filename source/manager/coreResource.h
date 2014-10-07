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

// TODO: ATI_meminfo and NVX_gpu_memory_info to check for memory usage ?
// TODO: variable templates for handle-list!
// TODO: replace reference-counter with atomic variable ?
// TODO: when and how to load default archive(s) ?
// TODO: assert for path and update-type in load-function
// TODO: check for resource context, with OGL spec 5.1.3: [gets not deleted when] the object is bound to a context bind point in any context
// TODO: extend OnLoad interface with new functions, call directly after load instead of threaded ?


// ****************************************************************
/* resource definitions */
enum coreResourceUpdate : bool
{
    CORE_RESOURCE_UPDATE_MANUAL = false,   //!< updated and managed by the developer
    CORE_RESOURCE_UPDATE_AUTO   = true     //!< updated automatically by the resource manager
};

enum coreResourceReset : bool
{
    CORE_RESOURCE_RESET_EXIT = false,   //!< invoke shut down routine
    CORE_RESOURCE_RESET_INIT = true     //!< invoke start up routine
};


// ****************************************************************
/* resource interface */
class coreResource
{
protected:
    std::string m_sPath;   //!< relative path of the resource file
    coreUint    m_iSize;   //!< data size in bytes


public:
    coreResource()noexcept : m_sPath (""), m_iSize (0) {}
    virtual ~coreResource()                            {}

    /*! load and unload resource data */
    //! @{
    virtual coreError Load(coreFile* pFile) = 0;
    virtual coreError Unload()              = 0;
    //! @}

    /*! get object properties */
    //! @{
    inline const char*     GetPath()const {return m_sPath.c_str();}
    inline const coreUint& GetSize()const {return m_iSize;}
    //! @}


private:
    DISABLE_COPY(coreResource)
};


// ****************************************************************
/* resource handle class */
class coreResourceHandle final
{
private:
    coreResource* m_pResource;   //!< holding resource object
    coreFile* m_pFile;           //!< pointer to resource file

    std::string m_sName;         //!< identifier of this resource handle
    bool m_bAutomatic;           //!< updated automatically by the resource manager

    int m_iRefCount;             //!< simple reference-counter
    coreError m_iStatus;         //!< current resource status


private:
    coreResourceHandle(coreResource* pResource, coreFile* pFile, const char* pcName, const bool& bAutomatic)noexcept;
    ~coreResourceHandle() {ASSERT(!m_iRefCount) SAFE_DELETE(m_pResource)}
    friend class coreResourceManager;


public:
    /*! access resource object and status */
    //! @{
    inline coreResource* GetResource()const {return m_pResource;}
    inline const bool&   IsAutomatic()const {return m_bAutomatic;}
    inline bool          IsLoaded   ()const {return (m_iStatus != CORE_BUSY) ? true : false;}
    //! @}

    /*! control the reference-counter */
    //! @{
    inline void RefIncrease() {++m_iRefCount;}
    inline void RefDecrease() {--m_iRefCount; ASSERT(m_iRefCount >= 0) if(!m_iRefCount) this->Nullify();}
    //! @}

    /*! handle resource loading */
    //! @{
    inline bool Update () {if(!this->IsLoaded() && m_iRefCount && !m_bAutomatic)       {m_iStatus = m_pResource->Load(m_pFile);                      return true;} return false;}     
    inline bool Reload () {if( this->IsLoaded() && m_iRefCount) {m_pResource->Unload(); m_iStatus = m_pResource->Load(m_pFile);                      return true;} return false;}
    inline bool Nullify() {if( this->IsLoaded())                {m_pResource->Unload(); m_iStatus = (m_pFile || m_bAutomatic) ? CORE_BUSY : CORE_OK; return true;} return false;}
    //! @}

    /*! inject asynchronous functions */
    //! @{
    template <typename F> inline void OnLoadOnce(F&& pFunction)const {Core::Manager::Resource->AttachFunction([=]() {if(this->IsLoaded()) {pFunction(); return CORE_OK;} return CORE_BUSY;});}
    //! @}

    /*! get object properties */
    //! @{
    inline const char*      GetName    ()const {return m_sName.c_str();}
    inline const int&       GetRefCount()const {return m_iRefCount;}
    inline const coreError& GetStatus  ()const {return m_iStatus;}
    //! @}


private:
    DISABLE_COPY(coreResourceHandle)
       
    /*! handle automatic resource loading */
    //! @{
    inline bool __AutoUpdate() {if(!this->IsLoaded() && m_iRefCount && m_bAutomatic) {m_iStatus = m_pResource->Load(m_pFile); return true;} return false;}
    //! @}
};


// ****************************************************************
/* resource access class */
template <typename T> class coreResourcePtr final
{
private:
    coreResourceHandle* m_pHandle;   //!< resource handle
    bool m_bActive;                  //!< status value to overload the reference-counter


public:
    constexpr_func coreResourcePtr()noexcept;
    coreResourcePtr(coreResourceHandle* pHandle)noexcept;
    coreResourcePtr(const coreResourcePtr<T>& c)noexcept;
    coreResourcePtr(coreResourcePtr<T>&&      m)noexcept;
    ~coreResourcePtr();

    /*! assignment operator */
    //! @{
    coreResourcePtr<T>& operator = (coreResourcePtr<T> o)noexcept;
    template <typename S> friend void swap(coreResourcePtr<S>& a, coreResourcePtr<S>& b)noexcept;
    //! @}

    /*! access resource object and resource handle */
    //! @{
    inline T* operator ->   ()const             {ASSERT(m_pHandle) return   s_cast<T*>(m_pHandle->GetResource());}
    inline T& operator *    ()const             {ASSERT(m_pHandle) return *(s_cast<T*>(m_pHandle->GetResource()));}
    inline    operator bool ()const             {return m_pHandle ? true : false;}
    inline coreResourceHandle* GetHandle()const {return m_pHandle;}
    //! @}

    /*! overload the reference-counter */
    //! @{
    void SetActive(const bool& bStatus);
    inline bool IsActive()const {return (m_pHandle && m_bActive)                          ? true : false;}
    inline bool IsUsable()const {return (m_pHandle && m_bActive && m_pHandle->IsLoaded()) ? true : false;}
    //! @}


private:
    DISABLE_HEAP
};


// ****************************************************************
/* relation interface */
class coreResourceRelation
{
public:
    coreResourceRelation()noexcept;
    virtual ~coreResourceRelation();
    friend class coreResourceManager;


private:
    DISABLE_COPY(coreResourceRelation)

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
    bool m_bActive;                                   //!< current management status


private:
    coreResourceManager()noexcept;
    ~coreResourceManager();
    friend class Core;


public:
    /*! update the resource manager */
    //! @{
    void UpdateResources();
    //! @}

    /*! create and delete resource and resource handle */
    //! @{
    template <typename T, typename... A>        coreResourceHandle* Load   (const char* pcName, const coreResourceUpdate& bUpdate, const char* pcPath, A&&... vArgs);
    template <typename T, typename... A> inline coreResourceHandle* LoadNew(A&&... vArgs)const {return new coreResourceHandle(new T(std::forward<A>(vArgs)...), NULL, "", false);}
    template <typename T> void Free(coreResourcePtr<T>* ppResourcePtr);
    //! @}

    /*! get existing resource handle */
    //! @{
    template <typename T> inline coreResourceHandle* Get(const char* pcName) {if(!pcName) return NULL; ASSERT(m_apHandle.count(pcName)) return this->Load<T>(pcName, CORE_RESOURCE_UPDATE_AUTO, NULL);}
    //! @}

    /*! retrieve archives and resource files */
    //! @{
    coreArchive* RetrieveArchive(const char* pcPath);
    coreFile*    RetrieveFile   (const char* pcPath);
    //! @}

    /*! reset all resources and relation-objects */
    //! @{
    void Reset(const coreResourceReset& bInit);
    inline void BindRelation  (coreResourceRelation* pObject) {ASSERT(!m_apRelation.count(pObject)) m_apRelation.insert(pObject);}
    inline void UnbindRelation(coreResourceRelation* pObject) {ASSERT( m_apRelation.count(pObject)) m_apRelation.erase (pObject);}
    //! @}


private:
    /*! resource thread implementations */
    //! @{
    int  __InitThread()override;
    int  __RunThread ()override;
    void __ExitThread()override;
    //! @}
};


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
    swap(*this, o);
    return *this;
}

template <typename S> void swap(coreResourcePtr<S>& a, coreResourcePtr<S>& b)noexcept
{
    using std::swap;
    swap(a.m_pHandle, b.m_pHandle);
    swap(a.m_bActive, b.m_bActive);
}


// ****************************************************************
/* overload the reference-counter */
template <typename T> void coreResourcePtr<T>::SetActive(const bool& bStatus)
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
template <typename T, typename... A> coreResourceHandle* coreResourceManager::Load(const char* pcName, const coreResourceUpdate& bUpdate, const char* pcPath, A&&... vArgs)
{
    // check for existing resource handle
    if(m_apHandle.count(pcName)) return m_apHandle.at(pcName);

    // create new resource handle
    coreResourceHandle* pNewHandle = new coreResourceHandle(new T(std::forward<A>(vArgs)...), pcPath ? this->RetrieveFile(pcPath) : NULL, pcName, bUpdate ? true : false);
    m_apHandle[pcName] = pNewHandle;

    return pNewHandle;
}


// ****************************************************************
/* delete resource and resource handle */
template <typename T> void coreResourceManager::Free(coreResourcePtr<T>* ppResourcePtr)
{
    coreResourceHandle* pHandle = ppResourcePtr->GetHandle();

    // remove resource handle from manager
    if(pHandle)
    {
        SDL_AtomicLock(&m_iLock);
        m_apHandle.erase(pHandle);
        SDL_AtomicUnlock(&m_iLock);
    }

    // delete resource and resource handle
    *ppResourcePtr = NULL;
    SAFE_DELETE(pHandle)
}


#endif /* _CORE_GUARD_RESOURCE_H_ */