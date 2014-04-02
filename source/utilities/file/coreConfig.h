//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_CONFIG_H_
#define _CORE_GUARD_CONFIG_H_


// ****************************************************************
// configuration definitions
#define CORE_CONFIG_SYSTEM_WIDTH            "System",   "Width",          800
#define CORE_CONFIG_SYSTEM_HEIGHT           "System",   "Height",         600
#define CORE_CONFIG_SYSTEM_FULLSCREEN       "System",   "Fullscreen",     0
#define CORE_CONFIG_SYSTEM_LANGUAGE         "System",   "Language",       "data/languages/english.lng"
#define CORE_CONFIG_SYSTEM_LOG              "System",   "Log",            -1
#define CORE_CONFIG_GRAPHICS_ANTIALIASING   "Graphics", "AntiAliasing",   2
#define CORE_CONFIG_GRAPHICS_TEXTUREFILTER  "Graphics", "TextureFilter",  4
#define CORE_CONFIG_GRAPHICS_POSTPROCESSING "Graphics", "PostProcessing", 0
#define CORE_CONFIG_GRAPHICS_DEBUGCONTEXT   "Graphics", "DebugContext",   false
#define CORE_CONFIG_AUDIO_SOURCES           "Audio",    "Sources",        16
#define CORE_CONFIG_AUDIO_VOLUME_GLOBAL     "Audio",    "VolumeGlobal",   1.0f
#define CORE_CONFIG_AUDIO_VOLUME_SOUND      "Audio",    "VolumeSound",    1.0f
#define CORE_CONFIG_AUDIO_VOLUME_MUSIC      "Audio",    "VolumeMusic",    1.0f

#if defined(_CORE_ANDROID_)
    #define CORE_CONFIG_GRAPHICS_FORCEOPENGL "Graphics", "ForceOpenGL", 2.0f
    #define CORE_CONFIG_GRAPHICS_DUALCONTEXT "Graphics", "DualContext", false
#else
    #define CORE_CONFIG_GRAPHICS_FORCEOPENGL "Graphics", "ForceOpenGL", 0.0f
    #define CORE_CONFIG_GRAPHICS_DUALCONTEXT "Graphics", "DualContext", true
#endif


// ****************************************************************
// configuration file class
class coreConfig final
{
private:
    std::string m_sPath;    //!< absolute path of the file
    CSimpleIniA m_Config;   //!< configuration file interface


public:
    explicit coreConfig(const char* pcName)noexcept;
    ~coreConfig();

    //! load and save configuration
    //! @{
    inline void Load() {m_Config.LoadFile(m_sPath.c_str());}
    inline void Save() {m_Config.SaveFile(m_sPath.c_str());}
    //! @}

    //! set configuration values
    //! @{
    inline void SetBool(const char* pcSection, const char* pcKey, const bool& bReserved, const bool& bValue)     {m_Config.SetBoolValue(pcSection, pcKey, bValue);}
    inline void SetInt(const char* pcSection, const char* pcKey, const int& iReserved, const int& iValue)        {m_Config.SetLongValue(pcSection, pcKey, iValue);}
    inline void SetFloat(const char* pcSection, const char* pcKey, const float& fReserved, const float& fValue)  {m_Config.SetDoubleValue(pcSection, pcKey, fValue);}
    inline void SetString(const char* pcSection, const char* pcKey, const char* pcReserved, const char* pcValue) {m_Config.SetValue(pcSection, pcKey, pcValue);}
    //! @}

    //! get configuration values
    //! @{
    bool GetBool(const char* pcSection, const char* pcKey, const bool& bDefault);
    int GetInt(const char* pcSection, const char* pcKey, const int& iDefault);
    float GetFloat(const char* pcSection, const char* pcKey, const float& fDefault);
    const char* GetString(const char* pcSection, const char* pcKey, const char* pcDefault);
    //! @}


private:
    DISABLE_COPY(coreConfig)
};


#endif // _CORE_GUARD_CONFIG_H_