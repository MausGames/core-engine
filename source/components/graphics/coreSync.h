//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SYNC_H_
#define _CORE_GUARD_SYNC_H_

// TODO: wrap glFlush and glFinish
// TODO: CheckAsync which uses the function-attachment interface (which thread?)


// ****************************************************************
// sync definitions
enum coreSyncCheck : coreByte
{
    CORE_SYNC_CHECK_ONLY    = 0,                            //!< check only for current status
    CORE_SYNC_CHECK_FLUSHED = GL_SYNC_FLUSH_COMMANDS_BIT    //!< check and flush the command buffer
};


// ****************************************************************
// sync class
class coreSync final
{
private:
    GLsync m_pSync;   //!< sync object for asynchronous OpenGL operations


public:
    constexpr_func coreSync()noexcept : m_pSync (NULL) {}
    ~coreSync() {this->Delete();}

    DISABLE_COPY(coreSync)

    //! handle the sync object
    //! @{
    bool Create();
    void Delete();
    //! @}

    //! check for sync object status
    //! @{
    coreError Check(const coreUint64& iNanoWait, const coreSyncCheck& iCheck);
    //! @}
};


#endif // _CORE_GUARD_SYNC_H_