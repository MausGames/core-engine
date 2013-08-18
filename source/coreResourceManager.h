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
    inline int Load(const char* pcPath) {return this->Load(&coreFile(pcPath));}
    virtual int Load(coreFile* pFile) = NULL;
    virtual int Unload()              = NULL;

    // get relative path
    inline const char* GetPath() {return m_sPath.c_str();}

    // get relative path to NULL resource (required for resource management)
    // static inline const char* GetNullPath() {return "";}
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
    void Unload();

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
    ~coreResourcePtr();

    // resource handle access operators
    inline T* operator -> ()const {SDL_assert(m_pHandle != NULL); return static_cast<T*>(m_pHandle->GetResource());}
    inline T& operator * ()const  {SDL_assert(m_pHandle != NULL); return *(static_cast<T*>(m_pHandle->GetResource()));}

    // assign resource handle
    coreResourcePtr<T>& operator = (coreResourceHandle* pHandle);

    // control active status
    void SetActive(const bool& bStatus);
    const bool GetActive()const {return (m_pHandle && m_bActive) ? true : false;}
};


// ****************************************************************
// resource access definitions
typedef coreResourcePtr<coreTexture> coreTexturePtr;
//typedef coreResourcePtr<coreModel>  coreModelPtr;
//typedef coreResourcePtr<coreShader> coreShaderPtr;
//typedef coreResourcePtr<coreSound>  coreSoundPtr;


// ****************************************************************
// resource manager
class coreResourceManager : public coreThread
{
private:
    std::map<std::string, coreResourceHandle*> m_apHandle;   // resource handles
    std::map<std::string, coreResource*> m_apNull;           // NULL resource objects

    std::vector<coreArchive*> m_apArchive;                   // archives with resource files
    std::map<std::string, coreFile*> m_apDirectFile;         // direct resource files


public:
    coreResourceManager();
    ~coreResourceManager();

    // load resource and return resource handle
    template <typename T> coreResourceHandle* Load(const char* pcPath);

    // add archive with resource files
    bool AddArchive(const char* pcPath);


private:
    // reset resource manager
    void __Reset(const bool& bInit);
    friend class Core;

    // thread implementations
    int __Init();
    int __Run();
    void __Exit();

    // get resource file
    coreFile* __GetResourceFile(const char* pcPath);
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
{
    m_pHandle = c.m_pHandle;
    m_bActive = c.m_bActive;
    if(m_bActive && m_pHandle) m_pHandle->RefIncrease();
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
        pNull = new T(this->__GetResourceFile(T::GetNullPath()));
        m_apNull[T::GetNullPath()] = pNull;
    }
    else pNull = m_apNull[T::GetNullPath()];

    // create new resource handle
    coreResourceHandle* pNewHandle = new coreResourceHandle(this->__GetResourceFile(pcPath), new T(), pNull);
    m_apHandle[pcPath] = pNewHandle;

    return pNewHandle;
}