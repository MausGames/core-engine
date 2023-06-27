///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
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
coreSync& coreSync::operator = (coreSync&& m)noexcept
{
    // swap properties
    std::swap(m_pSync, m.m_pSync);

    return *this;
}


// ****************************************************************
/* create sync object */
coreBool coreSync::Create(const coreSyncCreate eCreate)
{
    if(!m_pSync)
    {
        if(CORE_GL_SUPPORT(ARB_sync))
        {
            // generate new sync object
            m_pSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0u);
            if(eCreate) glClientWaitSync(m_pSync, GL_SYNC_FLUSH_COMMANDS_BIT, 0u);   // # flush needs to happen on the same thread/context
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
coreStatus coreSync::Check(const coreUint64 iNanoWait)
{
    if(!m_pSync) return CORE_INVALID_CALL;

    // retrieve and compare status
    if(glClientWaitSync(m_pSync, 0u, MAX(iNanoWait, CORE_SYNC_MINIMUM)) != GL_TIMEOUT_EXPIRED)
    {
        // delete sync object
        this->Delete();
        return CORE_OK;
    }

    return CORE_BUSY;
}