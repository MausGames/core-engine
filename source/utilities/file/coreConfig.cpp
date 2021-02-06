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
coreConfig::coreConfig(const coreChar* pcPath)noexcept
: m_aasSection {}
, m_sPath      (pcPath)
, m_bDirty     (false)
, m_Lock       ()
{
    // load configuration file
    this->Load();

    // write everything to log file
    Core::Log->ListStartInfo("Configuration Values");
    {
        // loop through all configuration sections
        FOR_EACH(it, m_aasSection)
        {
            Core::Log->ListDeeper(CORE_LOG_BOLD("%s"), m_aasSection.get_string(it));
            {
                // loop through all configuration entries
                FOR_EACH(et, *it)
                {
                    Core::Log->ListAdd("%s: %s", it->get_string(et), et->c_str());
                }
            }
            Core::Log->ListEnd();
        }
    }
    Core::Log->ListEnd();
}


// ****************************************************************
/* destructor */
coreConfig::~coreConfig()
{
    // save configuration file
    this->Save();
}


// ****************************************************************
/* load configuration file */
coreStatus coreConfig::Load()
{
    coreSpinLocker oLocker(&m_Lock);

    // load configuration file
    coreFile oFile(m_sPath.c_str());

    // get file data
    const coreChar* pcData = r_cast<const coreChar*>(oFile.GetData());
    if(!pcData)
    {
        Core::Log->Warning("Configuration (%s) could not be loaded", m_sPath.c_str());
        return CORE_ERROR_FILE;
    }

    // prepare range pointers (from, to) and end pointer (out of bound)
    const coreChar* pcFrom = pcData;
    const coreChar* pcTo   = pcData;
    const coreChar* pcEnd  = pcData + oFile.GetSize();

    const auto nAssignFunc = [&](std::string* OUTPUT pString)
    {
        ASSERT(pString->empty())

        // assign string currently in range
        pString->assign(pcFrom, pcTo - pcFrom);
        coreData::StrTrim(pString);

        // begin next string
        pcFrom = pcTo + 1u;
    };

    // clear all existing configuration sections
    m_aasSection.clear();

    std::string sSection = "";
    std::string sKey     = "";
    while(pcTo != pcEnd)
    {
        if((*pcTo) == '[' || (*pcTo) == ']')
        {
            // extract section
            sSection.clear();
            nAssignFunc(&sSection);
        }
        else if((*pcTo) == '=')
        {
            // extract key
            nAssignFunc(&sKey);
        }
        else if((*pcTo) == '\n')
        {
            if(!sKey.empty())
            {
                // extract value
                nAssignFunc(&m_aasSection[sSection.c_str()][sKey.c_str()]);
                sKey.clear();
            }
            else
            {
                // reset line (to improve stability)
                pcFrom = pcTo + 1u;
            }
        }

        ++pcTo;
    }
    if(!sKey.empty()) nAssignFunc(&m_aasSection[sSection.c_str()][sKey.c_str()]);

    // reduce memory consumption
    FOR_EACH(it, m_aasSection)
    {
        FOR_EACH(et, *it) et->shrink_to_fit();
        it->shrink_to_fit();
    }
    m_aasSection.shrink_to_fit();

    // clear status
    m_bDirty = false;

    Core::Log->Info("Configuration (%s) loaded", m_sPath.c_str());
    return CORE_OK;
}


// ****************************************************************
/* save configuration file */
coreStatus coreConfig::Save()
{
    // check for pending changes
    if(!m_bDirty) return CORE_BUSY;

    coreSpinLocker oLocker(&m_Lock);

    // prepare target buffer
    std::string sBuffer;
    sBuffer.reserve(0x1000u);

    FOR_EACH(it, m_aasSection)
    {
        // write configuration section
        if(it != m_aasSection.begin()) sBuffer.append("\n\n\n");
        sBuffer.append(PRINT("[%s]", m_aasSection.get_string(it)));

        // write configuration entries
        FOR_EACH(et, *it) sBuffer.append(PRINT("\n%s = %s", it->get_string(et), et->c_str()));
    }

    // create file data
    coreByte* pData = new coreByte[sBuffer.length()];
    std::memcpy(pData, sBuffer.c_str(), sBuffer.length());

    // save configuration file
    coreFile oFile(m_sPath.c_str(), pData, sBuffer.length());
    if(oFile.Save())
    {
        Core::Log->Warning("Configuration (%s) could not be saved", m_sPath.c_str());
        return CORE_ERROR_FILE;
    }

    // clear status
    m_bDirty = false;

    Core::Log->Info("Configuration (%s) saved", m_sPath.c_str());
    return CORE_OK;
}


// ****************************************************************
/* retrieve configuration entry */
coreBool coreConfig::__RetrieveEntry(const coreHashString& sSection, const coreHashString& sKey, std::string** OUTPUT ppsEntry)
{
    // check for existence
    const coreBool bExists = (m_aasSection.count(sSection) && m_aasSection.at(sSection).count(sKey));

    // create and return configuration entry
    if(!bExists) m_aasSection[sSection].emplace(sKey);
    (*ppsEntry) = &m_aasSection.at(sSection).at(sKey);

    return bExists;
}