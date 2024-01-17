///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
coreFile::coreFile(const coreChar* pcPath)noexcept
: m_sPath       (pcPath)
, m_pData       (NULL)
, m_iSize       (0u)
, m_iArchivePos (__CORE_FILE_TYPE_DIRECT)
, m_pArchive    (NULL)
, m_iRefCount   (0u)
, m_DataLock    ()
{
    if(m_sPath.empty()) return;

    // retrieve and clamp file size
    const coreInt64 iFullSize = coreData::FileSize(m_sPath.c_str());
    if((iFullSize >= 0) && (iFullSize <= 0xFFFFFFFF)) m_iSize = iFullSize;

    // check for success
    if(!m_iSize) Core::Log->Warning("File (%s) could not be opened", m_sPath.c_str());
            else Core::Log->Info   ("File (%s, %.1f KB) opened",     m_sPath.c_str(), I_TO_F(m_iSize) / 1024.0f);
}

coreFile::coreFile(const coreChar* pcPath, coreByte* pData, const coreUint32 iSize)noexcept
: m_sPath       (pcPath)
, m_pData       (pData)
, m_iSize       (iSize)
, m_iArchivePos (__CORE_FILE_TYPE_MEMORY)
, m_pArchive    (NULL)
, m_iRefCount   (0u)
, m_DataLock    ()
{
}


// ****************************************************************
/* destructor */
coreFile::~coreFile()
{
    ASSERT(!m_iRefCount)

    // delete file data
    SAFE_DELETE_ARRAY(m_pData)
}


// ****************************************************************
/* save file */
coreStatus coreFile::Save(const coreChar* pcPath)
{
    // check file data
    this->LoadData();
    if(!m_pData || !m_iSize) return CORE_INVALID_CALL;

    // save path
    if(pcPath) m_sPath = pcPath;
    ASSERT(!m_sPath.empty())

    // create folder hierarchy
    coreData::FolderCreate(m_sPath.c_str());

    // write to temporary file first (to improve robustness)
    const coreChar* pcTemp = DEFINED(CORE_FILE_SAFEWRITE) ? PRINT("%s.temp_%u", m_sPath.c_str(), coreData::ProcessID()) : m_sPath.c_str();

    // open file
    SDL_RWops* pFile = SDL_RWFromFile(pcTemp, "wb");
    if(!pFile)
    {
        Core::Log->Warning("File (%s) could not be saved (SDL: %s)", m_sPath.c_str(), SDL_GetError());
        return CORE_ERROR_FILE;
    }

    coreBool bSuccess = true;

    // save file data
    coreFile::__Write(pFile, m_pData, sizeof(coreByte), m_iSize, &bSuccess);

    // close file
    if(SDL_RWclose(pFile) || !bSuccess)
    {
        Core::Log->Warning("File (%s) could not be saved properly (SDL: %s)", m_sPath.c_str(), SDL_GetError());
        return CORE_ERROR_FILE;
    }

    // move temporary file over real file
    if(DEFINED(CORE_FILE_SAFEWRITE) && coreData::FileMove(pcTemp, m_sPath.c_str()))
    {
        Core::Log->Warning("File (%s) could not be moved", m_sPath.c_str());
        return CORE_ERROR_FILE;
    }

    // mark file as existing
    if(!m_pArchive && (m_iArchivePos == __CORE_FILE_TYPE_MEMORY)) m_iArchivePos = __CORE_FILE_TYPE_DIRECT;

    Core::Log->Info("File (%s, %.1f KB) written", m_sPath.c_str(), I_TO_F(m_iSize) / 1024.0f);
    return CORE_OK;
}


// ****************************************************************
/* compress file data */
coreStatus coreFile::Compress(const coreInt32 iLevel)
{
    // check file data
    this->LoadData();
    if(!m_pData || !m_iSize) return CORE_INVALID_CALL;

    // compress data
    coreByte*  pNewData;
    coreUint32 iNewSize;
    const coreStatus eError = coreData::Compress(m_pData, m_iSize, &pNewData, &iNewSize, iLevel);

    if(eError == CORE_OK)
    {
        // delete old data
        SAFE_DELETE_ARRAY(m_pData)

        // save new data
        m_pData = pNewData;
        m_iSize = iNewSize;
    }

    return eError;
}


