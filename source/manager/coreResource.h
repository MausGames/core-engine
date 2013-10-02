//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_RESOURCE_H_
#define _CORE_GUARD_RESOURCE_H_


// ****************************************************************
// resource interface class
class coreResource
{
protected:
    std::string m_sPath;   //!< relative path of the resource file
    coreUint m_iSize;      //!< data size in bytes


public:
    coreResource() : m_sPath (""), m_iSize (0) {}
    virtual ~coreResource()                    {}

    //! load and unload resource data
    //! @{
    inline coreError Load(const char* pcPath) {coreFile File(pcPath); return this->Load(&File);}
    virtual coreError Load(coreFile* pFile) = 0;
    virtual coreError Unload()              = 0;
    //! @}

    //! get object attributes
    //! @{
    inline const char* GetPath()const     {return m_sPath.c_str();}
    inline const coreUint& GetSize()const {return m_iSize;}
    //! @}

    //! get relative path to NULL resource
    //! @{
    static inline const char* GetNullPath() {SDL_assert(false); return NULL;}
    //! @}


private:
    CORE_DISABLE_COPY(coreResource)
};


// ****************************************************************
// resource handle class
class coreResourceHandle final
{
private:
    coreFile* m_pFile;           //!< resource file

    coreResource* m_pResource;   //!< associated resource object
    coreResource* m_pNull;       //!< NULL resource object

    coreResource* m_pCur;        //!< pointer to active resource object
    int m_iRef;                  //!< reference counter
    bool m_bManaged;             //!< actively updated by the resource manager


private:
    coreResourceHandle(coreFile* pFile, coreResource* pResource, coreResource* pNull);
    ~coreResourceHandle();
    friend class coreResourceManager;

    //! control resource loading
    //! @{
    void __Update();
    inline void __Nullify() {if(!m_bManaged) return; m_pCur = m_pNull; m_pResource->Unload();}
    //! @}


public:
    //! access active resource object
    //! @{
    inline coreResource* GetResource()const {return m_pCur;}
    inline bool IsLoaded()const             {return (m_pCur == m_pNull) ? false : true;}
    //! @}

    //! manipulate the reference counter
    //! @{
    inline void RefIncrease()       {++m_iRef;}
    inline void RefDecrease()       {--m_iRef; SDL_assert(m_iRef >= 0);}
    inline const int& GetRef()const {return m_iRef;}
    //! @}


private:
    CORE_DISABLE_COPY(coreResourceHandle)
};


// ****************************************************************
// resource access class
template <typename T> class coreResourcePtr
{
private:
    coreResourceHandle* m_pHandle;   //!< resource handle
    bool m_bActive;                  //!< status for dynamic reference counter control


public:
    coreResourcePtr(coreResourceHandle* pHandle = NULL);
    coreResourcePtr(const coreResourcePtr<T>& c)noexcept;
    coreResourcePtr(coreResourcePtr<T>&& m)noexcept;
    ~coreResourcePtr();

    //! assignment operator
    //! @{
    coreResourcePtr<T>& operator = (coreResourcePtr<T> o)noexcept;
    template <typename S> friend void swap(coreResourcePtr<S>& a, coreResourcePtr<S>& b)noexcept;
    //! @}

    //! access active resource object
    //! @{
    inline T* operator -> ()const {SDL_assert(m_pHandle != NULL); return   static_cast<T*>(m_pHandle->GetResource());}
    inline T& operator * ()const  {SDL_assert(m_pHandle != NULL); return *(static_cast<T*>(m_pHandle->GetResource()));}
    inline bool IsLoaded()const   {SDL_assert(m_pHandle != NULL); return m_pHandle->IsLoaded();}
    //! @}

    //! dynamically control the reference counter
    //! @{
    void SetActive(const bool& bStatus);
    inline bool IsActive()const {return (m_pHandle && m_bActive) ? true : false;}
    //! @}


private:
    CORE_DISABLE_HEAP
};


// ****************************************************************
// reset interface class
class coreReset
{
public:
    coreReset();
    virtual ~coreReset();

    //! reset the object with the resource manager
    //! @{
    virtual void Reset(const bool& bInit) = 0;
    //! @}


private:
    CORE_DISABLE_COPY(coreReset)
};


// ****************************************************************
// resource manager
// TODO: use load- and unload-stack
class coreResourceManager final : public coreThread
{
private:
    std::u_map<std::string, coreResourceHandle*> m_apHandle;   //!< resource handles
    std::u_map<std::string, coreResource*> m_apNull;           //!< NULL resource objects

    std::vector<coreArchive*> m_apArchive;                     //!< archives with resource files
    std::u_map<std::string, coreFile*> m_apDirectFile;         //!< direct resource files

    std::u_set<coreReset*> m_apReset;                          //!< objects to reset with the resource manager


private:
    coreResourceManager();
    ~coreResourceManager();
    friend class Core;


public:
    //! load resource and retrieve resource handle
    //! @{
    template <typename T> coreResourceHandle* Load(const char* pcPath);
    //! @}

    //! control resource files
    //! @{
    coreError AddArchive(const char* pcPath);
    coreFile* RetrieveResourceFile(const char* pcPath);
    //! @}

    //! reset all resources and reset-objects
    //! @{
    void Reset(const bool& bInit);
    inline void AddReset(coreReset* pObject)    {SDL_assert(!m_apReset.count(pObject)); m_apReset.insert(pObject);}
    inline void RemoveReset(coreReset* pObject) {SDL_assert( m_apReset.count(pObject)); m_apReset.erase(pObject);}
    //! @}


private:
    //! resource thread implementations
    //! @{
    int __Init()override;
    int __Run()override;
    void __Exit()override;
    //! @}
};


// ****************************************************************
// constructor
template <typename T> coreResourcePtr<T>::coreResourcePtr(coreResourceHandle* pHandle)
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
// dynamically control the reference counter
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
// load resource and retrieve resource handle
template <typename T> coreResourceHandle* coreResourceManager::Load(const char* pcPath)
{
    // check for existing resource handle
    if(m_apHandle.count(pcPath)) return m_apHandle[pcPath];

    // check for existing NULL resource
    coreResource* pNull;
    if(!m_apNull.count(T::GetNullPath()))
    {
        // load new NULL resource
        pNull = new T(this->RetrieveResourceFile(T::GetNullPath()));
        m_apNull[T::GetNullPath()] = pNull;
    }
    else pNull = m_apNull[T::GetNullPath()];

    // create new resource handle
    coreResourceHandle* pNewHandle = new coreResourceHandle(this->RetrieveResourceFile(pcPath), new T(), pNull);
    m_apHandle[pcPath] = pNewHandle;

    return pNewHandle;
}


#endif // _CORE_GUARD_RESOURCE_H_