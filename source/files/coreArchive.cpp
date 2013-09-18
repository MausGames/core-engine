//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
coreFile::coreFile(const char* pcPath)
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
        Core::Log->Error(0, coreUtils::Print("File (%s) could not be opened", pcPath));
        return;
    }

    // get file size
    m_iSize = (coreUint)SDL_RWsize(pFile);

    // close file
    SDL_RWclose(pFile);
    Core::Log->Info(coreUtils::Print("File (%s) opened", m_sPath.c_str()));
}

coreFile::coreFile(const char* pcPath, coreByte* pData, const coreUint& iSize)
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
        Core::Log->Error(0, coreUtils::Print("File (%s) could not be saved", m_sPath.c_str()));
        return CORE_FILE_ERROR;
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
        if(!pFile) return CORE_FILE_ERROR;

        // seek file data position
        SDL_RWseek(pFile, m_iArchivePos, RW_SEEK_SET);
    }
    else
    {
        // open direct file
        pFile = SDL_RWFromFile(m_sPath.c_str(), "rb");
        if(!pFile) return CORE_FILE_ERROR;
    }

    // cache file data
    m_pData = new coreByte[m_iSize];
    SDL_RWread(pFile, m_pData, sizeof(coreByte), m_iSize);

    // close file
    SDL_RWclose(pFile);

    return CORE_OK;
}


// ****************************************************************
// check if file exists
bool coreFile::FileExists(const char* pcPath)
{
    // open file
    SDL_RWops* pFile = SDL_RWFromFile(pcPath, "r");
    if(pFile)
    {
        // file exists
        SDL_RWclose(pFile);
        return true;
    }

    return false;
}


// ****************************************************************
// retrieve relative paths of all files from a folder
coreError coreFile::SearchFolder(const char* pcFolder, const char* pcFilter, std::vector<std::string>* pasOutput)
{
    if(!pcFolder || !pasOutput) return CORE_INVALID_INPUT;

#if defined(_CORE_WINDOWS_)

    HANDLE hFolder;
    WIN32_FIND_DATA hFile;

    // open folder
    hFolder = FindFirstFile(coreUtils::Print("%s/%s/%s", coreUtils::AppPath(), pcFolder, pcFilter), &hFile);
    if(hFolder == INVALID_HANDLE_VALUE)
    {
        Core::Log->Error(0, coreUtils::Print("Folder (%s) could not be opened", pcFolder));
        return CORE_FILE_ERROR;
    }

    do
    {
        // check and add file path
        if(hFile.cFileName[0] != '.')
            pasOutput->push_back(coreUtils::Print("%s/%s", pcFolder, hFile.cFileName));
    }
    while(FindNextFile(hFolder, &hFile));

    // close folder
    FindClose(hFolder);

#else

    DIR* pDir;
    struct dirent* pDirent;

    // open folder
    pDir = opendir(pcFolder);
    if(!pDir)
    {
        Core::Log->Error(0, coreUtils::Print("Folder (%s) could not be opened", pcFolder));
        return CORE_FILE_ERROR;
    }

    while((pDirent = readdir(pDir)) != NULL)
    {
        // check and add file path
        if(pDirent->d_name[0] != '.')
        {
            if(coreUtils::StrCmp(pDirent->d_name, pcFilter))
                pasOutput->push_back(coreUtils::Print("%s/%s", pcFolder, pDirent->d_name));
        }
    }

    // close folder
    closedir(pDir);

#endif

    return CORE_OK;
}


// ****************************************************************
// constructor
coreArchive::coreArchive()
: m_sPath ("")
{
    // reserve memory for file objects
    m_aFile.reserve(32);
}

coreArchive::coreArchive(const char* pcPath)
: m_sPath (pcPath)
{
    // open archive
    SDL_RWops* pArchive = SDL_RWFromFile(m_sPath.c_str(), "rb");
    if(!pArchive)
    {
        Core::Log->Error(0, coreUtils::Print("Archive (%s) could not be opened", m_sPath.c_str()));
        return;
    }

    // read number of files
    coreUint iSize;
    SDL_RWread(pArchive, &iSize, sizeof(coreUint), 1);

    // read file headers
    m_aFile.reserve(iSize);
    for(coreUint i = 0; i < iSize; ++i)
    {
        coreUint iLength;
        char acPath[256];
        coreUint iSize;
        coreUint iPos;

        // read file header data
        SDL_RWread(pArchive, &iLength, sizeof(coreUint), 1);
        SDL_RWread(pArchive, acPath,   sizeof(char),     MAX(iLength, (unsigned)255));
        SDL_RWread(pArchive, &iSize,   sizeof(coreUint), 1);
        SDL_RWread(pArchive, &iPos,    sizeof(coreUint), 1);

        // add new file object
        m_aFile.push_back(new coreFile(acPath, NULL, iSize));
        m_aFile.back()->m_pArchive    = this;
        m_aFile.back()->m_iArchivePos = iPos;
        m_aFileMap[acPath] = m_aFile.back();
    }

    // close archive
    SDL_RWclose(pArchive);
    Core::Log->Info(coreUtils::Print("Archive (%s) opened", m_sPath.c_str()));
}


