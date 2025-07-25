///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_CONFIG_H_
#define _CORE_GUARD_CONFIG_H_

// TODO 3: add a way to control default values per project (dynamically ?)
// TODO 3: changing a bool to an int can cause problems with existing config values, remove bool entirely ?
// TODO 3: allow loading config files from archives


// ****************************************************************
/* configuration definitions */
#define CORE_CONFIG_BASE_LANGUAGE               "Base",     "Language",           ("")
#define CORE_CONFIG_BASE_DEBUGMODE              "Base",     "DebugMode",          (false)   // enable debug features and extended logging
#define CORE_CONFIG_BASE_ASYNCMODE              "Base",     "AsyncMode",          (true)    // enable asynchronous processing and resource loading
#define CORE_CONFIG_BASE_PERSISTMODE            "Base",     "PersistMode",        (false)   // load all resources on startup and keep them in memory
#define CORE_CONFIG_BASE_VERSION                "Base",     "Version",            (0)

#define CORE_CONFIG_SYSTEM_DISPLAY              "System",   "Display",            (0)
#define CORE_CONFIG_SYSTEM_WIDTH                "System",   "Width",              (0)
#define CORE_CONFIG_SYSTEM_HEIGHT               "System",   "Height",             (0)
#define CORE_CONFIG_SYSTEM_FULLSCREEN           "System",   "Fullscreen",         (1)
#define CORE_CONFIG_SYSTEM_REFRESHRATE          "System",   "RefreshRate",        (0.0f)
#define CORE_CONFIG_SYSTEM_VSYNC                "System",   "Vsync",              (1)

#define CORE_CONFIG_GRAPHICS_QUALITY            "Graphics", "Quality",            (1)
#define CORE_CONFIG_GRAPHICS_ANTIALIASING       "Graphics", "AntiAliasing",       (4)
#define CORE_CONFIG_GRAPHICS_TEXTUREANISOTROPY  "Graphics", "TextureAnisotropy",  (8)
#define CORE_CONFIG_GRAPHICS_TEXTURETRILINEAR   "Graphics", "TextureTrilinear",   (true)
#define CORE_CONFIG_GRAPHICS_TEXTURECOMPRESSION "Graphics", "TextureCompression", (true)
#define CORE_CONFIG_GRAPHICS_SHADERCACHE        "Graphics", "ShaderCache",        (true)
#define CORE_CONFIG_GRAPHICS_DISABLEEXTENSIONS  "Graphics", "DisableExtensions",  ("")
#define CORE_CONFIG_GRAPHICS_COMPATCONTEXT      "Graphics", "CompatContext",      (false)

#define CORE_CONFIG_AUDIO_MODE                  "Audio",    "Mode",               (0)
#define CORE_CONFIG_AUDIO_HRTFINDEX             "Audio",    "HRTFIndex",          (-1)
#define CORE_CONFIG_AUDIO_RESAMPLERINDEX        "Audio",    "ResamplerIndex",     (-1)
#define CORE_CONFIG_AUDIO_GLOBALVOLUME          "Audio",    "GlobalVolume",       (1.0f)
#define CORE_CONFIG_AUDIO_MUSICVOLUME           "Audio",    "MusicVolume",        (1.0f)
#define CORE_CONFIG_AUDIO_SOUNDVOLUME           "Audio",    "SoundVolume",        (1.0f)

#define CORE_CONFIG_INPUT_RUMBLE                "Input",    "Rumble",             (true)
#define CORE_CONFIG_INPUT_JOYSTICKDEAD          "Input",    "JoystickDead",       (0x2800)
#define CORE_CONFIG_INPUT_JOYSTICKMAX           "Input",    "JoystickMax",        (0x7000)


// ****************************************************************
/* configuration file class */
class coreConfig final
{
private:
    /* internal types */
    using coreSection = coreMapStrFull<coreString>;


private:
    coreMapStrFull<coreSection> m_aasSection;   // configuration sections with configuration entries

    coreString   m_sPath;                       // relative path of the file
    coreBool     m_bDirty;                      // status flag for pending changes
    coreSpinLock m_Lock;                        // spinlock to prevent concurrent configuration access


public:
    explicit coreConfig(const coreChar* pcPath)noexcept;
    ~coreConfig();

    DISABLE_COPY(coreConfig)

    /* load and save configuration file */
    coreStatus Load();
    coreStatus Save();

    /* apply configuration overrides */
    void ApplyCommandline();
    void ApplyGlobalFile();

