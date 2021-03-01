///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
coreFile::coreFile(const coreChar* pcPath)noexcept
: m_sPath       (pcPath)
, m_pData       (NULL)
, m_iSize       (0u)
, m_iArchivePos (UINT32_MAX)
, m_pArchive    (NULL)
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
, m_iArchivePos (0u)
, m_pArchive    (NULL)
{
}


// ****************************************************************
/* destructor */
coreFile::~coreFile()
{
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
    coreData::CreateFolder(m_sPath.c_str());

    // write to temporary file (to prevent corruption)
    const coreChar* pcTemp = PRINT("%s.temp_%s", m_sPath.c_str(), coreData::DateTimePrint("%Y%m%d_%H%M%S"));

    // open file
    SDL_RWops* pFile = SDL_RWFromFile(pcTemp, "wb");
    if(!pFile)
    {
        Core::Log->Warning("File (%s) could not be saved (SDL: %s)", m_sPath.c_str(), SDL_GetError());
        return CORE_ERROR_FILE;
    }

    // save file data
    SDL_RWwrite(pFile, m_pData, sizeof(coreByte), m_iSize);

    // close file
    SDL_RWclose(pFile);

    // move temporary file over real file
    if(coreData::FileMove(pcTemp, m_sPath.c_str()))
    {
        Core::Log->Warning("File (%s) could not be moved", m_sPath.c_str());
        return CORE_ERROR_FILE;
    }

    // mark file as existing
    if(!m_pArchive && !m_iArchivePos) m_iArchivePos = UINT32_MAX;

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
coreStatus coreFile::Decompress()
{
    // check file data
    this->LoadData();
    if(!m_pData || !m_iSize) return CORE_INVALID_CALL;

    // decompress data
    coreByte*  pNewData;
    coreUint32 iNewSize;
    const coreStatus eError = coreData::Decompress(m_pData, m_iSize, &pNewData, &iNewSize);

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
coreStatus coreFile::Scramble(const coreUint32 iKey)
{
    // check file data
    this->LoadData();
    if(!m_pData || !m_iSize) return CORE_INVALID_CALL;

    // scramble data
    coreData::Scramble(m_pData, m_iSize, iKey);
    return CORE_OK;
}

coreStatus coreFile::Unscramble(const coreUint32 iKey)
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
    if(!m_iArchivePos)
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
        SDL_RWseek(pFile, m_iArchivePos, RW_SEEK_SET);
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
    // check file data
    if(m_pData || !m_iSize || !m_iArchivePos) return CORE_INVALID_CALL;

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
        SDL_RWseek(pFile, m_iArchivePos, RW_SEEK_SET);
    }
    else
    {
        // open direct file
        pFile = SDL_RWFromFile(m_sPath.c_str(), "rb");
        if(!pFile) return CORE_ERROR_FILE;
    }

    // cache file data
    m_pData = new coreByte[m_iSize];
    const coreUintW iRead = SDL_RWread(pFile, m_pData, sizeof(coreByte), m_iSize);
    ASSERT(iRead == m_iSize)

    // close file
    SDL_RWclose(pFile);

    return CORE_OK;
}


// ****************************************************************
/* handle explicit copy (for internal use) */
void coreFile::InternalNew(coreFile** OUTPUT ppTarget, const coreFile* pSource)
{
    ASSERT(ppTarget && pSource)

    coreByte* pData = NULL;
    if(!pSource->m_iArchivePos && pSource->m_pData && pSource->m_iSize)
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
}

