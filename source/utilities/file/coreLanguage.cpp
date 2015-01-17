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
/* constructor */
coreTranslate::coreTranslate()noexcept
: m_pLanguage (NULL)
{
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
/* change associated language file */
void coreTranslate::ChangeLanguage(coreLanguage* pLanguage)
{
    if(m_pLanguage == pLanguage) return;

    if(m_pLanguage)
    {
        // unbind everything from current language
        FOR_EACH(it, m_apsPointer) m_pLanguage->UnbindForeign(it->first);
        m_pLanguage->__UnbindObject(this);
    }

    // set new language file
    m_pLanguage = pLanguage;

    if(m_pLanguage)
    {
        // bind everything to new language
        m_pLanguage->__BindObject(this);
        FOR_EACH(it, m_apsPointer) m_pLanguage->BindForeign(it->first, it->second.c_str());

        // invoke object update
        this->__Update();
    }
}


// ****************************************************************
/* bind own string pointer */
void coreTranslate::_BindString(std::string* psString, const char* pcKey)
{
    ASSERT(psString && pcKey)

    if(!m_pLanguage)
    {
        // associate object with default language
        this->ChangeLanguage(Core::Language);
    }

    // bind string to language and save it internally
    m_pLanguage->BindForeign(psString, pcKey);
    m_apsPointer[psString] = pcKey;

    // invoke object update
    this->__Update();
}


// ****************************************************************
/* unbind own string pointer */
void coreTranslate::_UnbindString(std::string* psString)
{
    ASSERT(psString)

    // remove string internally
    if(m_apsPointer.erase(psString))
    {
        // unbind from language
        if(m_pLanguage) m_pLanguage->UnbindForeign(psString);
    }
}


// ****************************************************************
/* constructor */
coreLanguage::coreLanguage(const char* pcPath)noexcept
: m_sPath ("")
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
coreError coreLanguage::Load(const char* pcPath)
{
    coreFile* pFile = Core::Manager::Resource->RetrieveFile(pcPath);

    // get file data
    const char* pcData = r_cast<const char*>(pFile->GetData());
    if(!pcData) return CORE_ERROR_FILE;

    // prepare range pointers (from, to) and end pointer (out of bound)
    const char* pcFrom = pcData + 1;
    const char* pcTo   = pcFrom;
    const char* pcEnd  = pcFrom + pFile->GetSize() - 1;

    auto nAssignFunc = [&pcFrom, &pcTo](std::string* pString)
    {
        // assign string currently in range
        pString->assign(pcFrom, pcTo - pcFrom);
        coreData::StrTrim(pString);

        // begin next string
        pcFrom = pcTo + 1;
    };

    // clear all existing language strings
    FOR_EACH(it, m_asStringList) it->second.clear();

    std::string sKey = "";
    while(pcTo != pcEnd)
    {
        if(*pcTo == CORE_LANGUAGE_ASSIGN[0] && sKey.empty())
        {
            // extract key
            nAssignFunc(&sKey);
            WARN_IF(sKey.empty()) sKey.assign(1, ' ');
        }
        else if(*pcTo == CORE_LANGUAGE_KEY[0] && !sKey.empty())
        {
            // extract language-string
            nAssignFunc(&m_asStringList[sKey.c_str()]);
            sKey.clear();
        }

        ++pcTo;
    }
    if(!sKey.empty()) nAssignFunc(&m_asStringList[sKey.c_str()]);

    // save relative path and unload data
    m_sPath = pcPath;
    pFile->UnloadData();

    // reduce memory consumption
    FOR_EACH(it, m_asStringList)
    {
        std::string& sString = it->second;

        // assign key to empty language strings
        if(sString.empty()) sString = it->first;
        sString.shrink_to_fit();
    }
    m_asStringList.shrink_to_fit();

    // update all foreign strings and objects
    FOR_EACH(it, m_apsForeign) (*it->first) = (*it->second);
    FOR_EACH(it, m_apObject)   (*it)->__Update();

    Core::Log->Info("Language (%s) loaded", pFile->GetPath());
    return CORE_OK;
}


// ****************************************************************
/* bind foreign string pointer */
void coreLanguage::BindForeign(std::string* psForeign, const char* pcKey)
{
    ASSERT(psForeign && pcKey)

    // assign key as value to new language strings
    if(!m_asStringList.count(pcKey)) m_asStringList[pcKey].assign(pcKey);

    // retrieve language-string and save both pointers
    std::string& sString    = m_asStringList[pcKey];
    m_apsForeign[psForeign] = &sString;

    // initially update foreign string
    *psForeign = sString;
}