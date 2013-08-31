//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
// TODO: improve 3D sound integration and control
// TODO: implement sound pause
// TODO: implement global sound volume change


// ****************************************************************
// sound object class
class coreSound final : public coreResource
{
public:
    // WAVE-format struct
    struct coreWaveFormat
    {
        uint16_t iAudioFormat;     // internal audio format (1 = PCM)
        uint16_t iNumChannels;     // number of sound channels (1 = mono, 2 = stereo)
        uint32_t iSampleRate;      // playback frequency (e.g. 44100 Hz)
        uint32_t iByteRate;        // required data transfer rate (iSampleRate * iBlockAlign)
        uint16_t iBlockAlign;      // size per sound frame in bytes (iNumChannels * ((iBitsPerSample + 7) / 8))
        uint16_t iBitsPerSample;   // sample resolution
    };


private:
    ALuint m_iBuffer;                             // sound buffer
    coreWaveFormat m_Format;                      // format of the sound file

    std::u_map<const void*, ALuint> m_aiSource;   // currently used sound sources
    const void* m_pCurRef;                        // active reference pointer (can also be NULL)


public:
    coreSound();
    coreSound(const char* pcPath);
    coreSound(coreFile* pFile);
    ~coreSound();

    // load and unload sound resource data
    coreError Load(coreFile* pFile)override;
    coreError Unload()override;

    // control playback
    void Play(const float& fVolume, const float& fPitch, const float& fPitchRnd, const bool& bLoop, const coreVector3& vPosition);
    void Play(const float& fVolume, const float& fPitch, const float& fPitchRnd, const bool& bLoop);
    void Stop();
    bool IsPlaying();

    // control sound source properties
    void SetSource(const coreVector3* pvPosition, const coreVector3* pvVelocity);
    void SetVolume(const float& fVolume);
    inline void SetPitch(const float& fPitch) {const ALuint iSource = this->CheckRef(m_pCurRef); if(iSource) alSourcef(iSource, AL_PITCH,   fPitch);}
    inline void SetLoop(const bool& bLoop)    {const ALuint iSource = this->CheckRef(m_pCurRef); if(iSource) alSourcei(iSource, AL_LOOPING, bLoop);}

    // control the active reference pointer
    inline void SetCurRef(const void* pRef) {m_pCurRef = pRef;}
    ALuint CheckRef(const void* pRef)const;

    // get attributes
    inline const ALuint& GetBuffer()const         {return m_iBuffer;}
    inline const coreWaveFormat& GetFormat()const {return m_Format;}

    // get relative path to NULL resource
    static inline const char* GetNullPath() {return "";}
};


// ****************************************************************
// sound resource access type
class coreSoundPtr final : public coreResourcePtr<coreSound>
{
public:
    coreSoundPtr(coreResourceHandle* pHandle = NULL) : coreResourcePtr<coreSound>(pHandle) {}
    ~coreSoundPtr() {(*this)->SetLoop(false);}

    // resource access operators (overridden to forward own reference pointer)
    inline coreSound* operator -> ()const {coreSound* pSound = coreResourcePtr<coreSound>::operator -> (); pSound->SetCurRef(this); return  pSound;}
    inline coreSound& operator * ()const  {coreSound* pSound = coreResourcePtr<coreSound>::operator -> (); pSound->SetCurRef(this); return *pSound;}
};