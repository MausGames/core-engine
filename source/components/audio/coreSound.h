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


// ****************************************************************
// sound definitions
#define __CORE_SOUND_ASSERT {ASSERT(this->CheckRef(m_pCurRef) == m_iCurSource)}   //!< may check for missing reference pointer update


// ****************************************************************
// sound class
// TODO: improve 3D sound integration and control
// TODO: implement sound pause
// TODO: implement global sound volume change
// TODO: cache and check sound source properties
// TODO: problem with loop on unloaded sound
class coreSound final : public coreResource
{
public:
    //! WAVE-format structure
    struct coreWaveFormat
    {
        coreUshort iAudioFormat;     //!< internal audio format (1 = PCM)
        coreUshort iNumChannels;     //!< number of sound channels (1 = mono | 2 = stereo)
        coreUint   iSampleRate;      //!< playback frequency (e.g. 44100 Hz)
        coreUint   iByteRate;        //!< required data transfer rate (iSampleRate * iBlockAlign)
        coreUshort iBlockAlign;      //!< size per sound frame in bytes (iNumChannels * ((iBitsPerSample + 7) / 8))
        coreUshort iBitsPerSample;   //!< sample resolution

        constexpr_func coreWaveFormat()noexcept;
    };


private:
    ALuint m_iBuffer;                             //!< sound buffer identifier
    coreWaveFormat m_Format;                      //!< format of the sound file

    coreLookup<const void*, ALuint> m_aiSource;   //!< currently used sound sources
    ALuint m_iCurSource;                          //!< active sound source

    const void* m_pCurRef;                        //!< reference pointer to active sound source


public:
    coreSound()noexcept;
    ~coreSound();

    DISABLE_COPY(coreSound)

    //! load and unload sound resource data
    //! @{
    coreError Load(coreFile* pFile)override;
    coreError Unload()override;
    //! @}

    //! control playback
    //! @{
    void PlayPosition(const void* pRef, const float& fVolume, const float& fPitch, const float& fPitchRnd, const bool& bLoop, const coreVector3& vPosition);
    void PlayRelative(const void* pRef, const float& fVolume, const float& fPitch, const float& fPitchRnd, const bool& bLoop);
    void Stop();
    bool IsPlaying()const;
    //! @}

    //! set various sound source properties
    //! @{
    void SetSource(const coreVector3& vPosition, const coreVector3& vVelocity);
    inline void SetVolume(const float& fVolume) {__CORE_SOUND_ASSERT if(m_iCurSource) alSourcef(m_iCurSource, AL_GAIN,    fVolume);}
    inline void SetPitch (const float& fPitch)  {__CORE_SOUND_ASSERT if(m_iCurSource) alSourcef(m_iCurSource, AL_PITCH,   fPitch);}
    inline void SetLoop  (const bool&  bLoop)   {__CORE_SOUND_ASSERT if(m_iCurSource) alSourcei(m_iCurSource, AL_LOOPING, bLoop);}
    //! @}

    //! enable active sound source with reference pointer
    //! @{
    inline bool EnableRef(const void* pRef) {m_pCurRef = pRef; m_iCurSource = this->CheckRef(m_pCurRef); return m_iCurSource ? true : false;}
    ALuint      CheckRef (const void* pRef);
    //! @}

    //! get object properties
    //! @{
    inline const ALuint&         GetBuffer()const {return m_iBuffer;}
    inline const coreWaveFormat& GetFormat()const {return m_Format;}
    //! @}
};


// ****************************************************************
// constructor
constexpr_func coreSound::coreWaveFormat::coreWaveFormat()noexcept
: iAudioFormat   (0)
, iNumChannels   (0)
, iSampleRate    (0)
, iByteRate      (0)
, iBlockAlign    (0)
, iBitsPerSample (0)
{
}


// ****************************************************************
// sound resource access type
typedef coreResourcePtr<coreSound> coreSoundPtr;


#endif // _CORE_GUARD_SOUND_H_