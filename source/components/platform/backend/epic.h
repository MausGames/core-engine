///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_EPIC_H_
#define _CORE_GUARD_EPIC_H_

#if __has_include(<epic/eos_sdk.h>)

#include <epic/eos_sdk.h>
#include <epic/eos_auth.h>
#include <epic/eos_connect.h>
#include <epic/eos_userinfo.h>
#include <epic/eos_achievements.h>
#include <epic/eos_stats.h>
#include <epic/eos_presence.h>
#include <epic/eos_ui.h>
#include <epic/eos_logging.h>
#include <epic/eos_version.h>

// TODO 3: Ukrainian language is currently not supported on the Epic store


// ****************************************************************
/* Epic definitions */
#if defined(_CORE_WINDOWS_)
    #if defined(_CORE_64BIT_)
        #define EPIC_LIBRARY_NAME "EOSSDK-Win64-Shipping.dll"
    #else
        #define EPIC_LIBRARY_NAME "EOSSDK-Win32-Shipping.dll"
    #endif
#elif defined(_CORE_LINUX_)
    #define EPIC_LIBRARY_NAME "libEOSSDK-Linux-Shipping.so"
#elif defined(_CORE_MACOS_)
    #define EPIC_LIBRARY_NAME "libEOSSDK-Mac-Shipping.dylib"
#endif

#if defined(_CORE_WINDOWS_) && !defined(_CORE_64BIT_)
    #define __EPIC_DECORATION(x,y) "_" #x "@" #y   // __stdcall
#else
    #define __EPIC_DECORATION(x,y) #x
#endif

#define __EPIC_DEFINE_FUNCTION(x) static decltype(x)* n ## x = NULL;
#define __EPIC_LOAD_FUNCTION(x,y) n ## x = r_cast<decltype(x)*>(coreData::GetAddress(s_pEpicLibrary, __EPIC_DECORATION(x, y))); WARN_IF(!n ## x) return false;


// ****************************************************************
/* dynamic library loading */
static void* s_pEpicLibrary = NULL;   // Epic library handle

__EPIC_DEFINE_FUNCTION(EOS_Achievements_QueryDefinitions)
__EPIC_DEFINE_FUNCTION(EOS_Achievements_QueryPlayerAchievements)
__EPIC_DEFINE_FUNCTION(EOS_Achievements_UnlockAchievements)
__EPIC_DEFINE_FUNCTION(EOS_Auth_CopyIdToken)
__EPIC_DEFINE_FUNCTION(EOS_Auth_IdToken_Release)
__EPIC_DEFINE_FUNCTION(EOS_Auth_Login)
__EPIC_DEFINE_FUNCTION(EOS_Connect_AddNotifyAuthExpiration)
__EPIC_DEFINE_FUNCTION(EOS_Connect_CreateUser)
__EPIC_DEFINE_FUNCTION(EOS_Connect_Login)
__EPIC_DEFINE_FUNCTION(EOS_EpicAccountId_ToString)
__EPIC_DEFINE_FUNCTION(EOS_GetVersion)
__EPIC_DEFINE_FUNCTION(EOS_Initialize)
__EPIC_DEFINE_FUNCTION(EOS_Logging_SetCallback)
__EPIC_DEFINE_FUNCTION(EOS_Logging_SetLogLevel)
__EPIC_DEFINE_FUNCTION(EOS_Platform_CheckForLauncherAndRestart)
__EPIC_DEFINE_FUNCTION(EOS_Platform_Create)
__EPIC_DEFINE_FUNCTION(EOS_Platform_GetAchievementsInterface)
__EPIC_DEFINE_FUNCTION(EOS_Platform_GetActiveLocaleCode)
__EPIC_DEFINE_FUNCTION(EOS_Platform_GetAuthInterface)
__EPIC_DEFINE_FUNCTION(EOS_Platform_GetConnectInterface)
__EPIC_DEFINE_FUNCTION(EOS_Platform_GetPresenceInterface)
__EPIC_DEFINE_FUNCTION(EOS_Platform_GetStatsInterface)
__EPIC_DEFINE_FUNCTION(EOS_Platform_GetUIInterface)
__EPIC_DEFINE_FUNCTION(EOS_Platform_GetUserInfoInterface)
__EPIC_DEFINE_FUNCTION(EOS_Platform_Release)
__EPIC_DEFINE_FUNCTION(EOS_Platform_Tick)
__EPIC_DEFINE_FUNCTION(EOS_Presence_CreatePresenceModification)
__EPIC_DEFINE_FUNCTION(EOS_Presence_SetPresence)
__EPIC_DEFINE_FUNCTION(EOS_PresenceModification_Release)
__EPIC_DEFINE_FUNCTION(EOS_PresenceModification_SetData)
__EPIC_DEFINE_FUNCTION(EOS_PresenceModification_SetRawRichText)
__EPIC_DEFINE_FUNCTION(EOS_PresenceModification_SetStatus)
__EPIC_DEFINE_FUNCTION(EOS_Shutdown)
__EPIC_DEFINE_FUNCTION(EOS_Stats_IngestStat)
__EPIC_DEFINE_FUNCTION(EOS_UI_AddNotifyDisplaySettingsUpdated)
__EPIC_DEFINE_FUNCTION(EOS_UserInfo_CopyUserInfo)
__EPIC_DEFINE_FUNCTION(EOS_UserInfo_QueryUserInfo)
__EPIC_DEFINE_FUNCTION(EOS_UserInfo_Release)