    /* set configuration values */
    inline void SetBool  (const coreHashString& sSection, const coreHashString& sKey, const coreBool,  const coreBool  bValue)  {this->SetBool      (sSection, sKey, bValue);}
    inline void SetInt   (const coreHashString& sSection, const coreHashString& sKey, const coreInt32, const coreInt32 iValue)  {this->SetInt       (sSection, sKey, iValue);}
    inline void SetFloat (const coreHashString& sSection, const coreHashString& sKey, const coreFloat, const coreFloat fValue)  {this->SetFloat     (sSection, sKey, fValue);}
    inline void SetString(const coreHashString& sSection, const coreHashString& sKey, const coreChar*, const coreChar* pcValue) {this->SetString    (sSection, sKey, pcValue);}
    inline void SetBool  (const coreHashString& sSection, const coreHashString& sKey,                  const coreBool  bValue)  {this->__ChangeEntry(sSection, sKey, coreConfig::__FromBool (bValue));}
    inline void SetInt   (const coreHashString& sSection, const coreHashString& sKey,                  const coreInt32 iValue)  {this->__ChangeEntry(sSection, sKey, coreConfig::__FromInt  (iValue));}
    inline void SetFloat (const coreHashString& sSection, const coreHashString& sKey,                  const coreFloat fValue)  {this->__ChangeEntry(sSection, sKey, coreConfig::__FromFloat(fValue));}
    inline void SetString(const coreHashString& sSection, const coreHashString& sKey,                  const coreChar* pcValue) {this->__ChangeEntry(sSection, sKey, pcValue);}

    /* get configuration values */
    inline coreBool        GetBool  (const coreHashString& sSection, const coreHashString& sKey, const coreBool  bDefault)  {const coreSpinLocker oLocker(&m_Lock); coreString* psEntry; if(!this->__RetrieveEntry(sSection, sKey, &psEntry)) {m_bDirty = true; (*psEntry) = coreConfig::__FromBool (bDefault);} return coreConfig::__ToBool (*psEntry);}
    inline coreInt32       GetInt   (const coreHashString& sSection, const coreHashString& sKey, const coreInt32 iDefault)  {const coreSpinLocker oLocker(&m_Lock); coreString* psEntry; if(!this->__RetrieveEntry(sSection, sKey, &psEntry)) {m_bDirty = true; (*psEntry) = coreConfig::__FromInt  (iDefault);} return coreConfig::__ToInt  (*psEntry);}
    inline coreFloat       GetFloat (const coreHashString& sSection, const coreHashString& sKey, const coreFloat fDefault)  {const coreSpinLocker oLocker(&m_Lock); coreString* psEntry; if(!this->__RetrieveEntry(sSection, sKey, &psEntry)) {m_bDirty = true; (*psEntry) = coreConfig::__FromFloat(fDefault);} return coreConfig::__ToFloat(*psEntry);}
    inline const coreChar* GetString(const coreHashString& sSection, const coreHashString& sKey, const coreChar* pcDefault) {const coreSpinLocker oLocker(&m_Lock); coreString* psEntry; if(!this->__RetrieveEntry(sSection, sKey, &psEntry)) {m_bDirty = true; (*psEntry) = pcDefault;}                         return psEntry->c_str();}

    /* get object properties */
    inline const coreChar* GetPath()const {return m_sPath.c_str();}


private:
    /* handle configuration entries */
    coreBool __RetrieveEntry(const coreHashString& sSection, const coreHashString& sKey, coreString** OUTPUT ppsEntry);
    void     __ChangeEntry  (const coreHashString& sSection, const coreHashString& sKey, const coreChar* pcValue);

    /* convert to type */
    static inline coreBool  __ToBool (const coreString& sString) {return (sString[0] != 'f') && (sString[0] != 'F') && (sString[0] != '0');}
    static inline coreInt32 __ToInt  (const coreString& sString) {return coreData::FromChars<coreInt32>(sString.c_str(), sString.length());}
    static inline coreFloat __ToFloat(const coreString& sString) {return coreData::FromChars<coreFloat>(sString.c_str(), sString.length());}

    /* convert to string */
    static inline const coreChar* __FromBool (const coreBool  bValue) {return (bValue ? "true" : "false");}
    static inline const coreChar* __FromInt  (const coreInt32 iValue) {return coreData::ToChars(iValue);}
    static inline const coreChar* __FromFloat(const coreFloat fValue) {return coreData::ToChars(fValue);}
};


#endif /* _CORE_GUARD_CONFIG_H_ */