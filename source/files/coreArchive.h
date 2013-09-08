//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef GUARD_CORE_ARCHIVE_H
#define GUARD_CORE_ARCHIVE_H


// ****************************************************************
// file class
class coreFile final
{
private:
    std::string m_sPath;       //!< relative path of the file

    coreByte* m_pData;         //!< file data
    coreUint m_iSize;          //!< size of the file

    coreArchive* m_pArchive;   //!< associated archive
    coreUint m_iArchivePos;    //!< absolute data position in the associated archive (0 = file doesn't exist physically)


public:
    explicit coreFile(const char* pcPath);
    coreFile(const char* pcPath, coreByte* pData, const coreUint& iSize);
    ~coreFile();
    friend class coreArchive;

    //! \name save file
    //! @{
    coreError Save(const char* pcPath);
    //! @}

    //! \name load and unload file data
    //! @{
    coreError LoadData();
    inline coreError UnloadData() {if(!m_iArchivePos) return CORE_INVALID_CALL; SAFE_DELETE_ARRAY(m_pData) return CORE_OK;}
    //! @}

    //! \name get attributes
    //! @{
    inline const char* GetName()const     {return m_sPath.substr(m_sPath.find_last_of("/\\")+1).c_str();}
    inline const char* GetPath()const     {return m_sPath.c_str();}
    inline const coreByte* GetData()      {this->LoadData(); return m_pData;}
    inline const coreUint& GetSize()const {return m_iSize;}
    //! @}

    //! \name check if file exists physically
    //! @{
    static bool FileExists(const char* pcPath);
    //! @}

    //! \name retrieve relative paths of all files from a folder
    //! @{
    static coreError SearchFolder(const char* pcFolder, const char* pcFilter, std::vector<std::string>* pasOutput);
    //! @}


private:
    //! \name disable copy
    //! @{
    coreFile(const coreFile& c) deletefunc;
    coreFile& operator = (const coreFile& c) deletefunc;
    //! @}
};


// ****************************************************************
// archive class
class coreArchive final
{
private:
    std::string m_sPath;                             //!< relative path of the archive

    std::vector<coreFile*> m_aFile;                  //!< file objects
    std::u_map<std::string, coreFile*> m_aFileMap;   //!< path access for file objects


public:
    coreArchive();
    explicit coreArchive(const char* pcPath);
    ~coreArchive();

    //! \name save archive
    //! @{
    coreError Save(const char* pcPath);
    //! @}

    //! \name manage file objects
    //! @{
    coreError AddFile(const char* pcPath);
    coreError AddFile(coreFile* pFile);
    coreError DeleteFile(const coreUint& iIndex);
    coreError DeleteFile(const char* pcPath);
    coreError DeleteFile(coreFile* pFile);
    //! @}

    //! \name load and unload file data
    //! @{
    void LoadData();
    void UnloadData();
    //! @}

    //! \name access file objects
    //! @{
    inline coreFile* GetFile(const coreUint& iIndex) {if(iIndex >= m_aFile.size())  {SDL_assert(false); return NULL;} return m_aFile[iIndex];}
    inline coreFile* GetFile(const char* pcPath)     {if(!m_aFileMap.count(pcPath)) {SDL_assert(false); return NULL;} return m_aFileMap[pcPath];}
    //! @}

    //! \name get attributes
    //! @{
    inline const char* GetName()const {return m_sPath.substr(m_sPath.find_last_of("/\\")+1).c_str();}
    inline const char* GetPath()const {return m_sPath.c_str();}
    inline coreUint GetSize()const    {return m_aFile.size();}
    //! @}


private:
    //! \name disable copy
    //! @{
    coreArchive(const coreArchive& c) deletefunc;
    coreArchive& operator = (const coreArchive& c) deletefunc;
    //! @}

    //! \name calculate absolute data positions of all files
    //! @{
    void __CalculatePositions();
    //! @}
};


#endif // GUARD_CORE_ARCHIVE_H