static coreBool InitEpicLibrary()
{
    ASSERT(!s_pEpicLibrary)

    // open Epic library
    s_pEpicLibrary = coreData::OpenLibrary(EPIC_LIBRARY_NAME);
    if(s_pEpicLibrary)
    {
        // load all required functions
        __EPIC_LOAD_FUNCTION(EOS_Achievements_QueryDefinitions,        16)
        __EPIC_LOAD_FUNCTION(EOS_Achievements_QueryPlayerAchievements, 16)
        __EPIC_LOAD_FUNCTION(EOS_Achievements_UnlockAchievements,      16)
        __EPIC_LOAD_FUNCTION(EOS_Auth_CopyIdToken,                     12)
        __EPIC_LOAD_FUNCTION(EOS_Auth_IdToken_Release,                 4)
        __EPIC_LOAD_FUNCTION(EOS_Auth_Login,                           16)
        __EPIC_LOAD_FUNCTION(EOS_Connect_AddNotifyAuthExpiration,      16)
        __EPIC_LOAD_FUNCTION(EOS_Connect_CreateUser,                   16)
        __EPIC_LOAD_FUNCTION(EOS_Connect_Login,                        16)
        __EPIC_LOAD_FUNCTION(EOS_EpicAccountId_ToString,               12)
        __EPIC_LOAD_FUNCTION(EOS_GetVersion,                           0)
        __EPIC_LOAD_FUNCTION(EOS_Initialize,                           4)
        __EPIC_LOAD_FUNCTION(EOS_Logging_SetCallback,                  4)
        __EPIC_LOAD_FUNCTION(EOS_Logging_SetLogLevel,                  8)
        __EPIC_LOAD_FUNCTION(EOS_Platform_CheckForLauncherAndRestart,  4)
        __EPIC_LOAD_FUNCTION(EOS_Platform_Create,                      4)
        __EPIC_LOAD_FUNCTION(EOS_Platform_GetAchievementsInterface,    4)
        __EPIC_LOAD_FUNCTION(EOS_Platform_GetActiveLocaleCode,         16)
        __EPIC_LOAD_FUNCTION(EOS_Platform_GetAuthInterface,            4)
        __EPIC_LOAD_FUNCTION(EOS_Platform_GetConnectInterface,         4)
        __EPIC_LOAD_FUNCTION(EOS_Platform_GetPresenceInterface,        4)
        __EPIC_LOAD_FUNCTION(EOS_Platform_GetStatsInterface,           4)
        __EPIC_LOAD_FUNCTION(EOS_Platform_GetUIInterface,              4)
        __EPIC_LOAD_FUNCTION(EOS_Platform_GetUserInfoInterface,        4)
        __EPIC_LOAD_FUNCTION(EOS_Platform_Release,                     4)
        __EPIC_LOAD_FUNCTION(EOS_Platform_Tick,                        4)
        __EPIC_LOAD_FUNCTION(EOS_Presence_CreatePresenceModification,  12)
        __EPIC_LOAD_FUNCTION(EOS_Presence_SetPresence,                 16)
        __EPIC_LOAD_FUNCTION(EOS_PresenceModification_Release,         4)
        __EPIC_LOAD_FUNCTION(EOS_PresenceModification_SetData,         8)
        __EPIC_LOAD_FUNCTION(EOS_PresenceModification_SetRawRichText,  8)
        __EPIC_LOAD_FUNCTION(EOS_PresenceModification_SetStatus,       8)
        __EPIC_LOAD_FUNCTION(EOS_Shutdown,                             0)
        __EPIC_LOAD_FUNCTION(EOS_Stats_IngestStat,                     16)
        __EPIC_LOAD_FUNCTION(EOS_UI_AddNotifyDisplaySettingsUpdated,   16)
        __EPIC_LOAD_FUNCTION(EOS_UserInfo_CopyUserInfo,                12)
        __EPIC_LOAD_FUNCTION(EOS_UserInfo_QueryUserInfo,               16)
        __EPIC_LOAD_FUNCTION(EOS_UserInfo_Release,                     4)

        return true;
    }

    return false;
}

static void ExitEpicLibrary()
{
    if(s_pEpicLibrary)
    {
        // close Epic library
        coreData::CloseLibrary(s_pEpicLibrary);
        s_pEpicLibrary = NULL;
    }
}


// ****************************************************************
/* write Epic debug message */
static void EOS_CALL LoggingCallback(const EOS_LogMessage* pMessage)
{
    Core::Log->Warning(CORE_LOG_BOLD("Epic:") " %s (%s, level %d)", pMessage->Message, pMessage->Category, pMessage->Level);
}