// ****************************************************************
/* decompress file data */
coreStatus coreFile::Decompress(const coreUint32 iLimit)
{
    // check file data
    this->LoadData();
    if(!m_pData || !m_iSize) return CORE_INVALID_CALL;

    // decompress data
    coreByte*  pNewData;
    coreUint32 iNewSize;
    const coreStatus eError = coreData::Decompress(m_pData, m_iSize, &pNewData, &iNewSize, iLimit);

    if(eError == CORE_OK)
    {
        // delete old data
        SAFE_DELETE_ARRAY(m_pData)

        // save new data
        m_pData = pNewData;
        m_iSize = iNewSize;
    }

    return eError;
}


// ****************************************************************
/* scramble file data */
coreStatus coreFile::Scramble(const coreUint64 iKey)
{
    // check file data
    this->LoadData();
    if(!m_pData || !m_iSize) return CORE_INVALID_CALL;

    // scramble data
    coreData::Scramble(m_pData, m_iSize, iKey);
    return CORE_OK;
}

coreStatus coreFile::Unscramble(const coreUint64 iKey)
{
    // check file data
    this->LoadData();
    if(!m_pData || !m_iSize) return CORE_INVALID_CALL;

    // scramble data
    coreData::Unscramble(m_pData, m_iSize, iKey);
    return CORE_OK;
}


// ****************************************************************
/* create stream for reading file data */
SDL_RWops* coreFile::CreateReadStream()const
{
    SDL_RWops* pFile;
    if(m_iArchivePos == __CORE_FILE_TYPE_MEMORY)
    {
        // create memory stream
        if(!m_pData || !m_iSize) return NULL;
        pFile = SDL_RWFromConstMem(m_pData, m_iSize);
    }
    else if(m_pArchive)
    {
        // open archive
        pFile = SDL_RWFromFile(m_pArchive->GetPath(), "rb");
        if(!pFile) return NULL;

        // seek file data position
        if(SDL_RWseek(pFile, m_iArchivePos, RW_SEEK_SET) != m_iArchivePos)
        {
            SDL_RWclose(pFile);
            return NULL;
        }
    }
    else
    {
        // open direct file
        pFile = SDL_RWFromFile(m_sPath.c_str(), "rb");
        if(!pFile) return NULL;
    }

    return pFile;
}


// ****************************************************************
/* load file data */
coreStatus coreFile::LoadData()
{
    coreSpinLocker oLocker(&m_DataLock);

    // check current state
    if(m_pData || !m_iSize || (m_iArchivePos == __CORE_FILE_TYPE_MEMORY)) return CORE_INVALID_CALL;

#if defined(_CORE_DEBUG_)

    // for correct hot-reloading (not in release, not for archives)
    if(!m_sPath.empty() && !m_pArchive) m_iSize = coreData::FileSize(m_sPath.c_str());

#endif

    SDL_RWops* pFile;
    if(m_pArchive)
    {
        // open archive
        pFile = SDL_RWFromFile(m_pArchive->GetPath(), "rb");
        if(!pFile) return CORE_ERROR_FILE;

        // seek file data position
        if(SDL_RWseek(pFile, m_iArchivePos, RW_SEEK_SET) != m_iArchivePos)
        {
            SDL_RWclose(pFile);
            return CORE_ERROR_FILE;
        }
    }
    else
    {
        // open direct file
        pFile = SDL_RWFromFile(m_sPath.c_str(), "rb");
        if(!pFile) return CORE_ERROR_FILE;
    }

    coreBool bSuccess = true;

    // cache file data
    m_pData = new coreByte[m_iSize];
    coreFile::__Read(pFile, m_pData, sizeof(coreByte), m_iSize, &bSuccess);

    // close file
    if(SDL_RWclose(pFile) || !bSuccess)
    {
        SAFE_DELETE_ARRAY(m_pData)
        return CORE_ERROR_FILE;
    }

    return CORE_OK;
}


// ****************************************************************
/* unload file data */
coreStatus coreFile::UnloadData()
{
    coreSpinLocker oLocker(&m_DataLock);

    // check current state
    if(!m_pData || !m_iSize || (m_iArchivePos == __CORE_FILE_TYPE_MEMORY)) return CORE_INVALID_CALL;

    // prevent early unloading
    if(m_iRefCount) return CORE_BUSY;

    // delete file data
    SAFE_DELETE_ARRAY(m_pData)
    return CORE_OK;
}


