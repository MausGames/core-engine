//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
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
    ALCdevice* m_pDevice;                       //!< audio device
    ALCcontext* m_pContext;                     //!< OpenAL context
                                                
    coreVector3 m_vPosition;                    //!< position of the listener
    coreVector3 m_vVelocity;                    //!< velocity of the listener
    coreVector3 m_avDirection[2];               //!< direction and orientation of the listener
                                                
    ALuint* m_pSource;                          //!< sound sources
    coreByte m_NumSource;                       //!< number of sound sources
    coreByte m_CurSource;                       //!< current sound source

    coreLookupGen<ALuint, ALuint> m_aiBuffer;   //!< sound buffers currently bound to sound sources <source, buffer>

    float m_fVolume;                            //!< global volume


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
    inline bool CheckSource(const ALuint& iBuffer, const ALuint& iSource)const {if(!m_aiBuffer.count(iSource)) return false; return (m_aiBuffer.at(iSource) == iBuffer) ? true : false;}
    ALuint NextSource(const ALuint& iBuffer);
    void ClearSources(const ALuint& iBuffer);
    //! @}

    //! set global volume
    //! @{
    inline void SetVolume(const float& fVolume) {if(m_fVolume != fVolume) {m_fVolume = fVolume; alListenerf(AL_GAIN, m_fVolume);}}
    //! @}
};


#endif // _CORE_GUARD_AUDIO_H_