//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_ARCHIVE_H_
#define _CORE_GUARD_ARCHIVE_H_

#if defined(_CORE_WINDOWS_)
    #undef DeleteFile
#endif


// ****************************************************************
// file definitions
#define CORE_FILE_MAGIC     0x012F5B66    //!< magic number of core-archives
#define CORE_FILE_VERSION   0x00000001    //!< current file version of core-archives
#define CORE_FILE_EXTENSION ".cfa"        //!< extension of core-archives


// ****************************************************************
// file class
class coreFile final
{
private:
    std::string m_sPath;           //!< relative path of the file

    coreByte* m_pData;             //!< file data
    coreUint m_iSize;              //!< size of the file

    coreArchive* m_pArchive;       //!< associated archive
    coreUint m_iArchivePos;        //!< absolute data position in the associated archive (0 = file doesn't exist physically)


public:
    explicit coreFile(const char* pcPath)noexcept;
    coreFile(const char* pcPath, coreByte* pData, const coreUint& iSize)noexcept;
    ~coreFile();
    friend class coreArchive;

    //! save file
    //! @{
    coreError Save(const char* pcPath);
    //! @}

    //! load and unload file data
    //! @{
    coreError LoadData();
    coreByte* MoveData();
    inline coreError UnloadData() {if(!m_iArchivePos) return CORE_INVALID_CALL; SAFE_DELETE_ARRAY(m_pData) return CORE_OK;}
    //! @}

    //! get object properties
    //! @{
    inline const char* GetPath()const     {return m_sPath.c_str();}
    inline const coreByte* GetData()      {this->LoadData(); return m_pData;}
    inline const coreUint& GetSize()const {return m_iSize;}
    //! @}


private:
    DISABLE_COPY(coreFile)
};


// ****************************************************************
// archive class
// TODO: make archive a file
class coreArchive final
{
private:
    std::string m_sPath;              //!< relative path of the archive
    coreLookup<coreFile*> m_apFile;   //!< file objects


public:
    coreArchive()noexcept;
    explicit coreArchive(const char* pcPath)noexcept;
    ~coreArchive();

    //! save archive
    //! @{
    coreError Save(const char* pcPath);
    //! @}

    //! manage file objects
    //! @{
    coreError AddFile(const char* pcPath);
    coreError AddFile(coreFile* pFile);
    coreError DeleteFile(const coreUint& iIndex);
    coreError DeleteFile(const char* pcPath);
    coreError DeleteFile(coreFile* pFile);
    void ClearFiles();
    //! @}

    //! access file objects
    //! @{
    inline coreFile* GetFile(const coreUint& iIndex) {if(iIndex >= m_apFile.size()) return NULL; return m_apFile[iIndex];}
    inline coreFile* GetFile(const char* pcPath)     {if(!m_apFile.count(pcPath))   return NULL; return m_apFile[pcPath]; }
    //! @}

    //! get object properties
    //! @{
    inline const char* GetPath()const {return m_sPath.c_str();}
    inline coreUint GetSize()const    {return m_apFile.size();}
    //! @}


private:
    DISABLE_COPY(coreArchive)

    //! calculate absolute data positions of all files
    //! @{
    void __CalculatePositions();
    //! @}
};


// ****************************************************************
// file-unload helper class
class coreFileUnload final
{
private:
    coreFile* m_pFile;   //!< associated file object


public:
    explicit constexpr_func coreFileUnload(coreFile* pFile)noexcept : m_pFile (pFile) {}
    ~coreFileUnload() {SDL_assert(m_pFile); m_pFile->UnloadData();}


private:
    DISABLE_COPY(coreFileUnload)
    DISABLE_HEAP
};


#endif // _CORE_GUARD_ARCHIVE_H_