///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_STEAM_H_
#define _CORE_GUARD_STEAM_H_

#if __has_include(<steam/steam_api.h>)

#if defined(_CORE_MSVC_)
    #pragma warning(disable : 5204)   // class with virtual functions, but without virtual destructor
#else
    #pragma GCC diagnostic ignored "-Wnested-anon-types"
    #pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
    #pragma GCC diagnostic ignored "-Wsigned-enum-bitfield"
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
#elif defined(_CORE_MACOS_)
    #define STEAM_LIBRARY_NAME "libsteam_api.dylib"
#endif

#define __STEAM_DEFINE_FUNCTION(x) static decltype(x)* n ## x = NULL;
#define __STEAM_LOAD_FUNCTION(x)   n ## x = r_cast<decltype(x)*>(coreData::GetAddress(s_pSteamLibrary, #x)); WARN_IF(!n ## x) return false;
#define __STEAM_CALLBACK(x)        case x ## _t::k_iCallback: if(oMessage.m_pubParam) {ASSERT(oMessage.m_cubParam >= coreInt32(sizeof(x ## _t))) this->__On ## x(r_cast<const x ## _t*>(oMessage.m_pubParam));} break;


// ****************************************************************
/* dynamic library loading */
static void* s_pSteamLibrary = NULL;   // Steam library handle

__STEAM_DEFINE_FUNCTION(SteamAPI_GetHSteamPipe)
__STEAM_DEFINE_FUNCTION(SteamAPI_GetHSteamUser)
__STEAM_DEFINE_FUNCTION(SteamAPI_InitFlat)
__STEAM_DEFINE_FUNCTION(SteamAPI_ManualDispatch_FreeLastCallback)
__STEAM_DEFINE_FUNCTION(SteamAPI_ManualDispatch_GetAPICallResult)
__STEAM_DEFINE_FUNCTION(SteamAPI_ManualDispatch_GetNextCallback)
__STEAM_DEFINE_FUNCTION(SteamAPI_ManualDispatch_Init)
__STEAM_DEFINE_FUNCTION(SteamAPI_ManualDispatch_RunFrame)
__STEAM_DEFINE_FUNCTION(SteamAPI_ReleaseCurrentThreadMemory)
__STEAM_DEFINE_FUNCTION(SteamAPI_RestartAppIfNecessary)
__STEAM_DEFINE_FUNCTION(SteamAPI_Shutdown)
__STEAM_DEFINE_FUNCTION(SteamClient)

S_API ISteamTimeline* SteamAPI_SteamTimeline_v001();
__STEAM_DEFINE_FUNCTION(SteamAPI_SteamTimeline_v001)