// ****************************************************************
/* handle explicit copy (for internal use) */
void coreFile::InternalNew(coreFile** OUTPUT ppTarget, const coreFile* pSource)
{
    ASSERT(ppTarget && pSource)

    coreByte* pData = NULL;
    if((pSource->m_iArchivePos == __CORE_FILE_TYPE_MEMORY) && pSource->m_pData && pSource->m_iSize)
    {
        // copy file data
        pData = new coreByte[pSource->m_iSize];
        std::memcpy(pData, pSource->m_pData, pSource->m_iSize);
    }

    // create copy
    (*ppTarget) = MANAGED_NEW(coreFile, pSource->m_sPath.c_str(), pData, pSource->m_iSize);

    // forward archive attributes
    (*ppTarget)->m_pArchive    = pSource->m_pArchive;
    (*ppTarget)->m_iArchivePos = pSource->m_iArchivePos;

    if(Core::Config->GetBool(CORE_CONFIG_BASE_PERSISTMODE) || DEFINED(_CORE_SWITCH_))
    {
        // always load into memory
        (*ppTarget)->LoadData();

        // remove association
        (*ppTarget)->m_pArchive    = NULL;
        (*ppTarget)->m_iArchivePos = __CORE_FILE_TYPE_MEMORY;
    }
}

void coreFile::InternalDelete(coreFile** OUTPUT ppTarget)
{
    // delete copy
    MANAGED_DELETE(*ppTarget)
}


// ****************************************************************
/* flush all writes to disk */
void coreFile::FlushFilesystem()
{
#if defined(_CORE_EMSCRIPTEN_)

    // write in-memory data to persistent data store
    EM_ASM(
        FS.syncfs(false, function(sError)
        {
            if(sError) console.error(sError);
        });
    );

#endif
}


// ****************************************************************
/* safely read from stream */
void coreFile::__Read(SDL_RWops* pFile, void* pPointer, const coreUintW iSize, const coreUintW iNum, coreBool* OUTPUT pbSuccess)
{
    ASSERT(pFile && pPointer && iSize && iNum && pbSuccess)

    // read and check for errors
    if(!(*pbSuccess) || !((*pbSuccess) = (SDL_RWread(pFile, pPointer, iSize, iNum) == iNum)))
    {
        // reset output memory
        std::memset(pPointer, 0, iSize * iNum);
    }
}


// ****************************************************************
/* safely write to stream */
void coreFile::__Write(SDL_RWops* pFile, const void* pPointer, const coreUintW iSize, const coreUintW iNum, coreBool* OUTPUT pbSuccess)
{
    ASSERT(pFile && pPointer && iSize && iNum && pbSuccess)

    // write and check for errors
    if(*pbSuccess) (*pbSuccess) = (SDL_RWwrite(pFile, pPointer, iSize, iNum) == iNum);
}


// ****************************************************************
/* constructor */
coreArchive::coreArchive()noexcept
: m_sPath  ("")
, m_apFile {}
{
}

