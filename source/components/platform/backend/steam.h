///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_STEAM_H_
#define _CORE_GUARD_STEAM_H_

#if defined(_CORE_MSVC_)
    #pragma warning(disable : 5204)   // class with virtual functions, but without virtual destructor
#else
    #pragma GCC diagnostic ignored "-Wnested-anon-types"
#endif

#include <steam/steam_api.h>


// ****************************************************************
/* Steam definitions */
#if defined(_CORE_WINDOWS_)
    #if defined(_CORE_64BIT_)
        #define STEAM_LIBRARY_NAME "steam_api64.dll"
    #else
        #define STEAM_LIBRARY_NAME "steam_api.dll"
    #endif
#elif defined(_CORE_LINUX_)
    #define STEAM_LIBRARY_NAME "libsteam_api.so"
#endif

#define __DEFINE_FUNCTION(x) static decltype(x)* n ## x = NULL;
#define __LOAD_FUNCTION(x)   n ## x = r_cast<decltype(x)*>(coreData::GetAddress(s_pSteamLibrary, #x)); if(!n ## x) return false;
#define __CALLBACK(x)        case x ## _t::k_iCallback: if(oMessage.m_pubParam) {ASSERT(oMessage.m_cubParam == sizeof(x ## _t)) this->__On ## x(r_cast<const x ## _t*>(oMessage.m_pubParam));} break;


// ****************************************************************
/* dynamic library loading */
static void* s_pSteamLibrary = NULL;   // Steam library handle

__DEFINE_FUNCTION(SteamClient)
__DEFINE_FUNCTION(SteamAPI_Init)
__DEFINE_FUNCTION(SteamAPI_Shutdown)
__DEFINE_FUNCTION(SteamAPI_RestartAppIfNecessary)
__DEFINE_FUNCTION(SteamAPI_ReleaseCurrentThreadMemory)
__DEFINE_FUNCTION(SteamAPI_GetHSteamPipe)
__DEFINE_FUNCTION(SteamAPI_GetHSteamUser)
__DEFINE_FUNCTION(SteamAPI_ManualDispatch_Init)
__DEFINE_FUNCTION(SteamAPI_ManualDispatch_RunFrame)
__DEFINE_FUNCTION(SteamAPI_ManualDispatch_GetNextCallback)
__DEFINE_FUNCTION(SteamAPI_ManualDispatch_FreeLastCallback)

static coreBool InitSteamLibrary()
{
    ASSERT(!s_pSteamLibrary)

    // open Steam library
    s_pSteamLibrary = coreData::OpenLibrary(STEAM_LIBRARY_NAME);
    if(s_pSteamLibrary)
    {
        // load all required functions
        __LOAD_FUNCTION(SteamClient)
        __LOAD_FUNCTION(SteamAPI_Init)
        __LOAD_FUNCTION(SteamAPI_Shutdown)
        __LOAD_FUNCTION(SteamAPI_RestartAppIfNecessary)
        __LOAD_FUNCTION(SteamAPI_ReleaseCurrentThreadMemory)
        __LOAD_FUNCTION(SteamAPI_GetHSteamPipe)
        __LOAD_FUNCTION(SteamAPI_GetHSteamUser)
        __LOAD_FUNCTION(SteamAPI_ManualDispatch_Init)
        __LOAD_FUNCTION(SteamAPI_ManualDispatch_RunFrame)
        __LOAD_FUNCTION(SteamAPI_ManualDispatch_GetNextCallback)
        __LOAD_FUNCTION(SteamAPI_ManualDispatch_FreeLastCallback)

        return true;
    }

    return false;
}

static void ExitSteamLibrary()
{
    if(s_pSteamLibrary)
    {
        // close Steam library
        coreData::CloseLibrary(s_pSteamLibrary);
        s_pSteamLibrary = NULL;
    }
}


// ****************************************************************
/* write Steam debug message */
extern "C" void __cdecl WarningMessageCallback(const coreInt32 iSeverity, const coreChar* pcMessage)
{
    Core::Log->Warning(CORE_LOG_BOLD("Steam:") " %s (severity %d)", pcMessage, iSeverity);
}


// ****************************************************************
/* Steam backend class */
class coreBackendSteam final : public coreBackend
{
private:
    ISteamClient* m_pClient;         // main communication instance
    HSteamPipe    m_iPipe;           // Steam pipe handle
    HSteamUser    m_iUser;           // Steam user handle

    ISteamApps*      m_pApps;        // application interface
    ISteamFriends*   m_pFriends;     // friends interface
    ISteamUser*      m_pUser;        // user interface
    ISteamUserStats* m_pUserStats;   // stats, achievement and leaderboard interface
    ISteamUtils*     m_pUtils;       // utility interface

