//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_CONFIG_H_
#define _CORE_GUARD_CONFIG_H_


// ****************************************************************
// configuration definitions
#define CORE_CONFIG_SYSTEM_WIDTH            "System",   "Width"
#define CORE_CONFIG_SYSTEM_HEIGHT           "System",   "Height"
#define CORE_CONFIG_SYSTEM_FULLSCREEN       "System",   "Fullscreen"
#define CORE_CONFIG_SYSTEM_LOG              "System",   "Log"
#define CORE_CONFIG_GRAPHICS_FOV            "Graphics", "FOV"
#define CORE_CONFIG_GRAPHICS_CLIP_NEAR      "Graphics", "ClipNear"
#define CORE_CONFIG_GRAPHICS_CLIP_FAR       "Graphics", "ClipFar"
#define CORE_CONFIG_GRAPHICS_MULTISAMPLING  "Graphics", "AntiAliasing"
#define CORE_CONFIG_GRAPHICS_TEXTUREFILTER  "Graphics", "TextureFilter"
#define CORE_CONFIG_GRAPHICS_POSTPROCESSING "Graphics", "PostProcessing"
#define CORE_CONFIG_GRAPHICS_FORCEOPENGL    "Graphics", "ForceOpenGL"
#define CORE_CONFIG_AUDIO_VOLUME_GLOBAL     "Audio",    "VolumeGlobal"
#define CORE_CONFIG_AUDIO_VOLUME_SOUND      "Audio",    "VolumeSound"
#define CORE_CONFIG_AUDIO_VOLUME_MUSIC      "Audio",    "VolumeMusic"
#define CORE_CONFIG_AUDIO_SOURCES           "Audio",    "Sources"


// ****************************************************************
// configuration file class
class coreConfig final
{
private:
    std::string m_sPath;    //!< absolute path of the file
    CSimpleIniA m_Config;   //!< config file interface


public:
    coreConfig(const char* pcName);
    ~coreConfig();

    //! load and save configuration
    //! @{
    inline void Load() {m_Config.LoadFile(m_sPath.c_str());}
    inline void Save() {m_Config.SaveFile(m_sPath.c_str());}
    //! @}

    //! set configuration values
    //! @{
    inline void SetBool(const char* pcSection, const char* pcKey, const bool& bValue)   {m_Config.SetBoolValue(pcSection, pcKey, bValue);}
    inline void SetInt(const char* pcSection, const char* pcKey, const int& iValue)     {m_Config.SetLongValue(pcSection, pcKey, iValue);}
    inline void SetFloat(const char* pcSection, const char* pcKey, const float& fValue) {m_Config.SetDoubleValue(pcSection, pcKey, fValue);}
    //! @}

    //! get configuration values
    //! @{
    bool GetBool(const char* pcSection, const char* pcKey, const bool& bDefault);
    int GetInt(const char* pcSection, const char* pcKey, const int& iDefault);
    float GetFloat(const char* pcSection, const char* pcKey, const float& fDefault);
    //! @}

    //! direct access configuration file interface
    //! @{
    inline CSimpleIniA* Get() {return &m_Config;}
    //! @}


private:
    //! disable copy
    //! @{
    coreConfig(const coreConfig& c) deletefunc;
    coreConfig& operator = (const coreConfig& c) deletefunc;
    //! @}
};


#endif // _CORE_GUARD_CONFIG_H_