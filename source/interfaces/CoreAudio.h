//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef GUARD_CORE_AUDIO_H
#define GUARD_CORE_AUDIO_H


// ****************************************************************
// main audio interface
//! \ingroup interface
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

    std::u_map<ALuint, const void*> m_apSourceRef;   //!< reference pointers currently using sound sources

    float m_fVolume;                                 //!< global volume


private:
    CoreAudio();
    ~CoreAudio();
    friend class Core;


public:
    //! control the listener
    //! @{
    void SetListener(const coreVector3* pvPosition, const coreVector3* pvVelocity, const coreVector3* pvDirection, const coreVector3* pvOrientation);
    void SetListener(const float& fSpeed, const int iTimeID = -1);
    //! @}

    //! control sound source distribution
    //! @{
    ALuint NextSource(const void* pRef);
    inline ALuint CheckSource(const void* pRef, const ALuint& iSource)const {if(!m_apSourceRef.count(iSource)) return 0; return (m_apSourceRef.at(iSource) == pRef) ? iSource : 0;}
    //! @}

    //! set global volume
    //! @{
    inline void SetVolume(const float& fVolume) {if(m_fVolume != fVolume) {m_fVolume = fVolume; alListenerf(AL_GAIN, m_fVolume);}}
    //! @}
};


#endif // GUARD_CORE_AUDIO_H