    coreUint8 m_iStatsRequest;       // achievement request status (0 = idle | 1 = send query | 2 = wait on response)
    coreUint8 m_iStatsStore;         // achievement store status   (0 = idle | 1 = send query | 2 = wait on response)


public:
    coreBackendSteam()noexcept;

    DISABLE_COPY(coreBackendSteam)

    /* control the backend */
    coreBool Init  ()final;
    void     Exit  ()final;
    void     Update()final;

    /* process achievements */
    coreBool UnlockAchievement(const coreAchievement& oData)final;

    /* process general features */
    const coreChar* GetUserName()const final;
    const coreChar* GetLanguage()const final;

    /* get backend identifier */
    inline const coreChar* GetIdentifier()const final {return "Steam";}


private:
    /* callback handlers */
    void __OnGameOverlayActivated(const GameOverlayActivated_t* pCallback);
    void __OnUserStatsReceived   (const UserStatsReceived_t*    pCallback);
    void __OnUserStatsStored     (const UserStatsStored_t*      pCallback);

    /* exit the base system */
    void __ExitBase();
};


// ****************************************************************
/* constructor */
inline coreBackendSteam::coreBackendSteam()noexcept
: coreBackend     ()
, m_pClient       (NULL)
, m_iPipe         (0u)
, m_iUser         (0u)
, m_pApps         (NULL)
, m_pFriends      (NULL)
, m_pUser         (NULL)
, m_pUserStats    (NULL)
, m_pUtils        (NULL)
, m_iStatsRequest (0u)
, m_iStatsStore   (0u)
{
}


// ****************************************************************
/* init the backend */
inline coreBool coreBackendSteam::Init()
{
    ASSERT(!m_pClient)

    // load Steam library
    if(!CoreApp::Settings::Platform::SteamAppID || !InitSteamLibrary())
    {
        return false;   // DRM free
    }

#if !defined(_CORE_DEBUG_)

    // make sure Steam launched the application
    if(nSteamAPI_RestartAppIfNecessary(CoreApp::Settings::Platform::SteamAppID))
    {
        Core::Log->Warning("Steam will launch the application");
        std::exit(EXIT_SUCCESS);
    }

#endif

    // start up Steam library
    m_pClient = nSteamAPI_Init() ? nSteamClient() : NULL;

    // check for library errors
    WARN_IF(!m_pClient)
    {
        this->__ExitBase();

        Core::Log->Warning("Steam could not be initialized");
        return false;
    }

    // get communication handles
    m_iPipe = nSteamAPI_GetHSteamPipe();
    m_iUser = nSteamAPI_GetHSteamUser();

    // get communication interfaces
    m_pApps      = m_pClient->GetISteamApps     (m_iUser, m_iPipe, STEAMAPPS_INTERFACE_VERSION);
    m_pFriends   = m_pClient->GetISteamFriends  (m_iUser, m_iPipe, STEAMFRIENDS_INTERFACE_VERSION);
    m_pUser      = m_pClient->GetISteamUser     (m_iUser, m_iPipe, STEAMUSER_INTERFACE_VERSION);
    m_pUserStats = m_pClient->GetISteamUserStats(m_iUser, m_iPipe, STEAMUSERSTATS_INTERFACE_VERSION);
    m_pUtils     = m_pClient->GetISteamUtils    (m_iPipe,          STEAMUTILS_INTERFACE_VERSION);

    // check for interface errors
    WARN_IF(!m_pApps || !m_pFriends || !m_pUser || !m_pUserStats || !m_pUtils)
    {
        this->__ExitBase();

        Core::Log->Warning("Steam could not provide access");
        return false;
    }

    // enable Steam debug output (always)
    m_pUtils->SetWarningMessageHook(&WarningMessageCallback);

    // enable manual callback dispatch
    nSteamAPI_ManualDispatch_Init();

    // request achievement data
    m_iStatsRequest = 1u;
    m_iStatsStore   = 0u;

    Core::Log->Info("Steam initialized (app %u, user %llu)", m_pUtils->GetAppID(), m_pUser->GetSteamID().ConvertToUint64());
    return true;
}


// ****************************************************************
/* exit the backend */
inline void coreBackendSteam::Exit()
{
    if(m_pClient)
    {
        // exit the base system
        this->__ExitBase();

        Core::Log->Info("Steam uninitialized");
    }
}


