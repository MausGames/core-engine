///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_AUDIO_H_
#define _CORE_GUARD_AUDIO_H_


// ****************************************************************
// audio definitions
#define CORE_AUDIO_SOURCES_MUSIC (4u)                                                    //!< number of audio sources for music
#define CORE_AUDIO_SOURCES_SOUND (16u)                                                   //!< number of audio sources for sound
#define CORE_AUDIO_SOURCES       (CORE_AUDIO_SOURCES_MUSIC + CORE_AUDIO_SOURCES_SOUND)   //!< total number of audio sources
#define CORE_AUDIO_MUSIC_BUFFER  (0u)                                                    //!< sound buffer identifier for music


// ****************************************************************
// main audio component
class CoreAudio final
{
private:
    //! audio source data structure
    struct coreSourceData
    {
        ALuint    iBuffer;   //!< current sound buffer (for identification)
        coreFloat fVolume;   //!< current volume
    };


private:
    ALCdevice*  m_pDevice;                              //!< audio device
    ALCcontext* m_pContext;                             //!< primary OpenAL context (for all threads)

    coreVector3 m_vPosition;                            //!< position of the listener
    coreVector3 m_vVelocity;                            //!< velocity of the listener
    coreVector3 m_avDirection[2];                       //!< direction and orientation of the listener

    coreFloat m_afGlobalVolume[3];                      //!< global volume (0 = current | 1 = target | 2 = config reference)
    coreFloat m_afMusicVolume [3];                      //!< music volume
    coreFloat m_afSoundVolume [3];                      //!< sound volume

    ALuint         m_aiSource   [CORE_AUDIO_SOURCES];   //!< audio sources
    coreSourceData m_aSourceData[CORE_AUDIO_SOURCES];   //!< data associated with audio sources

    LPALDEFERUPDATESSOFT   m_nDeferUpdates;             //!< suspend immediate playback state changes
    LPALPROCESSUPDATESSOFT m_nProcessUpdates;           //!< catch-up and resume playback state changes


private:
    CoreAudio()noexcept;
    ~CoreAudio();


public:
    FRIEND_CLASS(Core)
    DISABLE_COPY(CoreAudio)

    //! control the listener
    //! @{
    void SetListener(const coreVector3& vPosition, const coreVector3& vVelocity, const coreVector3& vDirection, const coreVector3& vOrientation);
    void SetListener(const coreFloat fSpeed, const coreInt8 iTimeID = -1);
    //! @}

    //! override current volume
    //! @{
    inline void SetGlobalVolume(const coreFloat fVolume) {m_afGlobalVolume[1] = fVolume;}
    inline void SetMusicVolume (const coreFloat fVolume) {m_afMusicVolume [1] = fVolume;}
    inline void SetSoundVolume (const coreFloat fVolume) {m_afSoundVolume [1] = fVolume;}
    //! @}

    //! control sound playback
    //! @{
    void PauseSound ();
    void ResumeSound();
    void CancelSound();
    //! @}

    //! handle audio sources
    //! @{
    ALuint   NextSource  (const ALuint iBuffer, const coreFloat fVolume);
    void     FreeSources (const ALuint iBuffer);
    void     UpdateSource(const ALuint iSource, const coreFloat fVolume);
    coreBool CheckSource (const ALuint iBuffer, const ALuint iSource)const;
    //! @}

    //! combine playback state changes
    //! @{
    inline void DeferUpdates  ()const {m_nDeferUpdates  ();}
    inline void ProcessUpdates()const {m_nProcessUpdates();}
    //! @}


private:
    //! update all audio sources
    //! @{
    void __UpdateSources();
    //! @}
};


#endif // _CORE_GUARD_AUDIO_H_