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
// change associated language file
void coreTranslate::ChangeLanguage(coreLanguage* pLanguage)
{
    if(m_pLanguage)
    {
        // unbind everything from current language
        FOR_EACH(it, m_apsPointer) m_pLanguage->UnbindString(it->first);
        m_pLanguage->__UnbindObject(this);
    }

    // save language file object
    m_pLanguage = pLanguage;

    if(m_pLanguage)
    {
        // bind everything to new language
        m_pLanguage->__BindObject(this);
        FOR_EACH(it, m_apsPointer) m_pLanguage->BindString(it->first, it->second.c_str());

        // invoke object update
        this->__Update();
    }
}

    
// ****************************************************************
// bind own string pointer
void coreTranslate::_BindString(std::string* psString, const char* pcKey)
{
    SDL_assert(psString && pcKey);

    if(!m_pLanguage)
    {
        // bind object to default language
        m_pLanguage = Core::Language;
        m_pLanguage->__BindObject(this);
    }

    // bind string to language and save it internally
    m_pLanguage->BindString(psString, pcKey);
    m_apsPointer[psString] = pcKey;

    // invoke object update
    this->__Update();
}


// ****************************************************************
// unbind own string pointer
void coreTranslate::_UnbindString(std::string* psString)
{
    SDL_assert(psString);

    // remove string internally
    if(m_apsPointer.erase(psString))
    {
        // unbind from language
        if(m_pLanguage) m_pLanguage->UnbindString(psString);
    }
}


// ****************************************************************
// load language file
coreError coreLanguage::Load(const char* pcPath)
{
    coreFile* pFile = Core::Manager::Resource->RetrieveFile(pcPath);

    // get file data
    const char* pcData = r_cast<const char*>(pFile->GetData());
    if(!pcData) return CORE_ERROR_FILE;

    // prepare range pointers (from, to) and end pointer (out of bound) 
    const char* pcFrom = pcData+1;
    const char* pcTo   = pcFrom;
    const char* pcEnd  = pcFrom + pFile->GetSize();

    auto AssignLambda = [&](std::string* pString)
    {
        // assign string currently in range
        pString->assign(pcFrom, pcTo - pcFrom);
        coreData::StrTrim(pString);
        SDL_assert(!pString->empty());

        // begin next string
        pcFrom = pcTo+1;
    };

    std::string sKey;
    while(pcTo != pcEnd)
    {
        if(*pcTo == '=' && sKey.empty())
        {
            // extract key
            AssignLambda(&sKey);
            if(sKey.empty()) sKey.append(1, ' ');
        }
        else if((*pcTo == '$' || pcTo == pcEnd-1) && !sKey.empty())
        {
            // extract language-string
            AssignLambda(&m_asString[sKey.c_str()]);
            sKey.clear();
        }

        ++pcTo;
    }

    // save relative path and unload data
    m_sPath = pcPath;
    pFile->UnloadData();

    // reduce memory consumption
    FOR_EACH(it, m_asString) it->second.shrink_to_fit();
    m_asString.shrink_to_fit();

    // update all foreign strings and objects
    FOR_EACH(it, m_apsPointer) if(!it->second->empty()) *(it->first) = *(it->second);
    FOR_EACH(it, m_apObject) (*it)->__Update();

    Core::Log->Info("Language (%s) loaded", pFile->GetPath());
    return CORE_OK;
}


// ****************************************************************
// bind foreign string pointer
void coreLanguage::BindString(std::string* psString, const char* pcKey)
{
    SDL_assert(psString && pcKey);

    // retrieve language-string and save both pointers
    std::string& sString = m_asString[pcKey];
    m_apsPointer[psString] = &sString;

    // initially update foreign string
    *psString = sString.empty() ? pcKey : sString;
}