///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"
#include "codec/ALAW.h"
#include "codec/MULAW.h"


// ****************************************************************
/* constructor */
coreSound::coreSound(const coreSoundLoad eLoad)noexcept
: coreResource   ()
, m_iBuffer      (0u)
, m_Format       {}
, m_iCurSource   (0u)
, m_aiSource     {}
, m_pCurRef      (NULL)
, m_eLoad        (eLoad)
, m_pDeferStream (NULL)
, m_pDeferData   (NULL)
, m_iDeferOffset (0)
, m_iDeferTotal  (0)
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

    WARN_IF(m_iBuffer)    return CORE_INVALID_CALL;
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetSize()) return CORE_ERROR_FILE;   // do not load file data

    const coreByte* pSoundData = NULL;
    coreUint32      iSoundSize = 0u;
    coreByte*       pTempData  = NULL;

    // extract file extension
    const coreChar* pcExtension = coreData::StrToLower(coreData::StrExtension(pFile->GetPath()));

    // determine audio format
    if(!std::memcmp(pcExtension, "opus", 4u))
    {
        if(!m_pDeferStream)
        {
            coreInt32 iError;

            // open sound stream
            m_pDeferStream = op_open_callbacks(new coreOpusStream(pFile), &g_OpusCallbacks, NULL, 0u, &iError);
            if(!m_pDeferStream)
            {
                Core::Log->Warning("Sound (%s) is not a valid OPUS-file (OP Error Code: %d)", pFile->GetPath(), iError);
                return CORE_INVALID_DATA;
            }

            // disable noise-shaping dithering (to reduce decoding overhead)
            op_set_dither_enabled(m_pDeferStream, 0);

            // set format structure
            m_Format = coreSound::__CreateWaveFormat(CORE_SOUND_FORMAT_PCM, op_channel_count(m_pDeferStream, -1), CORE_MUSIC_OPUS_RATE, 16u);

            // prepare deferred loading
            m_iDeferTotal = op_pcm_total(m_pDeferStream, -1) * m_Format.iNumChannels;
            m_pDeferData  = new coreByte[m_iDeferTotal * sizeof(coreInt16)];
        }

        // limit decoding amount per iteration (with overflow)
        const coreInt32 iLimit = m_iDeferOffset + ((SDL_GetCurrentThreadID() == Core::System->GetMainThread()) ? 100 : 500) * 1024;

        do
        {
            if(m_iDeferOffset >= iLimit) return CORE_BUSY;

            // read and decode data from the sound stream
            const coreInt32 iResult = op_read(m_pDeferStream, r_cast<coreInt16*>(m_pDeferData) + m_iDeferOffset, m_iDeferTotal - m_iDeferOffset, NULL) * m_Format.iNumChannels;

            WARN_IF(iResult <= 0) break;
            m_iDeferOffset += iResult;
        }
        while(m_iDeferOffset < m_iDeferTotal);

        // forward decoded data
        pSoundData = m_pDeferData;
        iSoundSize = m_iDeferTotal * sizeof(coreInt16);

        // handle compression
             if(HAS_FLAG(m_eLoad, CORE_SOUND_LOAD_ALAW))  {if(Core::Audio->GetSupportALAW ()) {coreEncodeALAW (pSoundData, iSoundSize, &pTempData, &iSoundSize); pSoundData = pTempData; m_Format = coreSound::__CreateWaveFormat(CORE_SOUND_FORMAT_ALAW,  m_Format.iNumChannels, m_Format.iSampleRate, 8u);}}
        else if(HAS_FLAG(m_eLoad, CORE_SOUND_LOAD_MULAW)) {if(Core::Audio->GetSupportMULAW()) {coreEncodeMULAW(pSoundData, iSoundSize, &pTempData, &iSoundSize); pSoundData = pTempData; m_Format = coreSound::__CreateWaveFormat(CORE_SOUND_FORMAT_MULAW, m_Format.iNumChannels, m_Format.iSampleRate, 8u);}}
    }
    else if(!std::memcmp(pcExtension, "wav", 3u))
    {
        // get file data
        const coreByte* pData = pFile->GetData();
        const coreByte* pEnd  = pData + pFile->GetSize();

        // check file header
        if(std::memcmp(pData, "RIFF", 4u) || std::memcmp(pData + 8u, "WAVE", 4u))
        {
            Core::Log->Warning("Sound (%s) is not a valid WAVE-file", m_sName.c_str());
            return CORE_INVALID_DATA;
        }
        pData += 12u;

        // read sub-chunks
        while(pData < pEnd)
        {
            coreUint32 iID;   std::memcpy(&iID,   pData, 4u); pData += 4u;
            coreUint32 iSize; std::memcpy(&iSize, pData, 4u); pData += 4u;

                 if(!std::memcmp(&iID, "fmt ", 4u)) {std::memcpy(&m_Format, pData, sizeof(m_Format)); ASSERT(iSize >= sizeof(m_Format))}
            else if(!std::memcmp(&iID, "data", 4u)) {pSoundData = pData; iSoundSize = iSize; iSize = coreMath::CeilAlign(iSize, 2u);}

            pData += iSize;
        }

        // handle compression
             if(m_Format.iAudioFormat == CORE_SOUND_FORMAT_ALAW)  {if(!Core::Audio->GetSupportALAW ()) {coreDecodeALAW (pSoundData, iSoundSize, &pTempData, &iSoundSize); pSoundData = pTempData; m_Format = coreSound::__CreateWaveFormat(CORE_SOUND_FORMAT_PCM, m_Format.iNumChannels, m_Format.iSampleRate, 16u);}}
        else if(m_Format.iAudioFormat == CORE_SOUND_FORMAT_MULAW) {if(!Core::Audio->GetSupportMULAW()) {coreDecodeMULAW(pSoundData, iSoundSize, &pTempData, &iSoundSize); pSoundData = pTempData; m_Format = coreSound::__CreateWaveFormat(CORE_SOUND_FORMAT_PCM, m_Format.iNumChannels, m_Format.iSampleRate, 16u);}}
        else if(m_Format.iAudioFormat == CORE_SOUND_FORMAT_PCM)   {ASSERT(!HAS_FLAG(m_eLoad, CORE_SOUND_LOAD_ALAW) && !HAS_FLAG(m_eLoad, CORE_SOUND_LOAD_MULAW))}
        else
        {
            Core::Log->Warning("Sound (%s) has unsupported audio format %u (valid formats: PCM 1, ALAW 6, MULAW 7)", m_sName.c_str(), m_Format.iAudioFormat);
            return CORE_INVALID_DATA;
        }
    }
    else
    {
        Core::Log->Warning("Sound (%s) could not be identified (valid extensions: opus, wav)", m_sName.c_str());
        return CORE_INVALID_DATA;
    }

    // set sound data format
    ALenum iSoundFormat = 0;
    if(m_Format.iNumChannels == 1u)
    {
             if(m_Format.iAudioFormat   == CORE_SOUND_FORMAT_ALAW)  iSoundFormat = AL_FORMAT_MONO_ALAW_EXT;
        else if(m_Format.iAudioFormat   == CORE_SOUND_FORMAT_MULAW) iSoundFormat = AL_FORMAT_MONO_MULAW_EXT;
        else if(m_Format.iBitsPerSample ==  8u)                     iSoundFormat = AL_FORMAT_MONO8;
        else if(m_Format.iBitsPerSample == 16u)                     iSoundFormat = AL_FORMAT_MONO16;
    }
    else if(m_Format.iNumChannels == 2u)
    {
             if(m_Format.iAudioFormat   == CORE_SOUND_FORMAT_ALAW)  iSoundFormat = AL_FORMAT_STEREO_ALAW_EXT;
        else if(m_Format.iAudioFormat   == CORE_SOUND_FORMAT_MULAW) iSoundFormat = AL_FORMAT_STEREO_MULAW_EXT;
        else if(m_Format.iBitsPerSample ==  8u)                     iSoundFormat = AL_FORMAT_STEREO8;
        else if(m_Format.iBitsPerSample == 16u)                     iSoundFormat = AL_FORMAT_STEREO16;
    }
    ASSERT(iSoundFormat)

    // create sound buffer
    alGenBuffers(1, &m_iBuffer);
    alBufferData(m_iBuffer, iSoundFormat, pSoundData, iSoundSize, m_Format.iSampleRate);
    if(pTempData) SAFE_DELETE_ARRAY(pTempData)

    // clear deferred loading data
    this->__ClearDefer();

    // check for errors
    const ALenum iError = alGetError();
    if(iError != AL_NO_ERROR)
    {
        Core::Log->Warning("Sound (%s) could not be loaded (AL Error Code: 0x%08X)", m_sName.c_str(), iError);
        return CORE_INVALID_DATA;
    }

    // get sound length
    coreFloat fLength = 0.0f;
    if(Core::Audio->GetSupportQuery()) alGetBufferf(m_iBuffer, AL_SEC_LENGTH_SOFT, &fLength);

    // add debug label
    Core::Audio->LabelOpenAL(AL_BUFFER_EXT, m_iBuffer, m_sName.c_str());

    Core::Log->Info("Sound (%s, format %u, %.2f seconds, %u channels, %u bits, %u rate) loaded", m_sName.c_str(), m_Format.iAudioFormat, fLength, m_Format.iNumChannels, m_Format.iBitsPerSample, m_Format.iSampleRate);
    return CORE_OK;
}


