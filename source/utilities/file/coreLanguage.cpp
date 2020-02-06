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
coreTranslate::coreTranslate()noexcept
: m_pLanguage  (NULL)
, m_apsPointer {}
{
}

coreTranslate::coreTranslate(const coreTranslate& c)noexcept
: m_pLanguage  (NULL)
, m_apsPointer {}
{
    // associate object with language
    this->ChangeLanguage(c.m_pLanguage);
}


// ****************************************************************
/* destructor */
coreTranslate::~coreTranslate()
{
    // safely unbind last language file
    this->ChangeLanguage(NULL);

    // clear memory
    m_apsPointer.clear();
}


// ****************************************************************
/* assignment operations */
coreTranslate& coreTranslate::operator = (const coreTranslate& c)noexcept
{
    // associate object with language
    this->ChangeLanguage(c.m_pLanguage);

    return *this;
}


// ****************************************************************
/* change associated language file */
void coreTranslate::ChangeLanguage(coreLanguage* pLanguage)
{
    if(m_pLanguage == pLanguage) return;

    if(m_pLanguage)
    {
        // unbind everything from current language
        FOR_EACH(it, m_apsPointer) m_pLanguage->UnbindForeign(*m_apsPointer.get_key(it));
        m_pLanguage->__UnbindObject(this);
    }

    // set new language file
    m_pLanguage = pLanguage;

    if(m_pLanguage)
    {
        // bind everything to new language
        m_pLanguage->__BindObject(this);
        FOR_EACH(it, m_apsPointer) m_pLanguage->BindForeign(*m_apsPointer.get_key(it), it->c_str());

        // invoke object update
        this->__Update();
    }
}


// ****************************************************************
/* bind own string pointer */
void coreTranslate::_BindString(std::string* psString, const coreHashString& sKey)
{
    ASSERT(psString && sKey)

    if(!m_pLanguage)
    {
        // associate object with default language
        this->ChangeLanguage(Core::Language);
        ASSERT(m_pLanguage)
    }

    // bind string to language and save it internally
    m_pLanguage->BindForeign(psString, sKey);
    m_apsPointer[psString] = sKey.GetString();

    // invoke object update
    this->__Update();
}


// ****************************************************************
/* unbind own string pointer */
void coreTranslate::_UnbindString(std::string* psString)
{
    ASSERT(psString)

    // remove string internally
    if(m_pLanguage && m_apsPointer.erase(psString))
    {
        // unbind from language
        m_pLanguage->UnbindForeign(psString);
    }
}


// ****************************************************************
/* constructor */
coreLanguage::coreLanguage()noexcept
: m_asStringList {}
, m_apsForeign   {}
, m_apObject     {}
, m_sPath        ("")
{
}

coreLanguage::coreLanguage(const coreChar* pcPath)noexcept
: coreLanguage ()
{
    // load language file
    this->Load(pcPath);
}


// ****************************************************************
/* destructor */
coreLanguage::~coreLanguage()
{
    ASSERT(m_apsForeign.empty() && m_apObject.empty())

    // unbind missing translation objects
    FOR_EACH(it, m_apObject)
        (*it)->ChangeLanguage(NULL);

    // clear memory
    m_asStringList.clear();
}


// ****************************************************************
/* load language file */
coreStatus coreLanguage::Load(const coreChar* pcPath)
{
    coreFileScope pFile = Core::Manager::Resource->RetrieveFile(pcPath);

    // get file data
    const coreChar* pcData = r_cast<const coreChar*>(pFile->GetData());
    if(!pcData) return CORE_ERROR_FILE;

    // prepare range pointers (from, to) and end pointer (out of bound)
    const coreChar* pcFrom = pcData + 1u;
    const coreChar* pcTo   = pcFrom;
    const coreChar* pcEnd  = pcFrom + pFile->GetSize() - 1u;

    const auto nAssignFunc = [&](std::string* OUTPUT pString)
    {
        // assign string currently in range
        pString->assign(pcFrom, pcTo - pcFrom);
        coreData::StrTrim(pString);

        // begin next string
        pcFrom = pcTo + 1u;
    };

    // clear all existing language-strings
    FOR_EACH(it, m_asStringList) it->clear();

    std::string sKey = "";
    while(pcTo != pcEnd)
    {
        if((*pcTo) == CORE_LANGUAGE_ASSIGN[0] && sKey.empty())
        {
            // extract key
            nAssignFunc(&sKey);
            WARN_IF(sKey.empty()) sKey.assign(1u, ' ');
        }
        else if((*pcTo) == CORE_LANGUAGE_KEY[0] && !sKey.empty())
        {
            // extract language-string
            nAssignFunc(&m_asStringList[sKey.c_str()]);
            sKey.clear();
        }

        ++pcTo;
    }
    if(!sKey.empty()) nAssignFunc(&m_asStringList[sKey.c_str()]);

    // save relative path
    m_sPath = pcPath;

    // reduce memory consumption
    FOR_EACH(it, m_asStringList)
    {
        std::string& sString = (*it);

        // assign key as value to possible empty language-string
        if(sString.empty()) sString.assign(PRINT(CORE_LANGUAGE_KEY "%s", m_asStringList.get_string(it)));
        sString.shrink_to_fit();
    }
    m_asStringList.shrink_to_fit();

    // update all foreign strings and objects
    FOR_EACH(it, m_apsForeign) (*m_apsForeign.get_key(it))->assign(m_asStringList.at(it->c_str()));
    FOR_EACH(it, m_apObject)   (*it)->__Update();

    Core::Log->Info("Language (%s, %u strings) loaded", pFile->GetPath(), m_asStringList.size());
    return CORE_OK;
}


