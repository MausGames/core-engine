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


// ****************************************************************
/* configuration definitions */
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
#define CORE_CONFIG_GRAPHICS_RESOURCECONTEXT    "Graphics", "ResourceContext",    (DEFINED(_CORE_ANDROID_) ? false : true)
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
    CSimpleIni  m_Config;   //!< configuration file interface


public:
    explicit coreConfig(const coreChar* pcPath)noexcept;
    ~coreConfig();

    DISABLE_COPY(coreConfig)

    /*! load and save configuration file */
    //! @{
    coreStatus Load();
    coreStatus Save()const;
    //! @}

    /*! set configuration values */
    //! @{
    inline void SetBool  (const coreChar* pcSection, const coreChar* pcKey, const coreBool,  const coreBool  bValue)  {m_Config.SetBoolValue  (pcSection, pcKey, bValue);}
    inline void SetBool  (const coreChar* pcSection, const coreChar* pcKey,                  const coreBool  bValue)  {m_Config.SetBoolValue  (pcSection, pcKey, bValue);}
    inline void SetInt   (const coreChar* pcSection, const coreChar* pcKey, const coreInt32, const coreInt32 iValue)  {m_Config.SetLongValue  (pcSection, pcKey, iValue);}
    inline void SetInt   (const coreChar* pcSection, const coreChar* pcKey,                  const coreInt32 iValue)  {m_Config.SetLongValue  (pcSection, pcKey, iValue);}
    inline void SetFloat (const coreChar* pcSection, const coreChar* pcKey, const coreFloat, const coreFloat fValue)  {m_Config.SetDoubleValue(pcSection, pcKey, fValue);}
    inline void SetFloat (const coreChar* pcSection, const coreChar* pcKey,                  const coreFloat fValue)  {m_Config.SetDoubleValue(pcSection, pcKey, fValue);}
    inline void SetString(const coreChar* pcSection, const coreChar* pcKey, const coreChar*, const coreChar* pcValue) {m_Config.SetValue      (pcSection, pcKey, pcValue);}
    inline void SetString(const coreChar* pcSection, const coreChar* pcKey,                  const coreChar* pcValue) {m_Config.SetValue      (pcSection, pcKey, pcValue);}
    //! @}

    /*! get configuration values */
    //! @{
    inline coreBool        GetBool  (const coreChar* pcSection, const coreChar* pcKey, const coreBool  bDefault)  {const coreBool  bReturn  =            m_Config.GetBoolValue  (pcSection, pcKey, bDefault);  if(bReturn  == bDefault)  this->SetBool  (pcSection, pcKey, bDefault);  return bReturn;}
    inline coreInt32       GetInt   (const coreChar* pcSection, const coreChar* pcKey, const coreInt32 iDefault)  {const coreInt32 iReturn  = (coreInt32)m_Config.GetLongValue  (pcSection, pcKey, iDefault);  if(iReturn  == iDefault)  this->SetInt   (pcSection, pcKey, iDefault);  return iReturn;}
    inline coreFloat       GetFloat (const coreChar* pcSection, const coreChar* pcKey, const coreFloat fDefault)  {const coreFloat fReturn  = (coreFloat)m_Config.GetDoubleValue(pcSection, pcKey, fDefault);  if(fReturn  == fDefault)  this->SetFloat (pcSection, pcKey, fDefault);  return fReturn;}
    inline const coreChar* GetString(const coreChar* pcSection, const coreChar* pcKey, const coreChar* pcDefault) {const coreChar* pcReturn =            m_Config.GetValue      (pcSection, pcKey, pcDefault); if(pcReturn == pcDefault) this->SetString(pcSection, pcKey, pcDefault); return pcReturn;}
    //! @}

    /*! get object properties */
    //! @{
    inline const coreChar* GetPath()const {return m_sPath.c_str();}
    //! @}
};


#endif /* _CORE_GUARD_CONFIG_H_ */