// ****************************************************************
/* Epic backend class */
class coreBackendEpic final : public coreBackend
{
private:
    EOS_HPlatform m_pPlatform;           // main platform instance

    EOS_HAuth         m_pAuth;           // authentication interface
    EOS_HConnect      m_pConnect;        // connection interface
    EOS_HUserInfo     m_pUserInfo;       // user-info interface
    EOS_HAchievements m_pAchievements;   // achievements interface
    EOS_HStats        m_pStats;          // stats interface
    EOS_HPresence     m_pPresence;       // presence interface
    EOS_HUI           m_pUI;             // UI interface

    EOS_EpicAccountId m_pAccountId;      // Epic account identifier (authentication)
    EOS_ProductUserId m_pUserId;         // product user identifier (connection)

    coreAtomic<coreUint8> m_iState;      // processing status


public:
    coreBackendEpic()noexcept;

    DISABLE_COPY(coreBackendEpic)

    /* control the backend */
    coreBool Init  ()final;
    void     Exit  ()final;
    void     Update()final;

    /* process achievements */
    coreBool UnlockAchievement(const corePlatformAchievement& oEntry)final;

    /* process stats */
    coreBool ModifyStat(const corePlatformStat& oEntry, const coreInt32 iValue)final;

    /* process presence */
    coreBool SetRichPresence(const corePlatformPresence& oPresence)final;

    /* process general features */
    const coreChar* GetUserID  ()const final;
    const coreChar* GetUserName()const final;
    const coreChar* GetLanguage()const final;

    /* get backend identifier */
    inline const coreChar* GetIdentifier()const final {return "Epic";}


private:
    /* exit the base system */
    void __ExitBase();

    /* setup functions */
    void __LoginUser();
    void __QueryData();

    /* callback handlers */
    static void EOS_CALL __OnAuthLogin                          (const EOS_Auth_LoginCallbackInfo*                                     pData);
    static void EOS_CALL __OnConnectLogin                       (const EOS_Connect_LoginCallbackInfo*                                  pData);
    static void EOS_CALL __OnConnectCreateUser                  (const EOS_Connect_CreateUserCallbackInfo*                             pData);
    static void EOS_CALL __OnConnectAddNotifyAuthExpiration     (const EOS_Connect_AuthExpirationCallbackInfo*                         pData);
    static void EOS_CALL __OnUserInfoQueryUserInfo              (const EOS_UserInfo_QueryUserInfoCallbackInfo*                         pData);
    static void EOS_CALL __OnAchievementsQueryDefinitions       (const EOS_Achievements_OnQueryDefinitionsCompleteCallbackInfo*        pData);
    static void EOS_CALL __OnAchievementsQueryPlayerAchievements(const EOS_Achievements_OnQueryPlayerAchievementsCompleteCallbackInfo* pData);
    static void EOS_CALL __OnAchievementsUnlockAchievements     (const EOS_Achievements_OnUnlockAchievementsCompleteCallbackInfo*      pData);
    static void EOS_CALL __OnStatsIngestStat                    (const EOS_Stats_IngestStatCompleteCallbackInfo*                       pData);
    static void EOS_CALL __OnPresenceSetPresence                (const EOS_Presence_SetPresenceCallbackInfo*                           pData);
    static void EOS_CALL __OnUIAddNotifyDisplaySettingsUpdated  (const EOS_UI_OnDisplaySettingsUpdatedCallbackInfo*                    pData);
};


// ****************************************************************
/* constructor */
inline coreBackendEpic::coreBackendEpic()noexcept
: coreBackend     ()
, m_pPlatform     (NULL)
, m_pAuth         (NULL)
, m_pConnect      (NULL)
, m_pUserInfo     (NULL)
, m_pAchievements (NULL)
, m_pStats        (NULL)
, m_pPresence     (NULL)
, m_pUI           (NULL)
, m_pAccountId    (NULL)
, m_pUserId       (NULL)
, m_iState        (0u)
{
}