// ****************************************************************
/* update the backend */
inline void coreBackendSteam::Update()
{
    if(m_pClient)
    {
        // update achievement data
        if(m_iStatsRequest == 1u) if(m_pUserStats->RequestCurrentStats()) m_iStatsRequest = 2u;
        if(m_iStatsStore   == 1u) if(m_pUserStats->StoreStats         ()) m_iStatsStore   = 2u;

        // update manual callback dispatch
        nSteamAPI_ManualDispatch_RunFrame(m_iPipe);

        // process callbacks
        CallbackMsg_t oMessage;
        while(nSteamAPI_ManualDispatch_GetNextCallback(m_iPipe, &oMessage))
        {
            // dispatch to callback handler
            switch(oMessage.m_iCallback)
            {
            __CALLBACK(GameOverlayActivated)
            __CALLBACK(UserStatsReceived)
            __CALLBACK(UserStatsStored)
            }

            // finish callback
            nSteamAPI_ManualDispatch_FreeLastCallback(m_iPipe);
        }

        // cleanup thread-local memory
        nSteamAPI_ReleaseCurrentThreadMemory();
    }
}


// ****************************************************************
/* unlock achievement */
inline coreBool coreBackendSteam::UnlockAchievement(const coreAchievement& oData)
{
    if(m_pClient)
    {
        // unlock achievement in Steam
        if((m_iStatsRequest == 0u) && m_pUserStats->SetAchievement(oData.sSteamName.c_str()))
        {
            m_iStatsStore = 1u;
            return true;
        }
    }

    return false;
}


// ****************************************************************
/* get user name */
inline const coreChar* coreBackendSteam::GetUserName()const
{
    if(m_pClient)
    {
        // retrieve Steam player name
        return m_pFriends->GetPersonaName();
    }

    return this->coreBackend::GetUserName();
}


// ****************************************************************
/* get language */
inline const coreChar* coreBackendSteam::GetLanguage()const
{
    if(m_pClient)
    {
        // retrieve Steam language
        const coreChar* pcLanguage = m_pApps->GetCurrentGameLanguage();

        // map Steam language to common language
        if(!std::strcmp(pcLanguage, "brazilian"))  return CORE_LANGUAGE_BRAZILIAN;
        if(!std::strcmp(pcLanguage, "english"))    return CORE_LANGUAGE_ENGLISH;
        if(!std::strcmp(pcLanguage, "french"))     return CORE_LANGUAGE_FRENCH;
        if(!std::strcmp(pcLanguage, "german"))     return CORE_LANGUAGE_GERMAN;
        if(!std::strcmp(pcLanguage, "italian"))    return CORE_LANGUAGE_ITALIAN;
        if(!std::strcmp(pcLanguage, "japanese"))   return CORE_LANGUAGE_JAPANESE;
        if(!std::strcmp(pcLanguage, "koreana"))    return CORE_LANGUAGE_KOREAN;   // with a
        if(!std::strcmp(pcLanguage, "polish"))     return CORE_LANGUAGE_POLISH;
        if(!std::strcmp(pcLanguage, "portuguese")) return CORE_LANGUAGE_PORTUGUESE;
        if(!std::strcmp(pcLanguage, "russian"))    return CORE_LANGUAGE_RUSSIAN;
        if(!std::strcmp(pcLanguage, "schinese"))   return CORE_LANGUAGE_SCHINESE;
        if(!std::strcmp(pcLanguage, "spanish"))    return CORE_LANGUAGE_SPANISH;
        if(!std::strcmp(pcLanguage, "tchinese"))   return CORE_LANGUAGE_TCHINESE;
    }

    return this->coreBackend::GetLanguage();
}


// ****************************************************************
/* callback handlers */
inline void coreBackendSteam::__OnGameOverlayActivated(const GameOverlayActivated_t* pCallback)
{
    if(pCallback->m_bActive)
    {
        // notify about focus loss
        SDL_Event oEvent = {SDL_USEREVENT};
        SDL_PushEvent(&oEvent);
    }
}

inline void coreBackendSteam::__OnUserStatsReceived(const UserStatsReceived_t* pCallback)
{
    if(pCallback->m_nGameID == CoreApp::Settings::Platform::SteamAppID)
    {
        // check for success (or try again)
        m_iStatsRequest = (pCallback->m_eResult == k_EResultOK) ? 0u : 1u;
    }
}

inline void coreBackendSteam::__OnUserStatsStored(const UserStatsStored_t* pCallback)
{
    if(pCallback->m_nGameID == CoreApp::Settings::Platform::SteamAppID)
    {
        // check for success (or try again)
        m_iStatsStore = (pCallback->m_eResult == k_EResultOK) ? 0u : 1u;
    }
}


// ****************************************************************
/* exit the base system */
inline void coreBackendSteam::__ExitBase()
{
    // shut down Steam library
    nSteamAPI_Shutdown();
    m_pClient = NULL;

    // unload Steam library
    ExitSteamLibrary();
}


// ****************************************************************
/* Steam backend instance */
static coreBackendSteam s_BackendSteam;


#endif /* _CORE_GUARD_STEAM_H_ */