// ****************************************************************
// destructor
coreArchive::~coreArchive()
{
    // delete file objects
    for(coreUint i = 0; i < m_aFile.size(); ++i)
        SAFE_DELETE(m_aFile[i])

    // clear memory
    m_aFile.clear();
    m_aFileMap.clear();
}


// ****************************************************************
// save archive
coreError coreArchive::Save(const char* pcPath)
{
    if(m_aFile.empty()) return CORE_INVALID_CALL;

    // cache missing file data
    for(auto it = m_aFile.begin(); it != m_aFile.end(); ++it)
        (*it)->LoadData();

    // save path
    m_sPath = pcPath;

    // open archive
    SDL_RWops* pArchive = SDL_RWFromFile(m_sPath.c_str(), "wb");
    if(!pArchive)
    {
        Core::Log->Error(0, coreUtils::Print("Archive (%s) could not be saved", m_sPath.c_str()));
        return CORE_FILE_ERROR;
    }

    // save number of files
    const coreUint iSize = m_aFile.size();
    SDL_RWwrite(pArchive, &iSize, sizeof(coreUint), 1);

    // save file headers
    this->__CalculatePositions();
    for(auto it = m_aFile.begin(); it != m_aFile.end(); ++it)
    {
        // get path length
        const coreUint iLength = strlen((*it)->GetPath());

        // write header
        SDL_RWwrite(pArchive, &iLength,              sizeof(coreUint), 1);
        SDL_RWwrite(pArchive, (*it)->GetPath(),      sizeof(char),     iLength);
        SDL_RWwrite(pArchive, &(*it)->GetSize(),     sizeof(coreUint), 1);
        SDL_RWwrite(pArchive, &(*it)->m_iArchivePos, sizeof(coreUint), 1);
    }

    // save file data
    for(auto it = m_aFile.begin(); it != m_aFile.end(); ++it)
        SDL_RWwrite(pArchive, (*it)->GetData(), sizeof(coreByte), (*it)->GetSize());

    // close archive
    SDL_RWclose(pArchive);

    return CORE_OK;
}


// ****************************************************************
// add file object
coreError coreArchive::AddFile(const char* pcPath)
{
    // check already existing file
    if(m_aFileMap.count(pcPath))
    {
        Core::Log->Error(0, coreUtils::Print("File (%s) already exists in Archive (%s)", pcPath, m_sPath.c_str()));
        return CORE_INVALID_INPUT;
    }

    // open and add new file object
    return this->AddFile(new coreFile(pcPath));
}

coreError coreArchive::AddFile(coreFile* pFile)
{
    if(pFile->m_pArchive) return CORE_INVALID_INPUT;

    // check already existing file
    if(m_aFileMap.count(pFile->GetPath()))
    {
        Core::Log->Error(0, coreUtils::Print("File (%s) already exists in Archive (%s)", pFile->GetPath(), m_sPath.c_str()));
        return CORE_INVALID_INPUT;
    }

    // cache missing file data
    pFile->LoadData();

    // add new file object
    m_aFile.push_back(pFile);
    m_aFileMap[pFile->GetPath()] = m_aFile.back();

    // associate archive
    m_aFile.back()->m_pArchive    = this;
    m_aFile.back()->m_iArchivePos = 0;

    return CORE_OK;
}


// ****************************************************************
// delete file object
coreError coreArchive::DeleteFile(const coreUint& iIndex)
{
    if(iIndex >= m_aFile.size()) return CORE_INVALID_INPUT;

    coreFile* pFile = m_aFile[iIndex];
    const std::string sPath = pFile->GetPath();

    // delete file object
    SAFE_DELETE(pFile)

    // remove file object
    m_aFile.erase(m_aFile.begin()+iIndex);
    m_aFileMap.erase(sPath);

    return CORE_OK;
}

coreError coreArchive::DeleteFile(const char* pcPath)
{
    if(!m_aFileMap.count(pcPath)) return CORE_INVALID_INPUT;

    // search index and delete file
    for(coreUint i = 0; i < m_aFile.size(); ++i)
    {
        if(m_aFile[i]->GetPath() == pcPath)
            return this->DeleteFile(i);
    }

    return CORE_OK;
}

coreError coreArchive::DeleteFile(coreFile* pFile)
{
    if(pFile->m_pArchive != this) return CORE_INVALID_INPUT;
    return this->DeleteFile(pFile->GetPath());
}


// ****************************************************************
// calculate the data positions of all files
void coreArchive::__CalculatePositions()
{
    // calculate data start position
    coreUint iCurPosition = sizeof(coreUint);
    for(auto it = m_aFile.begin(); it != m_aFile.end(); ++it)
        iCurPosition += sizeof(coreUint) + strlen((*it)->GetPath()) + sizeof(coreUint) + sizeof(coreUint);

    for(auto it = m_aFile.begin(); it != m_aFile.end(); ++it)
    {
        // set absolute data position
        (*it)->m_pArchive    = this;
        (*it)->m_iArchivePos = iCurPosition;
        iCurPosition += (*it)->GetSize();
    }
}