// ****************************************************************
/* init the backend */
inline coreBool coreBackendEpic::Init()
{
    ASSERT(!m_pPlatform)

    // load Epic library
    if(!CoreApp::Settings::Platform::EpicProductID || !InitEpicLibrary())
    {
        return false;   // DRM free
    }

    // set init options
    EOS_InitializeOptions oInitializeOptions = {};
    oInitializeOptions.ApiVersion     = EOS_INITIALIZE_API_LATEST;
    oInitializeOptions.ProductName    = CoreApp::Settings::Name;
    oInitializeOptions.ProductVersion = CoreApp::Settings::Version;

    // start up Epic library
    WARN_IF(nEOS_Initialize(&oInitializeOptions) != EOS_EResult::EOS_Success)
    {
        this->__ExitBase();

        Core::Log->Warning("Epic could not be initialized");
        return false;
    }

    // enable Epic debug output (always)
    nEOS_Logging_SetCallback(LoggingCallback);
    nEOS_Logging_SetLogLevel(EOS_ELogCategory::EOS_LC_ALL_CATEGORIES, DEFINED(_CORE_DEBUG_) ? EOS_ELogLevel::EOS_LOG_Verbose : EOS_ELogLevel::EOS_LOG_Warning);

    // select sandbox ID and deployment ID
    const coreChar* pcSandboxID    = coreData::GetCommandLine("epicsandboxid");
    const coreChar* pcDeploymentID = coreData::GetCommandLine("epicdeploymentid");
    if(pcSandboxID)
    {
        // search requested IDs
        for(coreUintW i = 0u; i < ARRAY_SIZE(CoreApp::Settings::Platform::EpicSandboxID); ++i)
        {
            if(!std::strcmp(pcSandboxID, CoreApp::Settings::Platform::EpicSandboxID[i]))
            {
                pcDeploymentID = CoreApp::Settings::Platform::EpicDeploymentID[i];
                break;
            }
        }
    }
    if(!pcDeploymentID)
    {
        // use default IDs
        pcSandboxID    = CoreApp::Settings::Platform::EpicSandboxID   [DEFINED(_CORE_DEBUG_) ? 0u : 2u];
        pcDeploymentID = CoreApp::Settings::Platform::EpicDeploymentID[DEFINED(_CORE_DEBUG_) ? 0u : 2u];
    }
    STATIC_ASSERT((ARRAY_SIZE(CoreApp::Settings::Platform::EpicSandboxID) == 3u) && (ARRAY_SIZE(CoreApp::Settings::Platform::EpicDeploymentID) == 3u))

    // set platform options
    EOS_Platform_Options oPlatformOptions = {};
    oPlatformOptions.ApiVersion                     = EOS_PLATFORM_OPTIONS_API_LATEST;
    oPlatformOptions.ProductId                      = CoreApp::Settings::Platform::EpicProductID;
    oPlatformOptions.SandboxId                      = pcSandboxID;
    oPlatformOptions.ClientCredentials.ClientId     = CoreApp::Settings::Platform::EpicClientID;
    oPlatformOptions.ClientCredentials.ClientSecret = CoreApp::Settings::Platform::EpicClientSecret;
    oPlatformOptions.DeploymentId                   = pcDeploymentID;
    oPlatformOptions.Flags                          = EOS_PF_WINDOWS_ENABLE_OVERLAY_OPENGL;
    oPlatformOptions.TickBudgetInMilliseconds       = 5u;

    // create platform instance
    m_pPlatform = nEOS_Platform_Create(&oPlatformOptions);

    // get communication interfaces
    m_pAuth         = nEOS_Platform_GetAuthInterface        (m_pPlatform);
    m_pConnect      = nEOS_Platform_GetConnectInterface     (m_pPlatform);
    m_pUserInfo     = nEOS_Platform_GetUserInfoInterface    (m_pPlatform);
    m_pAchievements = nEOS_Platform_GetAchievementsInterface(m_pPlatform);
    m_pStats        = nEOS_Platform_GetStatsInterface       (m_pPlatform);
    m_pPresence     = nEOS_Platform_GetPresenceInterface    (m_pPlatform);
    m_pUI           = nEOS_Platform_GetUIInterface          (m_pPlatform);

    // check for interface errors
    WARN_IF(!m_pPlatform || !m_pAuth || !m_pConnect || !m_pUserInfo || !m_pAchievements || !m_pStats || !m_pPresence || !m_pUI)
    {
        this->__ExitBase();

        Core::Log->Warning("Epic could not provide access");
        return false;
    }

#if defined(_CORE_DEBUG_)

    // set authentication credentials (debug server)
    EOS_Auth_Credentials oCredentials = {};
    oCredentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
    oCredentials.Id         = "localhost:6547";
    oCredentials.Token      = "Martin";
    oCredentials.Type       = EOS_ELoginCredentialType::EOS_LCT_Developer;

#else

    // make sure Epic launched the application
    if(nEOS_Platform_CheckForLauncherAndRestart(m_pPlatform) == EOS_EResult::EOS_Success)
    {
        Core::Log->Warning("Epic will launch the application");
        std::exit(EXIT_SUCCESS);
    }

    // set authentication credentials (launcher)
    EOS_Auth_Credentials oCredentials = {};
    oCredentials.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
    oCredentials.Id         = "";
    oCredentials.Token      = coreData::GetCommandLine("auth_password");
    oCredentials.Type       = EOS_ELoginCredentialType::EOS_LCT_ExchangeCode;

#endif

    // set authentication options
    EOS_Auth_LoginOptions oLoginOptions = {};
    oLoginOptions.ApiVersion  = EOS_AUTH_LOGIN_API_LATEST;
    oLoginOptions.Credentials = &oCredentials;
    oLoginOptions.ScopeFlags  = EOS_EAuthScopeFlags::EOS_AS_BasicProfile | EOS_EAuthScopeFlags::EOS_AS_Presence | EOS_EAuthScopeFlags::EOS_AS_FriendsList;

    // authenticate local user
    nEOS_Auth_Login(m_pAuth, &oLoginOptions, this, coreBackendEpic::__OnAuthLogin);

    // wait on authentication (required to access user-private location)
    const std::time_t iLimit = std::time(NULL) + 5u;
    do
    {
        CORE_SPINLOCK_YIELD
        nEOS_Platform_Tick(m_pPlatform);
    }
    while(!HAS_BIT(m_iState, 0u) && (std::time(NULL) < iLimit));

    // check for successful authentication
    WARN_IF(!m_pAccountId)
    {
        this->__ExitBase();

        Core::Log->Warning("Epic could not authenticate");
        return false;
    }

    // set expiration options
    EOS_Connect_AddNotifyAuthExpirationOptions oExpirationOptions = {};
    oExpirationOptions.ApiVersion = EOS_CONNECT_ADDNOTIFYAUTHEXPIRATION_API_LATEST;

    // listen to expiration notifications
    nEOS_Connect_AddNotifyAuthExpiration(m_pConnect, &oExpirationOptions, this, coreBackendEpic::__OnConnectAddNotifyAuthExpiration);

    // set overlay options
    EOS_UI_AddNotifyDisplaySettingsUpdatedOptions oDisplayOptions = {};
    oDisplayOptions.ApiVersion = EOS_UI_ADDNOTIFYDISPLAYSETTINGSUPDATED_API_LATEST;

    // listen to overlay notifications
    nEOS_UI_AddNotifyDisplaySettingsUpdated(m_pUI, &oDisplayOptions, this, coreBackendEpic::__OnUIAddNotifyDisplaySettingsUpdated);

    Core::Log->Info("Epic initialized (%s, user %s)", nEOS_GetVersion(), this->GetUserID());
    return true;
}