coreArchive::coreArchive(const coreChar* pcPath)noexcept
: m_sPath  (pcPath)
, m_apFile {}
{
    // open archive
    SDL_RWops* pArchive = SDL_RWFromFile(m_sPath.c_str(), "rb");
    if(!pArchive)
    {
        Core::Log->Warning("Archive (%s) could not be opened (SDL: %s)", m_sPath.c_str(), SDL_GetError());
        return;
    }

    coreBool bSuccess = true;

    // read magic number and file version
    coreUint32 aiHead[2];
    coreFile::__Read(pArchive, &aiHead, sizeof(coreUint32), 2u, &bSuccess);

    // check magic number and file version
    if((aiHead[0] != CORE_FILE_MAGIC) || (aiHead[1] != CORE_FILE_VERSION))
    {
        SDL_RWclose(pArchive);
        Core::Log->Warning("Archive (%s) is not a valid CFA-file", m_sPath.c_str());
        return;
    }

    // read number of files
    coreUint16 iNumFiles;
    coreFile::__Read(pArchive, &iNumFiles, sizeof(coreUint16), 1u, &bSuccess);

    // read file headers
    for(coreUintW i = iNumFiles; i--; )
    {
        coreUint8  iPathLen;
        coreChar   acPath[256];
        coreUint32 iSize;
        coreUint32 iArchivePos;

        // read file header data
        coreFile::__Read(pArchive, &iPathLen,    sizeof(coreUint8),  1u,       &bSuccess);
        coreFile::__Read(pArchive, acPath,       sizeof(coreChar),   iPathLen, &bSuccess);
        coreFile::__Read(pArchive, &iSize,       sizeof(coreUint32), 1u,       &bSuccess);
        coreFile::__Read(pArchive, &iArchivePos, sizeof(coreUint32), 1u,       &bSuccess);
        acPath[iPathLen] = '\0';

        // add new file object
        if(bSuccess)
        {
            coreFile* pNewFile      = new coreFile(acPath, NULL, iSize);
            pNewFile->m_pArchive    = this;
            pNewFile->m_iArchivePos = iArchivePos;
            m_apFile.emplace_bs(acPath, pNewFile);
        }
    }

    // close archive
    if(SDL_RWclose(pArchive) || !bSuccess)
    {
        this->ClearFiles();
        Core::Log->Warning("Archive (%s) could not be opened properly (SDL: %s)", m_sPath.c_str(), SDL_GetError());
        return;
    }

    Core::Log->Info("Archive (%s, %u files) opened", m_sPath.c_str(), iNumFiles);
}


// ****************************************************************
/* destructor */
coreArchive::~coreArchive()
{
    // remove all file objects
    this->ClearFiles();
}


// ****************************************************************
/* save archive */
coreStatus coreArchive::Save(const coreChar* pcPath)
{
    // save path
    if(pcPath) m_sPath = pcPath;
    ASSERT(!m_sPath.empty())

    // create folder hierarchy
    coreData::FolderCreate(m_sPath.c_str());

    // cache missing file data
    FOR_EACH(it, m_apFile)
        (*it)->LoadData();

    // write to temporary file first (to improve robustness)
    const coreChar* pcTemp = DEFINED(CORE_FILE_SAFEWRITE) ? PRINT("%s.temp_%u", m_sPath.c_str(), coreData::ProcessID()) : m_sPath.c_str();

    // open archive
    SDL_RWops* pArchive = SDL_RWFromFile(pcTemp, "wb");
    if(!pArchive)
    {
        Core::Log->Warning("Archive (%s) could not be saved (SDL: %s)", m_sPath.c_str(), SDL_GetError());
        return CORE_ERROR_FILE;
    }

    coreBool bSuccess = true;

    // save magic number and file version
    const coreUint32 aiHead[2] = {CORE_FILE_MAGIC, CORE_FILE_VERSION};
    coreFile::__Write(pArchive, aiHead, sizeof(coreUint32), 2u, &bSuccess);

    // save number of files
    const coreUint16 iNumFiles = m_apFile.size();
    coreFile::__Write(pArchive, &iNumFiles, sizeof(coreUint16), 1u, &bSuccess);

    // save file headers
    this->__CalculatePositions();
    FOR_EACH(it, m_apFile)
    {
        // get path length
        const coreUint8 iPathLen = MIN(std::strlen((*it)->GetPath()), 255u);

        // write header
        coreFile::__Write(pArchive, &iPathLen,             sizeof(coreUint8),  1u,       &bSuccess);
        coreFile::__Write(pArchive,  (*it)->GetPath(),     sizeof(coreChar),   iPathLen, &bSuccess);
        coreFile::__Write(pArchive, &(*it)->GetSize(),     sizeof(coreUint32), 1u,       &bSuccess);
        coreFile::__Write(pArchive, &(*it)->m_iArchivePos, sizeof(coreUint32), 1u,       &bSuccess);
    }

    // save file data
    FOR_EACH(it, m_apFile)
    {
        coreFile::__Write(pArchive, (*it)->GetData(), sizeof(coreByte), (*it)->GetSize(), &bSuccess);
    }

    // close archive
    if(SDL_RWclose(pArchive) || !bSuccess)
    {
        Core::Log->Warning("Archive (%s) could not be saved properly (SDL: %s)", m_sPath.c_str(), SDL_GetError());
        return CORE_ERROR_FILE;
    }

    // move temporary file over real file
    if(DEFINED(CORE_FILE_SAFEWRITE) && coreData::FileMove(pcTemp, m_sPath.c_str()))
    {
        Core::Log->Warning("Archive (%s) could not be moved", m_sPath.c_str());
        return CORE_ERROR_FILE;
    }

    // unload file data
    FOR_EACH(it, m_apFile)
        (*it)->UnloadData();

    Core::Log->Info("Archive (%s, %u) written", m_sPath.c_str(), iNumFiles);
    return CORE_OK;
}


