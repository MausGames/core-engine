///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
CoreAudio::CoreAudio()noexcept
: m_pDevice         (NULL)
, m_pContext        (NULL)
, m_vPosition       (coreVector3(0.0f,0.0f,0.0f))
, m_vVelocity       (coreVector3(0.0f,0.0f,0.0f))
, m_avDirection     {coreVector3(0.0f,1.0f,0.0f), coreVector3(0.0f,0.0f,1.0f)}
, m_afGlobalVolume  {-1.0f, -1.0f, -1.0f}
, m_afMusicVolume   {-1.0f, -1.0f, -1.0f}
, m_afSoundVolume   {-1.0f, -1.0f, -1.0f}
, m_afTypeVolume    {}
, m_aiSource        {}
, m_aSourceData     {}
, m_nDeferUpdates   (NULL)
, m_nProcessUpdates (NULL)
, m_bSupportALAW    (false)
, m_bSupportMULAW   (false)
{
    Core::Log->Header("Audio Interface");

    // set OpenAL context attributes
    constexpr ALCint aiAttributes[] =
    {
        ALC_FREQUENCY,      48000,
        ALC_MONO_SOURCES,   CORE_AUDIO_SOURCES,
        ALC_STEREO_SOURCES, 0, 0
    };

    // open audio device and create OpenAL context
    m_pDevice  = alcOpenDevice(NULL);
    m_pContext = alcCreateContext(m_pDevice, aiAttributes);

    // activate OpenAL context
    if(!m_pDevice || !m_pContext || !alcMakeContextCurrent(m_pContext))
         Core::Log->Warning("OpenAL context could not be created (ALC Error Code: 0x%08X)", alcGetError(m_pDevice));
    else Core::Log->Info   ("OpenAL context created");

    // generate audio sources
    alGenSources(CORE_AUDIO_SOURCES, m_aiSource);

    // init deferred-updates extension
    if(alIsExtensionPresent("AL_SOFT_deferred_updates"))
    {
        m_nDeferUpdates   = r_cast<LPALDEFERUPDATESSOFT>  (alGetProcAddress("alDeferUpdatesSOFT"));
        m_nProcessUpdates = r_cast<LPALPROCESSUPDATESSOFT>(alGetProcAddress("alProcessUpdatesSOFT"));
    }
    if(!m_nDeferUpdates || !m_nProcessUpdates)
    {
        m_nDeferUpdates   = []() {alcSuspendContext(alcGetCurrentContext());};
        m_nProcessUpdates = []() {alcProcessContext(alcGetCurrentContext());};
    }

    // init direct-channels extension
    if(alIsExtensionPresent("AL_SOFT_direct_channels"))
    {
        for(coreUintW i = 0u; i < CORE_AUDIO_SOURCES; ++i)
            alSourcei(m_aiSource[i], AL_DIRECT_CHANNELS_SOFT, true);
    }

    // init format extensions
    if(alIsExtensionPresent("AL_EXT_ALAW"))  m_bSupportALAW  = true;
    if(alIsExtensionPresent("AL_EXT_MULAW")) m_bSupportMULAW = true;

    // reset listener
    this->DeferUpdates();
    {
        alListenerfv(AL_POSITION,    m_vPosition);
        alListenerfv(AL_VELOCITY,    m_vVelocity);
        alListenerfv(AL_ORIENTATION, m_avDirection[0]);
        alListenerf (AL_GAIN,        0.0f);
    }
    this->ProcessUpdates();

    // reset audio sources
    this->__UpdateSources();

    // reset sound types
    for(coreUintW i = 0u; i < CORE_AUDIO_TYPES; ++i)
        m_afTypeVolume[i] = 1.0f;

    // log audio device information
    Core::Log->ListStartInfo("Audio Device Information");
    {
        Core::Log->ListAdd(CORE_LOG_BOLD("Device:")   " %s", alcGetString(m_pDevice, ALC_DEVICE_SPECIFIER));
        Core::Log->ListAdd(CORE_LOG_BOLD("Vendor:")   " %s", alGetString(AL_VENDOR));
        Core::Log->ListAdd(CORE_LOG_BOLD("Renderer:") " %s", alGetString(AL_RENDERER));
        Core::Log->ListAdd(CORE_LOG_BOLD("Version:")  " %s", alGetString(AL_VERSION));
        if(m_pContext) Core::Log->ListAdd(alcGetString(m_pDevice, ALC_EXTENSIONS));
        if(m_pContext) Core::Log->ListAdd(alGetString(AL_EXTENSIONS));
    }
    Core::Log->ListEnd();

    // log Ogg Vorbis library version
    Core::Log->Info("Ogg Vorbis initialized (%s)", vorbis_version_string());

    // check for errors
    const ALenum iError = alGetError();
    WARN_IF(iError != AL_NO_ERROR) Core::Log->Warning("Error initializing Audio Interface (AL Error Code: 0x%08X)", iError);
}