// ****************************************************************
/* exit the backend */
inline void coreBackendEpic::Exit()
{
    if(m_pPlatform)
    {
        // exit the base system
        this->__ExitBase();

        Core::Log->Info("Epic uninitialized");
    }
}


// ****************************************************************
/* update the backend */
inline void coreBackendEpic::Update()
{
    if(m_pPlatform)
    {
        // update the Epic library (and execute callbacks)
        nEOS_Platform_Tick(m_pPlatform);
    }
}


// ****************************************************************
/* unlock achievement */
inline coreBool coreBackendEpic::UnlockAchievement(const corePlatformAchievement& oEntry)
{
    if(m_pPlatform)
    {
        if(HAS_BIT(m_iState, 2u) && HAS_BIT(m_iState, 3u))
        {
            const coreChar* apcName[] = {oEntry.sEpicName.c_str()};

            // set unlock options
            EOS_Achievements_UnlockAchievementsOptions oOptions = {};
            oOptions.ApiVersion        = EOS_ACHIEVEMENTS_UNLOCKACHIEVEMENTS_API_LATEST;
            oOptions.UserId            = m_pUserId;
            oOptions.AchievementIds    = apcName;
            oOptions.AchievementsCount = ARRAY_SIZE(apcName);

            // unlock achievement in Epic
            nEOS_Achievements_UnlockAchievements(m_pAchievements, &oOptions, this, coreBackendEpic::__OnAchievementsUnlockAchievements);
            return true;
        }
    }

    return false;
}


// ****************************************************************
/* modify stat */
inline coreBool coreBackendEpic::ModifyStat(const corePlatformStat& oEntry, const coreInt32 iValue)
{
    if(m_pPlatform)
    {
        if(HAS_BIT(m_iState, 1u))
        {
            // set stat data
            EOS_Stats_IngestData oStat = {};
            oStat.ApiVersion   = EOS_STATS_INGESTDATA_API_LATEST;
            oStat.StatName     = oEntry.sEpicName.c_str();
            oStat.IngestAmount = iValue;

            // set modify options
            EOS_Stats_IngestStatOptions oOptions = {};
            oOptions.ApiVersion   = EOS_STATS_INGESTSTAT_API_LATEST;
            oOptions.LocalUserId  = m_pUserId;
            oOptions.Stats        = &oStat;
            oOptions.StatsCount   = 1u;
            oOptions.TargetUserId = m_pUserId;

            // modify stat in Epic
            nEOS_Stats_IngestStat(m_pStats, &oOptions, this, coreBackendEpic::__OnStatsIngestStat);
            return true;
        }
    }

    return false;
}


