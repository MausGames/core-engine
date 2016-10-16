//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SOUND_H_
#define _CORE_GUARD_SOUND_H_

// TODO: improve 3D sound integration and control
// TODO: implement sound pause
// TODO: implement global sound volume change
// TODO: cache and check sound source properties
// TODO: problem with loop on unloaded sound


// ****************************************************************
// sound definitions
#define __CORE_SOUND_ASSERT {ASSERT(this->CheckRef(m_pCurRef) == m_iCurSource)}   //!< may check for missing reference pointer update


// ****************************************************************
// sound class
class coreSound final : public coreResource
{
public:
    //! WAVE-format structure
    struct coreWaveFormat final
    {
        coreUint16 iAudioFormat;     //!< internal audio format (1 = PCM)
        coreUint16 iNumChannels;     //!< number of sound channels (1 = mono | 2 = stereo)
        coreUint32 iSampleRate;      //!< playback frequency (e.g. 44100 Hz)
        coreUint32 iByteRate;        //!< required data transfer rate (iSampleRate * iBlockAlign)
        coreUint16 iBlockAlign;      //!< size per sound frame in bytes (iNumChannels * ((iBitsPerSample + 7) / 8))
        coreUint16 iBitsPerSample;   //!< sample resolution
    };


private:
    ALuint m_iBuffer;                             //!< sound buffer identifier
    coreWaveFormat m_Format;                      //!< format of the sound file

    ALuint m_iCurSource;                          //!< active sound source
    coreLookup<const void*, ALuint> m_aiSource;   //!< currently used sound sources

    const void* m_pCurRef;                        //!< reference pointer to active sound source


public:
    coreSound()noexcept;
    ~coreSound()final;

    DISABLE_COPY(coreSound)

    //! load and unload sound resource data
    //! @{
    coreStatus Load(coreFile* pFile)final;
    coreStatus Unload()final;
    //! @}

    //! control playback
    //! @{
    void PlayPosition(const void* pRef, const coreFloat fVolume, const coreFloat fPitch, const coreFloat fPitchRnd, const coreBool bLoop, const coreVector3& vPosition);
    void PlayRelative(const void* pRef, const coreFloat fVolume, const coreFloat fPitch, const coreFloat fPitchRnd, const coreBool bLoop);
    void Stop();
    coreBool IsPlaying()const;
    //! @}

    //! set various sound source properties
    //! @{
    void SetSource(const coreVector3& vPosition, const coreVector3& vVelocity);
    inline void SetVolume(const coreFloat fVolume) {__CORE_SOUND_ASSERT if(m_iCurSource) alSourcef(m_iCurSource, AL_GAIN,    fVolume); ASSERT(fVolume >= 0.0f)}
    inline void SetPitch (const coreFloat fPitch)  {__CORE_SOUND_ASSERT if(m_iCurSource) alSourcef(m_iCurSource, AL_PITCH,   fPitch);  ASSERT(fPitch  >= 0.0f)}
    inline void SetLoop  (const coreBool  bLoop)   {__CORE_SOUND_ASSERT if(m_iCurSource) alSourcei(m_iCurSource, AL_LOOPING, bLoop);}
    //! @}

    //! enable active sound source with reference pointer
    //! @{
    inline coreBool EnableRef(const void* pRef) {m_pCurRef = pRef; m_iCurSource = this->CheckRef(m_pCurRef); return m_iCurSource ? true : false;}
    ALuint          CheckRef (const void* pRef);
    //! @}

    //! get object properties
    //! @{
    inline const ALuint&         GetBuffer()const {return m_iBuffer;}
    inline const coreWaveFormat& GetFormat()const {return m_Format;}
    //! @}
};


// ****************************************************************
// sound resource access type
using coreSoundPtr = coreResourcePtr<coreSound>;


#endif // _CORE_GUARD_SOUND_H_