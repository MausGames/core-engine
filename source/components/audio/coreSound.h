///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SOUND_H_
#define _CORE_GUARD_SOUND_H_

// TODO 3: implement sound pause
// TODO 3: cache and check audio source properties (reduce OpenAL function calls)
// TODO 5: <old comment style>


// ****************************************************************
/* sound definitions */
#define CORE_SOUND_FORMAT_PCM   (0x0001u)   // linear pulse-code modulation
#define CORE_SOUND_FORMAT_ALAW  (0x0006u)   // logarithmic A-law compression (better proportional distortions for small signals)
#define CORE_SOUND_FORMAT_MULAW (0x0007u)   // logarithmic MU-law compression (slightly larger dynamic range)

#define __CORE_SOUND_ASSERT {ASSERT(this->CheckRef(m_pCurRef) == m_iCurSource)}   // may check for missing reference pointer update


// ****************************************************************
/* sound class */
class coreSound final : public coreResource
{
public:
    /* WAVE-format structure */
    struct coreWaveFormat final
    {
        coreUint16 iAudioFormat;     // internal audio format (1 = PCM | 6 = ALAW | 7 = MULAW)
        coreUint16 iNumChannels;     // number of sound channels (1 = mono | 2 = stereo)
        coreUint32 iSampleRate;      // playback frequency (e.g. 44100 Hz)
        coreUint32 iByteRate;        // required data transfer rate (iSampleRate * iBlockAlign)
        coreUint16 iBlockAlign;      // size per sound frame in bytes (iNumChannels * ((iBitsPerSample + 7) / 8))
        coreUint16 iBitsPerSample;   // sample resolution
    };


private:
    ALuint m_iBuffer;                          // sound buffer object
    coreWaveFormat m_Format;                   // format of the sound file

    ALuint m_iCurSource;                       // active audio source
    coreMap<const void*, ALuint> m_aiSource;   // currently used audio sources

    const void* m_pCurRef;                     // reference pointer to active audio source


public:
    coreSound()noexcept;
    ~coreSound()final;

    DISABLE_COPY(coreSound)

    /* load and unload sound resource data */
    coreStatus Load(coreFile* pFile)final;
    coreStatus Unload()final;

    /* control playback */
    void PlayPosition(const void* pRef, const coreFloat fVolume, const coreFloat fPitch, const coreBool bLoop, const coreUint8 iType, const coreVector3 vPosition);
    void PlayRelative(const void* pRef, const coreFloat fVolume, const coreFloat fPitch, const coreBool bLoop, const coreUint8 iType);
    void Stop();
    coreBool IsPlaying();

    /* set various audio source properties */
    void SetSource(const coreVector3 vPosition, const coreVector3 vVelocity);
    inline void SetVolume(const coreFloat fVolume) {__CORE_SOUND_ASSERT if(m_iCurSource) Core::Audio->UpdateSource(m_iCurSource, fVolume); ASSERT(fVolume >= 0.0f)}
    inline void SetPitch (const coreFloat fPitch)  {__CORE_SOUND_ASSERT if(m_iCurSource) alSourcef(m_iCurSource, AL_PITCH,   fPitch);      ASSERT(fPitch  >= 0.0f)}
    inline void SetLoop  (const coreBool  bLoop)   {__CORE_SOUND_ASSERT if(m_iCurSource) alSourcei(m_iCurSource, AL_LOOPING, bLoop);}

    /* enable active audio source with reference pointer */
    inline coreBool EnableRef(const void* pRef) {m_pCurRef = pRef; m_iCurSource = this->CheckRef(m_pCurRef); return (m_iCurSource != 0u);}
    ALuint          CheckRef (const void* pRef);

    /* get object properties */
    inline const ALuint&         GetBuffer()const {return m_iBuffer;}
    inline const coreWaveFormat& GetFormat()const {return m_Format;}
};


// ****************************************************************
/* sound resource access type */
using coreSoundPtr = coreResourcePtr<coreSound>;


#endif /* _CORE_GUARD_SOUND_H_ */