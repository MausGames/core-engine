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
coreConfig::coreConfig(const coreChar* pcPath)noexcept
: m_sPath  (pcPath)
, m_Config ()
{
    // define interface behavior
    m_Config.SetUnicode  (true);
    m_Config.SetMultiKey (false);
    m_Config.SetMultiLine(false);
    m_Config.SetSpaces   (true);

    // load configuration file
    this->Load();

    // write all loaded configuration values to the log file
    Core::Log->ListStartInfo("Configuration Values");
    {
        // retrieve all sections
        CSimpleIni::TNamesDepend apSection;
        m_Config.GetAllSections(apSection);

        FOR_EACH(pSection, apSection)
        {
            // retrieve all keys per section
            CSimpleIni::TNamesDepend apKey;
            m_Config.GetAllKeys(pSection->pItem, apKey);

            FOR_EACH(pKey, apKey)
            {
                // write specific configuration value
                Core::Log->ListAdd(CORE_LOG_BOLD("%s") ".%s: %s", pSection->pItem, pKey->pItem, m_Config.GetValue(pSection->pItem, pKey->pItem));
            }
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
    // load configuration file
    if(m_Config.LoadFile(m_sPath.c_str()) < 0)
    {
        Core::Log->Warning("Configuration (%s) could not be loaded", m_sPath.c_str());
        return CORE_ERROR_FILE;
    }

    Core::Log->Info("Configuration (%s) loaded", m_sPath.c_str());
    return CORE_OK;
}


// ****************************************************************
/* save configuration file */
coreStatus coreConfig::Save()const
{
    // save configuration file
    if(m_Config.SaveFile(m_sPath.c_str()) < 0)
    {
        Core::Log->Warning("Configuration (%s) could not be saved", m_sPath.c_str());
        return CORE_ERROR_FILE;
    }

    Core::Log->Info("Configuration (%s) saved", m_sPath.c_str());
    return CORE_OK;
}