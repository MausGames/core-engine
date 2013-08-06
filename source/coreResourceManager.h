#pragma once


// ****************************************************************
// resource interface class
class coreResource
{
protected:
    std::string m_sPath;   // relative path of the file


public:
    coreResource(const char* pcPath) : m_sPath (pcPath) {}
    virtual ~coreResource() {}

    // load and unload resource data
    virtual void Load(const char* pcPath) = NULL;
    virtual void Load(coreFile* pFile)    = NULL;
    virtual void Unload()                 = NULL;

    // get relative path
    inline const char* GetPath() {return m_sPath.c_str();}

    // get relative path to NULL resource (required for resource management)
    // static const char* GetNullPath() {return "";}
};


// ****************************************************************
// 
class coreResourceHandle
{
private:
    coreResource* m_pMainResource;   // main resource object
    coreResource* m_pNullResource;   // NULL resource object

    coreResource* m_pResource;

    int m_iRef;                      // reference counter


public:
    coreResourceHandle(coreResource* pMainResource, coreResource* pNullResource)
    : m_pMainResource (pMainResource)
    , m_pNullResource (pNullResource)
    , m_pResource     (pNullResource)
    , m_iRef          (0)
    {}

    

    void Load(const char* pcPath)
    {

    }

    void Unload()
    {
        m_pResource = m_pNullResource;
        SAFE_DELETE(m_pResource)
    }

    // get active resource object
    inline coreResource* GetResource()const {return m_pResource;}

     // control reference counter
    inline void RefIncrease() {++m_iRef;}
    inline void RefDecrease() {--m_iRef; assert(m_iRef >= 0);}
};


// ****************************************************************
// 
template <typename T> class coreResourcePtr
{
private:
    coreResourceHandle* m_pHandle;   // resource handle
    bool m_bActive;                  // active status for reference overloading


public:
    coreResourcePtr(coreResourceHandle* pHandle = NULL) : m_pHandle(pHandle), m_bActive(true)
    {
        if(m_pHandle) m_pHandle->RefIncrease();
    }
    ~coreResourcePtr()
    {
        if(m_pHandle) m_pHandle->RefDecrease();
    }

    inline coreResourcePtr& operator = (coreResourceHandle* pHandle)
    {
        if(m_bActive && m_pHandle) m_pHandle->RefDecrease();
        m_pHandle = pHandle;
        if(m_bActive && m_pHandle) m_pHandle->RefIncrease();

        return *this;
    }

    inline void SetActive(const bool& bStatus)
    {
        if(m_bActive && !bStatus)
        {
            if(m_pHandle) m_pHandle->RefDecrease();
            m_bActive = false;
        }
        else if(!m_bActive && bStatus)
        {
            if(m_pHandle) m_pHandle->RefIncrease();
            m_bActive = true;
        }
    }

    // resource handle access operators
    inline T* operator -> ()const
    {
        assert(m_pHandle != NULL);
        return static_cast<T*>(m_pHandle->GetResource());
    }
    inline T& operator * ()const 
    {
        assert(m_pHandle != NULL);
        return *(static_cast<T*>(m_pHandle->GetResource()));
    }
};


// ****************************************************************
// 
typedef coreResourcePtr<coreTexture> coreTexturePtr;



// ****************************************************************
// 
class coreResourceManager : public coreThread
{
private:
    std::map<std::string, coreResourceHandle*> m_apHandle;   // map with resource handles
    std::map<std::string, coreResource*> m_apNullResource;   // map with NULL resource objects

    std::vector<coreArchive*> m_apArchive;                   // loaded archive files
    std::vector<coreFile*> m_apDirectFile;                   // loaded direct files


public:
    coreResourceManager()
    {
        this->StartThread("resource_thread");
    }
    ~coreResourceManager()
    {
    }


    template <typename T> coreResourceHandle* Load(const char* pcPath)
    {
        // check for existing resource handle
        if(m_apHandle.count(pcPath)) return m_apHandle[pcPath];

        if(!m_apNullResource.count(T::GetNullPath())) m_apNullResource[T::GetNullPath()] = new T(this->__GetResourceFile(T::GetNullPath()));

        // create new resource handle
        coreResourceHandle* pNewHandle = new coreResourceHandle(new T(pcPath), m_apNullResource[T::GetNullPath()]);
        m_apHandle[pcPath] = pNewHandle;



        return pNewHandle;
    }


private:
    // thread implementations
    int __Init();
    int __Run();

    // get resource file
    coreFile* __GetResourceFile(const char* pcPath)
    {
        // check for direct file
        if(coreFile::FileExists(pcPath)) return new coreFile(pcPath);

        // check loaded archive files
        for(auto it = m_apArchive.begin(); it != m_apArchive.end(); ++it)
        {
            coreFile* pResource = (*it)->GetFile(pcPath);
            if(pResource) return pResource;
        }

        // resource file not found
        return NULL;
    }
};