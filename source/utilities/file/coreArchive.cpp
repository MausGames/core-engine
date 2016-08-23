//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
coreFile::coreFile(const coreChar* pcPath)noexcept
: m_sPath       (pcPath)
, m_pData       (NULL)
, m_iSize       (0u)
, m_pArchive    (NULL)
, m_iArchivePos (1u)
{
    if(m_sPath.empty()) return;

    // retrieve and clamp file size
    const coreInt64 iFullSize = coreData::FileSize(m_sPath.c_str());
    if((0 <= iFullSize) && (iFullSize <= 0xFFFFFFFF)) m_iSize = iFullSize;

    // check for success
    if(!m_iSize) Core::Log->Warning("File (%s) could not be opened", m_sPath.c_str());
            else Core::Log->Info   ("File (%s:%u) opened",           m_sPath.c_str(), m_iSize);
}

coreFile::coreFile(const coreChar* pcPath, coreByte* pData, const coreUint32 iSize)noexcept
: m_sPath       (pcPath)
, m_pData       (pData)
, m_iSize       (iSize)
, m_pArchive    (NULL)
, m_iArchivePos (0u)
{
}


// ****************************************************************
// destructor
coreFile::~coreFile()
{
    // delete file data
    SAFE_DELETE_ARRAY(m_pData)
}


// ****************************************************************
// save file
coreStatus coreFile::Save(const coreChar* pcPath)
{
    // check file data
    this->LoadData();
    if(!m_pData || !m_iSize) return CORE_INVALID_CALL;

    // save path
    if(pcPath) m_sPath = pcPath;
    ASSERT(!m_sPath.empty())

    // open file
    SDL_RWops* pFile = SDL_RWFromFile(m_sPath.c_str(), "wb");
    if(!pFile)
    {
        Core::Log->Warning("File (%s) could not be saved (SDL: %s)", m_sPath.c_str(), SDL_GetError());
        return CORE_ERROR_FILE;
    }

    // save file data
    SDL_RWwrite(pFile, m_pData, sizeof(coreByte), m_iSize);

    // close file
    SDL_RWclose(pFile);
    if(!m_iArchivePos) m_iArchivePos = 1u;

    return CORE_OK;
}


// ****************************************************************
// compress file data
coreStatus coreFile::Compress(const coreInt8 iCompression)
{
    // check file data
    this->LoadData();
    if(!m_pData || !m_iSize) return CORE_INVALID_CALL;

    // compress data with deflate algorithm
    coreByte*  pNewData;
    coreUint32 iNewSize;
    const coreStatus iError = coreData::CompressDeflate(m_pData, m_iSize, &pNewData, &iNewSize, iCompression);

    if(iError == CORE_OK)
    {
        // delete old data
        SAFE_DELETE_ARRAY(m_pData)

        // save new data
        m_pData = pNewData;
        m_iSize = iNewSize;
    }

    return iError;
}


// ****************************************************************
// decompress file data
coreStatus coreFile::Decompress()
{
    // check file data
    this->LoadData();
    if(!m_pData || !m_iSize) return CORE_INVALID_CALL;

    // decompress data with deflate algorithm
    coreByte*  pNewData;
    coreUint32 iNewSize;
    const coreStatus iError = coreData::DecompressDeflate(m_pData, m_iSize, &pNewData, &iNewSize);

    if(iError == CORE_OK)
    {
        // delete old data
        SAFE_DELETE_ARRAY(m_pData)

        // save new data
        m_pData = pNewData;
        m_iSize = iNewSize;
    }

    return iError;
}


// ****************************************************************
// load file data
coreStatus coreFile::LoadData()
{
    // check file data
    if(m_pData || !m_iSize || !m_iArchivePos) return CORE_INVALID_CALL;

#if defined(_CORE_DEBUG_)

    // for correct hot-reloading (not in release, not in archive)
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
    SDL_RWread(pFile, m_pData, sizeof(coreByte), m_iSize);

    // close file
    SDL_RWclose(pFile);

    return CORE_OK;
}


// ****************************************************************
// return allocated file data copy
coreByte* coreFile::MoveData()
{
    coreByte* pOutput;
    if(m_iArchivePos)
    {
        // load file data
        this->LoadData();

        // move pointer with file data
        pOutput = m_pData;
        m_pData = NULL;
    }
    else
    {
        if(!m_pData || !m_iSize) return NULL;

        // create real file data copy
        pOutput = new coreByte[m_iSize];
        std::memcpy(pOutput, m_pData, m_iSize);
    }

    return pOutput;
}


// ****************************************************************
// constructor
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
    coreUint16 iFiles;
    SDL_RWread(pArchive, &iFiles, sizeof(coreUint16), 1u);

    // read file headers
    for(coreUintW i = iFiles; i--; )
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
    Core::Log->Info("Archive (%s:%u) opened", m_sPath.c_str(), iFiles);
}


// ****************************************************************
// destructor
coreArchive::~coreArchive()
{
    this->ClearFiles();
}


// ****************************************************************
// save archive
coreStatus coreArchive::Save(const coreChar* pcPath)
{
    if(m_apFile.empty()) return CORE_INVALID_CALL;

    // save path
    if(pcPath) m_sPath = pcPath;
    ASSERT(!m_sPath.empty())

    // open archive
    SDL_RWops* pArchive = SDL_RWFromFile(m_sPath.c_str(), "wb");
    if(!pArchive)
    {
        Core::Log->Warning("Archive (%s) could not be saved (SDL: %s)", m_sPath.c_str(), SDL_GetError());
        return CORE_ERROR_FILE;
    }

    // save magic number and file version
    const coreUint32 aiHead[2] = {CORE_FILE_MAGIC, CORE_FILE_VERSION};
    SDL_RWwrite(pArchive, aiHead, sizeof(coreUint32), 2u);

    // save number of files
    const coreUint16 iFiles = m_apFile.size();
    SDL_RWwrite(pArchive, &iFiles, sizeof(coreUint16), 1u);

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

    return CORE_OK;
}


// ****************************************************************
// add file object
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
// remove file object
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
// remove all file objects
void coreArchive::ClearFiles()
{
    // delete file objects
    FOR_EACH(it, m_apFile)
        SAFE_DELETE(*it)

    // clear memory
    m_apFile.clear();
}


// ****************************************************************
// calculate the data positions of all files
void coreArchive::__CalculatePositions()
{
    // calculate data start position
    coreUint32 iCurPosition = 2u*sizeof(coreUint32) + sizeof(coreUint16);
    FOR_EACH(it, m_apFile)
        iCurPosition += sizeof(coreUint8) + std::strlen((*it)->GetPath()) + 2u*sizeof(coreUint32);

    FOR_EACH(it, m_apFile)
    {
        // set absolute data position
        (*it)->m_pArchive    = this;
        (*it)->m_iArchivePos = iCurPosition;
        iCurPosition += (*it)->GetSize();
    }
}