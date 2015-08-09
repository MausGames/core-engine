//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* destructor */
coreSync::~coreSync()
{
    // delete sync object
    this->Delete();
}


// ****************************************************************
/* assignment operations */
coreSync& coreSync::operator = (coreSync o)noexcept
{
    std::swap(m_pSync, o.m_pSync);
    return *this;
}


// ****************************************************************
/* create sync object */
coreBool coreSync::Create()
{
    if(!m_pSync)
    {
        if(CORE_GL_SUPPORT(ARB_sync))
        {
            // generate new sync object
            m_pSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0u);
            return true;
        }

        // or flush all commands
        coreSync::Flush();
    }

    return false;
}


// ****************************************************************
/* delete sync object */
void coreSync::Delete()
{
    if(m_pSync)
    {
        // delete sync object
        glDeleteSync(m_pSync);
        m_pSync = NULL;
    }
}


// ****************************************************************
/* check for sync object status */
coreStatus coreSync::Check(const coreUint64& iNanoWait, const coreSyncCheck& iCheck)
{
    if(!m_pSync) return CORE_INVALID_CALL;

    // retrieve and compare status
    if(glClientWaitSync(m_pSync, iCheck, iNanoWait) != GL_TIMEOUT_EXPIRED)
    {
        // delete sync object
        this->Delete();
        return CORE_OK;
    }

    return CORE_BUSY;
}