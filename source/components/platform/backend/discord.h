///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_DISCORD_H_
#define _CORE_GUARD_DISCORD_H_

#if __has_include(<discord/discord_game_sdk.h>)

#include <discord/discord_game_sdk.h>


// ****************************************************************
/* Discord definitions */
#if defined(_CORE_WINDOWS_)
    #define DISCORD_LIBRARY_NAME "discord_game_sdk.dll"
#elif defined(_CORE_LINUX_)
    #define DISCORD_LIBRARY_NAME "discord_game_sdk.so"
#elif defined(_CORE_MACOS_)
    #define DISCORD_LIBRARY_NAME "discord_game_sdk.dylib"
#endif

#define __DISCORD_DEFINE_FUNCTION(x) static decltype(x)* n ## x = NULL;
#define __DISCORD_LOAD_FUNCTION(x)   n ## x = r_cast<decltype(x)*>(coreData::GetAddress(s_pDiscordLibrary, #x)); WARN_IF(!n ## x) return false;


// ****************************************************************
/* dynamic library loading */
static void* s_pDiscordLibrary = NULL;   // Discord library handle

__DISCORD_DEFINE_FUNCTION(DiscordCreate)

static coreBool InitDiscordLibrary()
{
    ASSERT(!s_pDiscordLibrary)

    // open Discord library
    s_pDiscordLibrary = coreData::OpenLibrary(DISCORD_LIBRARY_NAME);
    if(s_pDiscordLibrary)
    {
        // load all required functions
        __DISCORD_LOAD_FUNCTION(DiscordCreate)

        return true;
    }

    return false;
}

static void ExitDiscordLibrary()
{
    if(s_pDiscordLibrary)
    {
        // close Discord library
        coreData::CloseLibrary(s_pDiscordLibrary);
        s_pDiscordLibrary = NULL;
    }
}


// ****************************************************************
/* write Discord debug message */
static void DISCORD_API LoggingCallback(void* pData, const EDiscordLogLevel eLevel, const coreChar* pcMessage)
{
    Core::Log->Warning(CORE_LOG_BOLD("Discord:") " %s (level %d)", pcMessage, eLevel);
}


// ****************************************************************
/* Discord backend class */
class coreDiscordBackend final : public coreBackend
{
private:
    IDiscordCore* m_pCore;                   // main application instance

    IDiscordActivityManager* m_pActivity;    // activity interface
    IDiscordUserManager*     m_pUser;        // user interface
    IDiscordOverlayManager*  m_pOverlay;     // overlay interface

    DiscordUser           m_UserData;        // user data
    IDiscordUserEvents    m_UserEvents;      // user callback object
    IDiscordOverlayEvents m_OverlayEvents;   // overlay callback object

    coreAtomic<coreUint8> m_iState;          // processing status


public:
    coreDiscordBackend()noexcept;

    DISABLE_COPY(coreDiscordBackend)

    /* control the backend */
    coreBool Init  ()final;
    void     Exit  ()final;
    void     Update()final;

    /* process presence */
    coreBool SetRichPresence(const corePlatformPresence& oPresence)final;

    /* process general features */
    const coreChar* GetUserID  ()const final;
    const coreChar* GetUserName()const final;

    /* get backend identifier */
    inline const coreChar* GetIdentifier()const final {return "Discord";}
    inline coreBool        IsPrimary    ()const final {return false;}


private:
    /* exit the base system */
    void __ExitBase();

    /* callback handlers */
    static void DISCORD_API __UserOnCurrentUserUpdate(void* pData);
    static void DISCORD_API __OverlayOnToggle        (void* pData, const coreBool bLocked);
};


// ****************************************************************
/* constructor */
inline coreDiscordBackend::coreDiscordBackend()noexcept
: coreBackend     ()
, m_pCore         (NULL)
, m_pActivity     (NULL)
, m_pUser         (NULL)
, m_pOverlay      (NULL)
, m_UserData      ()
, m_UserEvents    ()
, m_OverlayEvents ()
, m_iState        (0u)
{
}


