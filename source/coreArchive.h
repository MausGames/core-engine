#pragma once


// ****************************************************************
// file class
class coreFile
{
private:
    std::string m_sPath;       // relative path of the file
    coreUint m_iSize;          // size of the file

    coreArchive* m_pArchive;   // associated archive file
    coreUint m_iPosition;      // absolute data position in the archive

    coreByte* m_pData;         // file data cache


public:
    coreFile(const char* pcPath);
    coreFile(const char* pcPath, const coreUint& iSize, coreByte* pData);
    coreFile(const char* pcPath, const coreUint& iSize, coreArchive* pArchive, const coreUint& iPosition);
    ~coreFile();
    friend class coreArchive;

    // get attributes
    inline const char* GetName()const         {return m_sPath.substr(m_sPath.find_last_of("/\\")+1).c_str();}
    inline const char* GetPath()const         {return m_sPath.c_str();}
    inline const coreUint& GetSize()const     {return m_iSize;}
    inline coreArchive* GetArchive()const     {return m_pArchive;}
    inline const coreUint& GetPosition()const {return m_iPosition;}

    // cache and get file data
    inline const coreByte* GetData() {this->__CacheData(); return m_pData;}

    // check if file exists
    static bool FileExists(const char* pcPath);


private:
    // cache file data
    void __CacheData();

    // associate archive file
    inline void __AssociateArchive(coreArchive* pArchive, const coreUint& iPosition) {m_pArchive = pArchive; m_iPosition = iPosition;}
};


// ****************************************************************
// archive file class
class coreArchive
{
private:
    std::vector<coreFile*> m_aFile;                // file objects
    std::map<std::string, coreFile*> m_aFileMap;   // path access for file objects

    std::string m_sName;                           // name of the archive
    std::string m_sPath;                           // relative path of the archive


public:
    coreArchive();
    coreArchive(const char* pcPath);
    ~coreArchive();

    // save archive
    bool Save(const char* pcPath);

    // manage file objects
    bool AddFile(const char* pcPath);
    bool AddFile(coreFile* pFile);
    bool DeleteFile(const coreUint& iIndex);
    bool DeleteFile(const char* pcPath);
    bool DeleteFile(coreFile* pFile);

    // access file objects
    inline coreFile* GetFile(const coreUint& iIndex) {if(iIndex >= m_aFile.size()) return NULL; return m_aFile[iIndex];}
    inline coreFile* GetFile(const char* pcPath)     {if(!m_aFileMap.count(pcPath)) return NULL; return m_aFileMap[pcPath];}

    // get attributes
    inline const char* GetName()const {return m_sName.c_str();}
    inline const char* GetPath()const {return m_sPath.c_str();}
    inline coreUint GetSize()const    {return m_aFile.size();}


private:
    // calculate absolute data positions of all files
    void __CalculatePositions();
};