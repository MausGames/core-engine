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
// TODO 3: maybe remove flush on resource-loading and just flush once if work was done in resource-manager
// TODO 3: separate flush and wait on check (and parameter could be just wait true/false, not sure if nanosecond granularity is ever needed)


// ****************************************************************
/* sync definitions */
#define CORE_SYNC_WAIT_FOREVER (DEFINED(_CORE_EMSCRIPTEN_) ? 0u : 50'000'000u)
#define CORE_SYNC_MINIMUM      (DEFINED(_CORE_EMSCRIPTEN_) ? 0u : 1u)

enum coreSyncCreate : coreBool
{
    CORE_SYNC_CREATE_NORMAL  = false,   // create only the sync object         (when staying on the same thread)
    CORE_SYNC_CREATE_FLUSHED = true     // create and flush the command buffer (when switching threads)
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
    coreBool Create(const coreSyncCreate eCreate);
    void     Delete();

    /* check for sync object status */
    coreStatus Check(const coreUint64 iNanoWait);

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
: m_pSync (std::exchange(m.m_pSync, NULL))
{
}


#endif /* _CORE_GUARD_SYNC_H_ */