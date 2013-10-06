//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
coreSound::coreSound()noexcept
: m_iBuffer    (0)
, m_iCurSource (0)
, m_pCurRef    (NULL)
{
    memset(&m_Format, 0, sizeof(m_Format));
}

coreSound::coreSound(const char* pcPath)noexcept
: m_iBuffer    (0)
, m_iCurSource (0)
, m_pCurRef    (NULL)
{
    // load from path
    memset(&m_Format, 0, sizeof(m_Format));
    this->coreResource::Load(pcPath);
}

coreSound::coreSound(coreFile* pFile)noexcept
: m_iBuffer    (0)
, m_iCurSource (0)
, m_pCurRef    (NULL)
{
    // load from file
    memset(&m_Format, 0, sizeof(m_Format));
    this->Load(pFile);
}


// ****************************************************************
// destructor
coreSound::~coreSound()
{
    this->Unload();
}


// ****************************************************************
// load sound resource data
coreError coreSound::Load(coreFile* pFile)
{
    SDL_assert(!m_iBuffer);

    if(m_iBuffer) return CORE_INVALID_CALL;
    if(!pFile)    return CORE_INVALID_INPUT;

    char acID[4];
    coreUint iSize;

    // get file data
    const coreByte* pData = pFile->GetData();
    if(!pData) return CORE_FILE_ERROR;

    // read header
    memcpy(acID,   pData, 4); pData += 4;
    memcpy(&iSize, pData, 4); pData += 4;

    // check file format
    if(!strncmp(acID, "RIFF", 4)) {memcpy(acID, pData, 4); pData += 4;}
    if( strncmp(acID, "WAVE", 4))
    {
        Core::Log->Error(0, coreUtils::Print("Sound (%s) is not a valid WAVE-file", pFile->GetPath()));
        return CORE_INVALID_DATA;
    }

    // read sub-chunks
    const coreByte* pSoundData = NULL;
    coreUint iSoundSize = 0;
    while(true)
    {
        memcpy(acID,   pData, 4); pData += 4;
        memcpy(&iSize, pData, 4); pData += 4;

             if(!strncmp(acID, "fmt ", 4)) memcpy(&m_Format, pData, sizeof(m_Format));
        else if(!strncmp(acID, "data", 4)) {pSoundData = pData; iSoundSize = iSize;}
        else break;

        pData += iSize;
    }

    // check for compression
    if(m_Format.iAudioFormat != 1)
    {
        Core::Log->Error(0, coreUtils::Print("Sound (%s) is not PCM encoded, compression is not supported", pFile->GetPath()));
        return CORE_INVALID_DATA;
    }

    // set sound data format
    ALenum iSoundFormat = 0;
    if(m_Format.iNumChannels == 1)
    {
             if(m_Format.iBitsPerSample ==  8) iSoundFormat = AL_FORMAT_MONO8;
        else if(m_Format.iBitsPerSample == 16) iSoundFormat = AL_FORMAT_MONO16;
    }
    else if(m_Format.iNumChannels == 2)
    {
             if(m_Format.iBitsPerSample ==  8) iSoundFormat = AL_FORMAT_STEREO8;
        else if(m_Format.iBitsPerSample == 16) iSoundFormat = AL_FORMAT_STEREO16;
    }

    // create sound buffer
    alGenBuffers(1, &m_iBuffer);
    alBufferData(m_iBuffer, iSoundFormat, pSoundData, iSoundSize, m_Format.iSampleRate);

    // check for errors
    const ALenum iError = alGetError();
    if(iError != AL_NO_ERROR)
    {
        Core::Log->Error(0, coreUtils::Print("Sound (%s) could not be loaded (AL Error Code: %d)", pFile->GetPath(), iError));
        return CORE_INVALID_DATA;
    }

    // save sound attributes
    m_sPath = pFile->GetPath();
    m_iSize = iSoundSize;

    Core::Log->Info(coreUtils::Print("Sound (%s) loaded", m_sPath.c_str()));
    return CORE_OK;
}


// ****************************************************************
// unload sound resource data
coreError coreSound::Unload()
{
    if(!m_iBuffer) return CORE_INVALID_CALL;

    // unbind sound buffer from all used sound sources
    for(auto it = m_aiSource.begin(); it != m_aiSource.end(); ++it)
    {
        const ALuint iSource = this->CheckRef(it->first);
        if(iSource)
        {
#if defined(_CORE_DEBUG_)

            // check for loop property
            int iPlaying; alGetSourcei(iSource, AL_SOURCE_STATE, &iPlaying);
            int iLooping; alGetSourcei(iSource, AL_LOOPING,      &iLooping);
            SDL_assert(!iPlaying || !iLooping);

#endif
            // stop sound source
            alSourceStop(iSource);
            alSourcei(iSource, AL_BUFFER, 0);
        }
    }
    m_aiSource.clear();

    // delete sound buffer
    alDeleteBuffers(1, &m_iBuffer);
    Core::Log->Info(coreUtils::Print("Sound (%s) unloaded", m_sPath.c_str()));

    // reset attributes
    m_sPath      = "";
    m_iSize      = 0;
    m_iBuffer    = 0;
    m_iCurSource = 0;
    m_pCurRef    = NULL;
    memset(&m_Format, 0, sizeof(m_Format));

    return CORE_OK;
}


