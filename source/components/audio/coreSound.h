//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_SOUND_H_
#define _CORE_GUARD_SOUND_H_


// ****************************************************************
// sound definitions
#define CORE_SOUND_ASSERT(x) {SDL_assert((x)->CheckRef((x)->m_pCurRef) == (x)->m_iCurSource);}


// ****************************************************************
// sound object class
// TODO: improve 3D sound integration and control
// TODO: implement sound pause
// TODO: implement global sound volume change
// TODO: cache and check sound source properties
class coreSound final : public coreResource
{
public:
    //! WAVE-format structure
    struct coreWaveFormat
    {
        uint16_t iAudioFormat;     //!< internal audio format (1 = PCM)
        uint16_t iNumChannels;     //!< number of sound channels (1 = mono, 2 = stereo)
        uint32_t iSampleRate;      //!< playback frequency (e.g. 44100 Hz)
        uint32_t iByteRate;        //!< required data transfer rate (iSampleRate * iBlockAlign)
        uint16_t iBlockAlign;      //!< size per sound frame in bytes (iNumChannels * ((iBitsPerSample + 7) / 8))
        uint16_t iBitsPerSample;   //!< sample resolution
    };


private:
    ALuint m_iBuffer;                             //!< sound buffer
    coreWaveFormat m_Format;                      //!< format of the sound file

    std::u_map<const void*, ALuint> m_aiSource;   //!< currently used sound sources
    ALuint m_iCurSource;                          //!< active sound source

    const void* m_pCurRef;                        //!< reference pointer to active sound source


public:
    coreSound();
    coreSound(const char* pcPath);
    coreSound(coreFile* pFile);
    ~coreSound();

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
    bool IsPlaying();
    //! @}

    //! control sound source properties
    //! @{
    void SetSource(const coreVector3* pvPosition, const coreVector3* pvVelocity);
    void SetVolume(const float& fVolume);
    inline void SetPitch(const float& fPitch) {CORE_SOUND_ASSERT(this) if(m_iCurSource) alSourcef(m_iCurSource, AL_PITCH,   fPitch);}
    inline void SetLoop(const bool& bLoop)    {CORE_SOUND_ASSERT(this) if(m_iCurSource) alSourcei(m_iCurSource, AL_LOOPING, bLoop);}
    //! @}

    //! control active sound source with reference pointer
    //! @{
    inline bool SetCurRef(const void* pRef) {m_pCurRef = pRef; m_iCurSource = this->CheckRef(m_pCurRef); return m_iCurSource ? true : false;}
    ALuint CheckRef(const void* pRef);
    //! @}

    //! get attributes
    //! @{
    inline const ALuint& GetBuffer()const         {return m_iBuffer;}
    inline const coreWaveFormat& GetFormat()const {return m_Format;}
    //! @}

    //! get relative path to NULL resource
    //! @{
    static inline const char* GetNullPath() {return "data/sounds/default.wav";}
    //! @}
};


// ****************************************************************
// sound resource access type
typedef coreResourcePtr<coreSound> coreSoundPtr;


#endif // _CORE_GUARD_SOUND_H_