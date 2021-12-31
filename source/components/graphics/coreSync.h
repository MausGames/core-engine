///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SYNC_H_
#define _CORE_GUARD_SYNC_H_

// TODO 3: implement CheckAsync which uses the function-attachment interface (which thread?)
// TODO 3: how to handle MAX_CLIENT_WAIT_TIMEOUT_WEBGL? Firefox returns 1'000'000'000ns (1s), Chromium returns 0ns


// ****************************************************************
/* sync definitions */
#define CORE_SYNC_WAIT_FOREVER (DEFINED(_CORE_EMSCRIPTEN_) ? 0u : GL_TIMEOUT_IGNORED)

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