static coreBool InitSteamLibrary()
{
    ASSERT(!s_pSteamLibrary)

    // open Steam library
    s_pSteamLibrary = coreData::OpenLibrary(STEAM_LIBRARY_NAME);
    if(s_pSteamLibrary)
    {
        // load all required functions
        __STEAM_LOAD_FUNCTION(SteamAPI_GetHSteamPipe)
        __STEAM_LOAD_FUNCTION(SteamAPI_GetHSteamUser)
        __STEAM_LOAD_FUNCTION(SteamAPI_InitFlat)
        __STEAM_LOAD_FUNCTION(SteamAPI_ManualDispatch_FreeLastCallback)
        __STEAM_LOAD_FUNCTION(SteamAPI_ManualDispatch_GetAPICallResult)
        __STEAM_LOAD_FUNCTION(SteamAPI_ManualDispatch_GetNextCallback)
        __STEAM_LOAD_FUNCTION(SteamAPI_ManualDispatch_Init)
        __STEAM_LOAD_FUNCTION(SteamAPI_ManualDispatch_RunFrame)
        __STEAM_LOAD_FUNCTION(SteamAPI_ReleaseCurrentThreadMemory)
        __STEAM_LOAD_FUNCTION(SteamAPI_RestartAppIfNecessary)
        __STEAM_LOAD_FUNCTION(SteamAPI_Shutdown)
        __STEAM_LOAD_FUNCTION(SteamClient)

        nSteamAPI_SteamTimeline_v001 = r_cast<decltype(SteamAPI_SteamTimeline_v001)*>(coreData::GetAddress(s_pSteamLibrary, "SteamAPI_SteamTimeline_v001"));

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
static void S_CALLTYPE WarningMessageCallback(const coreInt32 iSeverity, const coreChar* pcMessage)
{
    Core::Log->Warning(CORE_LOG_BOLD("Steam:") " %s (severity %d)", pcMessage, iSeverity);
}


// ****************************************************************
/* Steam backend class */
class coreBackendSteam final : public coreBackend
{
private:
    /* internal types */
    using coreLeaderboardMap = coreMapStr<SteamLeaderboard_t>;
    using coreAsync          = std::function<void(void*)>;
    using coreAsyncMap       = coreMap<SteamAPICall_t, coreAsync>;


private:
    ISteamClient* m_pClient;                 // main communication instance
    HSteamPipe    m_iPipe;                   // Steam pipe handle
    HSteamUser    m_iUser;                   // Steam user handle

    ISteamApps*          m_pApps;            // application interface
    ISteamFriends*       m_pFriends;         // friends interface
    ISteamRemoteStorage* m_pRemoteStorage;   // remote storage interface
    ISteamTimeline*      m_pTimeline;        // timeline interface
    ISteamUser*          m_pUser;            // user interface
    ISteamUserStats*     m_pUserStats;       // achievement, stats and leaderboard interface
    ISteamUtils*         m_pUtils;           // utility interface

    coreUint8 m_iStatsRequest;               // achievement request status (0 = idle | 1 = send query | 2 = wait on response)
    coreUint8 m_iStatsStore;                 // achievement store status   (0 = idle | 1 = send query | 2 = wait on response)

    coreLeaderboardMap m_aiLeaderboard;      // leaderboard handles

    coreAsyncMap m_anAsyncMap;               // asynchronous callbacks
    coreByte*    m_pAsyncResult;             // callback result memory
    coreUint32   m_iAsyncSize;               // current size of callback result memory


public:
    coreBackendSteam()noexcept;
    ~coreBackendSteam();

    DISABLE_COPY(coreBackendSteam)

    /* control the backend */
    coreBool Init  ()final;
    void     Exit  ()final;
    void     Update()final;

    /* process achievements */
    coreBool UnlockAchievement(const corePlatformAchievement& oEntry)final;

    /* process stats */
    coreBool ModifyStat(const corePlatformStat& oEntry, const coreInt32 iValue)final;

    /* process leaderboards */
    coreBool UploadLeaderboard  (const corePlatformLeaderboard& oEntry, const coreUint32 iValue, const coreByte* pData, const coreUint16 iDataSize, const corePlatformFileHandle iFileHandle, const corePlatformScoreUploadCallback&   nCallback)final;
    coreBool DownloadLeaderboard(const corePlatformLeaderboard& oEntry, const corePlatformLeaderboardType eType, const coreInt32 iRangeFrom, const coreInt32 iRangeTo,                        const corePlatformScoreDownloadCallback& nCallback)final;

    /* process files */
    void     UploadFile  (const coreByte* pData, const coreUint32 iDataSize, const coreChar* pcName, const corePlatformFileUploadCallback   nCallback)final;
    void     DownloadFile(const corePlatformFileHandle iFileHandle,                                  const corePlatformFileDownloadCallback nCallback)final;
    coreBool ProgressFile(const corePlatformFileHandle iFileHandle, coreUint32* OUTPUT piCurrent, coreUint32* OUTPUT piTotal)const final;

    /* process presence */
    void     SetGameState   (const corePlatformState eState)final;
    coreBool SetRichPresence(const corePlatformPresence& oPresence)final;
    void     MarkEvent      (const coreChar* pcIcon, const coreChar* pcTitle)final;

    /* process connection state */
    coreBool HasConnection()const final;

    /* process ownership state */
    coreBool HasOwnership()const final;

    /* process general features */
    const coreChar* GetUserID  ()const final;
    const coreChar* GetUserName()const final;
    const coreChar* GetLanguage()const final;

    /* get backend identifier */
    inline const coreChar* GetIdentifier()const final {return "Steam";}


private:
    /* retrieve leaderboard handle */
    SteamLeaderboard_t __LoadLeaderboard(const corePlatformLeaderboard& oEntry);

    /* callback handlers */
    void __OnGameOverlayActivated(const GameOverlayActivated_t* pResult);
    void __OnUserStatsReceived   (const UserStatsReceived_t*    pResult);
    void __OnUserStatsStored     (const UserStatsStored_t*      pResult);

    /* exit the base system */
    void __ExitBase();
};


// ****************************************************************
/* constructor */
inline coreBackendSteam::coreBackendSteam()noexcept
: coreBackend      ()
, m_pClient        (NULL)
, m_iPipe          (0u)
, m_iUser          (0u)
, m_pApps          (NULL)
, m_pFriends       (NULL)
, m_pRemoteStorage (NULL)
, m_pTimeline      (NULL)
, m_pUser          (NULL)
, m_pUserStats     (NULL)
, m_pUtils         (NULL)
, m_iStatsRequest  (0u)
, m_iStatsStore    (0u)
, m_aiLeaderboard  {}
, m_anAsyncMap     {}
, m_pAsyncResult   (NULL)
, m_iAsyncSize     (0u)
{
}


// ****************************************************************
/* destructor */
inline coreBackendSteam::~coreBackendSteam()
{
    // delete callback result memory
    DYNAMIC_DELETE(m_pAsyncResult)
}


// ****************************************************************
/* init the backend */
inline coreBool coreBackendSteam::Init()
{
    ASSERT(!m_pClient)

    // load Steam library
    if(!CoreApp::Settings::Platform::SteamAppID[CoreApp::Settings::IsDemo()] || !InitSteamLibrary())
    {
        return false;   // DRM free
    }

#if !defined(_CORE_DEBUG_)

    // make sure Steam launched the application
    if(nSteamAPI_RestartAppIfNecessary(CoreApp::Settings::Platform::SteamAppID[CoreApp::Settings::IsDemo()]))
    {
        Core::Log->Warning("Steam will launch the application");
        std::exit(EXIT_SUCCESS);
    }

#endif

    // start up Steam library
    SteamErrMsg acError = {};
    m_pClient = (nSteamAPI_InitFlat(&acError) == k_ESteamAPIInitResult_OK) ? nSteamClient() : NULL;

    // check for library errors
    WARN_IF(!m_pClient)
    {
        this->__ExitBase();

        Core::Log->Warning("Steam could not be initialized (Steam: %s)", acError);
        return false;
    }

    // get communication handles
    m_iPipe = nSteamAPI_GetHSteamPipe();
    m_iUser = nSteamAPI_GetHSteamUser();

    // get communication interfaces
    m_pApps          = m_pClient->GetISteamApps         (m_iUser, m_iPipe, STEAMAPPS_INTERFACE_VERSION);
    m_pFriends       = m_pClient->GetISteamFriends      (m_iUser, m_iPipe, STEAMFRIENDS_INTERFACE_VERSION);
    m_pRemoteStorage = m_pClient->GetISteamRemoteStorage(m_iUser, m_iPipe, STEAMREMOTESTORAGE_INTERFACE_VERSION);
    m_pTimeline      = nSteamAPI_SteamTimeline_v001 ? nSteamAPI_SteamTimeline_v001() : NULL;   // TODO 1: when final, add NULL check below, and remove checks in functions
    m_pUser          = m_pClient->GetISteamUser         (m_iUser, m_iPipe, STEAMUSER_INTERFACE_VERSION);
    m_pUserStats     = m_pClient->GetISteamUserStats    (m_iUser, m_iPipe, STEAMUSERSTATS_INTERFACE_VERSION);
    m_pUtils         = m_pClient->GetISteamUtils        (m_iPipe,          STEAMUTILS_INTERFACE_VERSION);

    // check for interface errors
    WARN_IF(!m_pApps || !m_pFriends || !m_pRemoteStorage || !m_pUser || !m_pUserStats || !m_pUtils)
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

    Core::Log->Info("Steam initialized (app %u, user %s)", m_pUtils->GetAppID(), this->GetUserID());
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
            if(oMessage.m_iCallback == SteamAPICallCompleted_t::k_iCallback)
            {
                const SteamAPICallCompleted_t* pCallback = r_cast<SteamAPICallCompleted_t*>(oMessage.m_pubParam);

                // create callback result memory
                if(m_iAsyncSize < pCallback->m_cubParam)
                {
                    DYNAMIC_RESIZE(m_pAsyncResult, pCallback->m_cubParam)
                    m_iAsyncSize = pCallback->m_cubParam;
                }

                // retrieve callback result
                coreBool bFailed;
                if(nSteamAPI_ManualDispatch_GetAPICallResult(m_iPipe, pCallback->m_hAsyncCall, m_pAsyncResult, pCallback->m_cubParam, pCallback->m_iCallback, &bFailed))
                {
                    ASSERT(!bFailed)

                    // find and execute asynchronous callback
                    const auto it = m_anAsyncMap.find(pCallback->m_hAsyncCall);
                    if(it != m_anAsyncMap.end())
                    {
                        const coreAsync nLocal = std::move(*it);
                        m_anAsyncMap.erase(it);

                        nLocal(m_pAsyncResult);
                    }
                }
            }
            else
            {
                // dispatch to callback handler
                switch(oMessage.m_iCallback)
                {
                __STEAM_CALLBACK(GameOverlayActivated)
                __STEAM_CALLBACK(UserStatsReceived)
                __STEAM_CALLBACK(UserStatsStored)
                }
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
inline coreBool coreBackendSteam::UnlockAchievement(const corePlatformAchievement& oEntry)
{
    if(m_pClient)
    {
        if(m_iStatsRequest == 0u)
        {
            // unlock achievement in Steam
            if(m_pUserStats->SetAchievement(oEntry.sSteamName.c_str()))
            {
                m_iStatsStore = 1u;
            }
            return true;
        }
    }

    return false;
}


// ****************************************************************
/* modify stat */
inline coreBool coreBackendSteam::ModifyStat(const corePlatformStat& oEntry, const coreInt32 iValue)
{
    if(m_pClient)
    {
        if(m_iStatsRequest == 0u)
        {
            // modify stat in Steam
            if(m_pUserStats->SetStat(oEntry.sSteamName.c_str(), iValue))
            {
                m_iStatsStore = 1u;
            }
            return true;
        }
    }

    return false;
}


// ****************************************************************
/* upload score value to leaderboard */
inline coreBool coreBackendSteam::UploadLeaderboard(const corePlatformLeaderboard& oEntry, const coreUint32 iValue, const coreByte* pData, const coreUint16 iDataSize, const corePlatformFileHandle iFileHandle, const corePlatformScoreUploadCallback& nCallback)
{
    if(m_pClient)
    {
        // retrieve leaderboard handle
        const SteamLeaderboard_t iLeaderboard = this->__LoadLeaderboard(oEntry);
        if(iLeaderboard)
        {
            // convert context data size
            const coreInt32 iSize = coreMath::CeilAlign(iDataSize, sizeof(coreInt32)) / sizeof(coreInt32);
            ASSERT(iSize <= k_cLeaderboardDetailsMax)

            // start score upload
            const SteamAPICall_t iHandle1 = m_pUserStats->UploadLeaderboardScore(iLeaderboard, k_ELeaderboardUploadScoreMethodKeepBest, iValue, r_cast<const coreInt32*>(pData), iSize);
            if(iHandle1)
            {
                // add asynchronous callback
                m_anAsyncMap.emplace(iHandle1, [=, this](void* pResult1)
                {
                    const LeaderboardScoreUploaded_t* pStruct1 = s_cast<LeaderboardScoreUploaded_t*>(pResult1);

                    if(iFileHandle && (iFileHandle != k_UGCHandleInvalid))
                    {
                        // attach previously uploaded file
                        const SteamAPICall_t iHandle2 = m_pUserStats->AttachLeaderboardUGC(iLeaderboard, iFileHandle);
                        if(iHandle2)
                        {
                            // add asynchronous callback (nested)
                            m_anAsyncMap.emplace(iHandle2, [](void* pResult2)
                            {
                                const LeaderboardUGCSet_t* pStruct2 = s_cast<LeaderboardUGCSet_t*>(pResult2);

                                // only check for success
                                ASSERT(pStruct2->m_eResult == k_EResultOK)
                            });
                        }
                    }

                    // return results
                    nCallback(pStruct1->m_bSuccess, pResult1);
                });

                return true;
            }
        }
    }

    return false;
}


// ****************************************************************
/* download score values from leaderboard */
inline coreBool coreBackendSteam::DownloadLeaderboard(const corePlatformLeaderboard& oEntry, const corePlatformLeaderboardType eType, const coreInt32 iRangeFrom, const coreInt32 iRangeTo, const corePlatformScoreDownloadCallback& nCallback)
{
    if(m_pClient)
    {
        // retrieve leaderboard handle
        const SteamLeaderboard_t iLeaderboard = this->__LoadLeaderboard(oEntry);
        if(iLeaderboard)
        {
            // convert leaderboard request type
            ELeaderboardDataRequest eRequest;
            switch(eType)
            {
            default: UNREACHABLE
            case CORE_PLATFORM_LEADERBOARD_TYPE_GLOBAL:     eRequest = k_ELeaderboardDataRequestGlobal;           break;
            case CORE_PLATFORM_LEADERBOARD_TYPE_FRIENDS:    eRequest = k_ELeaderboardDataRequestFriends;          break;
            case CORE_PLATFORM_LEADERBOARD_TYPE_USER:       eRequest = k_ELeaderboardDataRequestGlobalAroundUser; break;
            case CORE_PLATFORM_LEADERBOARD_TYPE_USER_CHECK: eRequest = k_ELeaderboardDataRequestGlobalAroundUser; break;
            }

            // start score download
            const SteamAPICall_t iHandle = m_pUserStats->DownloadLeaderboardEntries(iLeaderboard, eRequest, iRangeFrom, iRangeTo);
            if(iHandle)
            {
                // add asynchronous callback
                m_anAsyncMap.emplace(iHandle, [=, this](void* pResult)
                {
                    const LeaderboardScoresDownloaded_t* pStruct = s_cast<LeaderboardScoresDownloaded_t*>(pResult);

                    // allocate output list
                    coreList<corePlatformScore> aScoreCache;
                    aScoreCache.reserve(pStruct->m_cEntryCount);

                    // loop through all leaderboard entries
                    for(coreUintW i = 0u, ie = pStruct->m_cEntryCount; i < ie; ++i)
                    {
                        coreInt32 aiData[k_cLeaderboardDetailsMax] = {};

                        // retrieve leaderboard entry
                        LeaderboardEntry_t oEntry;
                        if(m_pUserStats->GetDownloadedLeaderboardEntry(pStruct->m_hSteamLeaderboardEntries, i, &oEntry, aiData, k_cLeaderboardDetailsMax))
                        {
                            // copy score properties
                            corePlatformScore oNewScore = {};
                            oNewScore.pcName      = m_pFriends->GetFriendPersonaName(oEntry.m_steamIDUser);
                            oNewScore.iRank       = oEntry.m_nGlobalRank;
                            oNewScore.iValue      = oEntry.m_nScore;
                            oNewScore.iDataSize   = oEntry.m_cDetails * sizeof(coreInt32);
                            oNewScore.iFileHandle = (oEntry.m_hUGC != k_UGCHandleInvalid) ? oEntry.m_hUGC : 0u;

                            // copy context data
                            std::memcpy(oNewScore.aData, aiData, MIN(oNewScore.iDataSize, CORE_PLATFORM_SCORE_DATA_SIZE));

                            // add to output list
                            aScoreCache.push_back(oNewScore);
                        }
                    }

                    // return results
                    nCallback(aScoreCache.data(), pStruct->m_cEntryCount, m_pUserStats->GetLeaderboardEntryCount(iLeaderboard), pResult);
                });

                return true;
            }
        }
    }

    return false;
}


// ****************************************************************
/* upload file to remote share */
inline void coreBackendSteam::UploadFile(const coreByte* pData, const coreUint32 iDataSize, const coreChar* pcName, const corePlatformFileUploadCallback nCallback)
{
    if(m_pClient)
    {
        // start file upload
        const SteamAPICall_t iHandle1 = m_pRemoteStorage->FileWriteAsync(pcName, pData, iDataSize);
        if(iHandle1)
        {
            // copy path into lambda
            coreString sNameCopy = pcName;
            ASSERT(!sNameCopy.empty())

            // add asynchronous callback
            m_anAsyncMap.emplace(iHandle1, [=, this, sNameCopy = std::move(sNameCopy)](void* pResult1)
            {
                const RemoteStorageFileWriteAsyncComplete_t* pStruct1 = s_cast<RemoteStorageFileWriteAsyncComplete_t*>(pResult1);

                if(pStruct1->m_eResult == k_EResultOK)
                {
                    // mark file as public (to retrieve file handle)
                    const SteamAPICall_t iHandle2 = m_pRemoteStorage->FileShare(sNameCopy.c_str());
                    if(iHandle2)
                    {
                        // add asynchronous callback (nested)
                        m_anAsyncMap.emplace(iHandle2, [=](void* pResult2)
                        {
                            const RemoteStorageFileShareResult_t* pStruct2 = s_cast<RemoteStorageFileShareResult_t*>(pResult2);

                            // return results
                            nCallback((pStruct2->m_eResult == k_EResultOK) ? pStruct2->m_hFile : 0u, pResult2);
                        });
                    }
                }
                else
                {
                    // return failure
                    nCallback(0u, pResult1);
                }
            });

            return;
        }
    }

    this->coreBackend::UploadFile(pData, iDataSize, pcName, nCallback);
}


// ****************************************************************
/* download file from remote share */
inline void coreBackendSteam::DownloadFile(const corePlatformFileHandle iFileHandle, const corePlatformFileDownloadCallback nCallback)
{
    if(m_pClient)
    {
        // start file download
        const SteamAPICall_t iHandle = m_pRemoteStorage->UGCDownload(iFileHandle, 0u);
        if(iHandle)
        {
            // add asynchronous callback
            m_anAsyncMap.emplace(iHandle, [=, this](void* pResult)
            {
                const RemoteStorageDownloadUGCResult_t* pStruct = s_cast<RemoteStorageDownloadUGCResult_t*>(pResult);

                if(pStruct->m_eResult == k_EResultOK)
                {
                    coreByte* pBuffer = new coreByte[pStruct->m_nSizeInBytes];
                    coreInt32 iOffset = 0u;

                    do
                    {
                        // copy file into memory
                        const coreInt32 iResult = m_pRemoteStorage->UGCRead(pStruct->m_hFile, pBuffer + iOffset, 0x4000, iOffset, k_EUGCRead_ContinueReadingUntilFinished);

                        WARN_IF(iResult <= 0) break;
                        iOffset += iResult;
                    }
                    while(iOffset < pStruct->m_nSizeInBytes);

                    // return results
                    nCallback(pStruct->m_hFile, pBuffer, MIN(iOffset, pStruct->m_nSizeInBytes), pResult);
                    SAFE_DELETE_ARRAY(pBuffer)
                }
                else
                {
                    // return failure
                    nCallback(pStruct->m_hFile, NULL, 0u, pResult);
                }
            });

            return;
        }
    }

    this->coreBackend::DownloadFile(iFileHandle, nCallback);
}


// ****************************************************************
/* retrieve current file download progress */
inline coreBool coreBackendSteam::ProgressFile(const corePlatformFileHandle iFileHandle, coreUint32* OUTPUT piCurrent, coreUint32* OUTPUT piTotal)const
{
    if(m_pClient)
    {
        coreInt32 iCurrent = 0;
        coreInt32 iTotal   = 0;

        // query file download progress
        if(m_pRemoteStorage->GetUGCDownloadProgress(iFileHandle, &iCurrent, &iTotal))
        {
            // return progress values
            if(piCurrent) (*piCurrent) = CLAMP(iCurrent, 0, iTotal);
            if(piTotal)   (*piTotal)   = iTotal;

            return (iTotal > 0);
        }
    }

    return this->coreBackend::ProgressFile(iFileHandle, piCurrent, piTotal);
}


// ****************************************************************
/* set game state */
inline void coreBackendSteam::SetGameState(const corePlatformState eState)
{
    if(m_pClient && m_pTimeline)
    {
        // map game state to timeline game mode
        ETimelineGameMode eMode;
        switch(eState)
        {
        default: UNREACHABLE
        case CORE_PLATFORM_STATE_MENU:     eMode = k_ETimelineGameMode_Menus;   break;
        case CORE_PLATFORM_STATE_ACTIVE:   eMode = k_ETimelineGameMode_Playing; break;
        case CORE_PLATFORM_STATE_INACTIVE: eMode = k_ETimelineGameMode_Staging; break;
        }

        // set timeline game mode
        m_pTimeline->SetTimelineGameMode(eMode);
    }
}


// ****************************************************************
/* set full rich presence */
inline coreBool coreBackendSteam::SetRichPresence(const corePlatformPresence& oPresence)
{
    if(m_pClient)
    {
        ASSERT(oPresence.asValue.size() <= k_cchMaxRichPresenceKeys)

        FOR_EACH(it, oPresence.asValue)
        {
            const coreChar* pcKey   = oPresence.asValue.get_string(it);
            const coreChar* pcValue = it->c_str();

            ASSERT(std::strlen(pcKey)   < k_cchMaxRichPresenceKeyLength)
            ASSERT(std::strlen(pcValue) < k_cchMaxRichPresenceValueLength)

            // set rich presence values
            m_pFriends->SetRichPresence(pcKey, pcValue);
        }

        ASSERT(oPresence.sSteamText.length() < k_cchMaxRichPresenceValueLength)

        // change rich presence (localization token)
        return m_pFriends->SetRichPresence("steam_display", oPresence.sSteamText.c_str());
    }

    return false;
}


// ****************************************************************
/* mark specific event */
inline void coreBackendSteam::MarkEvent(const coreChar* pcIcon, const coreChar* pcTitle)
{
    if(m_pClient && m_pTimeline)
    {
        // add new timeline event
        m_pTimeline->AddTimelineEvent(pcIcon, pcTitle, "", 0u, 0.0f, 0.0f, k_ETimelineEventClipPriority_None);
    }
}


// ****************************************************************
/* check for network connection (without user-interaction) */
inline coreBool coreBackendSteam::HasConnection()const
{
    if(m_pClient)
    {
        // retrieve Steam server connection state
        return m_pUser->BLoggedOn();
    }

    return this->coreBackend::HasConnection();
}


// ****************************************************************
/* check for ownership */
inline coreBool coreBackendSteam::HasOwnership()const
{
    if(m_pClient)
    {
        // retrieve Steam app subscription state
        return m_pApps->BIsSubscribed();
    }

    return this->coreBackend::HasOwnership();
}


// ****************************************************************
/* get user identifier */
inline const coreChar* coreBackendSteam::GetUserID()const
{
    if(m_pClient)
    {
        // retrieve Steam player identifier
        return coreData::ToChars(m_pUser->GetSteamID().GetAccountID());
    }

    return this->coreBackend::GetUserID();
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
        if(pcLanguage)
        {
            // map Steam language to common language
            if(!std::strcmp(pcLanguage, "brazilian"))  return CORE_LANGUAGE_BRAZILIAN;
            if(!std::strcmp(pcLanguage, "english"))    return CORE_LANGUAGE_ENGLISH;
            if(!std::strcmp(pcLanguage, "french"))     return CORE_LANGUAGE_FRENCH;
            if(!std::strcmp(pcLanguage, "german"))     return CORE_LANGUAGE_GERMAN;
            if(!std::strcmp(pcLanguage, "italian"))    return CORE_LANGUAGE_ITALIAN;
            if(!std::strcmp(pcLanguage, "japanese"))   return CORE_LANGUAGE_JAPANESE;
            if(!std::strcmp(pcLanguage, "koreana"))    return CORE_LANGUAGE_KOREAN;   // with a
            if(!std::strcmp(pcLanguage, "latam"))      return CORE_LANGUAGE_LATAM;
            if(!std::strcmp(pcLanguage, "polish"))     return CORE_LANGUAGE_POLISH;
            if(!std::strcmp(pcLanguage, "portuguese")) return CORE_LANGUAGE_PORTUGUESE;
            if(!std::strcmp(pcLanguage, "russian"))    return CORE_LANGUAGE_RUSSIAN;
            if(!std::strcmp(pcLanguage, "schinese"))   return CORE_LANGUAGE_SCHINESE;
            if(!std::strcmp(pcLanguage, "spanish"))    return CORE_LANGUAGE_SPANISH;
            if(!std::strcmp(pcLanguage, "tchinese"))   return CORE_LANGUAGE_TCHINESE;
            if(!std::strcmp(pcLanguage, "ukrainian"))  return CORE_LANGUAGE_UKRAINIAN;
        }
    }

    return this->coreBackend::GetLanguage();
}


// ****************************************************************
/* retrieve leaderboard handle */
inline SteamLeaderboard_t coreBackendSteam::__LoadLeaderboard(const corePlatformLeaderboard& oEntry)
{
    // create hash-string
    const coreHashString sName = oEntry.sSteamName.c_str();

    if(!m_aiLeaderboard.count_bs(sName))
    {
        m_aiLeaderboard.emplace_bs(sName, 0u);

        // find Steam leaderboard
        const SteamAPICall_t iHandle = m_pUserStats->FindLeaderboard(sName.GetString());
        if(iHandle)
        {
            // add asynchronous callback
            m_anAsyncMap.emplace(iHandle, [=, this](void* pResult)
            {
                const LeaderboardFindResult_t* pStruct = s_cast<LeaderboardFindResult_t*>(pResult);

                // store leaderboard handle
                ASSERT(pStruct->m_bLeaderboardFound)
                m_aiLeaderboard.at_bs(sName) = pStruct->m_hSteamLeaderboard;
            });
        }
    }

    return m_aiLeaderboard.bs(sName);
}


// ****************************************************************
/* callback handlers */
inline void coreBackendSteam::__OnGameOverlayActivated(const GameOverlayActivated_t* pResult)
{
    if(pResult->m_bActive)
    {
        // notify about focus loss
        SDL_Event oEvent = {SDL_USEREVENT};
        SDL_PushEvent(&oEvent);
    }
}

inline void coreBackendSteam::__OnUserStatsReceived(const UserStatsReceived_t* pResult)
{
    if(pResult->m_nGameID == CoreApp::Settings::Platform::SteamAppID[CoreApp::Settings::IsDemo()])
    {
        // check for success (or try again)
        m_iStatsRequest = (pResult->m_eResult == k_EResultOK) ? 0u : 1u;
    }
}

inline void coreBackendSteam::__OnUserStatsStored(const UserStatsStored_t* pResult)
{
    if(pResult->m_nGameID == CoreApp::Settings::Platform::SteamAppID[CoreApp::Settings::IsDemo()])
    {
        // check for success (or try again)
        m_iStatsStore = (pResult->m_eResult == k_EResultOK) ? 0u : 1u;
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


#endif

#endif /* _CORE_GUARD_STEAM_H_ */