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
// create sync object
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

        // flush all commands
        glFlush();
    }

    return false;
}


// ****************************************************************
// delete sync object
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
// check for sync object status
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