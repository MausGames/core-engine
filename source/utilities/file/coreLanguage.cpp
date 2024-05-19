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
        this->__UpdateTranslate();
    }
}


// ****************************************************************
/* manually refresh own string pointers */
void coreTranslate::RefreshLanguage()
{
    if(m_pLanguage)
    {
        // refresh everything in current language
        FOR_EACH(it, m_apsPointer) m_pLanguage->RefreshForeign(*m_apsPointer.get_key(it), it->c_str());

        // invoke object update
        this->__UpdateTranslate();
    }
}


// ****************************************************************
/* bind own string pointer */
void coreTranslate::_BindString(coreString* psString, const coreHashString& sKey, coreAssembleFunc nFunc)
{
    ASSERT(psString && sKey)

    // check current pointer
    if((m_apsPointer[psString] == sKey.GetString()) && !nFunc) return;

    if(!m_pLanguage)
    {
        // associate object with default language
        this->ChangeLanguage(Core::Language);
        ASSERT(m_pLanguage)
    }

    // bind string to language and save it internally
    m_pLanguage->BindForeign(psString, sKey, std::move(nFunc));
    m_apsPointer.at(psString) = sKey.GetString();

    // invoke object update
    this->__UpdateTranslate();
}


// ****************************************************************
/* unbind own string pointer */
void coreTranslate::_UnbindString(coreString* psString)
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
, m_asForeign    {}
, m_anAssemble   {}
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
    ASSERT(m_asForeign.empty() && m_anAssemble.empty() && m_apObject.empty())

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
    const coreChar* pcFrom = pcData + 1u;   // # handle first key prefix
    const coreChar* pcTo   = pcFrom;
    const coreChar* pcEnd  = pcFrom + pFile->GetSize() - 1u;
    ASSERT(pFile->GetSize())

    const auto nAssignFunc = [&](coreString* OUTPUT pString)
    {
        ASSERT(pString->empty())

        // assign string currently in range
        pString->assign(pcFrom, pcTo - pcFrom);
        pString->trim();

        // begin next string
        pcFrom = pcTo + 1u;
    };

    // clear all existing language-strings
    FOR_EACH(it, m_asStringList) it->clear();

    coreString sKey = "";
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
            nAssignFunc(&m_asStringList.bs(sKey.c_str()));
            sKey.clear();
        }

        ++pcTo;
    }
    if(!sKey.empty()) nAssignFunc(&m_asStringList.bs(sKey.c_str()));

    // save relative path
    m_sPath = pcPath;

    FOR_EACH(it, m_asStringList)
    {
        // assign key as value to possible missing language-string
        if(it->empty()) it->assign(PRINT(CORE_LANGUAGE_KEY "%s", m_asStringList.get_string(it)));

        // handle empty language-strings
        else if(it->front() == CORE_LANGUAGE_EMPTY[0]) it->clear();
    }

    // reduce memory consumption
    FOR_EACH(it, m_asStringList) it->shrink_to_fit();
    m_asStringList.shrink_to_fit();

    // update all foreign strings and objects
    FOR_EACH(it, m_asForeign)  (*m_asForeign.get_key(it))->assign(m_asStringList.at_bs(it->c_str()));
    FOR_EACH(it, m_anAssemble) (*it)(*m_anAssemble.get_key(it));
    FOR_EACH(it, m_apObject)   (*it)->__UpdateTranslate();

    Core::Log->Info("Language (%s, %u strings) loaded", pFile->GetPath(), m_asStringList.size());
    return CORE_OK;
}


// ****************************************************************
/* bind foreign string pointer */
void coreLanguage::BindForeign(coreString* psForeign, const coreHashString& sKey, coreAssembleFunc nFunc)
{
    ASSERT(psForeign && sKey)

    // assign key as value to possible new language-string
    if(!m_asStringList.count_bs(sKey)) m_asStringList.emplace_bs(sKey, PRINT(CORE_LANGUAGE_KEY "%s", sKey.GetString()));

    // save foreign string pointer and key
    m_asForeign.bs(psForeign).assign(sKey.GetString());

    // save possible assemble function
    if(nFunc) m_anAssemble.bs(psForeign) = std::move(nFunc);

    // initially update the foreign string
    this->__Assemble(psForeign, m_asStringList.at_bs(sKey));
}


// ****************************************************************
/* manually refresh foreign string pointers */
void coreLanguage::RefreshForeign(coreString* psForeign, const coreHashString& sKey)
{
    ASSERT(m_asForeign.count_bs(psForeign))

    // just update the foreign string
    this->__Assemble(psForeign, m_asStringList.at_bs(sKey));
}


// ****************************************************************
/* find and read string from language file directly */
coreBool coreLanguage::FindString(const coreChar* pcPath, const coreChar* pcKey, coreString* OUTPUT psOutput)
{
    ASSERT(psOutput)

    coreFileScope pFile = Core::Manager::Resource->RetrieveFile(pcPath);

    // get file data
    const coreChar* pcData = r_cast<const coreChar*>(pFile->GetData());
    if(!pcData) return false;

    // save key length (file data not null-terminated)
    const coreUintW iKeyLen = std::strlen(pcKey);

    // prepare range pointers (from, to) and end pointer (out of bound)
    const coreChar* pcFrom = pcData + 1u;   // # handle first key prefix
    const coreChar* pcTo   = pcFrom;
    const coreChar* pcEnd  = pcFrom + pFile->GetSize() - 1u;
    ASSERT(pFile->GetSize())

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
        psOutput->trim();
    }

    return bFound;
}


// ****************************************************************
/* get list with all available languages <name, path> */
void coreLanguage::GetAvailableLanguages(const coreChar* pcPath, const coreChar* pcFilter, coreMap<coreString, coreString>* OUTPUT pasOutput)
{
    ASSERT(pasOutput)

    // retrieve files from the folder
    coreList<coreString> asFile;
    Core::Manager::Resource->FolderScan(pcPath, pcFilter, &asFile);

    // reserve some memory
    pasOutput->reserve(asFile.size());

    FOR_EACH(it, asFile)
    {
        // find and read language-name
        coreString sName;
        if(!coreLanguage::FindString(it->c_str(), "LANGUAGE", &sName))
        {
            Core::Log->Warning("Language (%s) does not contain a valid LANGUAGE key", it->c_str());
            sName = coreData::StrFilename(it->c_str());
        }

        // save language-name with file-path
        pasOutput->emplace(std::move(sName), std::move(*it));
    }
}

void coreLanguage::GetAvailableLanguages(coreMap<coreString, coreString>* OUTPUT pasOutput)
{
    // get list with all available languages from the default location
    coreLanguage::GetAvailableLanguages("data/languages", "*.lng", pasOutput);

    // check for success (# something has to be available in this location)
    WARN_IF(pasOutput->empty()) Core::Log->Warning("No language files found (data/languages/*.lng)");
}


// ****************************************************************
/* update the foreign string */
void coreLanguage::__Assemble(coreString* psForeign, const coreString& sText)
{
    ASSERT(psForeign)

    // assign new text
    psForeign->assign(sText);

    // apply possible assemble function
    if(m_anAssemble.count_bs(psForeign))
    {
        m_anAssemble.at_bs(psForeign)(psForeign);
    }
}