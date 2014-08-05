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
coreFile::coreFile(const char* pcPath)noexcept
: m_sPath       (pcPath)
, m_pData       (NULL)
, m_iSize       (0)
, m_pArchive    (NULL)
, m_iArchivePos (1)
{
    if(m_sPath.empty()) return;

    // open file
    SDL_RWops* pFile = SDL_RWFromFile(m_sPath.c_str(), "rb");
    if(!pFile)
    {
        Core::Log->Warning("File (%s) could not be opened (SDL: %s)", m_sPath.c_str(), SDL_GetError());
        return;
    }

    // get file size
    m_iSize = (coreUint)SDL_RWsize(pFile);

    // close file
    SDL_RWclose(pFile);
    Core::Log->Info("File (%s:%u) opened", m_sPath.c_str(), m_iSize);
}

coreFile::coreFile(const char* pcPath, coreByte* pData, const coreUint& iSize)noexcept
: m_sPath       (pcPath)
, m_pData       (pData)
, m_iSize       (iSize)
, m_pArchive    (NULL)
, m_iArchivePos (0)
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
coreError coreFile::Save(const char* pcPath)
{
    // check file data
    this->LoadData();
    if(!m_pData || !m_iSize) return CORE_INVALID_CALL;

    // save path
    m_sPath = pcPath;

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
    if(!m_iArchivePos) m_iArchivePos = 1;

    return CORE_OK;
}


// ****************************************************************
// load file data
coreError coreFile::LoadData()
{
    if(m_pData || !m_iSize || !m_iArchivePos) return CORE_INVALID_CALL;

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
: m_sPath ("")
{
}

coreArchive::coreArchive(const char* pcPath)noexcept
: m_sPath (pcPath)
{
    // open archive
    SDL_RWops* pArchive = SDL_RWFromFile(m_sPath.c_str(), "rb");
    if(!pArchive)
    {
        Core::Log->Warning("Archive (%s) could not be opened (SDL: %s)", m_sPath.c_str(), SDL_GetError());
        return;
    }

    // read magic number and file version
    coreUint aiHead[2];
    SDL_RWread(pArchive, &aiHead, sizeof(aiHead[0]), 2);

    // check magic number
    if(aiHead[0] != CORE_FILE_MAGIC)
    {
        SDL_RWclose(pArchive);
        Core::Log->Warning("Archive (%s) is not a valid CFA-file", m_sPath.c_str());
        return;
    }

    // read number of files
    coreUint iSize;
    SDL_RWread(pArchive, &iSize, sizeof(coreUint), 1);

    // read file headers
    for(coreUint i = 0; i < iSize; ++i)
    {
        coreUint iLength;
        char acPath[256];
        coreUint iSize;
        coreUint iPos;

        // read file header data
        SDL_RWread(pArchive, &iLength, sizeof(coreUint), 1);
        SDL_RWread(pArchive, acPath,   sizeof(char),     MAX(iLength, (coreUint)255));
        SDL_RWread(pArchive, &iSize,   sizeof(coreUint), 1);
        SDL_RWread(pArchive, &iPos,    sizeof(coreUint), 1);

        // add new file object
        coreFile* pNewFile      = new coreFile(acPath, NULL, iSize);
        pNewFile->m_pArchive    = this;
        pNewFile->m_iArchivePos = iPos;
        m_apFile[acPath]        = pNewFile;
    }

    // close archive
    SDL_RWclose(pArchive);
    Core::Log->Info("Archive (%s:%u) opened", m_sPath.c_str(), iSize);
}


// ****************************************************************
// destructor
coreArchive::~coreArchive()
{
    this->ClearFiles();
}


// ****************************************************************
// save archive
coreError coreArchive::Save(const char* pcPath)
{
    if(m_apFile.empty()) return CORE_INVALID_CALL;

    // save path
    m_sPath = PRINT(std::strcmp(coreData::StrRight(pcPath, 4), CORE_FILE_EXTENSION) ? "%s" CORE_FILE_EXTENSION : "%s", pcPath);

    // open archive
    SDL_RWops* pArchive = SDL_RWFromFile(m_sPath.c_str(), "wb");
    if(!pArchive)
    {
        Core::Log->Warning("Archive (%s) could not be saved (SDL: %s)", m_sPath.c_str(), SDL_GetError());
        return CORE_ERROR_FILE;
    }

    // save magic number and file version
    const coreUint aiHead[2] = {CORE_FILE_MAGIC, CORE_FILE_VERSION};
    SDL_RWwrite(pArchive, aiHead, sizeof(aiHead[0]), 2);

    // save number of files
    const coreUint iSize = m_apFile.size();
    SDL_RWwrite(pArchive, &iSize, sizeof(coreUint), 1);

    // cache missing file data
    FOR_EACH(it, m_apFile)
        it->second->LoadData();

    // save file headers
    this->__CalculatePositions();
    FOR_EACH(it, m_apFile)
    {
        // get path length
        const coreUint iLength = std::strlen(it->second->GetPath());

        // write header
        SDL_RWwrite(pArchive, &iLength,                   sizeof(coreUint), 1);
        SDL_RWwrite(pArchive, it->second->GetPath(),      sizeof(char),     iLength);
        SDL_RWwrite(pArchive, &it->second->GetSize(),     sizeof(coreUint), 1);
        SDL_RWwrite(pArchive, &it->second->m_iArchivePos, sizeof(coreUint), 1);
    }

    // save file data
    FOR_EACH(it, m_apFile)
    {
        SDL_RWwrite(pArchive, it->second->GetData(), sizeof(coreByte), it->second->GetSize());
        it->second->UnloadData();
    }

    // close archive
    SDL_RWclose(pArchive);

    return CORE_OK;
}


// ****************************************************************
// add file object
coreError coreArchive::AddFile(const char* pcPath)
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

coreError coreArchive::AddFile(coreFile* pFile)
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
    m_apFile[pFile->GetPath()] = pFile;

    // associate archive
    pFile->m_pArchive    = this;
    pFile->m_iArchivePos = 0;

    return CORE_OK;
}


// ****************************************************************
// remove file object
coreError coreArchive::DeleteFile(const coreUint& iIndex)
{
    if(iIndex >= m_apFile.size()) return CORE_INVALID_INPUT;

    // remove and delete file object
    SAFE_DELETE(m_apFile[iIndex])
    m_apFile.erase(iIndex);

    return CORE_OK;
}

coreError coreArchive::DeleteFile(const char* pcPath)
{
    if(!m_apFile.count(pcPath)) return CORE_INVALID_INPUT;

    // remove and delete file object
    SAFE_DELETE(m_apFile[pcPath])
    m_apFile.erase(pcPath);

    return CORE_OK;
}

coreError coreArchive::DeleteFile(coreFile* pFile)
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
        SAFE_DELETE(it->second)

    // clear memory
    m_apFile.clear();
}


// ****************************************************************
// calculate the data positions of all files
void coreArchive::__CalculatePositions()
{
    // calculate data start position
    coreUint iCurPosition = sizeof(coreUint);
    FOR_EACH(it, m_apFile)
        iCurPosition += sizeof(coreUint) + std::strlen(it->second->GetPath()) + sizeof(coreUint) + sizeof(coreUint);

    FOR_EACH(it, m_apFile)
    {
        // set absolute data position
        it->second->m_pArchive    = this;
        it->second->m_iArchivePos = iCurPosition;
        iCurPosition += it->second->GetSize();
    }
}