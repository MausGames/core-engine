///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_CONFIG_H_
#define _CORE_GUARD_CONFIG_H_

// TODO: don't set an already defined value or default value (use map ?)
// TODO: check out templated Get and Set
// TODO: remove simpleIni, need more specialized functionality
// TODO: don't do two lookups when getting a value


// ****************************************************************
/* configuration definitions */
#define CORE_CONFIG_BASE_LANGUAGE               "Base",     "Language",           ("data/languages/english.lng")
#define CORE_CONFIG_BASE_DEBUGMODE              "Base",     "DebugMode",          (false)                    //!< enable debug capabilities and extended logging
#define CORE_CONFIG_BASE_ASYNCMODE              "Base",     "AsyncMode",          (DEFINED(_CORE_ASYNC_))    //!< enable asynchronous processing and resource loading
#define CORE_CONFIG_BASE_FALLBACKMODE           "Base",     "FallbackMode",       (false)                    //!< disable all possible hardware features
#define CORE_CONFIG_BASE_PERSISTMODE            "Base",     "PersistMode",        (DEFINED(_CORE_MOBILE_))   //!< keep resources in memory

#define CORE_CONFIG_SYSTEM_DISPLAY              "System",   "Display",            (0)
#define CORE_CONFIG_SYSTEM_WIDTH                "System",   "Width",              (800)
#define CORE_CONFIG_SYSTEM_HEIGHT               "System",   "Height",             (600)
#define CORE_CONFIG_SYSTEM_FULLSCREEN           "System",   "Fullscreen",         (0)
#define CORE_CONFIG_SYSTEM_VSYNC                "System",   "Vsync",              (true)

#define CORE_CONFIG_GRAPHICS_QUALITY            "Graphics", "Quality",            (0)
#define CORE_CONFIG_GRAPHICS_ANTIALIASING       "Graphics", "AntiAliasing",       (2)
#define CORE_CONFIG_GRAPHICS_TEXTUREANISOTROPY  "Graphics", "TextureAnisotropy",  (4)
#define CORE_CONFIG_GRAPHICS_TEXTURETRILINEAR   "Graphics", "TextureTrilinear",   (true)
#define CORE_CONFIG_GRAPHICS_TEXTURECOMPRESSION "Graphics", "TextureCompression", (true)
#define CORE_CONFIG_GRAPHICS_ENABLEEXTENSIONS   "Graphics", "EnableExtensions",   ("")
#define CORE_CONFIG_GRAPHICS_DISABLEEXTENSIONS  "Graphics", "DisableExtensions",  ("")

#define CORE_CONFIG_AUDIO_SOURCES               "Audio",    "Sources",            (16)
#define CORE_CONFIG_AUDIO_GLOBALVOLUME          "Audio",    "GlobalVolume",       (1.0f)
#define CORE_CONFIG_AUDIO_SOUNDVOLUME           "Audio",    "SoundVolume",        (1.0f)
#define CORE_CONFIG_AUDIO_MUSICVOLUME           "Audio",    "MusicVolume",        (1.0f)

#define CORE_CONFIG_INPUT_HAPTIC                "Input",    "Haptic",             (true)
#define CORE_CONFIG_INPUT_JOYSTICKDEAD          "Input",    "JoystickDead",       (0x2000)
#define CORE_CONFIG_INPUT_JOYSTICKMAX           "Input",    "JoystickMax",        (0x7000)

extern template CSimpleIniA;   //!< do not create template in every compilation unit


// ****************************************************************
/* configuration file class */
class coreConfig final
{
private:
    std::string m_sPath;    //!< relative path of the file
    CSimpleIniA m_Config;   //!< configuration file interface

    coreBool m_bDirty;      //!< status flag for pending changes


public:
    explicit coreConfig(const coreChar* pcPath)noexcept;
    ~coreConfig();

    DISABLE_COPY(coreConfig)

    /*! load and save configuration file */
    //! @{
    coreStatus Load(const coreBool bSaveDirty);
    coreStatus Save();
    //! @}

    /*! set configuration values */
    //! @{
    inline void SetBool  (const coreChar* pcSection, const coreChar* pcKey, const coreBool,  const coreBool  bValue)  {if(m_Config.SetBoolValue  (pcSection, pcKey, bValue)  != SI_SAME) m_bDirty = true;}
    inline void SetBool  (const coreChar* pcSection, const coreChar* pcKey,                  const coreBool  bValue)  {if(m_Config.SetBoolValue  (pcSection, pcKey, bValue)  != SI_SAME) m_bDirty = true;}
    inline void SetInt   (const coreChar* pcSection, const coreChar* pcKey, const coreInt32, const coreInt32 iValue)  {if(m_Config.SetLongValue  (pcSection, pcKey, iValue)  != SI_SAME) m_bDirty = true;}
    inline void SetInt   (const coreChar* pcSection, const coreChar* pcKey,                  const coreInt32 iValue)  {if(m_Config.SetLongValue  (pcSection, pcKey, iValue)  != SI_SAME) m_bDirty = true;}
    inline void SetFloat (const coreChar* pcSection, const coreChar* pcKey, const coreFloat, const coreFloat fValue)  {if(m_Config.SetDoubleValue(pcSection, pcKey, fValue)  != SI_SAME) m_bDirty = true;}
    inline void SetFloat (const coreChar* pcSection, const coreChar* pcKey,                  const coreFloat fValue)  {if(m_Config.SetDoubleValue(pcSection, pcKey, fValue)  != SI_SAME) m_bDirty = true;}
    inline void SetString(const coreChar* pcSection, const coreChar* pcKey, const coreChar*, const coreChar* pcValue) {if(m_Config.SetValue      (pcSection, pcKey, pcValue) != SI_SAME) m_bDirty = true;}
    inline void SetString(const coreChar* pcSection, const coreChar* pcKey,                  const coreChar* pcValue) {if(m_Config.SetValue      (pcSection, pcKey, pcValue) != SI_SAME) m_bDirty = true;}
    //! @}

    /*! get configuration values */
    //! @{
    inline coreBool        GetBool  (const coreChar* pcSection, const coreChar* pcKey, const coreBool  bDefault)  {if(!m_Config.GetValue(pcSection, pcKey, NULL)) this->SetBool  (pcSection, pcKey, bDefault);  return m_Config.GetBoolValue  (pcSection, pcKey, bDefault); }
    inline coreInt32       GetInt   (const coreChar* pcSection, const coreChar* pcKey, const coreInt32 iDefault)  {if(!m_Config.GetValue(pcSection, pcKey, NULL)) this->SetInt   (pcSection, pcKey, iDefault);  return m_Config.GetLongValue  (pcSection, pcKey, iDefault); }
    inline coreFloat       GetFloat (const coreChar* pcSection, const coreChar* pcKey, const coreFloat fDefault)  {if(!m_Config.GetValue(pcSection, pcKey, NULL)) this->SetFloat (pcSection, pcKey, fDefault);  return m_Config.GetDoubleValue(pcSection, pcKey, fDefault); }
    inline const coreChar* GetString(const coreChar* pcSection, const coreChar* pcKey, const coreChar* pcDefault) {if(!m_Config.GetValue(pcSection, pcKey, NULL)) this->SetString(pcSection, pcKey, pcDefault); return m_Config.GetValue      (pcSection, pcKey, pcDefault);}
    //! @}

    /*! get object properties */
    //! @{
    inline const coreChar* GetPath()const {return m_sPath.c_str();}
    //! @}
};


#endif /* _CORE_GUARD_CONFIG_H_ */