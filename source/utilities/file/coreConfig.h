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

// TODO: don't set an already defined value or default value (use map ?)
// TODO: check out templated Get and Set
// TODO: remove simpleIni, need more specialized functionality
// TODO: improve dirty flag: don't set if value was the same
// TODO: don't do two lookups when getting a value


// ****************************************************************
/* configuration definitions */
#define CORE_CONFIG_SYSTEM_DISPLAY              "System",   "Display",            (0)
#define CORE_CONFIG_SYSTEM_WIDTH                "System",   "Width",              (800)
#define CORE_CONFIG_SYSTEM_HEIGHT               "System",   "Height",             (600)
#define CORE_CONFIG_SYSTEM_FULLSCREEN           "System",   "Fullscreen",         (0)
#define CORE_CONFIG_SYSTEM_VSYNC                "System",   "Vsync",              (true)
#define CORE_CONFIG_SYSTEM_LANGUAGE             "System",   "Language",           ("data/languages/english.lng")
#define CORE_CONFIG_SYSTEM_DEBUGMODE            "System",   "DebugMode",          (false)

#define CORE_CONFIG_GRAPHICS_QUALITY            "Graphics", "Quality",            (0)
#define CORE_CONFIG_GRAPHICS_ANTIALIASING       "Graphics", "AntiAliasing",       (2)
#define CORE_CONFIG_GRAPHICS_TEXTUREFILTER      "Graphics", "TextureFilter",      (4)
#define CORE_CONFIG_GRAPHICS_TEXTURECOMPRESSION "Graphics", "TextureCompression", (true)
#define CORE_CONFIG_GRAPHICS_DEPTHSIZE          "Graphics", "DepthSize",          (24)
#define CORE_CONFIG_GRAPHICS_STENCILSIZE        "Graphics", "StencilSize",        (8)
#define CORE_CONFIG_GRAPHICS_ALPHACHANNEL       "Graphics", "AlphaChannel",       (true)
#define CORE_CONFIG_GRAPHICS_DOUBLEBUFFER       "Graphics", "DoubleBuffer",       (true)
#define CORE_CONFIG_GRAPHICS_RESOURCECONTEXT    "Graphics", "ResourceContext",    (DEFINED(_CORE_PARALLEL_))
#define CORE_CONFIG_GRAPHICS_FALLBACKMODE       "Graphics", "FallbackMode",       (false)

#define CORE_CONFIG_AUDIO_SOURCES               "Audio",    "Sources",            (16)
#define CORE_CONFIG_AUDIO_GLOBALVOLUME          "Audio",    "GlobalVolume",       (1.0f)
#define CORE_CONFIG_AUDIO_SOUNDVOLUME           "Audio",    "SoundVolume",        (1.0f)
#define CORE_CONFIG_AUDIO_MUSICVOLUME           "Audio",    "MusicVolume",        (1.0f)

#define CORE_CONFIG_INPUT_HAPTIC                "Input",    "Haptic",             (true)


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
    coreStatus Load();
    coreStatus Save();
    //! @}

    /*! set configuration values */
    //! @{
    inline void SetBool  (const coreChar* pcSection, const coreChar* pcKey, const coreBool,  const coreBool  bValue)  {m_Config.SetBoolValue  (pcSection, pcKey, bValue);  m_bDirty = true;}
    inline void SetBool  (const coreChar* pcSection, const coreChar* pcKey,                  const coreBool  bValue)  {m_Config.SetBoolValue  (pcSection, pcKey, bValue);  m_bDirty = true;}
    inline void SetInt   (const coreChar* pcSection, const coreChar* pcKey, const coreInt32, const coreInt32 iValue)  {m_Config.SetLongValue  (pcSection, pcKey, iValue);  m_bDirty = true;}
    inline void SetInt   (const coreChar* pcSection, const coreChar* pcKey,                  const coreInt32 iValue)  {m_Config.SetLongValue  (pcSection, pcKey, iValue);  m_bDirty = true;}
    inline void SetFloat (const coreChar* pcSection, const coreChar* pcKey, const coreFloat, const coreFloat fValue)  {m_Config.SetDoubleValue(pcSection, pcKey, fValue);  m_bDirty = true;}
    inline void SetFloat (const coreChar* pcSection, const coreChar* pcKey,                  const coreFloat fValue)  {m_Config.SetDoubleValue(pcSection, pcKey, fValue);  m_bDirty = true;}
    inline void SetString(const coreChar* pcSection, const coreChar* pcKey, const coreChar*, const coreChar* pcValue) {m_Config.SetValue      (pcSection, pcKey, pcValue); m_bDirty = true;}
    inline void SetString(const coreChar* pcSection, const coreChar* pcKey,                  const coreChar* pcValue) {m_Config.SetValue      (pcSection, pcKey, pcValue); m_bDirty = true;}
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