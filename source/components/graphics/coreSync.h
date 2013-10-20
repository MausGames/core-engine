//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SYNC_H_
#define _CORE_GUARD_SYNC_H_


// ****************************************************************
// sync class
class coreSync final
{
private:
    GLsync m_pSync;   //!< sync object for asynchronous OpenGL operations


public:
    constexpr coreSync()noexcept : m_pSync (NULL) {}
    ~coreSync()                                   {this->DeleteSync();}

    //! handle the sync object
    //! @{
    bool CreateSync();
    void DeleteSync();
    //! @}

    //! check for sync object status
    //! @{
    coreError CheckSync(const coreUint& iWait);
    //! @}


private:
    CORE_DISABLE_COPY(coreSync)
};


#endif // _CORE_GUARD_SYNC_H_