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
/* configuration definitions */
#define CORE_CONFIG_SYSTEM_WIDTH           "System",   "Width",         (800)
#define CORE_CONFIG_SYSTEM_HEIGHT          "System",   "Height",        (600)
#define CORE_CONFIG_SYSTEM_FULLSCREEN      "System",   "Fullscreen",    (0)
#define CORE_CONFIG_SYSTEM_LANGUAGE        "System",   "Language",      ("data/languages/english.lng")
#define CORE_CONFIG_SYSTEM_DEBUG           "System",   "Debug",         (false)
                                                                        
#define CORE_CONFIG_GRAPHICS_QUALITY       "Graphics", "Quality",       (0)
#define CORE_CONFIG_GRAPHICS_ANTIALIASING  "Graphics", "AntiAliasing",  (2)
#define CORE_CONFIG_GRAPHICS_TEXTUREFILTER "Graphics", "TextureFilter", (4)
#define CORE_CONFIG_GRAPHICS_DUALCONTEXT   "Graphics", "DualContext",   (false)
#define CORE_CONFIG_GRAPHICS_FORCEOPENGL   "Graphics", "ForceOpenGL",   (g_bCoreGLES ? 2.0f : 0.0f)
                                                                          
#define CORE_CONFIG_AUDIO_SOURCES          "Audio",    "Sources",       (16)
#define CORE_CONFIG_AUDIO_GLOBALVOLUME     "Audio",    "GlobalVolume",  (1.0f)
#define CORE_CONFIG_AUDIO_SOUNDVOLUME      "Audio",    "SoundVolume",   (1.0f)
#define CORE_CONFIG_AUDIO_MUSICVOLUME      "Audio",    "MusicVolume",   (1.0f)


// ****************************************************************
/* configuration file class */
class coreConfig final
{
private:
    std::string m_sPath;   //!< relative path of the file
    CSimpleIni m_Config;   //!< configuration file interface


public:
    explicit coreConfig(const char* pcPath)noexcept;
    ~coreConfig();

    /*! load and save configuration file */
    //! @{
    inline void Load()      {m_Config.LoadFile(m_sPath.c_str());}
    inline void Save()const {m_Config.SaveFile(m_sPath.c_str());}
    //! @}

    /*! set configuration values */
    //! @{
    inline void SetBool  (const char* pcSection, const char* pcKey, const bool&  bReserved,  const bool&  bValue)  {m_Config.SetBoolValue  (pcSection, pcKey, bValue);}
    inline void SetInt   (const char* pcSection, const char* pcKey, const int&   iReserved,  const int&   iValue)  {m_Config.SetLongValue  (pcSection, pcKey, iValue);}
    inline void SetFloat (const char* pcSection, const char* pcKey, const float& fReserved,  const float& fValue)  {m_Config.SetDoubleValue(pcSection, pcKey, fValue);}
    inline void SetString(const char* pcSection, const char* pcKey, const char*  pcReserved, const char*  pcValue) {m_Config.SetValue      (pcSection, pcKey, pcValue);}
    //! @}

    /*! get configuration values */
    //! @{
    bool        GetBool  (const char* pcSection, const char* pcKey, const bool&  bDefault);
    int         GetInt   (const char* pcSection, const char* pcKey, const int&   iDefault);
    float       GetFloat (const char* pcSection, const char* pcKey, const float& fDefault);
    const char* GetString(const char* pcSection, const char* pcKey, const char*  pcDefault);
    //! @}


private:
    DISABLE_COPY(coreConfig)
};


#endif /* _CORE_GUARD_CONFIG_H_ */