// ****************************************************************
/* unload sound resource data */
coreStatus coreSound::Unload()
{
    // clear deferred loading data (always)
    this->__ClearDefer();

    if(!m_iBuffer) return CORE_INVALID_CALL;

    // unbind sound buffer from all audio sources
    Core::Audio->FreeSources(m_iBuffer);
    m_aiSource.clear();

    // delete sound buffer
    alDeleteBuffers(1, &m_iBuffer);
    if(!m_sName.empty()) Core::Log->Info("Sound (%s) unloaded", m_sName.c_str());

    // reset properties
    m_iBuffer    = 0u;
    m_Format     = {};
    m_iCurSource = 0u;
    m_pCurRef    = NULL;

    return CORE_OK;
}


// ****************************************************************
/* play the sound with positional behavior */
void coreSound::PlayPosition(const void* pRef, const coreFloat fVolume, const coreFloat fPitch, const coreBool bLoop, const coreUint8 iType, const coreVector3 vPosition, const coreFloat fRefDistance, const coreFloat fMaxDistance, const coreFloat fRolloff)
{
    WARN_IF(!m_iBuffer) return;

    __CORE_AUDIO_CHECK_VOLUME(fVolume)
    __CORE_AUDIO_CHECK_PITCH (fPitch)

    // set active reference pointer
    m_pCurRef = pRef;
    ASSERT(m_pCurRef || !bLoop)

    // retrieve next free audio source
    m_iCurSource = Core::Audio->NextSource(m_pCurRef, m_iBuffer, fVolume, iType);
    if(m_iCurSource)
    {
        // add debug label
        Core::Audio->LabelOpenAL(AL_SOURCE_EXT, m_iCurSource, m_sName.c_str());

        // save audio source
        if(m_pCurRef) m_aiSource[m_pCurRef] = m_iCurSource;

        Core::Audio->DeferUpdates();
        {
            // set initial audio source properties
            alSourcei (m_iCurSource, AL_BUFFER,             m_iBuffer);
            alSourcei (m_iCurSource, AL_SOURCE_RELATIVE,    false);

            alSourcef (m_iCurSource, AL_PITCH,              fPitch);
            alSourcei (m_iCurSource, AL_LOOPING,            bLoop);

            alSourcefv(m_iCurSource, AL_POSITION,           vPosition.ptr());
            alSourcefv(m_iCurSource, AL_VELOCITY,           coreVector3(0.0f,0.0f,0.0f).ptr());
            alSourcef (m_iCurSource, AL_REFERENCE_DISTANCE, fRefDistance);
            alSourcef (m_iCurSource, AL_MAX_DISTANCE,       fMaxDistance);
            alSourcef (m_iCurSource, AL_ROLLOFF_FACTOR,     fRolloff);
        }
        Core::Audio->ProcessUpdates();

        // start playback
        alSourcePlay(m_iCurSource);
    }
}


