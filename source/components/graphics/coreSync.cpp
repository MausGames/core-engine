//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// create sync object
bool coreSync::CreateSync()
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
void coreSync::DeleteSync()
{
    if(m_pSync)
    {
        glDeleteSync(m_pSync);
        m_pSync = NULL;
    }
}


// ****************************************************************
// check for sync object status
coreError coreSync::CheckSync(const coreUint& iWait)
{
    if(!m_pSync) return CORE_INVALID_CALL;

    // retrieve and compare status
    if(glClientWaitSync(m_pSync, GL_SYNC_FLUSH_COMMANDS_BIT, iWait) != GL_TIMEOUT_EXPIRED)
    {
        // delete sync object
        this->DeleteSync();
        return CORE_OK;
    }

    return CORE_BUSY;
}