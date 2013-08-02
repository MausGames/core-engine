#include "Core.h"


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
{
    // save name and path
    m_sPath = pcPath;
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
        coreFile* pNewFile = new coreFile();

        coreUint iLength; 
        char acName[256];

        // read name 
        fread(&iLength, sizeof(coreUint), 1,                           pArchive);
        fread(acName,   sizeof(char),     MAX(iLength, (unsigned)256), pArchive);
        pNewFile->sName = acName;

        // read remaining file header
        fread(&pNewFile->iSize,     sizeof(coreUint), 1, pArchive);
        fread(&pNewFile->iPosition, sizeof(coreUint), 1, pArchive);
        pNewFile->pData = NULL;

        // add new file object
        m_aFile.push_back(pNewFile);
        m_aFileMap[pNewFile->sName] = pNewFile;
    }

    // close archive
    fclose(pArchive);
}


// ****************************************************************
// destructor
coreArchive::~coreArchive()
{
    // delete file objects
    for(coreUint i = 0; i < m_aFile.size(); ++i)
    {
        SAFE_DELETE_ARRAY(m_aFile[i]->pData)
        SAFE_DELETE(m_aFile[i])
    }

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
        if(!(*it)->pData) this->__CacheFile(*it);

    // save name and path
    m_sPath = pcPath;
    m_sName = m_sPath.substr(m_sPath.find_last_of("/\\")+1);
    m_sName = m_sName.substr(0, m_sName.find_last_of("_.\0")-1);

    // open archive
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
        // get name length
        const coreUint iLength = (*it)->sName.length();

        // write header
        fwrite(&iLength,             sizeof(coreUint), 1,       pArchive);
        fwrite((*it)->sName.c_str(), sizeof(char),     iLength, pArchive);
        fwrite(&(*it)->iSize,        sizeof(coreUint), 1,       pArchive);
        fwrite(&(*it)->iPosition,    sizeof(coreUint), 1,       pArchive);
    }

    // save file data
    for(auto it = m_aFile.begin(); it != m_aFile.end(); ++it)
        fwrite((*it)->pData, sizeof(coreByte), (*it)->iSize, pArchive);

    // close archive
    fclose(pArchive);

    return true;
}


// ****************************************************************
// add file object
bool coreArchive::AddFile(const char* pcPath)
{
    coreFile* pNewFile = new coreFile();

    // save file name
    pNewFile->sName = pcPath;
    pNewFile->sName = pNewFile->sName.substr(pNewFile->sName.find_last_of("/\\")+1);

    // check already existing file
    if(m_aFileMap.count(pNewFile->sName))
    {
        SAFE_DELETE(pNewFile)
        return false;
    }

    // open file
    FILE* pFile = fopen(pcPath, "rb");
    if(!pFile)
    {
        Core::Log->Error(0, coreUtils::Print("File (%s) could not be opened", pcPath));
        SAFE_DELETE(pNewFile)
        return false;
    }

    // get file data and size
    coreUtils::FileDataSize(pFile, &pNewFile->pData, &pNewFile->iSize);

    // close file
    fclose(pFile);

    // add new file object
    m_aFile.push_back(pNewFile);
    m_aFileMap[pNewFile->sName] = pNewFile;

    // recalculate the data positions
    this->__CalculatePositions();

    return true;
}


// ****************************************************************
// delete file object
bool coreArchive::DeleteFile(const coreUint& iIndex)
{
    if(iIndex >= m_aFile.size()) return false;

    coreFile* pFile = m_aFile[iIndex];
    const std::string sName = pFile->sName;

    // delete cached data and file object
    SAFE_DELETE_ARRAY(pFile->pData)
    SAFE_DELETE(pFile)

    // remove file object
    m_aFile.erase(m_aFile.begin()+iIndex);
    m_aFileMap.erase(m_aFileMap.find(sName));

    // recalculate the data positions
    this->__CalculatePositions();

    return true;
}

bool coreArchive::DeleteFile(const char* pcName)
{
    if(!m_aFileMap.count(pcName)) return false;

    // search index and delete file
    for(coreUint i = 0; i < m_aFile.size(); ++i)
    {
        if(m_aFile[i]->sName == pcName)
            return this->DeleteFile(i);
    }

    return false;
}


// ****************************************************************
// cache and access file data
const coreByte* coreArchive::GetFileData(const coreUint& iIndex)
{
    if(iIndex >= m_aFile.size()) return NULL; 

    // cache data if necessary
    coreFile* pFile = m_aFile[iIndex];
    if(!pFile->pData) this->__CacheFile(pFile); 
    
    return pFile->pData;
}

const coreByte* coreArchive::GetFileData(const char* pcName)
{
    if(!m_aFileMap.count(pcName)) return NULL;

    // cache data if necessary
    coreFile* pFile = m_aFileMap[pcName];
    if(!pFile->pData) this->__CacheFile(pFile); 
    
    return pFile->pData;
}


// ****************************************************************
// cache file data
void coreArchive::__CacheFile(coreFile* pFile)
{
    // open archive
    FILE* pArchive = fopen(m_sPath.c_str(), "wb");
    if(!pArchive) return;

    // seek file data position
    fseek(pArchive, pFile->iPosition, SEEK_SET);

    // cache file data
    pFile->pData = new coreByte[pFile->iSize];
    fread(pFile->pData, sizeof(coreByte), pFile->iSize, pArchive);

    // close archive
    fclose(pArchive);
}


// ****************************************************************
// calculate the data positions of all files
void coreArchive::__CalculatePositions()
{
    // calculate data start position
    coreUint iCurPosition = sizeof(coreUint);
    for(auto it = m_aFile.begin(); it != m_aFile.end(); ++it)
        iCurPosition += sizeof(coreUint) + (*it)->sName.length() + sizeof(coreUint) + sizeof(coreUint);

    for(auto it = m_aFile.begin(); it != m_aFile.end(); ++it)
    {
        // calculate data position
        (*it)->iPosition = iCurPosition;
        iCurPosition += (*it)->iSize;
    }
}