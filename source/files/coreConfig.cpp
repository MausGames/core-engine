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
coreConfig::coreConfig(const char* pcName)
{
    // save absolute path
    m_sPath = coreUtils::AppPath();
    m_sPath.append(pcName);

    // load configuration
    m_Config.SetUnicode();
    this->Load();
}


// ****************************************************************
// destructor
coreConfig::~coreConfig()
{
    // save configuration
    this->Save();
}


// ****************************************************************
// access bool value
bool coreConfig::GetBool(const char* pcSection, const char* pcKey, const bool& bDefault)
{
    const bool bReturn = m_Config.GetBoolValue(pcSection, pcKey, bDefault);
    if(bReturn == bDefault) this->SetBool(pcSection, pcKey, false, bDefault);

    return bReturn;
}


// ****************************************************************
// access int value
int coreConfig::GetInt(const char* pcSection, const char* pcKey, const int& iDefault)
{
    const int iReturn = (int)m_Config.GetLongValue(pcSection, pcKey, iDefault);
    if(iReturn == iDefault) this->SetInt(pcSection, pcKey, 0, iDefault);

    return iReturn;
}


// ****************************************************************
// access float value
float coreConfig::GetFloat(const char* pcSection, const char* pcKey, const float& fDefault)
{
    const float fReturn = (float)m_Config.GetDoubleValue(pcSection, pcKey, fDefault);
    if(fReturn == fDefault) this->SetFloat(pcSection, pcKey, 0.0f, fDefault);

    return fReturn;
}