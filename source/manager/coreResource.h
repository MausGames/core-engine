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


// ****************************************************************
// resource interface
// TODO: ATI_meminfo and NVX_gpu_memory_info instead of size ? or both ?
class coreResource
{
protected:
    std::string m_sPath;   //!< relative path of the resource file
    coreUint m_iSize;      //!< data size in bytes


public:
    coreResource()noexcept : m_sPath (""), m_iSize (0) {}
    virtual ~coreResource()                            {}

    //! load and unload resource data
    //! @{
    virtual coreError Load(coreFile* pFile) = 0;
    virtual coreError Unload() = 0;
    //! @}

    //! get object properties
    //! @{
    inline const char* GetPath()const     {return m_sPath.c_str();}
    inline const coreUint& GetSize()const {return m_iSize;}
    //! @}

    //! get relative path to default resource
    //! @{
    static inline const char* GetDefaultPath() {SDL_assert(false); return NULL;}
    //! @}


private:
    DISABLE_COPY(coreResource)
};


// ****************************************************************
// resource handle class
class coreResourceHandle final
{
private:
    coreResource* m_pResource;   //!< associated resource object (in handle)
    coreResource* m_pDefault;    //!< default resource object (in manager)
    coreFile* m_pFile;           //!< resource file (in manager)

    coreResource* m_pCur;        //!< pointer to active resource object
    int m_iRef;                  //!< reference-counter

    bool m_bLoaded;              //!< succesfully loaded
    bool m_bManaged;             //!< actively updated by the resource manager


private:
    coreResourceHandle(coreResource* pResource, coreResource* pDefault, coreFile* pFile)noexcept;
    ~coreResourceHandle();
    friend class coreResourceManager;


public:
    //! access active resource object
    //! @{
    inline coreResource* GetResource()const {return m_pCur;}
    inline const bool& IsLoaded()const      {return m_bLoaded;}
    inline const bool& IsManaged()const     {return m_bManaged;}
    //! @}

    //! manipulate the reference-counter
    //! @{
    inline void RefIncrease()       {++m_iRef;}
    inline void RefDecrease()       {--m_iRef; SDL_assert(m_iRef >= 0); if(!m_iRef) this->__Nullify();}
    inline const int& GetRef()const {return m_iRef;}
    //! @}


private:
    DISABLE_COPY(coreResourceHandle)

    //! control resource loading
    //! @{
    void __Update();
    inline void __Nullify() {if(!m_bManaged || !m_bLoaded) return; m_bLoaded = false; m_pCur = m_pDefault; m_pResource->Unload();}
    //! @}
};


// ****************************************************************
// resource access class
template <typename T> class coreResourcePtr final
{
private:
    coreResourceHandle* m_pHandle;   //!< resource handle
    bool m_bActive;                  //!< status for dynamic reference-counter control


public:
    constexpr_func coreResourcePtr()noexcept;
    coreResourcePtr(coreResourceHandle* pHandle)noexcept;
    coreResourcePtr(const coreResourcePtr<T>& c)noexcept;
    coreResourcePtr(coreResourcePtr<T>&& m)noexcept;
    ~coreResourcePtr();
    friend class coreResourceManager;

    //! assignment operator
    //! @{
    coreResourcePtr<T>& operator = (coreResourcePtr<T> o)noexcept;
    template <typename S> friend void swap(coreResourcePtr<S>& a, coreResourcePtr<S>& b)noexcept;
    //! @}

    //! access active resource object
    //! @{
    inline T* operator -> ()const noexcept {SDL_assert(m_pHandle != NULL); return   s_cast<T*>(m_pHandle->GetResource());}
    inline T& operator * ()const noexcept  {SDL_assert(m_pHandle != NULL); return *(s_cast<T*>(m_pHandle->GetResource()));}
    inline const bool& IsLoaded()const     {SDL_assert(m_pHandle != NULL); return m_pHandle->IsLoaded();}
    inline const bool& IsManaged()const    {SDL_assert(m_pHandle != NULL); return m_pHandle->IsManaged();}
    //! @}

    //! dynamically control the reference-counter
    //! @{
    void SetActive(const bool& bStatus);
    inline bool IsActive()const {return (m_pHandle && m_bActive) ? true : false;}
    //! @}

    //! check for valid resource handle
    //! @{
    inline operator bool ()const noexcept {return m_pHandle ? true : false;}
    //! @}


private:
    DISABLE_HEAP
};