// ****************************************************************
/* play the sound with relative behavior */
void coreSound::PlayRelative(const void* pRef, const coreFloat fVolume, const coreFloat fPitch, const coreBool bLoop, const coreUint8 iType)
{
    WARN_IF(!m_iBuffer) return;

    __CORE_AUDIO_CHECK_VOLUME(fVolume)
    __CORE_AUDIO_CHECK_PITCH (fPitch)

    // set active reference pointer
    m_pCurRef = pRef;
    ASSERT(m_pCurRef || !bLoop)

    // retrieve next free audio source
    m_iCurSource = Core::Audio->NextSource(m_pCurRef, m_iBuffer, fVolume, iType);
    if(m_iCurSource)
    {
        // add debug label
        Core::Audio->LabelOpenAL(AL_SOURCE_EXT, m_iCurSource, m_sName.c_str());

        // save audio source
        if(m_pCurRef) m_aiSource[m_pCurRef] = m_iCurSource;

        Core::Audio->DeferUpdates();
        {
            // set initial audio source properties
            alSourcei (m_iCurSource, AL_BUFFER,          m_iBuffer);
            alSourcei (m_iCurSource, AL_SOURCE_RELATIVE, true);

            alSourcef (m_iCurSource, AL_PITCH,           fPitch);
            alSourcei (m_iCurSource, AL_LOOPING,         bLoop);

            alSourcefv(m_iCurSource, AL_POSITION,        coreVector3(0.0f,0.0f,0.0f).ptr());
            alSourcefv(m_iCurSource, AL_VELOCITY,        coreVector3(0.0f,0.0f,0.0f).ptr());
        }
        Core::Audio->ProcessUpdates();

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
void coreSound::SetSource(const coreVector3 vPosition)
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
        // set position
        alSourcefv(m_iCurSource, AL_POSITION, vPosition.ptr());
    }
}

