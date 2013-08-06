#include "Core.h"


// ****************************************************************
// constructor
coreFile::coreFile(const char* pcPath)
: m_sPath     (pcPath)
, m_iSize     (0)
, m_pData     (NULL)
, m_pArchive  (NULL)
, m_iPosition (0)
{
    // open file
    FILE* pFile = fopen(m_sPath.c_str(), "rb");
    if(!pFile)
    {
        Core::Log->Error(0, coreUtils::Print("File (%s) could not be opened", pcPath));
        return;
    }

    // get file size
    fseek(pFile, 0, SEEK_END);
    m_iSize = (coreUint)ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    // get file data
    m_pData = new coreByte[m_iSize];
    fread(m_pData, sizeof(coreByte), m_iSize, pFile);

    // close file
    fclose(pFile);
}

coreFile::coreFile(const char* pcPath, const coreUint& iSize, coreByte* pData)
: m_sPath     (pcPath)
, m_iSize     (iSize)
, m_pData     (pData)
, m_pArchive  (NULL)
, m_iPosition (0)
{
}

coreFile::coreFile(const char* pcPath, const coreUint& iSize, coreArchive* pArchive, const coreUint& iPosition)
: m_sPath     (pcPath)
, m_iSize     (iSize)
, m_pData     (NULL)
, m_pArchive  (pArchive)
, m_iPosition (iPosition)
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
// check if file exists
bool coreFile::FileExists(const char* pcPath)
{
    // open file
    FILE* pFile = fopen(pcPath, "r");
    if(pFile)
    {
        // file exists
        fclose(pFile);
        return true;
    }

    return false;
}


// ****************************************************************
// cache file data
void coreFile::__CacheData()
{
    if(!m_pArchive) return;
    if(m_pData) return;

    // open archive file
    FILE* pArchive = fopen(m_pArchive->GetPath(), "wb");
    if(!pArchive) return;

    // seek file data position
    fseek(pArchive, m_iPosition, SEEK_SET);

    // cache file data
    m_pData = new coreByte[m_iSize];
    fread(m_pData, sizeof(coreByte), m_iSize, pArchive);

    // close archive file
    fclose(pArchive);
}


// ****************************************************************
// constructor
coreArchive::coreArchive()
: m_sName ("")
, m_sPath ("")
{
    // reserve some memory
    m_aFile.reserve(32);
}

coreArchive::coreArchive(const char* pcPath)
: m_sPath (pcPath)
{
    // save name
    m_sName = m_sPath.substr(m_sPath.find_last_of("/\\")+1);
    m_sName = m_sName.substr(0, m_sName.find_last_of("_.\0")-1);

    // open archive
    FILE* pArchive = fopen(m_sPath.c_str(), "rb");
    if(!pArchive)
    {
        Core::Log->Error(0, coreUtils::Print("Archive (%s) could not be opened", m_sPath.c_str()));
        return;
    }

    // read number of files
    coreUint iSize;
    fread(&iSize, sizeof(coreUint), 1, pArchive);

    // read file headers
    m_aFile.reserve(iSize);
    for(coreUint i = 0; i < iSize; ++i)
    {
        coreUint iLength; 
        char acPath[256];
        coreUint iSize;
        coreUint iPosition;

        // read file header data
        fread(&iLength,   sizeof(coreUint), 1,                           pArchive);
        fread(acPath,     sizeof(char),     MAX(iLength, (unsigned)255), pArchive);
        fread(&iSize,     sizeof(coreUint), 1,                           pArchive);
        fread(&iPosition, sizeof(coreUint), 1,                           pArchive);

        // add new file object
        m_aFile.push_back(new coreFile(acPath, iSize, this, iPosition));
        m_aFileMap[acPath] = m_aFile.back();
    }

    // close archive file
    fclose(pArchive);
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
// save archive file
bool coreArchive::Save(const char* pcPath)
{
    if(m_aFile.empty()) return false;

    // cache missing file data
    for(auto it = m_aFile.begin(); it != m_aFile.end(); ++it)
        (*it)->GetData();

    // save name and path
    m_sPath = pcPath;
    m_sName = m_sPath.substr(m_sPath.find_last_of("/\\")+1);
    m_sName = m_sName.substr(0, m_sName.find_last_of("_.\0")-1);

    // open archive file
    FILE* pArchive = fopen(m_sPath.c_str(), "wb");
    if(!pArchive)
    {
        Core::Log->Error(0, coreUtils::Print("Archive (%s) could not be saved", m_sPath.c_str()));
        return false;
    }

    // save number of files
    const coreUint iSize = m_aFile.size();
    fwrite(&iSize, sizeof(coreUint), 1, pArchive);

    // save file headers
    this->__CalculatePositions();
    for(auto it = m_aFile.begin(); it != m_aFile.end(); ++it)
    {
        // get path length
        const coreUint iLength = strlen((*it)->GetPath());

        // write header
        fwrite(&iLength,              sizeof(coreUint), 1,       pArchive);
        fwrite((*it)->GetPath(),      sizeof(char),     iLength, pArchive);
        fwrite(&(*it)->GetSize(),     sizeof(coreUint), 1,       pArchive);
        fwrite(&(*it)->GetPosition(), sizeof(coreUint), 1,       pArchive);
    }

    // save file data
    for(auto it = m_aFile.begin(); it != m_aFile.end(); ++it)
        fwrite((*it)->GetData(), sizeof(coreByte), (*it)->GetSize(), pArchive);

    // close archive file
    fclose(pArchive);

    return true;
}


// ****************************************************************
// add file object
bool coreArchive::AddFile(const char* pcPath)
{
    // check already existing file
    if(m_aFileMap.count(pcPath))
    {
        Core::Log->Error(0, coreUtils::Print("File (%s) already exists in Archive (%s)", pcPath, m_sPath.c_str()));
        return false;
    }

    // add new file object
    m_aFile.push_back(new coreFile(pcPath));
    m_aFileMap[pcPath] = m_aFile.back();

    // associate archive file
    m_aFile.back()->__AssociateArchive(this, 0);

    return true;
}

bool coreArchive::AddFile(coreFile* pFile)
{
    // check already existing file
    if(m_aFileMap.count(pFile->GetPath()))
    {
        Core::Log->Error(0, coreUtils::Print("File (%s) already exists in Archive (%s)", pFile->GetPath(), m_sPath.c_str()));
        return false;
    }

    // add new file object
    m_aFile.push_back(pFile);
    m_aFileMap[pFile->GetPath()] = m_aFile.back();

    // associate archive file
    m_aFile.back()->__AssociateArchive(this, 0);

    return true;
}


// ****************************************************************
// delete file object
bool coreArchive::DeleteFile(const coreUint& iIndex)
{
    if(iIndex >= m_aFile.size()) return false;

    coreFile* pFile = m_aFile[iIndex];
    const std::string sPath = pFile->GetPath();

    // delete file object
    SAFE_DELETE(pFile)

    // remove file object
    m_aFile.erase(m_aFile.begin()+iIndex);
    m_aFileMap.erase(m_aFileMap.find(sPath));

    return true;
}

bool coreArchive::DeleteFile(const char* pcPath)
{
    if(!m_aFileMap.count(pcPath)) return false;

    // search index and delete file
    for(coreUint i = 0; i < m_aFile.size(); ++i)
    {
        if(m_aFile[i]->GetPath() == pcPath)
            return this->DeleteFile(i);
    }

    return false;
}

bool coreArchive::DeleteFile(coreFile* pFile)
{
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
        (*it)->__AssociateArchive(this, iCurPosition);
        iCurPosition += (*it)->GetSize();
    }
}