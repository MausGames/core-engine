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
// TODO: implement for Android
coreError coreFile::SearchFolder(const char* pcPath, const char* pcFilter, std::vector<std::string>* pasOutput)
{
    if(!pcPath || !pasOutput) return CORE_INVALID_INPUT;

#if defined(_CORE_WINDOWS_)

    HANDLE hFolder;
    WIN32_FIND_DATA hFile;

    // open folder
    hFolder = FindFirstFile(coreUtils::Print("%s/%s/%s", coreUtils::AppPath(), pcPath, pcFilter), &hFile);
    if(hFolder == INVALID_HANDLE_VALUE)
    {
        Core::Log->Error(0, coreUtils::Print("Folder (%s) could not be opened", pcPath));
        return CORE_FILE_ERROR;
    }

    do
    {
        // check and add file path
        if(hFile.cFileName[0] != '.')
            pasOutput->push_back(coreUtils::Print("%s/%s", pcPath, hFile.cFileName));
    }
    while(FindNextFile(hFolder, &hFile));

    // close folder
    FindClose(hFolder);

#else

    DIR* pDir;
    struct dirent* pDirent;

    // open folder
    pDir = opendir(pcPath);
    if(!pDir)
    {
        Core::Log->Error(0, coreUtils::Print("Folder (%s) could not be opened", pcPath));
        return CORE_FILE_ERROR;
    }

    while((pDirent = readdir(pDir)) != NULL)
    {
        // check and add file path
        if(pDirent->d_name[0] != '.')
        {
            if(coreUtils::StrCmp(pDirent->d_name, pcFilter))
                pasOutput->push_back(coreUtils::Print("%s/%s", pcPath, pDirent->d_name));
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
    m_apFile.reserve(32);
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
    m_apFile.reserve(iSize);
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
        m_apFile.push_back(new coreFile(acPath, NULL, iSize));
        m_apFile.back()->m_pArchive    = this;
        m_apFile.back()->m_iArchivePos = iPos;
        m_apFileMap[acPath] = m_apFile.back();
    }

    // close archive
    SDL_RWclose(pArchive);
    Core::Log->Info(coreUtils::Print("Archive (%s) opened", m_sPath.c_str()));
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

    // cache missing file data
    for(auto it = m_apFile.begin(); it != m_apFile.end(); ++it)
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
    const coreUint iSize = m_apFile.size();
    SDL_RWwrite(pArchive, &iSize, sizeof(coreUint), 1);

    // save file headers
    this->__CalculatePositions();
    for(auto it = m_apFile.begin(); it != m_apFile.end(); ++it)
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
    for(auto it = m_apFile.begin(); it != m_apFile.end(); ++it)
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
    if(m_apFileMap.count(pcPath))
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
    if(m_apFileMap.count(pFile->GetPath()))
    {
        Core::Log->Error(0, coreUtils::Print("File (%s) already exists in Archive (%s)", pFile->GetPath(), m_sPath.c_str()));
        return CORE_INVALID_INPUT;
    }

    // cache missing file data
    pFile->LoadData();

    // add new file object
    m_apFile.push_back(pFile);
    m_apFileMap[pFile->GetPath()] = m_apFile.back();

    // associate archive
    m_apFile.back()->m_pArchive    = this;
    m_apFile.back()->m_iArchivePos = 0;

    return CORE_OK;
}


// ****************************************************************
// remove file object
coreError coreArchive::DeleteFile(const coreUint& iIndex)
{
    if(iIndex >= m_apFile.size()) return CORE_INVALID_INPUT;

    coreFile* pFile = m_apFile[iIndex];

    // remove file object
    m_apFile.erase(m_apFile.begin()+iIndex);
    m_apFileMap.erase(pFile->GetPath());

    // delete file object
    SAFE_DELETE(pFile)

    return CORE_OK;
}

coreError coreArchive::DeleteFile(const char* pcPath)
{
    if(!m_apFileMap.count(pcPath)) return CORE_INVALID_INPUT;

    // search index and delete file
    for(coreUint i = 0; i < m_apFile.size(); ++i)
    {
        if(m_apFile[i]->GetPath() == pcPath)
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
// remove all files
void coreArchive::ClearFiles()
{
    // delete file objects
    for(auto it = m_apFile.begin(); it != m_apFile.end(); ++it)
        SAFE_DELETE(*it)

    // clear memory
    m_apFile.clear();
    m_apFileMap.clear();
}


// ****************************************************************
// calculate the data positions of all files
void coreArchive::__CalculatePositions()
{
    // calculate data start position
    coreUint iCurPosition = sizeof(coreUint);
    for(auto it = m_apFile.begin(); it != m_apFile.end(); ++it)
        iCurPosition += sizeof(coreUint) + strlen((*it)->GetPath()) + sizeof(coreUint) + sizeof(coreUint);

    for(auto it = m_apFile.begin(); it != m_apFile.end(); ++it)
    {
        // set absolute data position
        (*it)->m_pArchive    = this;
        (*it)->m_iArchivePos = iCurPosition;
        iCurPosition += (*it)->GetSize();
    }
}