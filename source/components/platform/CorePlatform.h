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


// ****************************************************************
/* achievement structure */
struct coreAchievement final
{
    coreString sSteamName;   // internal Steam name
    coreString sEpicName;    // internal Epic name
    coreUint8  iStatus;      // current unlock status (0 = not unlocked | 1 = unlocked | 2 = verified)
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
    virtual coreBool UnlockAchievement(const coreAchievement& oData) {return true;}

    /* process general features */
    virtual const coreChar* GetUserID  ()const {return NULL;}
    virtual const coreChar* GetUserName()const {return NULL;}
    virtual const coreChar* GetLanguage()const {return NULL;}

    /* get backend identifier */
    virtual const coreChar* GetIdentifier()const {return NULL;}
};


// ****************************************************************
/* main platform component */
class CorePlatform final
{
private:
    coreBackend* m_pBackend;                      // selected backend

    coreMapStr<coreAchievement> m_aAchievement;   // achievement data


private:
    CorePlatform()noexcept;
    ~CorePlatform();


public:
    FRIEND_CLASS(Core)
    DISABLE_COPY(CorePlatform)

    /* handle achievements */
    void DefineAchievement(const coreHashString& sName, const coreChar* pcSteamName, const coreChar* pcEpicName);
    void UnlockAchievement(const coreHashString& sName);

    /* handle general features */
    const coreChar* GetUserID  ()const;
    const coreChar* GetUserName()const;
    const coreChar* GetLanguage()const;


private:
    /* update the backend */
    void __UpdateBackend();
};


#endif /* _CORE_GUARD_PLATFORM_H_ */