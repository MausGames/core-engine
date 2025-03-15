///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

coreSet<coreBackend*> coreBackend::s_apBackendSet = {};

#if defined(_CORE_WINDOWS_)
    #include "backend/steam.h"
    #include "backend/epic.h"
    #include "backend/discord.h"
    #include "backend/windows.h"
#elif defined(_CORE_LINUX_)
    #include "backend/steam.h"
    #include "backend/discord.h"
#elif defined(_CORE_MACOS_)
    #include "backend/steam.h"
    #include "backend/epic.h"
    #include "backend/discord.h"
#endif

static class coreBackendDefault final : public coreBackend {inline const coreChar* GetIdentifier()const final {return "Default";} inline coreBool IsPrimary()const final {return true;}} s_BackendDefault;


// ****************************************************************
/* constructor */
CorePlatform::CorePlatform()noexcept
: m_pBackend     (NULL)
, m_aAchievement {}
, m_aStat        {}
, m_aLeaderboard {}
, m_Presence     {}
{
    Core::Log->Header("Platform Interface");

    // loop through all registered backends
    FOR_EACH(it, coreBackend::s_apBackendSet)
    {
        Core::Log->Info("Testing %s backend", (*it)->GetIdentifier());

        // init and select first valid backend
        if((*it)->Init())
        {
            m_pBackend = (*it);
            break;
        }
    }

    ASSERT(m_pBackend)
    Core::Log->Info("Selecting %s backend", m_pBackend->GetIdentifier());
}


// ****************************************************************
/* destructor */
CorePlatform::~CorePlatform()
{
    // exit the backend
    m_pBackend->Exit();

    Core::Log->Info(CORE_LOG_BOLD("Platform Interface shut down"));
}


// ****************************************************************
/* define achievement */
void CorePlatform::DefineAchievement(const coreHashString& sName, const coreChar* pcSteamName, const coreChar* pcEpicName)
{
    // create new achievement definition
    corePlatformAchievement oData = {};
    oData.sSteamName = pcSteamName;
    oData.sEpicName  = pcEpicName;

    // add definition to map
    m_aAchievement.emplace_bs(sName, std::move(oData));
}


// ****************************************************************
/* unlock achievement */
void CorePlatform::UnlockAchievement(const coreHashString& sName)
{
    WARN_IF(!m_aAchievement.count_bs(sName)) return;

    // find achievement and update status
    corePlatformAchievement& oData = m_aAchievement.at_bs(sName);
    if(oData.iStatus == 0u)
    {
        oData.iStatus = 1u;
        Core::Log->Info("Achievement (%s) unlocked", sName.GetString());
    }
}


// ****************************************************************
/* define stat */
void CorePlatform::DefineStat(const coreHashString& sName, const coreChar* pcSteamName, const coreChar* pcEpicName)
{
    // create new stat definition
    corePlatformStat oData = {};
    oData.sSteamName = pcSteamName;
    oData.sEpicName  = pcEpicName;

    // add definition to map
    m_aStat.emplace_bs(sName, std::move(oData));
}


// ****************************************************************
/* modify stat */
void CorePlatform::ModifyStat(const coreHashString& sName, const coreInt32 iValue)
{
    WARN_IF(!m_aStat.count_bs(sName)) return;

    // find stat and update value
    corePlatformStat& oData = m_aStat.at_bs(sName);
    if(oData.iValue != iValue)
    {
        const coreInt32 iOldValue = oData.iValue;

        oData.iValue = iValue;
        oData.bDirty = true;
        Core::Log->Info("Stat (%s, %d -> %d) modified", sName.GetString(), iOldValue, iValue);
    }
}


// ****************************************************************
/* define leaderboard */
void CorePlatform::DefineLeaderboard(const coreHashString& sName, const coreChar* pcSteamName, const coreChar* pcSwitchName)
{
    // create new leaderboard definition
    corePlatformLeaderboard oData = {};
    oData.sSteamName  = pcSteamName;
    oData.sSwitchName = pcSwitchName;

    // add definition to map
    m_aLeaderboard.emplace_bs(sName, std::move(oData));
}


// ****************************************************************
/* upload score value to leaderboard */
void CorePlatform::UploadLeaderboard(const coreHashString& sName, const coreUint32 iValue, const coreByte* pData, const coreUint16 iDataSize, const corePlatformFileHandle iFileHandle, corePlatformScoreUploadCallback nCallback)
{
    WARN_IF(!m_aLeaderboard.count_bs(sName)) return;

    // set upload properties
    corePlatformScoreUpload oEntry = {};
    oEntry.iValue      = iValue;
    oEntry.iDataSize   = iDataSize;
    oEntry.iFileHandle = iFileHandle;
    oEntry.nCallback   = std::move(nCallback);

    // copy context data
    ASSERT(iDataSize <= CORE_PLATFORM_SCORE_DATA_SIZE)
    if(pData && iDataSize) std::memcpy(oEntry.aData, pData, iDataSize);

    // add request to queue
    m_aLeaderboard.at_bs(sName).aQueueUpload.push_back(std::move(oEntry));
    Core::Log->Info("Leaderboard (%s, %u) uploaded", sName.GetString(), iValue);
}


// ****************************************************************
/* download score values from leaderboard */
void CorePlatform::DownloadLeaderboard(const coreHashString& sName, const corePlatformLeaderboardType eType, const coreInt32 iRangeFrom, const coreInt32 iRangeTo, corePlatformScoreDownloadCallback nCallback)
{
    WARN_IF(!m_aLeaderboard.count_bs(sName)) return;

    // set download properties
    corePlatformScoreDownload oEntry = {};
    oEntry.eType      = eType;
    oEntry.iRangeFrom = iRangeFrom;
    oEntry.iRangeTo   = iRangeTo;
    oEntry.nCallback  = std::move(nCallback);

    // add request to queue
    m_aLeaderboard.at_bs(sName).aQueueDownload.push_back(std::move(oEntry));
    Core::Log->Info("Leaderboard (%s, %u, %d-%d) downloaded", sName.GetString(), eType, iRangeFrom, iRangeTo);
}