// ****************************************************************
// reset interface
class coreReset
{
public:
    coreReset()noexcept;
    virtual ~coreReset();
    friend class coreResourceManager;


private:
    DISABLE_COPY(coreReset)

    //! reset with the resource manager
    //! @{
    virtual void __Reset(const bool& bInit) = 0;
    //! @}
};


// ****************************************************************
// resource manager
// TODO: use load-stack
// TODO: default resources necessary ? (currently problem when playing unloaded looped sounds)
class coreResourceManager final : public coreThread
{
private:
    coreLookup<coreResourceHandle*> m_apHandle;   //!< resource handles
    coreLookup<coreResource*> m_apDefault;        //!< default resource objects

    coreLookup<coreArchive*> m_apArchive;         //!< archives with resource files
    coreLookup<coreFile*> m_apDirectFile;         //!< direct resource files

    std::u_set<coreReset*> m_apReset;             //!< objects to reset with the resource manager

    bool m_bActive;                               //!< current management status


private:
    coreResourceManager()noexcept;
    ~coreResourceManager();
    friend class Core;


public:
    //! create and return resource with resource handle
    //! @{
    template <typename T, typename... A> inline coreResourceHandle* LoadNew(const A&... vArgs)const {return new coreResourceHandle(new T(vArgs...), NULL, NULL);}
    template <typename T, typename... A> coreResourceHandle* LoadFile(const char* pcPath, const A&... vArgs);
    template <typename T, typename... A> coreResourceHandle* LoadLink(const char* pcName, const A&... vArgs);
    template <typename T> void Free(T* pResourcePtr);
    //! @}

    //! retrieve archives and resource files
    //! @{
    coreArchive* RetrieveArchive(const char* pcPath);
    coreFile* RetrieveFile(const char* pcPath);
    //! @}

    //! reset all resources and reset-objects
    //! @{
    void Reset(const bool& bInit);
    inline void BindReset(coreReset* pObject)   {SDL_assert(!m_apReset.count(pObject)); m_apReset.insert(pObject);}
    inline void UnbindReset(coreReset* pObject) {SDL_assert( m_apReset.count(pObject)); m_apReset.erase(pObject);}
    //! @}


private:
    //! resource thread implementations
    //! @{
    int __InitThread()override;
    int __RunThread()override;
    void __ExitThread()override;
    //! @}
};


// ****************************************************************
// constructor
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
// destructor
template <typename T> coreResourcePtr<T>::~coreResourcePtr()
{
    if(this->IsActive()) m_pHandle->RefDecrease();
}


// ****************************************************************
// assignment operator
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
// dynamically control the reference-counter
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
// create and return resource with resource handle
template <typename T, typename... A> coreResourceHandle* coreResourceManager::LoadFile(const char* pcPath, const A&... vArgs)
{
    // check for existing resource handle
    if(m_apHandle.count(pcPath)) return m_apHandle[pcPath];

    // check for existing default resource
    coreResource* pDefault;
    if(!m_apDefault.count(T::GetDefaultPath()))
    {
        // load new default resource
        pDefault = new T(vArgs...);
        pDefault->Load(this->RetrieveFile(T::GetDefaultPath()));
        m_apDefault[T::GetDefaultPath()] = pDefault;
    }
    else pDefault = m_apDefault[T::GetDefaultPath()];

    // create new resource handle
    coreResourceHandle* pNewHandle = new coreResourceHandle(new T(vArgs...), pDefault, this->RetrieveFile(pcPath));
    m_apHandle[pcPath] = pNewHandle;

    return pNewHandle;
}

template <typename T, typename... A> coreResourceHandle* coreResourceManager::LoadLink(const char* pcName, const A&... vArgs)
{
    // check for existing resource handle
    if(m_apHandle.count(pcName)) return m_apHandle[pcName];

    // create new resource handle
    coreResourceHandle* pNewHandle = new coreResourceHandle(new T(vArgs...), NULL, NULL);
    m_apHandle[pcName] = pNewHandle;

    return pNewHandle;
}


// ****************************************************************
// delete resource and resource handle
template <typename T> void coreResourceManager::Free(T* pResourcePtr)
{
    // remove resource handle from container
    if(pResourcePtr->m_pHandle) m_apHandle.erase(pResourcePtr->m_pHandle);

    // delete resource and resource handle
    pResourcePtr->SetActive(false);
    SAFE_DELETE(pResourcePtr->m_pHandle)
}


#endif // _CORE_GUARD_RESOURCE_H_