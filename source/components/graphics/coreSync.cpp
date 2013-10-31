//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// create sync object
bool coreSync::Create()
{
    if(!m_pSync)
    {
        // check for available extension
        if(Core::Graphics->SupportFeature("GL_ARB_sync"))
        {
            // generate new sync object
            m_pSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
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
        glDeleteSync(m_pSync);
        m_pSync = NULL;
    }
}


// ****************************************************************
// check for sync object status
coreError coreSync::Check(const coreUint& iWait)
{
    if(!m_pSync) return CORE_INVALID_CALL;

    // retrieve and compare status
    if(glClientWaitSync(m_pSync, GL_SYNC_FLUSH_COMMANDS_BIT, iWait) != GL_TIMEOUT_EXPIRED)
    {
        // delete sync object
        this->Delete();
        return CORE_OK;
    }

    return CORE_BUSY;
}