void coreSound::SetSource(const coreVector3 vPosition, const coreVector3 vVelocity)
{
    // set position (with checks)
    this->SetSource(vPosition);

    // set velocity
    if(m_iCurSource) alSourcefv(m_iCurSource, AL_VELOCITY, vVelocity.ptr());
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
    if(Core::Audio->CheckSource(pRef, m_iBuffer, iSource)) return iSource;

    // remove invalid audio source
    m_aiSource.erase(pRef);
    return 0u;
}


// ****************************************************************
/* clear deferred loading data */
void coreSound::__ClearDefer()
{
    if(m_pDeferStream)
    {
        // close sound stream
        op_free(m_pDeferStream);
        m_pDeferStream = NULL;
    }

    // delete target buffer
    SAFE_DELETE_ARRAY(m_pDeferData)
    m_iDeferOffset = 0;
    m_iDeferTotal  = 0;
}


// ****************************************************************
/* create format structure */
coreSound::coreWaveFormat coreSound::__CreateWaveFormat(const coreUint16 iAudioFormat, const coreUint16 iNumChannels, const coreUint32 iSampleRate, const coreUint16 iBitsPerSample)
{
    // calculate missing parameters
    const coreUint16 iBlockAlign = iNumChannels * ((iBitsPerSample + 7u) / 8u);
    const coreUint32 iByteRate   = iSampleRate  * iBlockAlign;

    // create structure
    coreWaveFormat oFormat;
    oFormat.iAudioFormat   = iAudioFormat;
    oFormat.iNumChannels   = iNumChannels;
    oFormat.iSampleRate    = iSampleRate;
    oFormat.iByteRate      = iByteRate;
    oFormat.iBlockAlign    = iBlockAlign;
    oFormat.iBitsPerSample = iBitsPerSample;

    return oFormat;
}