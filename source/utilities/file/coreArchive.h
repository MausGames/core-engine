///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_ARCHIVE_H_
#define _CORE_GUARD_ARCHIVE_H_

// TODO 3: make archive a file
// TODO 3: archive check sum (would need to read full file, maybe just for header, or check for correct size with offsets)
// TODO 5: <old comment style>
// TODO 4: "0 = does not exist physically" should be moved into own bool and -1 should become 0 ? (could simplify if-else, seeking)
// TODO 3: allow referencing allocation instead of owning
// TODO 4: get rid of Internal* functions ? but files should not be copied (normally)
// TODO 3: add status-enum to file and store in archive: compressed, scrambled
// TODO 3: mmap, CreateFileMapping+MapViewOfFile instead of reading into a buffer ? unbuffered reading ? batching (e.g. DirectStorage) ?
// TODO 2: when saving an archive fails late, file-state was already adjusted and cannot be recovered (archive-pos)
// TODO 3: get size from archive (sum up all files and headers, but paths have variable sizes)


// ****************************************************************
/* file definitions */
#define CORE_FILE_MAGIC   (UINT_LITERAL("CFA0"))   // magic number of core-archives
#define CORE_FILE_VERSION (0x00000001u)            // current file version of core-archives

#define __CORE_FILE_TYPE_MEMORY (0u)
#define __CORE_FILE_TYPE_DIRECT (UINT32_MAX)

#if !defined(_CORE_EMSCRIPTEN_) && !defined(_CORE_SWITCH_)
    #define CORE_FILE_SAFEWRITE   // always write to temporary file first (to improve robustness)
#endif


// ****************************************************************
/* file class */
class coreFile final
{
private:
    coreString m_sPath;                  // relative path of the file

    coreByte*  m_pData;                  // file data
    coreUint32 m_iSize;                  // size of the file

    coreUint32   m_iArchivePos;          // absolute data position in the associated archive (0 = does not exist physically | -1 = not associated with an archive)
    coreArchive* m_pArchive;             // associated archive

    coreAtomic<coreUint8> m_iRefCount;   // reference-counter to prevent early unloading
    coreSpinLock          m_DataLock;    // spinlock to prevent concurrent loading and unloading


public:
    explicit coreFile(const coreChar* pcPath)noexcept;
    coreFile(const coreChar* pcPath, coreByte* pData, const coreUint32 iSize)noexcept;
    ~coreFile();

    FRIEND_CLASS(coreArchive)
    DISABLE_COPY(coreFile)

    /* save file */
    coreStatus Save(const coreChar* pcPath = NULL);

    /* compress and decompress file data */
    coreStatus Compress  (const coreInt32  iLevel = ZSTD_CLEVEL_DEFAULT);
    coreStatus Decompress(const coreUint32 iLimit = UINT32_MAX);
    coreStatus Scramble  (const coreUint64 iKey   = 0u);
    coreStatus Unscramble(const coreUint64 iKey   = 0u);

    /* create stream for reading file data */
    SDL_IOStream* CreateReadStream()const;

    /* load and unload file data */
    coreStatus LoadData();
    coreStatus UnloadData();

    /* edit file data directly */
    inline coreByte* EditData() {this->LoadData(); return m_pData;}

    /* handle concurrent file access */
    inline void Acquire() {m_iRefCount.FetchAdd(1u); ASSERT(m_iRefCount)}
    inline void Release() {ASSERT(m_iRefCount) if(!m_iRefCount.SubFetch(1u)) this->UnloadData();}

    /* get object properties */
    inline const coreChar*   GetPath()const {return m_sPath.c_str();}
    inline const coreByte*   GetData()      {this->LoadData(); return m_pData;}
    inline const coreUint32& GetSize()const {return m_iSize;}

    /* handle explicit copy (for internal use) */
    static void InternalNew   (coreFile** OUTPUT ppTarget, const coreFile* pSource);
    static void InternalDelete(coreFile** OUTPUT ppTarget);

    /* flush all writes to disk */
    static void FlushFilesystem();


private:
    /* safely read and write */
    static void __Read (SDL_IOStream* pFile, void*       pPointer, const coreUintW iSize, coreBool* OUTPUT pbSuccess);
    static void __Write(SDL_IOStream* pFile, const void* pPointer, const coreUintW iSize, coreBool* OUTPUT pbSuccess);
};


// ****************************************************************
/* archive class */
class coreArchive final
{
private:
    coreString            m_sPath;    // relative path of the archive
    coreMapStr<coreFile*> m_apFile;   // file objects


public:
    coreArchive()noexcept;
    explicit coreArchive(const coreChar* pcPath)noexcept;
    ~coreArchive();

    DISABLE_COPY(coreArchive)

    /* save archive */
    coreStatus Save(const coreChar* pcPath = NULL);

    /* manage file objects */
    coreFile*  CreateFile(const coreChar* pcPath, coreByte* pData, const coreUint32 iSize);
    coreStatus AddFile   (const coreChar* pcPath, const coreChar* pcNewPath = NULL);
    coreStatus AddFile   (coreFile*       pFile,  const coreChar* pcNewPath = NULL);
    coreStatus DeleteFile(const coreUintW iIndex);
    coreStatus DeleteFile(const coreChar* pcPath);
    coreStatus DeleteFile(coreFile*       pFile);
    void ClearFiles();

    /* access file objects */
    inline coreFile* GetFile(const coreUintW       iIndex) {return (iIndex < m_apFile.size()) ? m_apFile[iIndex]      : NULL;}
    inline coreFile* GetFile(const coreHashString& sPath)  {return (m_apFile.count_bs(sPath)) ? m_apFile.at_bs(sPath) : NULL;}

    /* get object properties */
    inline const coreChar* GetPath    ()const {return m_sPath.c_str();}
    inline coreUintW       GetNumFiles()const {return m_apFile.size();}


private:
    /* calculate absolute data positions of all files */
    void __CalculatePositions();
};


#endif /* _CORE_GUARD_ARCHIVE_H_ */