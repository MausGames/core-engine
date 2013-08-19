//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once


// ****************************************************************
// resource interface class
class coreResource
{
protected:
    std::string m_sPath;   // relative path of the resource file


public:
    coreResource() : m_sPath ("") {}
    virtual ~coreResource()       {}

    // load and unload resource data
    inline coreError Load(const char* pcPath) {return this->Load(&coreFile(pcPath));}
    virtual coreError Load(coreFile* pFile) = 0;
    virtual coreError Unload()              = 0;

    // get relative path
    inline const char* GetPath() {return m_sPath.c_str();}

    // get relative path to NULL resource
    static inline const char* GetNullPath() {SDL_assert(false); return NULL;}
};


// ****************************************************************
// resource handle class
class coreResourceHandle
{
private:
    coreFile* m_pFile;           // resource file

    coreResource* m_pResource;   // associated resource object
    coreResource* m_pNull;       // NULL resource object

    coreResource* m_pCur;        // pointer to active resource object
    int m_iRef;                  // reference counter


public:
    coreResourceHandle(coreFile* pFile, coreResource* pResource, coreResource* pNull);
    ~coreResourceHandle();
    
    // get active resource object
    inline coreResource* GetResource()const {return m_pCur;}

    // control resource loading
    void Update();
    inline void Nullify() {m_pCur = m_pNull; m_pResource->Unload();}

    // control reference counter
    inline void RefIncrease()       {++m_iRef;}
    inline void RefDecrease()       {--m_iRef; SDL_assert(m_iRef >= 0);}
    inline const int& GetRef()const {return m_iRef;}
};


// ****************************************************************
// resource access class
template <typename T> class coreResourcePtr
{
private:
    coreResourceHandle* m_pHandle;   // resource handle
    bool m_bActive;                  // active status for reference overloading


public:
    coreResourcePtr(coreResourceHandle* pHandle = NULL);
    coreResourcePtr(const coreResourcePtr& c);
    coreResourcePtr(coreResourcePtr&& c);
    ~coreResourcePtr();

    // assignment operator
    coreResourcePtr<T>& operator = (coreResourceHandle* pHandle);

    // resource access operators
    inline T* operator -> ()const {SDL_assert(m_pHandle != NULL); return static_cast<T*>(m_pHandle->GetResource());}
    inline T& operator * ()const  {SDL_assert(m_pHandle != NULL); return *(static_cast<T*>(m_pHandle->GetResource()));}

    // control active status
    void SetActive(const bool& bStatus);
    inline const bool IsActive()const {return (m_pHandle && m_bActive) ? true : false;}
};


// ****************************************************************
// reset interface class
class coreReset
{
public:
    coreReset();
    virtual ~coreReset();

    // reset the object with the resource manager
    virtual void Reset(const bool& bInit) {}
};


// ****************************************************************
// resource manager
class coreResourceManager : public coreThread
{
private:
    std::map<std::string, coreResourceHandle*> m_apHandle;   // resource handles
    std::map<std::string, coreResource*> m_apNull;           // NULL resource objects

    std::vector<coreArchive*> m_apArchive;                   // archives with resource files
    std::map<std::string, coreFile*> m_apDirectFile;         // direct resource files

    std::set<coreReset*> m_apReset;                          // objects to reset with the resource manager


public:
    coreResourceManager();
    ~coreResourceManager();

    // load resource and return resource handle
    template <typename T> coreResourceHandle* Load(const char* pcPath);

    // control resource files
    coreError AddArchive(const char* pcPath);
    coreFile* RetrieveResourceFile(const char* pcPath);

    // control resource manager reset
    void Reset(const bool& bInit);
    inline void AddReset(coreReset* pObject)    {m_apReset.insert(pObject);}
    inline void RemoveReset(coreReset* pObject) {m_apReset.erase(pObject);}


private:
    // thread implementations
    int __Init();
    int __Run();
    void __Exit();  
};


// ****************************************************************
// constructor
template <typename T> coreResourcePtr<T>::coreResourcePtr(coreResourceHandle* pHandle)
: m_pHandle (pHandle)
, m_bActive (true)
{
    if(m_pHandle) m_pHandle->RefIncrease();
}

template <typename T> coreResourcePtr<T>::coreResourcePtr(const coreResourcePtr& c)
: m_pHandle (c.m_pHandle)
, m_bActive (c.m_bActive)
{
    if(m_bActive && m_pHandle) m_pHandle->RefIncrease();
}

template <typename T> coreResourcePtr<T>::coreResourcePtr(coreResourcePtr&& c)
: m_pHandle (c.m_pHandle)
, m_bActive (c.m_bActive)
{
    c.m_pHandle = 0;
    c.m_bActive = 0;
}


// ****************************************************************
// destructor
template <typename T> coreResourcePtr<T>::~coreResourcePtr()
{
    if(m_bActive && m_pHandle) m_pHandle->RefDecrease();
}


// ****************************************************************
// assign resource handle
template <typename T> coreResourcePtr<T>& coreResourcePtr<T>::operator = (coreResourceHandle* pHandle)
{
    // swap resource handles
    if(m_bActive && m_pHandle) m_pHandle->RefDecrease();
    m_pHandle = pHandle;
    if(m_bActive && m_pHandle) m_pHandle->RefIncrease();

    return *this;
}


// ****************************************************************
// set active status
template <typename T> void coreResourcePtr<T>::SetActive(const bool& bStatus)
{
    if(m_bActive && !bStatus)
    {
        // set resource inactive
        if(m_pHandle) m_pHandle->RefDecrease();
        m_bActive = false;
    }
    else if(!m_bActive && bStatus)
    {
        // set resource active
        if(m_pHandle) m_pHandle->RefIncrease();
        m_bActive = true;
    }
}


// ****************************************************************
// load resource and return resource handle
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