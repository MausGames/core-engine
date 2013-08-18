#pragma once


// ****************************************************************
// main configuration definitions
#define CORE_CONFIG_SYSTEM_WIDTH           "System",  "Width"
#define CORE_CONFIG_SYSTEM_HEIGHT          "System",  "Height"
#define CORE_CONFIG_SYSTEM_FULLSCREEN      "System",  "Fullscreen"
#define CORE_CONFIG_SYSTEM_LOG             "System",  "Log"
#define CORE_CONFIG_GRAPHIC_FOV            "Graphic", "FOV"
#define CORE_CONFIG_GRAPHIC_CLIP_NEAR      "Graphic", "ClipNear"
#define CORE_CONFIG_GRAPHIC_CLIP_FAR       "Graphic", "ClipFar"
#define CORE_CONFIG_GRAPHIC_MULTISAMPLING  "Graphic", "AntiAliasing"
#define CORE_CONFIG_GRAPHIC_TEXTUREFILTER  "Graphic", "TextureFilter"
#define CORE_CONFIG_GRAPHIC_POSTPROCESSING "Graphic", "PostProcessing"
#define CORE_CONFIG_GRAPHIC_FORCEOPENGL    "Graphic", "ForceOpenGL"
#define CORE_CONFIG_SOUND_VOLUME_GLOBAL    "Sound",   "VolumeGlobal"
#define CORE_CONFIG_SOUND_VOLUME_SOUND     "Sound",   "VolumeSound"
#define CORE_CONFIG_SOUND_VOLUME_MUSIC     "Sound",   "VolumeMusic"
#define CORE_CONFIG_SOUND_CHANNELS         "Sound",   "Channels"


// ****************************************************************
// configuration file class
class coreConfig
{
private:
    std::string m_sPath;    // absolute path of the file
    CSimpleIniA m_Config;   // config file interface


public:
    coreConfig(const char* pcName);
    ~coreConfig();

    // load and save configuration
    inline void Load() {m_Config.LoadFile(m_sPath.c_str());}
    inline void Save() {m_Config.SaveFile(m_sPath.c_str());}

    // set configuration values
    inline void SetBool(const char* pcSection, const char* pcKey, const bool& bValue)   {m_Config.SetBoolValue(pcSection, pcKey, bValue);}
    inline void SetInt(const char* pcSection, const char* pcKey, const int& iValue)     {m_Config.SetLongValue(pcSection, pcKey, iValue);}
    inline void SetFloat(const char* pcSection, const char* pcKey, const float& fValue) {m_Config.SetDoubleValue(pcSection, pcKey, fValue);}

    // get configuration values
    bool GetBool(const char* pcSection, const char* pcKey, const bool& bDefault);
    int GetInt(const char* pcSection, const char* pcKey, const int& iDefault);
    float GetFloat(const char* pcSection, const char* pcKey, const float& fDefault);

    // direct access configuration file interface
    inline CSimpleIniA* Get() {return &m_Config;}
};