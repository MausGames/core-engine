#pragma once


// ****************************************************************
// file class
class coreFile
{
private:
    std::string m_sPath;       // relative path of the file
                               
    coreByte* m_pData;         // file data
    coreUint m_iSize;          // size of the file

    coreArchive* m_pArchive;   // associated archive
    coreUint m_iArchivePos;    // absolute data position in the associated archive (0 = file doesn't exist physically)


public:
    coreFile(const char* pcPath);
    coreFile(const char* pcPath, coreByte* pData, const coreUint& iSize);
    ~coreFile();
    friend class coreArchive;

    // save file
    bool Save(const char* pcPath);

    // load and unload file data
    void LoadData();
    inline void UnloadData() {if(m_iArchivePos) SAFE_DELETE_ARRAY(m_pData)}

    // get attributes
    inline const char* GetName()const     {return m_sPath.substr(m_sPath.find_last_of("/\\")+1).c_str();}
    inline const char* GetPath()const     {return m_sPath.c_str();}
    inline const coreByte* GetData()      {this->LoadData(); return m_pData;}
    inline const coreUint& GetSize()const {return m_iSize;}

    // check if file exists physically
    static bool FileExists(const char* pcPath);
};


// ****************************************************************
// archive class
class coreArchive
{
private:
    std::string m_sPath;                           // relative path of the archive

    std::vector<coreFile*> m_aFile;                // file objects
    std::map<std::string, coreFile*> m_aFileMap;   // path access for file objects


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

    // load and unload file data
    void LoadData();
    void UnloadData();

    // access file objects
    inline coreFile* GetFile(const coreUint& iIndex) {if(iIndex >= m_aFile.size()) return NULL; return m_aFile[iIndex];}
    inline coreFile* GetFile(const char* pcPath)     {if(!m_aFileMap.count(pcPath)) return NULL; return m_aFileMap[pcPath];}

    // get attributes
    inline const char* GetName()const {return m_sPath.substr(m_sPath.find_last_of("/\\")+1).c_str();}
    inline const char* GetPath()const {return m_sPath.c_str();}
    inline coreUint GetSize()const    {return m_aFile.size();}


private:
    // calculate absolute data positions of all files
    void __CalculatePositions();
};