// ****************************************************************
/* set full rich presence */
inline coreBool coreBackendEpic::SetRichPresence(const corePlatformPresence& oPresence)
{
    if(m_pPlatform)
    {
        // set change options
        EOS_Presence_CreatePresenceModificationOptions oModificationOptions = {};
        oModificationOptions.ApiVersion  = EOS_PRESENCE_CREATEPRESENCEMODIFICATION_API_LATEST;
        oModificationOptions.LocalUserId = m_pAccountId;

        // create rich presence change handle
        EOS_HPresenceModification pHandle;
        if(nEOS_Presence_CreatePresenceModification(m_pPresence, &oModificationOptions, &pHandle) == EOS_EResult::EOS_Success)
        {
            // set text options
            EOS_PresenceModification_SetRawRichTextOptions oTextOptions = {};
            oTextOptions.ApiVersion = EOS_PRESENCEMODIFICATION_SETRAWRICHTEXT_API_LATEST;
            oTextOptions.RichText   = oPresence.sDefaultText.c_str();

            // add rich presence text
            nEOS_PresenceModification_SetRawRichText(pHandle, &oTextOptions);

            // set status options
            EOS_PresenceModification_SetStatusOptions oStatusOptions = {};
            oStatusOptions.ApiVersion = EOS_PRESENCEMODIFICATION_SETSTATUS_API_LATEST;
            oStatusOptions.Status     = EOS_Presence_EStatus::EOS_PS_Online;

            // add rich presence status
            nEOS_PresenceModification_SetStatus(pHandle, &oStatusOptions);

            if(!oPresence.asValue.empty())
            {
                coreList<EOS_Presence_DataRecord> aRecord;
                aRecord.resize(oPresence.asValue.size());

                FOR_EACH(it, oPresence.asValue)
                {
                    // set data records
                    EOS_Presence_DataRecord& oRecord = aRecord[oPresence.asValue.index(it)];
                    oRecord.ApiVersion = EOS_PRESENCE_DATARECORD_API_LATEST;
                    oRecord.Key        = oPresence.asValue.get_string(it);
                    oRecord.Value      = it->c_str();
                }

                // set data options
                EOS_PresenceModification_SetDataOptions oDataOptions = {};
                oDataOptions.ApiVersion   = EOS_PRESENCEMODIFICATION_SETDATA_API_LATEST;
                oDataOptions.RecordsCount = aRecord.size();
                oDataOptions.Records      = aRecord.data();

                // add rich presence values
                nEOS_PresenceModification_SetData(pHandle, &oDataOptions);
            }

            // set presence options
            EOS_Presence_SetPresenceOptions oSetOptions = {};
            oSetOptions.ApiVersion                 = EOS_PRESENCE_SETPRESENCE_API_LATEST;
            oSetOptions.LocalUserId                = m_pAccountId;
            oSetOptions.PresenceModificationHandle = pHandle;

            // change rich presence
            nEOS_Presence_SetPresence(m_pPresence, &oSetOptions, this, coreBackendEpic::__OnPresenceSetPresence);
            nEOS_PresenceModification_Release(pHandle);
            return true;
        }
    }

    return false;
}


// ****************************************************************
/* get user identifier */
inline const coreChar* coreBackendEpic::GetUserID()const
{
    if(m_pPlatform)
    {
        coreChar  acString[EOS_EPICACCOUNTID_MAX_LENGTH + 1u];
        coreInt32 iSize = ARRAY_SIZE(acString);

        // retrieve Epic account identifier (as string)
        if(nEOS_EpicAccountId_ToString(m_pAccountId, acString, &iSize) == EOS_EResult::EOS_Success)
        {
            return PRINT("%s", acString);
        }
    }

    return this->coreBackend::GetUserID();
}


// ****************************************************************
/* get user name */
inline const coreChar* coreBackendEpic::GetUserName()const
{
    if(m_pPlatform)
    {
        // set user options
        EOS_UserInfo_CopyUserInfoOptions oOptions = {};
        oOptions.ApiVersion   = EOS_USERINFO_COPYUSERINFO_API_LATEST;
        oOptions.LocalUserId  = m_pAccountId;
        oOptions.TargetUserId = m_pAccountId;

        // retrieve user information
        EOS_UserInfo* pUserInfo;
        if(nEOS_UserInfo_CopyUserInfo(m_pUserInfo, &oOptions, &pUserInfo) == EOS_EResult::EOS_Success)
        {
            // copy display name
            const coreChar* pcCopy = PRINT("%s", pUserInfo->DisplayName);
            nEOS_UserInfo_Release(pUserInfo);

            return pcCopy;
        }
    }

    return this->coreBackend::GetUserName();
}


