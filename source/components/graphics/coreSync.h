///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SYNC_H_
#define _CORE_GUARD_SYNC_H_

// TODO 3: implement CheckAsync which uses the function-attachment interface (which thread?)
// TODO 4: wrap GL_TIMEOUT_IGNORED


// ****************************************************************
/* sync definitions */
enum coreSyncCheck : coreUint8
{
    CORE_SYNC_CHECK_NORMAL  = 0u,                           // check only for current status
    CORE_SYNC_CHECK_FLUSHED = GL_SYNC_FLUSH_COMMANDS_BIT    // check and flush the command buffer
};


// ****************************************************************
/* sync class */
class coreSync final
{
private:
    GLsync m_pSync;   // sync object for asynchronous OpenGL operations


public:
    constexpr coreSync()noexcept;
    inline coreSync(coreSync&& m)noexcept;
    ~coreSync();

    /* assignment operations */
    coreSync& operator = (coreSync&& m)noexcept;

    /* handle the sync object */
    coreBool Create();
    void     Delete();

    /* check for sync object status */
    coreStatus Check(const coreUint64 iNanoWait, const coreSyncCheck eCheck);

    /* invoke explicit synchronization */
    static inline void Flush () {glFlush ();}
    static inline void Finish() {glFinish();}
};


// ****************************************************************
/* constructor */
constexpr coreSync::coreSync()noexcept
: m_pSync (NULL)
{
}

inline coreSync::coreSync(coreSync&& m)noexcept
: m_pSync (m.m_pSync)
{
    m.m_pSync = NULL;
}


#endif /* _CORE_GUARD_SYNC_H_ */