// ****************************************************************
/* bind foreign string pointer */
void coreLanguage::BindForeign(std::string* psForeign, const coreHashString& sKey)
{
    ASSERT(psForeign && sKey)

    // assign key as value to possible new language-string
    if(!m_asStringList.count(sKey)) m_asStringList.emplace(sKey, PRINT(CORE_LANGUAGE_KEY "%s", sKey.GetString()));

    // save foreign string pointer and key
    m_apsForeign[psForeign].assign(sKey.GetString());

    // initially update the foreign string
    psForeign->assign(m_asStringList.at(sKey));
}


// ****************************************************************
/* find and read string from language file directly */
coreBool coreLanguage::FindString(const coreChar* pcPath, const coreChar* pcKey, std::string* OUTPUT psOutput)
{
    ASSERT(psOutput)

    coreFileScope pFile = Core::Manager::Resource->RetrieveFile(pcPath);

    // get file data
    const coreChar* pcData = r_cast<const coreChar*>(pFile->GetData());
    if(!pcData) return false;

    // save key length (file data not null-terminated)
    const coreUintW iKeyLen = std::strlen(pcKey);

    // prepare range pointers (from, to) and end pointer (out of bound)
    const coreChar* pcFrom = pcData + 1u;
    const coreChar* pcTo   = pcFrom;
    const coreChar* pcEnd  = pcFrom + pFile->GetSize() - 1u;

    coreBool bFound = false;
    while(pcTo != pcEnd)
    {
        if((*pcTo) == CORE_LANGUAGE_ASSIGN[0])
        {
            // search for key
            if(!std::memcmp(pcFrom, pcKey, iKeyLen)) bFound = true;
            pcFrom = pcTo + 1u;
        }
        else if((*pcTo) == CORE_LANGUAGE_KEY[0])
        {
            // stop searching
            if(bFound) break;
            pcFrom = pcTo + 1u;
        }

        ++pcTo;
    }
    if(bFound)
    {
        // extract language-string
        psOutput->assign(pcFrom, pcTo - pcFrom);
        coreData::StrTrim(psOutput);
    }

    return bFound;
}


// ****************************************************************
/* get list with all available languages <name, path> */
void coreLanguage::GetAvailableLanguages(const coreChar* pcPath, const coreChar* pcFilter, coreLookup<std::string, std::string>* OUTPUT pasOutput)
{
    ASSERT(pasOutput)

    // retrieve files from the folder
    std::vector<std::string> asFile;
    coreData::ScanFolder(pcPath, pcFilter, &asFile);

    // reserve some memory
    pasOutput->reserve(asFile.size());

    FOR_EACH(it, asFile)
    {
        // find and read language-name
        std::string sName;
        if(!coreLanguage::FindString(it->c_str(), "LANGUAGE", &sName))
        {
            Core::Log->Warning("Language (%s) does not contain a valid LANGUAGE key", it->c_str());
            sName = coreData::StrFilename(it->c_str());
        }

        // save language-name with file-path
        pasOutput->emplace(std::move(sName), std::move(*it));
    }
}

void coreLanguage::GetAvailableLanguages(coreLookup<std::string, std::string>* OUTPUT pasOutput)
{
    // get list with all available languages from the default location
    coreLanguage::GetAvailableLanguages("data/languages", "*.lng", pasOutput);

    // check for success (# something has to be available in this location)
    WARN_IF(pasOutput->empty()) Core::Log->Warning("No language files found (data/languages/*.lng)");
}