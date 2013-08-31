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
coreSound::coreSound()
: m_iBuffer (0)
, m_pCurRef (NULL)
{
    memset(&m_Format, 0, sizeof(m_Format));
}

coreSound::coreSound(const char* pcPath)
: m_iBuffer (0)
, m_pCurRef (NULL)
{
    // load from path
    memset(&m_Format, 0, sizeof(m_Format));
    this->coreResource::Load(pcPath);
}

coreSound::coreSound(coreFile* pFile)
: m_iBuffer (0)
, m_pCurRef (NULL)
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
    SDL_assert(pFile != NULL);
    SDL_assert(m_iBuffer == 0);

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
        return CORE_FILE_ERROR;
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
        return CORE_FILE_ERROR;
    }

    // set sound data format
    ALenum iSoundFormat = 0;
    if(m_Format.iNumChannels == 1)
    {
        if(m_Format.iBitsPerSample ==  8) iSoundFormat = AL_FORMAT_MONO8;
        if(m_Format.iBitsPerSample == 16) iSoundFormat = AL_FORMAT_MONO16;
    }
    else if(m_Format.iNumChannels == 2)
    {
        if(m_Format.iBitsPerSample ==  8) iSoundFormat = AL_FORMAT_STEREO8;
        if(m_Format.iBitsPerSample == 16) iSoundFormat = AL_FORMAT_STEREO16;
    }

    // create sound buffer
    alGenBuffers(1, &m_iBuffer);
    alBufferData(m_iBuffer, iSoundFormat, pSoundData, iSoundSize, m_Format.iSampleRate);

    // check for errors
    const ALenum iError = alGetError();
    if(iError != AL_NO_ERROR)
    {
        Core::Log->Error(0, coreUtils::Print("Sound (%s) could not be loaded (AL Error Code: %d)", pFile->GetPath(), iError));
        return CORE_FILE_ERROR;
    }

    // save sound attributes
    m_sPath = pFile->GetPath();
    m_iSize = pFile->GetSize();

    return CORE_OK;
}


// ****************************************************************
// unload sound resource data
coreError coreSound::Unload()
{
    if(!m_iBuffer) return CORE_INVALID_CALL;

    // unbind sound buffer from all associated sound sources
    for(auto it = m_aiSource.begin(); it != m_aiSource.end(); ++it)
    {
        const ALuint iSource = this->CheckRef(it->first);
        if(iSource)
        {
            alSourceStop(iSource);
            alSourcei(iSource, AL_BUFFER, 0);
        }
    }
    m_aiSource.clear();

    // delete sound buffer
    alDeleteBuffers(1, &m_iBuffer);
    Core::Log->Info(coreUtils::Print("Sound (%s) unloaded", m_sPath.c_str()));

    // reset attributes
    m_sPath   = "";
    m_iSize   = 0;
    m_iBuffer = 0;
    memset(&m_Format, 0, sizeof(m_Format));

    return CORE_OK;
}


// ****************************************************************
// play the sound
void coreSound::Play(const float& fVolume, const float& fPitch, const float& fPitchRnd, const bool& bLoop, const coreVector3& vPosition)
{
    SDL_assert(m_pCurRef || !bLoop);

    // retrieve next free sound source
    const ALuint iSource = Core::Audio->NextSource(m_pCurRef);
    if(m_pCurRef) m_aiSource[m_pCurRef] = iSource;
    alSourceStop(iSource);
    
    // set initial sound source properties
    alSourcei(iSource,  AL_BUFFER,             m_iBuffer);
    alSourcei(iSource,  AL_SOURCE_RELATIVE,    false);

    alSourcef(iSource,  AL_GAIN,               fVolume * Core::Config->GetFloat(CORE_CONFIG_AUDIO_VOLUME_SOUND, 0.5f));
    alSourcef(iSource,  AL_PITCH,              fPitch * (fPitchRnd ? 1.0f+Core::Rand->Float(-fPitchRnd, fPitchRnd) : 1.0f));
    alSourcei(iSource,  AL_LOOPING,            bLoop);

    alSourcefv(iSource, AL_POSITION,           vPosition);
    alSourcefv(iSource, AL_VELOCITY,           coreVector3(0.0f,0.0f,0.0f));
    alSourcef(iSource,  AL_REFERENCE_DISTANCE, 1.0f);
    alSourcef(iSource,  AL_MAX_DISTANCE,       5.0f);
    alSourcef(iSource,  AL_ROLLOFF_FACTOR,     1.0f);

    // start playback
    alSourcePlay(iSource);
}

void coreSound::Play(const float& fVolume, const float& fPitch, const float& fPitchRnd, const bool& bLoop)
{
    SDL_assert(m_pCurRef || !bLoop);

    // retrieve next free sound source
    const ALuint iSource = Core::Audio->NextSource(m_pCurRef);
    if(m_pCurRef) m_aiSource[m_pCurRef] = iSource;
    alSourceStop(iSource);
    
    // set initial sound source properties
    alSourcei(iSource,  AL_BUFFER,          m_iBuffer);
    alSourcei(iSource,  AL_SOURCE_RELATIVE, true);

    alSourcef(iSource,  AL_GAIN,            fVolume * Core::Config->GetFloat(CORE_CONFIG_AUDIO_VOLUME_SOUND, 0.5f));
    alSourcef(iSource,  AL_PITCH,           fPitch * (fPitchRnd ? 1.0f+Core::Rand->Float(-fPitchRnd, fPitchRnd) : 1.0f));
    alSourcei(iSource,  AL_LOOPING,         bLoop);

    // start playback
    alSourcePlay(iSource);
}


// ****************************************************************
// stop the sound
void coreSound::Stop()
{
    const ALuint iSource = this->CheckRef(m_pCurRef);
    if(iSource) alSourceStop(iSource);
}


// ****************************************************************
// get playback status
bool coreSound::IsPlaying()
{
    const ALuint iSource = this->CheckRef(m_pCurRef);
    if(!iSource) return false;

    // retrieve current status
    int iStatus;
    alGetSourcei(iSource, AL_SOURCE_STATE, &iStatus);

    return (iStatus == AL_PLAYING) ? true : false;
}


// ****************************************************************
// control the sound source
void coreSound::SetSource(const coreVector3* pvPosition, const coreVector3* pvVelocity)
{
    const ALuint iSource = this->CheckRef(m_pCurRef);

    if(iSource)
    {
        // set position and velocity property
        if(pvPosition) alSourcefv(iSource, AL_POSITION, *pvPosition);
        if(pvVelocity) alSourcefv(iSource, AL_VELOCITY, *pvVelocity);
    }
}


// ****************************************************************
// control the volume
void coreSound::SetVolume(const float& fVolume)
{
    const ALuint iSource = this->CheckRef(m_pCurRef);
    if(iSource) alSourcef(iSource, AL_GAIN, fVolume * Core::Config->GetFloat(CORE_CONFIG_AUDIO_VOLUME_SOUND, 0.5f));
}


// ****************************************************************
// check reference pointer for valid sound source
ALuint coreSound::CheckRef(const void* pRef)const
{
    SDL_assert(pRef);

    if(!m_aiSource.count(pRef)) return 0;
    return Core::Audio->CheckSource(pRef, m_aiSource.at(pRef));
}