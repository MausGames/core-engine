//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
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
#define CORE_FILE_MAGIC   0x012F5B66    //!< magic number of core-archives
#define CORE_FILE_VERSION 0x00000001    //!< current file version of core-archives


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

    SDL_SpinLock m_iLock;          //!< spinlock to prevent asynchronous file data access


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
    inline coreError UnloadData() {if(!m_iArchivePos) return CORE_INVALID_CALL; SAFE_DELETE_ARRAY(m_pData) return CORE_OK;}
    inline coreByte* MoveData()   {this->LoadData(); coreByte* pOutput = m_pData; m_pData = NULL; return pOutput;}
    //! @}

    //! control asynchronous file data access
    //! @{
    inline void Lock()   {SDL_AtomicLock(&m_iLock);}
    inline void Unlock() {SDL_AtomicUnlock(&m_iLock);}
    //! @}

    //! get object attributes
    //! @{
    inline const char* GetName()const     {return m_sPath.substr(m_sPath.find_last_of("/\\")+1).c_str();}
    inline const char* GetPath()const     {return m_sPath.c_str();}
    inline const coreByte* GetData()      {this->LoadData(); return m_pData;}
    inline const coreUint& GetSize()const {return m_iSize;}
    //! @}


private:
    CORE_DISABLE_COPY(coreFile)
};


// ****************************************************************
// archive class
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
    inline coreFile* GetFile(const coreUint& iIndex) {SDL_assert(iIndex < m_apFile.size()); if(iIndex < m_apFile.size()) return m_apFile[iIndex]; return NULL;}
    inline coreFile* GetFile(const char* pcPath)     {SDL_assert(m_apFile.count(pcPath));   if(m_apFile.count(pcPath))   return m_apFile[pcPath]; return NULL;}
    //! @}

    //! get object attributes
    //! @{
    inline const char* GetName()const {return m_sPath.substr(m_sPath.find_last_of("/\\")+1).c_str();}
    inline const char* GetPath()const {return m_sPath.c_str();}
    inline coreUint GetSize()const    {return m_apFile.size();}
    //! @}


private:
    CORE_DISABLE_COPY(coreArchive)

    //! calculate absolute data positions of all files
    //! @{
    void __CalculatePositions();
    //! @}
};


// ****************************************************************
// file-lock helper class
class coreFileLock final
{
private:
    coreFile* m_pFile;   //!< associated file object
    bool m_bUnload;      //!< unload file data on unlock


public:
    explicit coreFileLock(coreFile* pFile, const bool& bUnload)noexcept;
    ~coreFileLock() {this->Release();}

    //! release the lock
    //! @{
    void Release();
    //! @}


private:
    CORE_DISABLE_COPY(coreFileLock)
    CORE_DISABLE_HEAP
};


#endif // _CORE_GUARD_ARCHIVE_H_