// ****************************************************************
/* get language */
inline const coreChar* coreBackendEpic::GetLanguage()const
{
    if(m_pPlatform)
    {
        coreChar  acLanguage[EOS_LOCALECODE_MAX_LENGTH + 1u];
        coreInt32 iSize = ARRAY_SIZE(acLanguage);

        // retrieve Epic language (ISO 639)
        if(nEOS_Platform_GetActiveLocaleCode(m_pPlatform, m_pAccountId, acLanguage, &iSize) == EOS_EResult::EOS_Success)
        {
            // map Epic language to common language
            if(!std::memcmp(acLanguage, "de", 2u)) return CORE_LANGUAGE_GERMAN;
            if(!std::memcmp(acLanguage, "en", 2u)) return CORE_LANGUAGE_ENGLISH;
            if(!std::memcmp(acLanguage, "es", 2u)) return (!std::strcmp(acLanguage, "es-MX")) ? CORE_LANGUAGE_LATAM : CORE_LANGUAGE_SPANISH;
            if(!std::memcmp(acLanguage, "fr", 2u)) return CORE_LANGUAGE_FRENCH;
            if(!std::memcmp(acLanguage, "it", 2u)) return CORE_LANGUAGE_ITALIAN;
            if(!std::memcmp(acLanguage, "ja", 2u)) return CORE_LANGUAGE_JAPANESE;
            if(!std::memcmp(acLanguage, "ko", 2u)) return CORE_LANGUAGE_KOREAN;
            if(!std::memcmp(acLanguage, "pl", 2u)) return CORE_LANGUAGE_POLISH;
            if(!std::memcmp(acLanguage, "pt", 2u)) return (!std::strcmp(acLanguage, "pt-BR")) ? CORE_LANGUAGE_BRAZILIAN : CORE_LANGUAGE_PORTUGUESE;
            if(!std::memcmp(acLanguage, "ru", 2u)) return CORE_LANGUAGE_RUSSIAN;
            if(!std::memcmp(acLanguage, "uk", 2u)) return CORE_LANGUAGE_UKRAINIAN;
            if(!std::memcmp(acLanguage, "zh", 2u)) return (!std::strcmp(acLanguage, "zh-Hant")) ? CORE_LANGUAGE_TCHINESE : CORE_LANGUAGE_SCHINESE;
        }
    }

    return this->coreBackend::GetLanguage();
}


// ****************************************************************
/* exit the base system */
inline void coreBackendEpic::__ExitBase()
{
    // delete platform instance
    if(m_pPlatform) nEOS_Platform_Release(m_pPlatform);
    m_pPlatform = NULL;

    // shut down Epic library
    nEOS_Shutdown();

    // unload Epic library
    ExitEpicLibrary();
}


// ****************************************************************
/* connect an authenticated user */
inline void coreBackendEpic::__LoginUser()
{
    ASSERT(m_pAccountId)

    // set token options
    EOS_Auth_CopyIdTokenOptions oTokenOptions = {};
    oTokenOptions.ApiVersion = EOS_AUTH_COPYIDTOKEN_API_LATEST;
    oTokenOptions.AccountId  = m_pAccountId;

    // retrieve authentication token
    EOS_Auth_IdToken* pToken = NULL;
    if(nEOS_Auth_CopyIdToken(m_pAuth, &oTokenOptions, &pToken) == EOS_EResult::EOS_Success)
    {
        // set login credentials
        EOS_Connect_Credentials oCredentials = {};
        oCredentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
        oCredentials.Token      = pToken->JsonWebToken;
        oCredentials.Type       = EOS_EExternalCredentialType::EOS_ECT_EPIC_ID_TOKEN;

        // set login options
        EOS_Connect_LoginOptions oLoginOptions = {};
        oLoginOptions.ApiVersion  = EOS_CONNECT_LOGIN_API_LATEST;
        oLoginOptions.Credentials = &oCredentials;

        // login local user
        nEOS_Connect_Login(m_pConnect, &oLoginOptions, this, coreBackendEpic::__OnConnectLogin);

        // free authentication token
        nEOS_Auth_IdToken_Release(pToken);
    }
}


// ****************************************************************
/* query base data */
inline void coreBackendEpic::__QueryData()
{
    ASSERT(m_pAccountId && m_pUserId)

    // set user options
    EOS_UserInfo_QueryUserInfoOptions oUserOptions = {};
    oUserOptions.ApiVersion   = EOS_USERINFO_QUERYUSERINFO_API_LATEST;
    oUserOptions.LocalUserId  = m_pAccountId;
    oUserOptions.TargetUserId = m_pAccountId;

    // query user information
    nEOS_UserInfo_QueryUserInfo(m_pUserInfo, &oUserOptions, this, coreBackendEpic::__OnUserInfoQueryUserInfo);

    // set definition options
    EOS_Achievements_QueryDefinitionsOptions oDefinitionsOptions = {};
    oDefinitionsOptions.ApiVersion  = EOS_ACHIEVEMENTS_QUERYDEFINITIONS_API_LATEST;
    oDefinitionsOptions.LocalUserId = m_pUserId;

    // query achievement definitions
    nEOS_Achievements_QueryDefinitions(m_pAchievements, &oDefinitionsOptions, this, coreBackendEpic::__OnAchievementsQueryDefinitions);

    // set status options
    EOS_Achievements_QueryPlayerAchievementsOptions oAchievementsOptions = {};
    oAchievementsOptions.ApiVersion   = EOS_ACHIEVEMENTS_QUERYPLAYERACHIEVEMENTS_API_LATEST;
    oAchievementsOptions.LocalUserId  = m_pUserId;
    oAchievementsOptions.TargetUserId = m_pUserId;

    // query achievement status
    nEOS_Achievements_QueryPlayerAchievements(m_pAchievements, &oAchievementsOptions, this, coreBackendEpic::__OnAchievementsQueryPlayerAchievements);
}