// ****************************************************************
/* create file object */
coreFile* coreArchive::CreateFile(const coreChar* pcPath, coreByte* pData, const coreUint32 iSize)
{
    // check already existing file
    WARN_IF(m_apFile.count_bs(pcPath))
    {
        Core::Log->Warning("File (%s) already exists in Archive (%s)", pcPath, m_sPath.c_str());
        return NULL;
    }

    // create and add new file object
    coreFile* pFile = new coreFile(pcPath, pData, iSize);
    this->AddFile(pFile);

    return pFile;
}


// ****************************************************************
/* add file object */
coreStatus coreArchive::AddFile(const coreChar* pcPath)
{
    // check already existing file
    WARN_IF(m_apFile.count_bs(pcPath))
    {
        Core::Log->Warning("File (%s) already exists in Archive (%s)", pcPath, m_sPath.c_str());
        return CORE_INVALID_INPUT;
    }

    // open and add new file object
    return this->AddFile(new coreFile(pcPath));
}

coreStatus coreArchive::AddFile(coreFile* pFile)
{
    if(pFile->m_pArchive) return CORE_INVALID_INPUT;

    // check already existing file
    WARN_IF(m_apFile.count_bs(pFile->GetPath()))
    {
        Core::Log->Warning("File (%s) already exists in Archive (%s)", pFile->GetPath(), m_sPath.c_str());
        return CORE_INVALID_INPUT;
    }

    // cache missing file data
    pFile->LoadData();

    // add new file object
    m_apFile.emplace_bs(pFile->GetPath(), pFile);

    // associate archive
    pFile->m_pArchive    = this;
    pFile->m_iArchivePos = __CORE_FILE_TYPE_MEMORY;

    return CORE_OK;
}


// ****************************************************************
/* remove file object */
coreStatus coreArchive::DeleteFile(const coreUintW iIndex)
{
    if(iIndex >= m_apFile.size()) return CORE_INVALID_INPUT;

    // remove and delete file object
    SAFE_DELETE(m_apFile[iIndex])
    m_apFile.erase(iIndex);

    return CORE_OK;
}

coreStatus coreArchive::DeleteFile(const coreChar* pcPath)
{
    if(!m_apFile.count_bs(pcPath)) return CORE_INVALID_INPUT;

    // remove and delete file object
    SAFE_DELETE(m_apFile.at_bs(pcPath))
    m_apFile.erase_bs(pcPath);

    return CORE_OK;
}

coreStatus coreArchive::DeleteFile(coreFile* pFile)
{
    if(pFile->m_pArchive != this) return CORE_INVALID_INPUT;
    return this->DeleteFile(pFile->GetPath());
}


// ****************************************************************
/* remove all file objects */
void coreArchive::ClearFiles()
{
    // delete file objects
    FOR_EACH(it, m_apFile)
        SAFE_DELETE(*it)

    // clear memory
    m_apFile.clear();
}


// ****************************************************************
/* calculate the data positions of all files */
void coreArchive::__CalculatePositions()
{
    // calculate data start position
    coreUint32 iCurPosition = 2u*sizeof(coreUint32) + sizeof(coreUint16);
    FOR_EACH(it, m_apFile)
    {
        iCurPosition += sizeof(coreUint8) + MIN(std::strlen((*it)->GetPath()), 255u) + 2u*sizeof(coreUint32);
    }

    // set absolute data position
    FOR_EACH(it, m_apFile)
    {
        (*it)->m_pArchive    = this;
        (*it)->m_iArchivePos = iCurPosition;

        iCurPosition += (*it)->GetSize();
    }
}