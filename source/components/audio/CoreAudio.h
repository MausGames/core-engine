//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_AUDIO_H_
#define _CORE_GUARD_AUDIO_H_


// ****************************************************************
// main audio component
//! \ingroup component
class CoreAudio final
{
private:
    ALCdevice* m_pDevice;                            //!< audio device
    ALCcontext* m_pContext;                          //!< OpenAL context

    coreVector3 m_vPosition;                         //!< position of the listener
    coreVector3 m_vVelocity;                         //!< velocity of the listener
    coreVector3 m_avDirection[2];                    //!< direction and orientation of the listener

    ALuint* m_pSource;                               //!< sound sources
    coreByte m_NumSource;                            //!< number of sound sources
    coreByte m_CurSource;                            //!< current sound source

    std::u_map<ALuint, const void*> m_apSourceRef;   //!< reference pointer currently using sound sources

    float m_fVolume;                                 //!< global volume


private:
    CoreAudio()noexcept;
    ~CoreAudio();
    friend class Core;


public:
    //! control the listener
    //! @{
    void SetListener(const coreVector3& vPosition, const coreVector3& vVelocity, const coreVector3& vDirection, const coreVector3& vOrientation);
    void SetListener(const float& fSpeed, const int iTimeID = -1);
    //! @}

    //! distribute sound sources
    //! @{
    ALuint NextSource(const void* pRef);
    inline ALuint CheckSource(const void* pRef, const ALuint& iSource)const {if(!m_apSourceRef.count(iSource)) return 0; return (m_apSourceRef.at(iSource) == pRef) ? iSource : 0;}
    //! @}

    //! set global volume
    //! @{
    inline void SetVolume(const float& fVolume) {if(m_fVolume != fVolume) {m_fVolume = fVolume; alListenerf(AL_GAIN, m_fVolume);}}
    //! @}
};


#endif // _CORE_GUARD_AUDIO_H_