// ****************************************************************
// play the sound with positional behavior
void coreSound::PlayPosition(const void* pRef, const float& fVolume, const float& fPitch, const float& fPitchRnd, const bool& bLoop, const coreVector3& vPosition)
{
    // set active reference pointer
    m_pCurRef = pRef;
    SDL_assert(m_pCurRef || !bLoop);

    // retrieve next free sound source
    m_iCurSource = Core::Audio->NextSource(m_pCurRef);
    if(m_iCurSource)
    {
        // save sound source
        if(m_pCurRef) m_aiSource[m_pCurRef] = m_iCurSource;

        // set initial sound source properties
        alSourcei(m_iCurSource,  AL_BUFFER,             m_iBuffer);
        alSourcei(m_iCurSource,  AL_SOURCE_RELATIVE,    false);

        alSourcef(m_iCurSource,  AL_GAIN,               fVolume * Core::Config->GetFloat(CORE_CONFIG_AUDIO_VOLUME_SOUND));
        alSourcef(m_iCurSource,  AL_PITCH,              fPitch * (fPitchRnd ? 1.0f+Core::Rand->Float(-fPitchRnd, fPitchRnd) : 1.0f));
        alSourcei(m_iCurSource,  AL_LOOPING,            bLoop);

        alSourcefv(m_iCurSource, AL_POSITION,           vPosition);
        alSourcefv(m_iCurSource, AL_VELOCITY,           coreVector3(0.0f,0.0f,0.0f));
        alSourcef(m_iCurSource,  AL_REFERENCE_DISTANCE, 1.0f);
        alSourcef(m_iCurSource,  AL_MAX_DISTANCE,       5.0f);
        alSourcef(m_iCurSource,  AL_ROLLOFF_FACTOR,     1.0f);

        // start playback
        alSourcePlay(m_iCurSource);
    }
}


// ****************************************************************
// play the sound with relative behavior
void coreSound::PlayRelative(const void* pRef, const float& fVolume, const float& fPitch, const float& fPitchRnd, const bool& bLoop)
{
    // set active reference pointer
    m_pCurRef = pRef;
    SDL_assert(m_pCurRef || !bLoop);

    // retrieve next free sound source
    m_iCurSource = Core::Audio->NextSource(m_pCurRef);
    if(m_iCurSource)
    {
        // save sound source
        if(m_pCurRef) m_aiSource[m_pCurRef] = m_iCurSource;

        // set initial sound source properties
        alSourcei(m_iCurSource, AL_BUFFER,          m_iBuffer);
        alSourcei(m_iCurSource, AL_SOURCE_RELATIVE, true);

        alSourcef(m_iCurSource, AL_GAIN,            fVolume * Core::Config->GetFloat(CORE_CONFIG_AUDIO_VOLUME_SOUND));
        alSourcef(m_iCurSource, AL_PITCH,           fPitch * (fPitchRnd ? 1.0f+Core::Rand->Float(-fPitchRnd, fPitchRnd) : 1.0f));
        alSourcei(m_iCurSource, AL_LOOPING,         bLoop);

        // start playback
        alSourcePlay(m_iCurSource);
    }
}


// ****************************************************************
// stop the sound
void coreSound::Stop()
{
    CORE_SOUND_ASSERT
    if(m_iCurSource)
    {
        // stop sound source
        alSourceStop(m_iCurSource);
        alSourcei(m_iCurSource, AL_BUFFER, 0);

        // remove invalid sound source
        m_aiSource.erase(m_pCurRef);
        m_iCurSource = 0;
    }
}


// ****************************************************************
// get playback status
bool coreSound::IsPlaying()const
{
    if(!m_iCurSource) return false;

    // retrieve current status
    int iStatus;
    alGetSourcei(m_iCurSource, AL_SOURCE_STATE, &iStatus);

    // check for playback
    return (iStatus == AL_PLAYING) ? true : false;
}


// ****************************************************************
// change the sound source position and velocity
void coreSound::SetSource(const coreVector3* pvPosition, const coreVector3* pvVelocity)
{
    CORE_SOUND_ASSERT
    if(m_iCurSource)
    {
#if defined(_CORE_DEBUG_)

        // check for relative property
        int iStatus;
        alGetSourcei(m_iCurSource, AL_SOURCE_RELATIVE, &iStatus);
        SDL_assert(!iStatus);

#endif
        // set position and velocity
        if(pvPosition) alSourcefv(m_iCurSource, AL_POSITION, *pvPosition);
        if(pvVelocity) alSourcefv(m_iCurSource, AL_VELOCITY, *pvVelocity);
    }
}


// ****************************************************************
// change the sound source volume
void coreSound::SetVolume(const float& fVolume)
{
    CORE_SOUND_ASSERT
    if(m_iCurSource) alSourcef(m_iCurSource, AL_GAIN, fVolume * Core::Config->GetFloat(CORE_CONFIG_AUDIO_VOLUME_SOUND));
}


// ****************************************************************
// check reference pointer for valid sound source
ALuint coreSound::CheckRef(const void* pRef)
{
    // check if sound source is available
    if(!pRef) return 0;
    if(!m_aiSource.count(pRef)) return 0;

    // check if sound source is still valid
    const ALuint iSource = Core::Audio->CheckSource(pRef, m_aiSource.at(pRef));
    if(!iSource) m_aiSource.erase(pRef);

    return iSource;
}