// ****************************************************************
/* init the backend */
inline coreBool coreDiscordBackend::Init()
{
    // load Discord library
    if(!CoreApp::Settings::Platform::DiscordClientID || !InitDiscordLibrary())
    {
        return false;
    }

    // set callback handlers
    m_UserEvents   .on_current_user_update = coreDiscordBackend::__UserOnCurrentUserUpdate;
    m_OverlayEvents.on_toggle              = coreDiscordBackend::__OverlayOnToggle;

    // set init parameters
    DiscordCreateParams oParams;
    DiscordCreateParamsSetDefault(&oParams);
    oParams.client_id      = CoreApp::Settings::Platform::DiscordClientID;
    oParams.flags          = DiscordCreateFlags_NoRequireDiscord;
    oParams.user_events    = &m_UserEvents;
    oParams.overlay_events = &m_OverlayEvents;
    oParams.event_data     = this;

    // start up Discord library
    WARN_IF((nDiscordCreate(DISCORD_VERSION, &oParams, &m_pCore) != DiscordResult_Ok) || !m_pCore)
    {
        this->__ExitBase();

        Core::Log->Warning("Discord could not be initialized");
        return false;
    }

    // get communication interfaces
    m_pActivity = m_pCore->get_activity_manager(m_pCore);
    m_pUser     = m_pCore->get_user_manager    (m_pCore);
    m_pOverlay  = m_pCore->get_overlay_manager (m_pCore);

    // check for interface errors
    WARN_IF(!m_pActivity || !m_pUser || !m_pOverlay)
    {
        this->__ExitBase();

        Core::Log->Warning("Discord could not provide access");
        return false;
    }

    // enable Discord debug output (always)
    m_pCore->set_log_hook(m_pCore, DEFINED(_CORE_DEBUG_) ? DiscordLogLevel_Debug : DiscordLogLevel_Warn, NULL, &LoggingCallback);

    // register Steam application ID
    if(CoreApp::Settings::Platform::SteamAppID[CoreApp::Settings::IsDemo()])
    {
        m_pActivity->register_steam(m_pActivity, CoreApp::Settings::Platform::SteamAppID[CoreApp::Settings::IsDemo()]);
    }

    // wait on user data
    const std::time_t iLimit = std::time(NULL) + 5;
    do
    {
        CORE_SPINLOCK_YIELD
        m_pCore->run_callbacks(m_pCore);
    }
    while(!HAS_BIT(m_iState, 0u) && (std::time(NULL) < iLimit));

    // check for valid user data
    WARN_IF(!m_UserData.id)
    {
        this->__ExitBase();

        Core::Log->Warning("Discord could not authenticate");
        return false;
    }

    Core::Log->Info("Discord initialized (user %s)", this->GetUserID());
    return true;
}


// ****************************************************************
/* exit the backend */
inline void coreDiscordBackend::Exit()
{
    if(m_pCore)
    {
        // exit the base system
        this->__ExitBase();

        Core::Log->Info("Discord uninitialized");
    }
}


// ****************************************************************
/* update the backend */
inline void coreDiscordBackend::Update()
{
    if(m_pCore)
    {
        // update the Discord library (and execute callbacks)
        WARN_IF(m_pCore->run_callbacks(m_pCore) != DiscordResult_Ok) {}
    }
}


// ****************************************************************
/* set full rich presence */
inline coreBool coreDiscordBackend::SetRichPresence(const corePlatformPresence& oPresence)
{
    if(m_pCore)
    {
        // set rich presence text
        DiscordActivity oActivity = {};
        coreData::StrCopy(oActivity.details, ARRAY_SIZE(oActivity.details), oPresence.sDefaultText.c_str());

        // change rich presence
        m_pActivity->update_activity(m_pActivity, &oActivity, NULL, NULL);
        return true;
    }

    return false;
}


// ****************************************************************
/* get user identifier */
inline const coreChar* coreDiscordBackend::GetUserID()const
{
    if(m_pCore)
    {
        // retrieve Discord player identifier
        if(m_UserData.id) return PRINT("%lld", coreInt64(m_UserData.id));
    }

    return this->coreBackend::GetUserID();
}


// ****************************************************************
/* get user name */
inline const coreChar* coreDiscordBackend::GetUserName()const
{
    if(m_pCore)
    {
        // retrieve Discord player name
        if(m_UserData.username[0]) return m_UserData.username;
    }

    return this->coreBackend::GetUserName();
}


// ****************************************************************
/* callback handlers */
inline void DISCORD_API coreDiscordBackend::__UserOnCurrentUserUpdate(void* pData)
{
    coreDiscordBackend* pThis = s_cast<coreDiscordBackend*>(pData);

    // copy user data
    pThis->m_pUser->get_current_user(pThis->m_pUser, &pThis->m_UserData);

    // mark authentication as finished
    ADD_BIT(pThis->m_iState, 0u)
}

inline void DISCORD_API coreDiscordBackend::__OverlayOnToggle(void* pData, const coreBool bLocked)
{
    if(bLocked)
    {
        // notify about focus loss
        SDL_Event oEvent = {SDL_EVENT_USER};
        SDL_PushEvent(&oEvent);
    }
}


// ****************************************************************
/* exit the base system */
inline void coreDiscordBackend::__ExitBase()
{
    // shut down Discord library
    if(m_pCore) m_pCore->destroy(m_pCore);
    m_pCore = NULL;

    // unload Discord library
    ExitDiscordLibrary();
}


// ****************************************************************
/* Discord backend instance */
static coreDiscordBackend s_BackendDiscord;


#endif

#endif /* _CORE_GUARD_DISCORD_H_ */