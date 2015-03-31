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

coreTranslate::coreTranslate(const coreTranslate& c)noexcept
: m_pLanguage  (NULL)
, m_apsPointer (c.m_apsPointer)
{
    // associate object with language
    this->ChangeLanguage(c.m_pLanguage);
}

coreTranslate::coreTranslate(coreTranslate&& m)noexcept
: m_pLanguage  (NULL)
, m_apsPointer (std::move(m.m_apsPointer))
{
    // associate object with language
    this->ChangeLanguage(m.m_pLanguage);
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

    // copy remaining properties
    m_apsPointer = c.m_apsPointer;

    return *this;
}

coreTranslate& coreTranslate::operator = (coreTranslate&& m)noexcept
{
    // associate object with language
    this->ChangeLanguage(m.m_pLanguage);

    // move remaining properties
    m_apsPointer = std::move(m.m_apsPointer);

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
void coreTranslate::_BindString(std::string* psString, const coreChar* pcKey)
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
coreLanguage::coreLanguage(const coreChar* pcPath)noexcept
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
coreStatus coreLanguage::Load(const coreChar* pcPath)
{
    coreFile* pFile = Core::Manager::Resource->RetrieveFile(pcPath);

    // get file data
    const coreChar* pcData = r_cast<const coreChar*>(pFile->GetData());
    if(!pcData) return CORE_ERROR_FILE;

    // prepare range pointers (from, to) and end pointer (out of bound)
    const coreChar* pcFrom = pcData + 1u;
    const coreChar* pcTo   = pcFrom;
    const coreChar* pcEnd  = pcFrom + pFile->GetSize() - 1u;

    auto nAssignFunc = [&pcFrom, &pcTo](std::string* pString)
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
        if(*pcTo == CORE_LANGUAGE_ASSIGN[0] && sKey.empty())
        {
            // extract key
            nAssignFunc(&sKey);
            WARN_IF(sKey.empty()) sKey.assign(1u, ' ');
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
        std::string& sString = (*it);

        // assign key as value to possible empty language-string
        if(sString.empty()) sString.assign(*m_asStringList.get_key(it));
        sString.shrink_to_fit();
    }
    m_asStringList.shrink_to_fit();

    // update all foreign strings and objects
    FOR_EACH(it, m_apsForeign) (*m_apsForeign.get_key(it))->assign(m_asStringList[it->c_str()]);
    FOR_EACH(it, m_apObject)   (*it)->__Update();

    Core::Log->Info("Language (%s) loaded", pFile->GetPath());
    return CORE_OK;
}


// ****************************************************************
/* bind foreign string pointer */
void coreLanguage::BindForeign(std::string* psForeign, const coreChar* pcKey)
{
    ASSERT(psForeign && pcKey)

    // assign key as value to possible new language-string
    if(!m_asStringList.count(pcKey)) m_asStringList[pcKey].assign(pcKey);

    // save foreign string pointer and key
    m_apsForeign[psForeign].assign(pcKey);

    // initially update the foreign string
    psForeign->assign(m_asStringList[pcKey]);
}