// ****************************************************************
/* destructor */
CoreAudio::~CoreAudio()
{
    // delete audio sources
    alDeleteSources(CORE_AUDIO_SOURCES, m_aiSource);

    // delete OpenAL context and close audio device
    alcMakeContextCurrent(NULL);
    alcDestroyContext(m_pContext);
    alcCloseDevice(m_pDevice);

    Core::Log->Info(CORE_LOG_BOLD("Audio Interface shut down"));
}


// ****************************************************************
/* control the listener */
void CoreAudio::SetListener(const coreVector3& vPosition, const coreVector3& vVelocity, const coreVector3& vDirection, const coreVector3& vOrientation)
{
    this->DeferUpdates();
    {
        coreBool bNewOrientation = false;

        // set and update properties of the listener
        ASSERT(vDirection.IsNormalized() && vOrientation.IsNormalized())
        if(m_vPosition      != vPosition)    {m_vPosition      = vPosition;    alListenerfv(AL_POSITION, m_vPosition);}
        if(m_vVelocity      != vVelocity)    {m_vVelocity      = vVelocity;    alListenerfv(AL_VELOCITY, m_vVelocity);}
        if(m_avDirection[0] != vDirection)   {m_avDirection[0] = vDirection;   bNewOrientation = true;}
        if(m_avDirection[1] != vOrientation) {m_avDirection[1] = vOrientation; bNewOrientation = true;}

        // update direction and orientation
        if(bNewOrientation) alListenerfv(AL_ORIENTATION, m_avDirection[0]);
    }
    this->ProcessUpdates();
}

void CoreAudio::SetListener(const coreFloat fSpeed, const coreInt8 iTimeID)
{
    // calculate velocity as relative camera movement
    const coreFloat   fTime     =  Core::System->GetTime(iTimeID);
    const coreVector3 vVelocity = (Core::Graphics->GetCamPosition() - m_vPosition) * (fTime ? (fSpeed * RCP(fTime)) : 0.0f);

    // adjust listener with camera properties
    this->SetListener(Core::Graphics->GetCamPosition(),
                      vVelocity,
                      Core::Graphics->GetCamDirection(),
                      Core::Graphics->GetCamOrientation());
}


// ****************************************************************
/* retrieve next free audio source */
ALuint CoreAudio::NextSource(const ALuint iBuffer, const coreFloat fVolume, const coreUint8 iType)
{
    ASSERT(iType < CORE_AUDIO_TYPES)

    // define search range
    const coreUintW iFrom = (iBuffer == CORE_AUDIO_MUSIC_BUFFER) ? 0u                       : CORE_AUDIO_SOURCES_MUSIC;
    const coreUintW iTo   = (iBuffer == CORE_AUDIO_MUSIC_BUFFER) ? CORE_AUDIO_SOURCES_MUSIC : CORE_AUDIO_SOURCES;

    // search for next free audio source
    for(coreUintW i = iFrom, ie = iTo; i < ie; ++i)
    {
        const ALuint iSource = m_aiSource[i];

        // check status
        ALint iStatus;
        alGetSourcei(iSource, AL_SOURCE_STATE, &iStatus);
        if((iStatus != AL_PLAYING) && (iStatus != AL_PAUSED))
        {
            // set current volume
            const coreFloat fBase = (iBuffer == CORE_AUDIO_MUSIC_BUFFER) ? m_afMusicVolume[1] : (m_afSoundVolume[1] * m_afTypeVolume[iType]);
            alSourcef(iSource, AL_GAIN, fVolume * fBase);

            // save audio source data
            m_aSourceData[i].iBuffer = iBuffer;
            m_aSourceData[i].fVolume = fVolume;
            m_aSourceData[i].iType   = iType;

            // return audio source
            return iSource;
        }
    }

    // no free audio source available
    WARN_IF(true) {}
    return 0u;
}


// ****************************************************************
/* unbind sound buffer from all audio sources */
void CoreAudio::FreeSources(const ALuint iBuffer)
{
    for(coreUintW i = 0u; i < CORE_AUDIO_SOURCES; ++i)
    {
        if(m_aSourceData[i].iBuffer == iBuffer)
        {
            const ALuint iSource = m_aiSource[i];

#if defined(_CORE_DEBUG_)

            // check for loop property
            ALint iPlaying; alGetSourcei(iSource, AL_SOURCE_STATE, &iPlaying);
            ALint iLooping; alGetSourcei(iSource, AL_LOOPING,      &iLooping);
            ASSERT((iPlaying != AL_PLAYING) || !iLooping)

#endif
            // stop audio source
            alSourceStop(iSource);
            alSourcei(iSource, AL_BUFFER, 0);

            // reset sound buffer
            m_aSourceData[i].iBuffer = 0u;
        }
    }
}


