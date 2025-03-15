///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_WINDOWS_H_
#define _CORE_GUARD_WINDOWS_H_


// ****************************************************************
/* Windows backend class */
class coreWindowsBackend final : public coreBackend
{
public:
    coreWindowsBackend() = default;

    DISABLE_COPY(coreWindowsBackend)

    /* process general features */
    const coreChar* GetLanguage()const final;

    /* get backend identifier */
    inline const coreChar* GetIdentifier()const final {return "Windows";}
    inline coreBool        IsPrimary    ()const final {return false;}
};


// ****************************************************************
/* get language */
inline const coreChar* coreWindowsBackend::GetLanguage()const
{
    // retrieve Windows language
    const LANGID iLanguage = GetUserDefaultUILanguage();

    // map Windows language to common language
    switch(iLanguage & 0xFFu)
    {
    case 0x04u: return (iLanguage == 0x0404u || iLanguage == 0x0C04u || iLanguage == 0x1404u || iLanguage == 0x7C04u) ? CORE_LANGUAGE_TCHINESE : CORE_LANGUAGE_SCHINESE;
    case 0x07u: return CORE_LANGUAGE_GERMAN;
    case 0x09u: return CORE_LANGUAGE_ENGLISH;
    case 0x0Au: return (iLanguage == 0x000Au || iLanguage == 0x040Au || iLanguage == 0x0C0Au) ? CORE_LANGUAGE_SPANISH : CORE_LANGUAGE_LATAM;
    case 0x0Cu: return CORE_LANGUAGE_FRENCH;
    case 0x10u: return CORE_LANGUAGE_ITALIAN;
    case 0x11u: return CORE_LANGUAGE_JAPANESE;
    case 0x12u: return CORE_LANGUAGE_KOREAN;
    case 0x15u: return CORE_LANGUAGE_POLISH;
    case 0x16u: return (iLanguage == 0x0416u) ? CORE_LANGUAGE_BRAZILIAN : CORE_LANGUAGE_PORTUGUESE;
    case 0x19u: return CORE_LANGUAGE_RUSSIAN;
    case 0x22u: return CORE_LANGUAGE_UKRAINIAN;
    }

    return this->coreBackend::GetLanguage();
}


// ****************************************************************
/* Windows backend instance */
static coreWindowsBackend s_BackendWindows;


#endif /* _CORE_GUARD_WINDOWS_H_ */