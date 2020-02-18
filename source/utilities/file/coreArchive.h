///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_ARCHIVE_H_
#define _CORE_GUARD_ARCHIVE_H_

// TODO: make archive a file
// TODO: check sum
// TODO: allocate page-size aligned
// TODO: static and dynamic archives (with fixed-size array for renaming)
// TODO: reference-counting wrapper instead of coreFileScope
// TODO: use hash-strings, e.g. for GetFile(const coreChar* pcPath)


// ****************************************************************
// file definitions
#define CORE_FILE_MAGIC   (UINT_LITERAL("CFA0"))   //!< magic number of core-archives
#define CORE_FILE_VERSION (0x00000001u)            //!< current file version of core-archives


// ****************************************************************
// file class
class coreFile final
{
private:
    std::string m_sPath;          //!< relative path of the file

    coreByte*  m_pData;           //!< file data
    coreUint32 m_iSize;           //!< size of the file

    coreUint32   m_iArchivePos;   //!< absolute data position in the associated archive (0 = does not exist physically | -1 = not associated with an archive)
    coreArchive* m_pArchive;      //!< associated archive


public:
    explicit coreFile(const coreChar* pcPath)noexcept;
    coreFile(const coreChar* pcPath, coreByte* pData, const coreUint32 iSize)noexcept;
    ~coreFile();

    FRIEND_CLASS(coreArchive)
    DISABLE_COPY(coreFile)

    //! save file
    //! @{
    coreStatus Save(const coreChar* pcPath = NULL);
    //! @}

    //! compress and decompress file data
    //! @{
    coreStatus Compress  (const coreInt32 iLevel = ZSTD_CLEVEL_DEFAULT);
    coreStatus Decompress();
    coreStatus Scramble  (const coreUint32 iKey = 0u);
    coreStatus Unscramble(const coreUint32 iKey = 0u);
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
    coreStatus Save(const coreChar* pcPath = NULL);
    //! @}

    //! manage file objects
    //! @{
    coreStatus AddFile   (const coreChar* pcPath);
    coreStatus AddFile   (coreFile*       pFile);
    coreStatus DeleteFile(const coreUintW iIndex);
    coreStatus DeleteFile(const coreChar* pcPath);
    coreStatus DeleteFile(coreFile*       pFile);
    void ClearFiles();
    //! @}

    //! access file objects
    //! @{
    inline coreFile* GetFile(const coreUintW iIndex) {return (iIndex < m_apFile.size()) ? m_apFile[iIndex]    : NULL;}
    inline coreFile* GetFile(const coreChar* pcPath) {return (m_apFile.count(pcPath))   ? m_apFile.at(pcPath) : NULL;}
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


#endif // _CORE_GUARD_ARCHIVE_H_