// ****************************************************************
/* callback handlers */
inline void EOS_CALL coreBackendEpic::__OnAuthLogin(const EOS_Auth_LoginCallbackInfo* pData)
{
    coreBackendEpic* pThis = s_cast<coreBackendEpic*>(pData->ClientData);

    if(pData->ResultCode == EOS_EResult::EOS_Success)
    {
        // save Epic account identifier
        pThis->m_pAccountId = pData->LocalUserId;

        // start user connection
        pThis->__LoginUser();
    }

    // mark authentication as finished
    ADD_BIT(pThis->m_iState, 0u)
}

inline void EOS_CALL coreBackendEpic::__OnConnectLogin(const EOS_Connect_LoginCallbackInfo* pData)
{
    coreBackendEpic* pThis = s_cast<coreBackendEpic*>(pData->ClientData);

    if(pData->ResultCode == EOS_EResult::EOS_InvalidUser)
    {
        // set creation options
        EOS_Connect_CreateUserOptions oOptions = {};
        oOptions.ApiVersion       = EOS_CONNECT_CREATEUSER_API_LATEST;
        oOptions.ContinuanceToken = pData->ContinuanceToken;

        // create product user
        nEOS_Connect_CreateUser(pThis->m_pConnect, &oOptions, pThis, coreBackendEpic::__OnConnectCreateUser);
    }
    else if(pData->ResultCode == EOS_EResult::EOS_Success)
    {
        // save product user identifier
        pThis->m_pUserId = pData->LocalUserId;

        // query base data
        pThis->__QueryData();

        // mark login as finished
        ADD_BIT(pThis->m_iState, 1u)
    }
}

inline void EOS_CALL coreBackendEpic::__OnConnectCreateUser(const EOS_Connect_CreateUserCallbackInfo* pData)
{
    coreBackendEpic* pThis = s_cast<coreBackendEpic*>(pData->ClientData);

    if(pData->ResultCode == EOS_EResult::EOS_Success)
    {
        // save product user identifier
        pThis->m_pUserId = pData->LocalUserId;

        // query base data
        pThis->__QueryData();

        // mark login as finished
        ADD_BIT(pThis->m_iState, 1u)
    }
}

inline void EOS_CALL coreBackendEpic::__OnConnectAddNotifyAuthExpiration(const EOS_Connect_AuthExpirationCallbackInfo* pData)
{
    coreBackendEpic* pThis = s_cast<coreBackendEpic*>(pData->ClientData);

    // refresh user connection
    pThis->__LoginUser();
}

inline void EOS_CALL coreBackendEpic::__OnUserInfoQueryUserInfo(const EOS_UserInfo_QueryUserInfoCallbackInfo* pData)
{
    ASSERT(pData->ResultCode == EOS_EResult::EOS_Success)
}

inline void EOS_CALL coreBackendEpic::__OnAchievementsQueryDefinitions(const EOS_Achievements_OnQueryDefinitionsCompleteCallbackInfo* pData)
{
    coreBackendEpic* pThis = s_cast<coreBackendEpic*>(pData->ClientData);

    if(pData->ResultCode == EOS_EResult::EOS_Success)
    {
        // mark achievement definitions as valid
        ADD_BIT(pThis->m_iState, 2u)
    }
}

inline void EOS_CALL coreBackendEpic::__OnAchievementsQueryPlayerAchievements(const EOS_Achievements_OnQueryPlayerAchievementsCompleteCallbackInfo* pData)
{
    coreBackendEpic* pThis = s_cast<coreBackendEpic*>(pData->ClientData);

    if(pData->ResultCode == EOS_EResult::EOS_Success)
    {
        // mark achievement status as valid
        ADD_BIT(pThis->m_iState, 3u)
    }
}

inline void EOS_CALL coreBackendEpic::__OnAchievementsUnlockAchievements(const EOS_Achievements_OnUnlockAchievementsCompleteCallbackInfo* pData)
{
    ASSERT(pData->ResultCode == EOS_EResult::EOS_Success)
}

inline void EOS_CALL coreBackendEpic::__OnStatsIngestStat(const EOS_Stats_IngestStatCompleteCallbackInfo* pData)
{
    ASSERT(pData->ResultCode == EOS_EResult::EOS_Success)
}

inline void EOS_CALL coreBackendEpic::__OnPresenceSetPresence(const EOS_Presence_SetPresenceCallbackInfo* pData)
{
    ASSERT(pData->ResultCode == EOS_EResult::EOS_Success)
}

inline void EOS_CALL coreBackendEpic::__OnUIAddNotifyDisplaySettingsUpdated(const EOS_UI_OnDisplaySettingsUpdatedCallbackInfo* pData)
{
    if(pData->bIsExclusiveInput)
    {
        // notify about focus loss
        SDL_Event oEvent = {SDL_USEREVENT};
        SDL_PushEvent(&oEvent);
    }
}


// ****************************************************************
/* Epic backend instance */
static coreBackendEpic s_BackendEpic;


#endif

#endif /* _CORE_GUARD_EPIC_H_ */