// ****************************************************************
/* upload file to remote share */
void CorePlatform::UploadFile(const coreByte* pData, const coreUint32 iDataSize, const coreChar* pcName, corePlatformFileUploadCallback nCallback)
{
    ASSERT(pData && iDataSize && pcName)

    m_pBackend->UploadFile(pData, iDataSize, pcName, std::move(nCallback));
    Core::Log->Info("File (%s, %u bytes) uploaded", pcName, iDataSize);
}


// ****************************************************************
/* download file from remote share */
void CorePlatform::DownloadFile(const corePlatformFileHandle iFileHandle, corePlatformFileDownloadCallback nCallback)
{
    ASSERT(iFileHandle)

    m_pBackend->DownloadFile(iFileHandle, std::move(nCallback));
    Core::Log->Info("File (%llu) downloaded", iFileHandle);
}


// ****************************************************************
/* retrieve current file download progress */
coreBool CorePlatform::ProgressFile(const corePlatformFileHandle iFileHandle, coreUint32* OUTPUT piCurrent, coreUint32* OUTPUT piTotal)const
{
    ASSERT(iFileHandle)

    return m_pBackend->ProgressFile(iFileHandle, piCurrent, piTotal);
}


// ****************************************************************
/* set game state */
void CorePlatform::SetGameState(const corePlatformState eState)
{
    if(m_Presence.eState == eState) return;
    m_Presence.eState = eState;

    // just forward new game state
    m_pBackend->SetGameState(eState);
}


// ****************************************************************
/* set rich presence value */
void CorePlatform::SetRichValue(const coreHashString& sKey, const coreChar* pcValue)
{
    if(m_Presence.asValue.count(sKey))
    {
        if(!std::strcmp(m_Presence.asValue.at(sKey).c_str(), pcValue)) return;
    }
    else
    {
        m_Presence.asValue.emplace(sKey);
    }

    // update rich presence
    m_Presence.asValue.at(sKey) = pcValue;
    m_Presence.bDirty           = true;
}


// ****************************************************************
/* set rich presence text */
void CorePlatform::SetRichText(const coreChar* pcDefaultText, const coreChar* pcSteamText)
{
    if(!std::strcmp(m_Presence.sDefaultText.c_str(), pcDefaultText) &&
       !std::strcmp(m_Presence.sSteamText  .c_str(), pcSteamText)) return;

    // update rich presence
    m_Presence.sDefaultText = pcDefaultText;
    m_Presence.sSteamText   = pcSteamText;
    m_Presence.bDirty       = true;
}


// ****************************************************************
/* mark specific event */
void CorePlatform::MarkEvent(const coreChar* pcIcon, const coreChar* pcTitle)
{
    m_pBackend->MarkEvent(pcIcon, pcTitle);
    Core::Log->Info("Event (%s, %s) marked", pcIcon, pcTitle);
}


// ****************************************************************
/* check for network connection (without user-interaction) */
coreBool CorePlatform::HasConnection()const
{
    return m_pBackend->HasConnection();
}


// ****************************************************************
/* try to establish network connection (with user-interaction) */
coreBool CorePlatform::EnsureConnection()
{
    return m_pBackend->EnsureConnection();
}


// ****************************************************************
/* check for ownership */
coreBool CorePlatform::HasOwnership()const
{
    return m_pBackend->HasOwnership();
}


// ****************************************************************
/* get user identifier */
const coreChar* CorePlatform::GetUserID()const
{
    return m_pBackend->GetUserID();
}


// ****************************************************************
/* get user name */
const coreChar* CorePlatform::GetUserName()const
{
    return m_pBackend->GetUserName();
}


// ****************************************************************
/* get language */
const coreChar* CorePlatform::GetLanguage()const
{
    return m_pBackend->GetLanguage();
}


// ****************************************************************
/* update the backend */
void CorePlatform::__UpdateBackend()
{
    // loop through all achievements
    FOR_EACH(it, m_aAchievement)
    {
        // unlock achievement in backend
        if((it->iStatus == 1u) && m_pBackend->UnlockAchievement(*it))
        {
            it->iStatus = 2u;
        }
    }

    // loop through all stats
    FOR_EACH(it, m_aStat)
    {
        // modify stat in backend
        if(it->bDirty && m_pBackend->ModifyStat(*it, it->iValue))
        {
            it->bDirty = false;
        }
    }

    // loop through all leaderboards
    FOR_EACH(it, m_aLeaderboard)
    {
        FOR_EACH_DYN(et, it->aQueueUpload)
        {
            // upload score value in backend
            if(m_pBackend->UploadLeaderboard(*it, et->iValue, et->aData, et->iDataSize, et->iFileHandle, et->nCallback))
                 DYN_REMOVE(et, it->aQueueUpload)
            else DYN_KEEP  (et, it->aQueueUpload)
        }

        FOR_EACH_DYN(et, it->aQueueDownload)
        {
            // download score value in backend
            if(m_pBackend->DownloadLeaderboard(*it, et->eType, et->iRangeFrom, et->iRangeTo, et->nCallback))
                 DYN_REMOVE(et, it->aQueueDownload)
            else DYN_KEEP  (et, it->aQueueDownload)
        }
    }

    // set full rich presence
    if(m_Presence.bDirty && m_pBackend->SetRichPresence(m_Presence))
    {
        m_Presence.bDirty = false;
    }

    // update the backend (actually)
    m_pBackend->Update();
}