// ****************************************************************
/* update audio source data */
void CoreAudio::UpdateSource(const ALuint iSource, const coreFloat fVolume)
{
    for(coreUintW i = 0u; i < CORE_AUDIO_SOURCES; ++i)
    {
        if(m_aiSource[i] == iSource)
        {
            if(m_aSourceData[i].fVolume != fVolume)
            {
                // update current volume
                const coreFloat fBase = (m_aSourceData[i].iBuffer == CORE_AUDIO_MUSIC_BUFFER) ? m_afMusicVolume[1] : (m_afSoundVolume[1] * m_afTypeVolume[m_aSourceData[i].iType]);
                alSourcef(iSource, AL_GAIN, fVolume * fBase);

                // save new audio source data
                m_aSourceData[i].fVolume = fVolume;
            }
            break;
        }
    }
}


// ****************************************************************
/* check if audio source is still valid */
coreBool CoreAudio::CheckSource(const ALuint iBuffer, const ALuint iSource)const
{
    for(coreUintW i = 0u; i < CORE_AUDIO_SOURCES; ++i)
    {
        if(m_aiSource[i] == iSource)
        {
            // test with sound buffer
            return (m_aSourceData[i].iBuffer == iBuffer);
        }
    }
    return false;
}


// ****************************************************************
/* control sound playback */
void CoreAudio::PauseSound()
{
    // pause all audio sources
    alSourcePausev(CORE_AUDIO_SOURCES_SOUND, m_aiSource + CORE_AUDIO_SOURCES_MUSIC);
}

void CoreAudio::ResumeSound()
{
    ALuint    aiPaused[CORE_AUDIO_SOURCES_SOUND];
    coreUintW iNum = 0u;

    for(coreUintW i = CORE_AUDIO_SOURCES_MUSIC; i < CORE_AUDIO_SOURCES; ++i)
    {
        // check status
        ALint iStatus;
        alGetSourcei(m_aiSource[i], AL_SOURCE_STATE, &iStatus);

        // collect paused audio sources
        if(iStatus == AL_PAUSED) aiPaused[iNum++] = m_aiSource[i];
    }

    // resume paused audio sources
    if(iNum) alSourcePlayv(iNum, aiPaused);
}

void CoreAudio::CancelSound()
{
    // stop all audio sources
    alSourceStopv(CORE_AUDIO_SOURCES_SOUND, m_aiSource + CORE_AUDIO_SOURCES_MUSIC);
}


// ****************************************************************
/* update all audio sources */
void CoreAudio::__UpdateSources()
{
    const auto nUpdateVolumeFunc = [](coreFloat* OUTPUT pfVolume, const coreChar* pcSection, const coreChar* pcKey, const coreFloat fDefault)
    {
        // read current config value
        const coreFloat fNewVolume = MAX(Core::Config->GetFloat(pcSection, pcKey, fDefault), 0.0f);

        // compare and forward
        if(pfVolume[2] != fNewVolume)  {pfVolume[1] = pfVolume[2] = fNewVolume;}   // forward config
        if(pfVolume[0] != pfVolume[1]) {pfVolume[0] = pfVolume[1]; return true;}   // forward target

        return false;
    };

    // update listener volume
    if(nUpdateVolumeFunc(m_afGlobalVolume, CORE_CONFIG_AUDIO_GLOBALVOLUME))
    {
        alListenerf(AL_GAIN, m_afGlobalVolume[0]);
    }

    // update music volume
    if(nUpdateVolumeFunc(m_afMusicVolume, CORE_CONFIG_AUDIO_MUSICVOLUME))
    {
        this->DeferUpdates();
        {
            for(coreUintW i = 0u; i < CORE_AUDIO_SOURCES_MUSIC; ++i)
                alSourcef(m_aiSource[i], AL_GAIN, m_aSourceData[i].fVolume * m_afMusicVolume[0]);
        }
        this->ProcessUpdates();
    }

    // update sound volume
    if(nUpdateVolumeFunc(m_afSoundVolume, CORE_CONFIG_AUDIO_SOUNDVOLUME))
    {
        this->DeferUpdates();
        {
            for(coreUintW i = CORE_AUDIO_SOURCES_MUSIC; i < CORE_AUDIO_SOURCES; ++i)
                alSourcef(m_aiSource[i], AL_GAIN, m_aSourceData[i].fVolume * m_afSoundVolume[0] * m_afTypeVolume[m_aSourceData[i].iType]);
        }
        this->ProcessUpdates();
    }
}