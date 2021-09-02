///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

coreSet<coreBackend*> coreBackend::s_apBackendSet = {};

#if defined(_CORE_WINDOWS_)
    #include "backend/steam.h"
    #include "backend/windows.h"
#elif defined(_CORE_LINUX_)
    #include "backend/steam.h"
#endif

static class coreBackendDummy final : public coreBackend {inline const coreChar* GetIdentifier()const final {return "Dummy";}} s_BackendDummy;


// ****************************************************************
/* constructor */
CorePlatform::CorePlatform()noexcept
: m_pBackend     (NULL)
, m_aAchievement {}
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
void CorePlatform::DefineAchievement(const coreHashString& sName, const coreChar* pcSteamName)
{
    // create new achievement definition
    coreAchievement oData = {};
    oData.sSteamName = pcSteamName;

    // add definition to map
    m_aAchievement.emplace_bs(sName, std::move(oData));
}


// ****************************************************************
/* unlock achievement */
void CorePlatform::UnlockAchievement(const coreHashString& sName)
{
    WARN_IF(!m_aAchievement.count_bs(sName)) return;

    // find achievement and update status
    coreAchievement& oData = m_aAchievement.at_bs(sName);
    if(oData.iStatus == 0u)
    {
        oData.iStatus = 1u;
        Core::Log->Info("Achievement (%s) unlocked", sName.GetString());
    }
}


// ****************************************************************
/* get user name */
const coreChar* CorePlatform::GetUserName()const
{
    // get user name from backend
    return m_pBackend->GetUserName();
}


// ****************************************************************
/* get language */
const coreChar* CorePlatform::GetLanguage()const
{
    // get language from backend
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

    // update the backend (actually)
    m_pBackend->Update();
}