///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_AUDIO_H_
#define _CORE_GUARD_AUDIO_H_

// TODO 5: <old comment style>


// ****************************************************************
/* audio definitions */
#define CORE_AUDIO_SOURCES_MUSIC (4u)                                                    // number of audio sources for music
#define CORE_AUDIO_SOURCES_SOUND (32u)                                                   // number of audio sources for sound
#define CORE_AUDIO_SOURCES       (CORE_AUDIO_SOURCES_MUSIC + CORE_AUDIO_SOURCES_SOUND)   // total number of audio sources
#define CORE_AUDIO_TYPES         (8u)                                                    // number of sound types
#define CORE_AUDIO_MUSIC_BUFFER  (0u)                                                    // sound buffer identifier for music
#define CORE_AUDIO_MAX_GAIN      (4.0f)                                                  // maximum supported gain per audio source

#define __CORE_AUDIO_CHECK_VOLUME(x) {ASSERT(((x) >= 0.0f) && ((x) <= CORE_AUDIO_MAX_GAIN))}
#define __CORE_AUDIO_CHECK_PITCH(x)  {ASSERT(((x) >= 0.5f) && ((x) <= 2.0f))}

enum coreAudioMode : coreUint8
{
    CORE_AUDIO_MODE_AUTO       = 0u,   // auto-detect from system when possible
    CORE_AUDIO_MODE_MONO       = 1u,   // force monaural output     (HRTF disabled)
    CORE_AUDIO_MODE_SPEAKERS   = 2u,   // allow any output          (HRTF disabled)
    CORE_AUDIO_MODE_HEADPHONES = 3u    // force stereophonic output (HRTF enabled)
};


// ****************************************************************
/* main audio component */
class CoreAudio final
{
private:
    /* audio source data structure */
    struct coreSourceData
    {
        const void* pRef;      // current reference pointer (for identification)
        ALuint      iBuffer;   // current sound buffer      (for identification)
        coreFloat   fVolume;   // current volume
        coreUint8   iType;     // sound type (e.g. effect, ambient, voice)
    };


private:
    ALCdevice*  m_pDevice;                              // audio device
    ALCcontext* m_pContext;                             // primary OpenAL context (for all threads)

    coreVector3 m_vPosition;                            // position of the listener
    coreVector3 m_vVelocity;                            // velocity of the listener
    coreVector3 m_avDirection[2];                       // direction and orientation of the listener

    coreFloat m_afGlobalVolume[3];                      // global volume (0 = current | 1 = target | 2 = config reference)
    coreFloat m_afMusicVolume [3];                      // music volume
    coreFloat m_afSoundVolume [3];                      // sound volume

    coreFloat m_afTypeVolume[CORE_AUDIO_TYPES];         // volume for each sound type

    ALuint         m_aiSource   [CORE_AUDIO_SOURCES];   // audio sources
    coreSourceData m_aSourceData[CORE_AUDIO_SOURCES];   // data associated with audio sources

    LPALDEFERUPDATESSOFT   m_nDeferUpdates;             // suspend immediate playback state changes
    LPALPROCESSUPDATESSOFT m_nProcessUpdates;           // catch-up and resume playback state changes
    LPALCRESETDEVICESOFT   m_nResetDevice;              // reset audio device with different attributes

    coreBool m_bSupportALAW;                            // support for A-law compression
    coreBool m_bSupportMULAW;                           // support for MU-law compression
    coreBool m_bSupportFloat;                           // support for raw float-data
    coreBool m_bSupportQuery;                           // support for buffer queries

    ALint m_aiAttributes[11];                           // OpenAL context attributes


private:
    CoreAudio()noexcept;
    ~CoreAudio();


public:
    FRIEND_CLASS(Core)
    DISABLE_COPY(CoreAudio)

    /* control the listener */
    void SetListener(const coreVector3 vPosition, const coreVector3 vVelocity, const coreVector3 vDirection, const coreVector3 vOrientation);
    void SetListener(const coreFloat fSpeed, const coreInt8 iTimeID = -1);

    /* override current volume */
    inline void SetGlobalVolume(const coreFloat fVolume)                        {ASSERT(fVolume >= 0.0f) m_afGlobalVolume[1] = fVolume;}
    inline void SetMusicVolume (const coreFloat fVolume)                        {ASSERT(fVolume >= 0.0f) m_afMusicVolume [1] = fVolume;}
    inline void SetSoundVolume (const coreFloat fVolume)                        {ASSERT(fVolume >= 0.0f) m_afSoundVolume [1] = fVolume;}
    inline void SetTypeVolume  (const coreFloat fVolume, const coreUint8 iType) {ASSERT(fVolume >= 0.0f && iType < CORE_AUDIO_TYPES) if(m_afTypeVolume[iType] != fVolume) {m_afTypeVolume[iType] = fVolume; m_afSoundVolume[0] = -1.0f;}}

    /* control sound playback */
    void PauseSound (const coreUint8 iType = CORE_AUDIO_TYPES);
    void ResumeSound(const coreUint8 iType = CORE_AUDIO_TYPES);
    void CancelSound(const coreUint8 iType = CORE_AUDIO_TYPES);

    /* handle audio sources */
    ALuint   NextSource  (const void* pRef, const ALuint iBuffer, const coreFloat fVolume, const coreUint8 iType);
    void     FreeSources (const ALuint iBuffer);
    void     UpdateSource(const ALuint iSource, const coreFloat fVolume);
    coreBool CheckSource (const void* pRef, const ALuint iBuffer, const ALuint iSource)const;

    /* combine playback state changes */
    inline void DeferUpdates  ()const {m_nDeferUpdates  ();}
    inline void ProcessUpdates()const {m_nProcessUpdates();}

    /* reconfigure audio interface */
    void Reconfigure();

    /* get component properties */
    inline const coreVector3& GetListenerPosition   ()const {return m_vPosition;}
    inline const coreVector3& GetListenerVelocity   ()const {return m_vVelocity;}
    inline const coreVector3& GetListenerDirection  ()const {return m_avDirection[0];}
    inline const coreVector3& GetListenerOrientation()const {return m_avDirection[1];}

    /* check OpenAL properties */
    inline const coreBool& GetSupportALAW ()const {return m_bSupportALAW;}
    inline const coreBool& GetSupportMULAW()const {return m_bSupportMULAW;}
    inline const coreBool& GetSupportFloat()const {return m_bSupportFloat;}
    inline const coreBool& GetSupportQuery()const {return m_bSupportQuery;}


private:
    /* update all audio sources */
    void __UpdateSources();

    /* change resampler of all audio sources */
    void __ChangeResampler(const ALint iResampler);

    /* assemble OpenAL context attributes */
    const ALint* __RetrieveAttributes();
};


#endif /* _CORE_GUARD_AUDIO_H_ */