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
// constructor
coreConfig::coreConfig(const char* pcPath)noexcept
: m_sPath (pcPath)
{
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


// ****************************************************************
// access string value
const char* coreConfig::GetString(const char* pcSection, const char* pcKey, const char* pcDefault)
{
    const char* pcReturn = m_Config.GetValue(pcSection, pcKey, pcDefault);
    if(!std::strcmp(pcReturn, pcDefault)) this->SetString(pcSection, pcKey, NULL, pcDefault);

    return pcReturn;
}