void coreFile::InternalDelete(coreFile** OUTPUT ppTarget)
{
    // delete copy
    MANAGED_DELETE(*ppTarget)
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

    // read magic number and file version
    coreUint32 aiHead[2];
    SDL_RWread(pArchive, &aiHead, sizeof(coreUint32), 2u);

    // check magic number
    if(aiHead[0] != CORE_FILE_MAGIC)
    {
        SDL_RWclose(pArchive);
        Core::Log->Warning("Archive (%s) is not a valid CFA-file", m_sPath.c_str());
        return;
    }

    // read number of files
    coreUint16 iNumFiles;
    SDL_RWread(pArchive, &iNumFiles, sizeof(coreUint16), 1u);

    // read file headers
    for(coreUintW i = iNumFiles; i--; )
    {
        coreUint8  iPathLen;
        coreChar   acPath[256];
        coreUint32 iSize;
        coreUint32 iArchivePos;

        // read file header data
        SDL_RWread(pArchive, &iPathLen,    sizeof(coreUint8),  1u);
        SDL_RWread(pArchive, acPath,       sizeof(coreChar),   iPathLen);
        SDL_RWread(pArchive, &iSize,       sizeof(coreUint32), 1u);
        SDL_RWread(pArchive, &iArchivePos, sizeof(coreUint32), 1u);
        acPath[iPathLen] = '\0';

        // add new file object
        coreFile* pNewFile      = new coreFile(acPath, NULL, iSize);
        pNewFile->m_pArchive    = this;
        pNewFile->m_iArchivePos = iArchivePos;
        m_apFile.emplace(acPath, pNewFile);
    }

    // close archive
    SDL_RWclose(pArchive);
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
    if(m_apFile.empty()) return CORE_INVALID_CALL;

    // save path
    if(pcPath) m_sPath = pcPath;
    ASSERT(!m_sPath.empty())

    // create folder hierarchy
    coreData::CreateFolder(m_sPath.c_str());

    // write to temporary file (to prevent corruption)
    const coreChar* pcTemp = PRINT("%s.temp_%s", m_sPath.c_str(), coreData::DateTimePrint("%Y%m%d_%H%M%S"));

    // open archive
    SDL_RWops* pArchive = SDL_RWFromFile(pcTemp, "wb");
    if(!pArchive)
    {
        Core::Log->Warning("Archive (%s) could not be saved (SDL: %s)", m_sPath.c_str(), SDL_GetError());
        return CORE_ERROR_FILE;
    }

    // save magic number and file version
    const coreUint32 aiHead[2] = {CORE_FILE_MAGIC, CORE_FILE_VERSION};
    SDL_RWwrite(pArchive, aiHead, sizeof(coreUint32), 2u);

    // save number of files
    const coreUint16 iNumFiles = m_apFile.size();
    SDL_RWwrite(pArchive, &iNumFiles, sizeof(coreUint16), 1u);

    // cache missing file data
    FOR_EACH(it, m_apFile)
        (*it)->LoadData();

    // save file headers
    this->__CalculatePositions();
    FOR_EACH(it, m_apFile)
    {
        // get path length
        const coreUint8 iPathLen = MIN(std::strlen((*it)->GetPath()), 255u);

        // write header
        SDL_RWwrite(pArchive, &iPathLen,             sizeof(coreUint8),  1u);
        SDL_RWwrite(pArchive,  (*it)->GetPath(),     sizeof(coreChar),   iPathLen);
        SDL_RWwrite(pArchive, &(*it)->GetSize(),     sizeof(coreUint32), 1u);
        SDL_RWwrite(pArchive, &(*it)->m_iArchivePos, sizeof(coreUint32), 1u);
    }

    // save file data
    FOR_EACH(it, m_apFile)
    {
        SDL_RWwrite(pArchive, (*it)->GetData(), sizeof(coreByte), (*it)->GetSize());
        (*it)->UnloadData();
    }

    // close archive
    SDL_RWclose(pArchive);

    // move temporary file over real file
    if(coreData::FileMove(pcTemp, m_sPath.c_str()))
    {
        Core::Log->Warning("Archive (%s) could not be moved", m_sPath.c_str());
        return CORE_ERROR_FILE;
    }

    return CORE_OK;
}


// ****************************************************************
/* add file object */
coreStatus coreArchive::AddFile(const coreChar* pcPath)
{
    // check already existing file
    if(m_apFile.count(pcPath))
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
    if(m_apFile.count(pFile->GetPath()))
    {
        Core::Log->Warning("File (%s) already exists in Archive (%s)", pFile->GetPath(), m_sPath.c_str());
        return CORE_INVALID_INPUT;
    }

    // cache missing file data
    pFile->LoadData();

    // add new file object
    m_apFile.emplace(pFile->GetPath(), pFile);

    // associate archive
    pFile->m_pArchive    = this;
    pFile->m_iArchivePos = 0u;

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
    if(!m_apFile.count(pcPath)) return CORE_INVALID_INPUT;

    // remove and delete file object
    SAFE_DELETE(m_apFile.at(pcPath))
    m_apFile.erase(pcPath);

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
        iCurPosition += sizeof(coreUint8) + std::strlen((*it)->GetPath()) + 2u*sizeof(coreUint32);
    }

    // set absolute data position
    FOR_EACH(it, m_apFile)
    {
        (*it)->m_pArchive    = this;
        (*it)->m_iArchivePos = iCurPosition;

        iCurPosition += (*it)->GetSize();
    }
}