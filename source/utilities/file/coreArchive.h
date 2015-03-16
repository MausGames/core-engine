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

// TODO: make archive a file
// TODO: implement Unload into coreFile, so the unload is better
// TODO: check sum


// ****************************************************************
// file definitions
#define CORE_FILE_MAGIC     0x012F5B66    //!< magic number of core-archives
#define CORE_FILE_VERSION   0x00000001    //!< current file version of core-archives
#define CORE_FILE_EXTENSION "cfa"         //!< extension of core-archives


// ****************************************************************
// file class
class coreFile final
{
private:
    std::string m_sPath;          //!< relative path of the file

    coreByte*  m_pData;           //!< file data
    coreUint32 m_iSize;           //!< size of the file

    coreArchive* m_pArchive;      //!< associated archive
    coreUint32   m_iArchivePos;   //!< absolute data position in the associated archive (0 = file doesn't exist physically)


public:
    explicit coreFile(const coreChar* pcPath)noexcept;
    coreFile(const coreChar* pcPath, coreByte* pData, const coreUint32& iSize)noexcept;
    ~coreFile();

    FRIEND_CLASS(coreArchive)
    DISABLE_COPY(coreFile)

    //! save file
    //! @{
    coreStatus Save(const coreChar* pcPath);
    //! @}

    //! load and unload file data
    //! @{
    coreStatus LoadData();
    coreByte*  MoveData();
    inline coreStatus UnloadData() {if(!m_iArchivePos) return CORE_INVALID_CALL; SAFE_DELETE_ARRAY(m_pData) return CORE_OK;}
    //! @}

    //! get object properties
    //! @{
    inline const coreChar*   GetPath()const {return m_sPath.c_str();}
    inline const coreByte*   GetData()      {this->LoadData(); return m_pData;}
    inline const coreUint32& GetSize()const {return m_iSize;}
    //! @}
};


// ****************************************************************
// archive class
class coreArchive final
{
private:
    std::string m_sPath;                 //!< relative path of the archive
    coreLookupStr<coreFile*> m_apFile;   //!< file objects


public:
    coreArchive()noexcept;
    explicit coreArchive(const coreChar* pcPath)noexcept;
    ~coreArchive();

    DISABLE_COPY(coreArchive)

    //! save archive
    //! @{
    coreStatus Save(const coreChar* pcPath);
    //! @}

    //! manage file objects
    //! @{
    coreStatus AddFile   (const coreChar*  pcPath);
    coreStatus AddFile   (coreFile*        pFile);
    coreStatus DeleteFile(const coreUintW& iIndex);
    coreStatus DeleteFile(const coreChar*  pcPath);
    coreStatus DeleteFile(coreFile*        pFile);
    void ClearFiles();
    //! @}

    //! access file objects
    //! @{
    inline coreFile* GetFile(const coreUintW& iIndex) {return (iIndex < m_apFile.size()) ? m_apFile[iIndex] : NULL;}
    inline coreFile* GetFile(const coreChar* pcPath)  {return (m_apFile.count(pcPath))   ? m_apFile[pcPath] : NULL;}
    //! @}

    //! get object properties
    //! @{
    inline const coreChar* GetPath    ()const {return m_sPath.c_str();}
    inline       coreUintW GetNumFiles()const {return m_apFile.size();}
    //! @}


private:
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
    ~coreFileUnload() {ASSERT(m_pFile) m_pFile->UnloadData();}

    DISABLE_COPY(coreFileUnload)
    DISABLE_NEW
    DISABLE_DELETE
};


#endif // _CORE_GUARD_ARCHIVE_H_