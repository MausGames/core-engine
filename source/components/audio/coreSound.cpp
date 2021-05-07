///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"
#include "codecs/ALAW.h"
#include "codecs/MULAW.h"


// ****************************************************************
/* constructor */
coreSound::coreSound()noexcept
: coreResource ()
, m_iBuffer    (0u)
, m_Format     {}
, m_iCurSource (0u)
, m_aiSource   {}
, m_pCurRef    (NULL)
{
}


// ****************************************************************
/* destructor */
coreSound::~coreSound()
{
    this->Unload();
}


// ****************************************************************
/* load sound resource data */
coreStatus coreSound::Load(coreFile* pFile)
{
    coreFileScope oUnloader(pFile);

    WARN_IF(m_iBuffer) return CORE_INVALID_CALL;
    if(!pFile)         return CORE_INVALID_INPUT;

    // get file data
    const coreByte* pData = pFile->GetData();
    const coreByte* pEnd  = pData + pFile->GetSize();
    if(!pData) return CORE_ERROR_FILE;

    // check file header
    if(std::memcmp(pData, "RIFF", 4u) || std::memcmp(pData + 8u, "WAVE", 4u))
    {
        Core::Log->Warning("Sound (%s) is not a valid WAVE-file", pFile->GetPath());
        return CORE_INVALID_DATA;
    }
    pData += 12u;

    // read sub-chunks
    const coreByte* pSoundData = NULL;
    coreUint32      iSoundSize = 0u;
    while(pData < pEnd)
    {
        coreChar   acID[4]; std::memcpy(acID,   pData, 4u); pData += 4u;
        coreUint32 iSize;   std::memcpy(&iSize, pData, 4u); pData += 4u;

             if(!std::memcmp(acID, "fmt ", 4u)) {std::memcpy(&m_Format, pData, sizeof(m_Format));}
        else if(!std::memcmp(acID, "data", 4u)) {pSoundData = pData; iSoundSize = iSize;}

        pData += iSize;
    }

    // handle compression
    coreByte* pRawData = NULL;
         if(m_Format.iAudioFormat == CORE_SOUND_FORMAT_ALAW)  {if(!Core::Audio->GetSupportALAW ()) {coreDecodeALAW (pSoundData, iSoundSize, &pRawData, &iSoundSize); pSoundData = pRawData;}}
    else if(m_Format.iAudioFormat == CORE_SOUND_FORMAT_MULAW) {if(!Core::Audio->GetSupportMULAW()) {coreDecodeMULAW(pSoundData, iSoundSize, &pRawData, &iSoundSize); pSoundData = pRawData;}}
    else if(m_Format.iAudioFormat != CORE_SOUND_FORMAT_PCM)
    {
        Core::Log->Warning("Sound (%s) has unsupported audio format %u (valid formats: PCM 1, ALAW 6, MULAW 7)", pFile->GetPath(), m_Format.iAudioFormat);
        return CORE_INVALID_DATA;
    }

    // set sound data format
    ALenum iSoundFormat = 0;
    if(m_Format.iNumChannels == 1u)
    {
             if(pRawData)                                           iSoundFormat = AL_FORMAT_MONO16;
        else if(m_Format.iAudioFormat   == CORE_SOUND_FORMAT_ALAW)  iSoundFormat = AL_FORMAT_MONO_ALAW_EXT;
        else if(m_Format.iAudioFormat   == CORE_SOUND_FORMAT_MULAW) iSoundFormat = AL_FORMAT_MONO_MULAW_EXT;
        else if(m_Format.iBitsPerSample ==  8u)                     iSoundFormat = AL_FORMAT_MONO8;
        else if(m_Format.iBitsPerSample == 16u)                     iSoundFormat = AL_FORMAT_MONO16;
    }
    else if(m_Format.iNumChannels == 2u)
    {
             if(pRawData)                                           iSoundFormat = AL_FORMAT_STEREO16;
        else if(m_Format.iAudioFormat   == CORE_SOUND_FORMAT_ALAW)  iSoundFormat = AL_FORMAT_STEREO_ALAW_EXT;
        else if(m_Format.iAudioFormat   == CORE_SOUND_FORMAT_MULAW) iSoundFormat = AL_FORMAT_STEREO_MULAW_EXT;
        else if(m_Format.iBitsPerSample ==  8u)                     iSoundFormat = AL_FORMAT_STEREO8;
        else if(m_Format.iBitsPerSample == 16u)                     iSoundFormat = AL_FORMAT_STEREO16;
    }
    ASSERT(iSoundFormat)

    // create sound buffer
    alGenBuffers(1, &m_iBuffer);
    alBufferData(m_iBuffer, iSoundFormat, pSoundData, iSoundSize, m_Format.iSampleRate);
    if(pRawData) SAFE_DELETE_ARRAY(pRawData)

    // save properties
    m_sPath = pFile->GetPath();

    // check for errors
    const ALenum iError = alGetError();
    if(iError != AL_NO_ERROR)
    {
        Core::Log->Warning("Sound (%s) could not be loaded (AL Error Code: 0x%08X)", pFile->GetPath(), iError);
        return CORE_INVALID_DATA;
    }

    Core::Log->Info("Sound (%s, format %u, %u channels, %u bits, %u rate) loaded", pFile->GetPath(), m_Format.iAudioFormat, m_Format.iNumChannels, m_Format.iBitsPerSample, m_Format.iSampleRate);
    return CORE_OK;
}


