///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_PLATFORM_H_
#define _CORE_GUARD_PLATFORM_H_

// TODO 3: automatically replace parameters in the default rich-text with rich-values (when dirty) (what delimiters? $, %%, {})
// TODO 3: expose more Steam timeline features (e.g. set default rich-text as description) (event priority, durations)
// TODO 3: add a game state for minimized/out-of-focus
// TODO 3: implement rate-limits (specifically for leaderboards), copy code from P1


// ****************************************************************
/* platform file definitions */
using corePlatformFileHandle           = coreUint64;
using corePlatformFileUploadCallback   = std::function<void(const corePlatformFileHandle, const void*)>;
using corePlatformFileDownloadCallback = std::function<void(const corePlatformFileHandle, const coreByte*, const coreUint32, const void*)>;


// ****************************************************************
/* platform achievement definitions */
struct corePlatformAchievement final
{
    coreString sSteamName;    // internal Steam name
    coreString sEpicName;     // internal Epic name
    coreUint8  iStatus;       // current unlock status (0 = not unlocked | 1 = unlocked | 2 = verified)
};


// ****************************************************************
/* platform stat definitions */
struct corePlatformStat final
{
    coreString sSteamName;   // internal Steam name
    coreString sEpicName;    // internal Epic name
    coreInt32  iValue;       // current stat value
    coreBool   bDirty;       // current modification status
};


// ****************************************************************
/* platform leaderboard definitions */
#define CORE_PLATFORM_SCORE_DATA_SIZE (256u)   // max size for score context data

enum corePlatformLeaderboardType : coreUint8
{
    CORE_PLATFORM_LEADERBOARD_TYPE_GLOBAL     = 0u,   // request global scores
    CORE_PLATFORM_LEADERBOARD_TYPE_FRIENDS    = 1u,   // request friends-only scores
    CORE_PLATFORM_LEADERBOARD_TYPE_USER       = 2u,   // request global scores around current user
    CORE_PLATFORM_LEADERBOARD_TYPE_USER_CHECK = 3u    // request only current user score (fast, but only for getting the score value)
};

struct corePlatformScore final
{
    const coreChar*        pcName;                                 // user name
    coreUint32             iRank;                                  // leaderboard rank (based on type)
    coreUint32             iValue;                                 // score value
    coreByte               aData[CORE_PLATFORM_SCORE_DATA_SIZE];   // context data
    coreUint16             iDataSize;                              // context data size
    corePlatformFileHandle iFileHandle;                            // file attachment handle
};

using corePlatformScoreUploadCallback   = std::function<void(const coreBool, const void*)>;
using corePlatformScoreDownloadCallback = std::function<void(const corePlatformScore*, const coreUint32, const coreUint32, const void*)>;

struct corePlatformScoreUpload final
{
    coreUint32                      iValue;                                 // score value
    coreByte                        aData[CORE_PLATFORM_SCORE_DATA_SIZE];   // context data
    coreUint16                      iDataSize;                              // context data size
    corePlatformFileHandle          iFileHandle;                            // file attachment handle
    corePlatformScoreUploadCallback nCallback;                              // callback function
};

struct corePlatformScoreDownload final
{
    corePlatformLeaderboardType       eType;        // leaderboard request type
    coreInt32                         iRangeFrom;   // start range (based on type)
    coreInt32                         iRangeTo;     // end range   (based on type)
    corePlatformScoreDownloadCallback nCallback;    // callback function
};

struct corePlatformLeaderboard final
{
    coreString                          sSteamName;       // internal Steam name
    coreString                          sSwitchName;      // internal Switch name
    coreList<corePlatformScoreUpload>   aQueueUpload;     // score upload queue
    coreList<corePlatformScoreDownload> aQueueDownload;   // score download queue
};


// ****************************************************************
/* platform presence definitions */
enum corePlatformState : coreUint8
{
    CORE_PLATFORM_STATE_UNDEFINED = 0u,   // initial state
    CORE_PLATFORM_STATE_MENU      = 1u,   // still in main menu
    CORE_PLATFORM_STATE_ACTIVE    = 2u,   // playing actively
    CORE_PLATFORM_STATE_INACTIVE  = 3u    // playing inactively (e.g. pause)
};

struct corePlatformPresence final
{
    corePlatformState          eState;         // current game state
    coreMapStrFull<coreString> asValue;        // rich presence values
    coreString                 sDefaultText;   // rich presence text (default)
    coreString                 sSteamText;     // rich presence text (Steam)
    coreBool                   bDirty;         // current modification status
};


// ****************************************************************
/* backend interface */
class INTERFACE coreBackend
{
private:
    static coreSet<coreBackend*> s_apBackendSet;   // registered backends


protected:
    coreBackend()noexcept {s_apBackendSet.insert(this);}
    ~coreBackend() = default;


public:
    FRIEND_CLASS(CorePlatform)
    DISABLE_COPY(coreBackend)

    /* control the backend */
    virtual coreBool Init  () {return true;}
    virtual void     Exit  () {}
    virtual void     Update() {}

    /* process achievements */
    virtual coreBool UnlockAchievement(const corePlatformAchievement& oEntry) {return true;}

