#pragma once


// ****************************************************************
// archive file class
class coreArchive
{
private:
    // file struct
    struct coreFile
    {
        std::string sName;    // name of the file
        coreUint iSize;       // size of the file
        coreUint iPosition;   // absolute data position in the archive

        coreByte* pData;      // data cache
    };


private:
    std::vector<coreFile*> m_aFile;                // file objects
    std::map<std::string, coreFile*> m_aFileMap;   // name access for file objects

    std::string m_sName;                           // name of the archive
    std::string m_sPath;                           // path of the archive


public:
    coreArchive();
    coreArchive(const char* pcPath);
    ~coreArchive();

    // save archive
    bool Save(const char* pcPath);

    // manage file objects
    bool AddFile(const char* pcPath);
    bool DeleteFile(const coreUint& iIndex);
    bool DeleteFile(const char* pcName);

    // access file data
    inline const char* GetFileName(const coreUint& iIndex)const {if(iIndex >= m_aFile.size()) return NULL; return m_aFile[iIndex]->sName.c_str();}
    inline coreUint GetFileSize(const coreUint& iIndex)const    {if(iIndex >= m_aFile.size()) return NULL; return m_aFile[iIndex]->iSize;}
    inline coreUint GetFileSize(const char* pcName)const        {if(!m_aFileMap.count(pcName)) return NULL; return m_aFileMap.at(pcName)->iSize;}
    const coreByte* GetFileData(const coreUint& iIndex);
    const coreByte* GetFileData(const char* pcName);

    // get attributes
    inline const char* GetName()const {return m_sName.c_str();}
    inline const char* GetPath()const {return m_sPath.c_str();}
    inline coreUint GetSize()const    {return m_aFile.size();}


private:
    // cache file data
    void __CacheFile(coreFile* pFile);

    // calculate the data positions of all files
    void __CalculatePositions();
};