// ****************************************************************
/* unload sound resource data */
coreStatus coreSound::Unload()
{
    if(!m_iBuffer) return CORE_INVALID_CALL;

    // unbind sound buffer from all audio sources
    Core::Audio->FreeSources(m_iBuffer);
    m_aiSource.clear();

    // delete sound buffer
    alDeleteBuffers(1, &m_iBuffer);
    if(!m_sPath.empty()) Core::Log->Info("Sound (%s) unloaded", m_sPath.c_str());

    // reset properties
    m_sPath      = "";
    m_iBuffer    = 0u;
    m_Format     = {};
    m_iCurSource = 0u;
    m_pCurRef    = NULL;

    return CORE_OK;
}


// ****************************************************************
/* play the sound with positional behavior */
void coreSound::PlayPosition(const void* pRef, const coreFloat fVolume, const coreFloat fPitch, const coreBool bLoop, const coreUint8 iType, const coreVector3& vPosition)
{
    ASSERT(m_iBuffer)

    // set active reference pointer
    m_pCurRef = pRef;
    ASSERT(m_pCurRef || !bLoop)

    // retrieve next free audio source
    m_iCurSource = Core::Audio->NextSource(m_iBuffer, fVolume, iType);
    if(m_iCurSource)
    {
        // save audio source
        if(m_pCurRef) m_aiSource[m_pCurRef] = m_iCurSource;

        // set initial audio source properties
        alSourcei (m_iCurSource, AL_BUFFER,             m_iBuffer);
        alSourcei (m_iCurSource, AL_SOURCE_RELATIVE,    false);

        alSourcef (m_iCurSource, AL_PITCH,              fPitch);
        alSourcei (m_iCurSource, AL_LOOPING,            bLoop);

        alSourcefv(m_iCurSource, AL_POSITION,           vPosition);
        alSourcefv(m_iCurSource, AL_VELOCITY,           coreVector3(0.0f,0.0f,0.0f));
        alSourcef (m_iCurSource, AL_REFERENCE_DISTANCE, 1.0f);
        alSourcef (m_iCurSource, AL_MAX_DISTANCE,       5.0f);
        alSourcef (m_iCurSource, AL_ROLLOFF_FACTOR,     1.0f);

        // start playback
        alSourcePlay(m_iCurSource);
    }
}


// ****************************************************************
/* play the sound with relative behavior */
void coreSound::PlayRelative(const void* pRef, const coreFloat fVolume, const coreFloat fPitch, const coreBool bLoop, const coreUint8 iType)
{
    ASSERT(m_iBuffer)

    // set active reference pointer
    m_pCurRef = pRef;
    ASSERT(m_pCurRef || !bLoop)

    // retrieve next free audio source
    m_iCurSource = Core::Audio->NextSource(m_iBuffer, fVolume, iType);
    if(m_iCurSource)
    {
        // save audio source
        if(m_pCurRef) m_aiSource[m_pCurRef] = m_iCurSource;

        // set initial audio source properties
        alSourcei (m_iCurSource, AL_BUFFER,          m_iBuffer);
        alSourcei (m_iCurSource, AL_SOURCE_RELATIVE, true);

        alSourcef (m_iCurSource, AL_PITCH,           fPitch);
        alSourcei (m_iCurSource, AL_LOOPING,         bLoop);

        alSourcefv(m_iCurSource, AL_POSITION,        coreVector3(0.0f,0.0f,0.0f));
        alSourcefv(m_iCurSource, AL_VELOCITY,        coreVector3(0.0f,0.0f,0.0f));

        // start playback
        alSourcePlay(m_iCurSource);
    }
}


// ****************************************************************
/* stop the sound */
void coreSound::Stop()
{
    __CORE_SOUND_ASSERT
    if(m_iCurSource)
    {
        // stop audio source
        alSourceStop(m_iCurSource);
        alSourcei(m_iCurSource, AL_BUFFER, 0);

        // remove invalid audio source
        m_aiSource.erase(m_pCurRef);
        m_iCurSource = 0u;
    }
}


// ****************************************************************
/* get playback status */
coreBool coreSound::IsPlaying()
{
    __CORE_SOUND_ASSERT
    if(m_iCurSource)
    {
        // retrieve current status
        ALint iStatus;
        alGetSourcei(m_iCurSource, AL_SOURCE_STATE, &iStatus);

        // check for playback
        return (iStatus == AL_PLAYING);
    }
    return false;
}


// ****************************************************************
/* change the audio source position and velocity */
void coreSound::SetSource(const coreVector3& vPosition, const coreVector3& vVelocity)
{
    __CORE_SOUND_ASSERT
    if(m_iCurSource)
    {
#if defined(_CORE_DEBUG_)

        // check for relative property
        ALint iStatus;
        alGetSourcei(m_iCurSource, AL_SOURCE_RELATIVE, &iStatus);
        ASSERT(!iStatus)

#endif
        // set position and velocity
        alSourcefv(m_iCurSource, AL_POSITION, vPosition);
        alSourcefv(m_iCurSource, AL_VELOCITY, vVelocity);
    }
}


// ****************************************************************
/* check reference pointer for valid audio source */
ALuint coreSound::CheckRef(const void* pRef)
{
    // check if audio source is available
    if(!pRef) return 0u;
    if(!m_aiSource.count(pRef)) return 0u;

    // check if audio source is still valid
    const ALuint iSource = m_aiSource.at(pRef);
    if(Core::Audio->CheckSource(m_iBuffer, iSource)) return iSource;

    // remove invalid audio source
    m_aiSource.erase(pRef);
    return 0u;
}