    /* process stats */
    virtual coreBool ModifyStat(const corePlatformStat& oEntry, const coreInt32 iValue) {return true;}

    /* process leaderboards */
    virtual coreBool UploadLeaderboard  (const corePlatformLeaderboard& oEntry, const coreUint32 iValue, const coreByte* pData, const coreUint16 iDataSize, const corePlatformFileHandle iFileHandle, const corePlatformScoreUploadCallback&   nCallback) {return true;}
    virtual coreBool DownloadLeaderboard(const corePlatformLeaderboard& oEntry, const corePlatformLeaderboardType eType, const coreInt32 iRangeFrom, const coreInt32 iRangeTo,                        const corePlatformScoreDownloadCallback& nCallback) {return true;}

    /* process files */
    virtual void     UploadFile  (const coreByte* pData, const coreUint32 iDataSize, const coreChar* pcName, const corePlatformFileUploadCallback   nCallback) {nCallback(0u, NULL);}
    virtual void     DownloadFile(const corePlatformFileHandle iFileHandle,                                  const corePlatformFileDownloadCallback nCallback) {nCallback(0u, NULL, 0u, NULL);}
    virtual coreBool ProgressFile(const corePlatformFileHandle iFileHandle, coreUint32* OUTPUT piCurrent, coreUint32* OUTPUT piTotal)const                     {if(piCurrent) (*piCurrent) = 0u; if(piTotal) (*piTotal) = 0u; return false;}

    /* process presence */
    virtual void     SetGameState   (const corePlatformState eState)                  {}
    virtual coreBool SetRichPresence(const corePlatformPresence& oPresence)           {return true;}
    virtual void     MarkEvent      (const coreChar* pcIcon, const coreChar* pcTitle) {}

    /* process connection state */
    virtual coreBool HasConnection   ()const {return true;}
    virtual coreBool EnsureConnection()      {return true;}

    /* process ownership state */
    virtual coreBool HasOwnership()const {return true;}

    /* process general features */
    virtual const coreChar* GetUserID  ()const {return NULL;}
    virtual const coreChar* GetUserName()const {return NULL;}
    virtual const coreChar* GetLanguage()const {return NULL;}

    /* get backend identifier */
    virtual const coreChar* GetIdentifier()const = 0;
};


// ****************************************************************
/* main platform component */
class CorePlatform final
{
private:
    coreBackend* m_pBackend;                              // selected backend

    coreMapStr<corePlatformAchievement> m_aAchievement;   // achievement data
    coreMapStr<corePlatformStat>        m_aStat;          // stat data
    coreMapStr<corePlatformLeaderboard> m_aLeaderboard;   // leaderboard data

    corePlatformPresence m_Presence;                      // presence data


private:
    CorePlatform()noexcept;
    ~CorePlatform();


public:
    FRIEND_CLASS(Core)
    DISABLE_COPY(CorePlatform)

    /* handle achievements */
    void DefineAchievement(const coreHashString& sName, const coreChar* pcSteamName, const coreChar* pcEpicName);
    void UnlockAchievement(const coreHashString& sName);

    /* handle stats */
    void DefineStat(const coreHashString& sName, const coreChar* pcSteamName, const coreChar* pcEpicName);
    void ModifyStat(const coreHashString& sName, const coreInt32 iValue);

    /* handle leaderboards */
    void DefineLeaderboard  (const coreHashString& sName, const coreChar* pcSteamName, const coreChar* pcSwitchName);
    void UploadLeaderboard  (const coreHashString& sName, const coreUint32 iValue, const coreByte* pData, const coreUint16 iDataSize, const corePlatformFileHandle iFileHandle, corePlatformScoreUploadCallback   nCallback);
    void DownloadLeaderboard(const coreHashString& sName, const corePlatformLeaderboardType eType, const coreInt32 iRangeFrom, const coreInt32 iRangeTo,                        corePlatformScoreDownloadCallback nCallback);

    /* handle files */
    void     UploadFile  (const coreByte* pData, const coreUint32 iDataSize, const coreChar* pcName, corePlatformFileUploadCallback   nCallback);
    void     DownloadFile(const corePlatformFileHandle iFileHandle,                                  corePlatformFileDownloadCallback nCallback);
    coreBool ProgressFile(const corePlatformFileHandle iFileHandle, coreUint32* OUTPUT piCurrent, coreUint32* OUTPUT piTotal)const;

    /* handle presence */
    void SetGameState(const corePlatformState eState);
    void SetRichValue(const coreHashString& sKey, const coreChar* pcValue);
    void SetRichText (const coreChar* pcDefaultText, const coreChar* pcSteamText);
    void MarkEvent   (const coreChar* pcIcon, const coreChar* pcTitle);

    /* handle connection state */
    coreBool HasConnection   ()const;
    coreBool EnsureConnection();

    /* handle ownership state */
    coreBool HasOwnership()const;

    /* handle general features */
    const coreChar* GetUserID  ()const;
    const coreChar* GetUserName()const;
    const coreChar* GetLanguage()const;

    /* get backend identifier */
    inline const coreChar* GetIdentifier()const {return m_pBackend->GetIdentifier();}


private:
    /* update the backend */
    void __UpdateBackend();
};


#endif /* _CORE_GUARD_PLATFORM_H_ */