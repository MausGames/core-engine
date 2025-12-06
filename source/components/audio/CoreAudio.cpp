///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
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
, m_bDeviceCheck    (false)
, m_iDeviceFix      (0u)
, m_aiAttributes    {}
{
    Core::Log->Header("Audio Interface");

#if !defined(_CORE_EMSCRIPTEN_) && !defined(_CORE_SWITCH_)

    // enable OpenAL logging
    if(Core::Debug->IsEnabled())
    {
        coreData::SetEnvironment("ALSOFT_TRAP_ERROR", "1");
        coreData::SetEnvironment("ALSOFT_LOGLEVEL",   "3");
        coreData::SetEnvironment("ALSOFT_LOGFILE",    coreData::UserFolderShared("log_openal.txt"));
    }

    // include additional config file
    const coreChar* pcUserFile = coreData::UserFolderPrivate("config_openal.ini");
    const coreChar* pcDataFile = "data/other/config_openal.ini";
    coreData::SetEnvironment("ALSOFT_CONF", coreData::FileExists(pcUserFile) ? pcUserFile : pcDataFile);

#endif

    // open audio device and create OpenAL context
    m_pDevice  = alcOpenDevice(NULL);
    m_pContext = alcCreateContext(m_pDevice, this->__RetrieveAttributes());

    // activate OpenAL context
    if(!m_pDevice || !m_pContext || !alcMakeContextCurrent(m_pContext))
         Core::Log->Warning("OpenAL context could not be created (ALC Error Code: 0x%08X)", alcGetError(m_pDevice));
    else Core::Log->Info   ("OpenAL context created");

    // init OpenAL
    coreInitOpenAL();

    // enable OpenAL debug output
    this->DebugOpenAL();

    // generate audio sources
    alGenSources(CORE_AUDIO_SOURCES, m_aiSource);

    // init enumeration extension
    if(CORE_ALC_SUPPORT(ENUMERATE_ALL_EXT))
    {
        // retrieve audio device list
        const coreChar* pcDeviceList = alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
        if(pcDeviceList)
        {
            // log all available audio devices
            Core::Log->ListStartInfo("Available Devices");
            {
                while(*pcDeviceList)
                {
                    Core::Log->ListAdd(pcDeviceList);
                    pcDeviceList += std::strlen(pcDeviceList) + 1u;
                }
            }
            Core::Log->ListEnd();
        }
    }

    // init HRTF extension
    if(CORE_ALC_SUPPORT(SOFT_HRTF))
    {
        // retrieve number of HRTFs
        ALCint iNum = 0; alcGetIntegerv(m_pDevice, ALC_NUM_HRTF_SPECIFIERS_SOFT, 1, &iNum);
        if(iNum > 0)
        {
            // log all available HRTFs
            Core::Log->ListStartInfo("Available HRTFs");
            {
                for(coreUintW i = 0u, ie = iNum; i < ie; ++i)
                {
                    Core::Log->ListAdd("%zu: %s", i, alcGetStringiSOFT(m_pDevice, ALC_HRTF_SPECIFIER_SOFT, i));
                }
            }
            Core::Log->ListEnd();
        }
    }

    // init system-events extension
    if(CORE_ALC_SUPPORT(SOFT_system_events))
    {
        // set callback function
        alcEventCallbackSOFT([](const ALCenum iEventType, const ALCenum iDeviceType, ALCdevice* pDevice, const ALCsizei iLength, const ALCchar* pcMessage, void* pUserParam)
        {
            Core::Log->Warning(CORE_LOG_BOLD("OpenAL:") " %s", pcMessage);

            if((iEventType == ALC_EVENT_TYPE_DEFAULT_DEVICE_CHANGED_SOFT) && (iDeviceType == ALC_PLAYBACK_DEVICE_SOFT))
            {
                s_cast<CoreAudio*>(pUserParam)->m_iDeviceFix.FetchMax(1u);
            }
        },
        this);

        // enable all events
        constexpr ALCenum aiEvent[] = {ALC_EVENT_TYPE_DEFAULT_DEVICE_CHANGED_SOFT, ALC_EVENT_TYPE_DEVICE_ADDED_SOFT, ALC_EVENT_TYPE_DEVICE_REMOVED_SOFT};
        alcEventControlSOFT(ARRAY_SIZE(aiEvent), aiEvent, true);
    }

    // init disconnect extension
    if(CORE_ALC_SUPPORT(EXT_disconnect))
    {
        m_bDeviceCheck = true;
    }

    // init deferred-updates extension
    if(CORE_AL_SUPPORT(SOFT_deferred_updates))
    {
        m_nDeferUpdates   = alDeferUpdatesSOFT;
        m_nProcessUpdates = alProcessUpdatesSOFT;
    }
    if(!m_nDeferUpdates || !m_nProcessUpdates)
    {
        m_nDeferUpdates   = []() {alcSuspendContext(alcGetCurrentContext());};
        m_nProcessUpdates = []() {alcProcessContext(alcGetCurrentContext());};
    }

    // init source-resampler extension
    const coreChar* pcResamplerName = NULL;
    if(CORE_AL_SUPPORT(SOFT_source_resampler))
    {
        // change resampler of all audio sources
        this->__ChangeResampler(Core::Config->GetInt(CORE_CONFIG_AUDIO_RESAMPLERINDEX));

        // retrieve number of resamplers
        const ALint iNum = alGetInteger(AL_NUM_RESAMPLERS_SOFT);
        if(iNum > 0)
        {
            const ALint iDefault = alGetInteger(AL_DEFAULT_RESAMPLER_SOFT);

            // log all available resamplers
            Core::Log->ListStartInfo("Available Resamplers");
            {
                for(coreUintW i = 0u, ie = iNum; i < ie; ++i)
                {
                    Core::Log->ListAdd("%zu: %s%s", i, alGetStringiSOFT(AL_RESAMPLER_NAME_SOFT, i), (ALint(i) == iDefault) ? "*" : "");
                }
            }
            Core::Log->ListEnd();
        }

        // retrieve selected resampler name
        ALint iSelect = 0; alGetSourcei(m_aiSource[0], AL_SOURCE_RESAMPLER_SOFT, &iSelect);
        pcResamplerName = alGetStringiSOFT(AL_RESAMPLER_NAME_SOFT, iSelect);
    }

    // init direct-channels extension
    if(CORE_AL_SUPPORT(SOFT_direct_channels) && CORE_AL_SUPPORT(SOFT_direct_channels_remix))
    {
        this->DeferUpdates();
        {
            for(coreUintW i = 0u; i < CORE_AUDIO_SOURCES; ++i)
            {
                alSourcei(m_aiSource[i], AL_DIRECT_CHANNELS_SOFT, AL_REMIX_UNMATCHED_SOFT);
            }
        }
        this->ProcessUpdates();
    }

    // reset listener
    this->DeferUpdates();
    {
        alListenerfv(AL_POSITION,    m_vPosition.ptr());
        alListenerfv(AL_VELOCITY,    m_vVelocity.ptr());
        alListenerfv(AL_ORIENTATION, m_avDirection[0].ptr());
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
        ALCint aiStatus[9] = {};
        alcGetIntegerv(m_pDevice, ALC_FREQUENCY,           1, &aiStatus[0]);
        alcGetIntegerv(m_pDevice, ALC_REFRESH,             1, &aiStatus[1]);
        alcGetIntegerv(m_pDevice, ALC_SYNC,                1, &aiStatus[2]);
        alcGetIntegerv(m_pDevice, ALC_MONO_SOURCES,        1, &aiStatus[3]);
        alcGetIntegerv(m_pDevice, ALC_STEREO_SOURCES,      1, &aiStatus[4]);
        alcGetIntegerv(m_pDevice, ALC_OUTPUT_MODE_SOFT,    1, &aiStatus[5]);
        alcGetIntegerv(m_pDevice, ALC_OUTPUT_LIMITER_SOFT, 1, &aiStatus[6]);
        alcGetIntegerv(m_pDevice, ALC_HRTF_SOFT,           1, &aiStatus[7]);
        alcGetIntegerv(m_pDevice, ALC_HRTF_STATUS_SOFT,    1, &aiStatus[8]);

        Core::Log->ListAdd(CORE_LOG_BOLD("Device:")   " %s (%s, %s (%d), %s)", alcGetString(m_pDevice, ALC_DEVICE_SPECIFIER), alcGetString(m_pDevice, ALC_ALL_DEVICES_SPECIFIER), aiStatus[7] ? alcGetString(m_pDevice, ALC_HRTF_SPECIFIER_SOFT) : "HRTF disabled", aiStatus[8], pcResamplerName ? pcResamplerName : "Unknown Resampler");
        Core::Log->ListAdd(CORE_LOG_BOLD("Vendor:")   " %s",                   alGetString(AL_VENDOR));
        Core::Log->ListAdd(CORE_LOG_BOLD("Renderer:") " %s",                   alGetString(AL_RENDERER));
        Core::Log->ListAdd(CORE_LOG_BOLD("Version:")  " %s",                   alGetString(AL_VERSION));
        Core::Log->ListAdd(alcGetString(m_pDevice, ALC_EXTENSIONS));
        Core::Log->ListAdd(alGetString(AL_EXTENSIONS));
        Core::Log->ListAdd("ALC_FREQUENCY (%d) ALC_REFRESH (%d) ALC_SYNC (%d) ALC_MONO_SOURCES (%d) ALC_STEREO_SOURCES (%d) ALC_OUTPUT_MODE_SOFT (0x%04X) ALC_OUTPUT_LIMITER_SOFT (%d)", aiStatus[0], aiStatus[1], aiStatus[2], aiStatus[3], aiStatus[4], aiStatus[5], aiStatus[6]);
    }
    Core::Log->ListEnd();

    // check for errors
    const ALCenum iErrorALC = alcGetError(m_pDevice);
    const ALenum  iErrorAL  = alGetError();
    WARN_IF((iErrorALC != ALC_NO_ERROR) || (iErrorAL != AL_NO_ERROR)) Core::Log->Warning("Error initializing Audio Interface (ALC Error Code: 0x%08X, AL Error Code: 0x%08X)", iErrorALC, iErrorAL);

    // log Opus library version
    Core::Log->Info("Opus initialized (%s)", opus_get_version_string());
}


// ****************************************************************
/* destructor */
CoreAudio::~CoreAudio()
{
    // exit OpenAL
    coreExitOpenAL();

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
void CoreAudio::SetListener(const coreVector3 vPosition, const coreVector3 vVelocity, const coreVector3 vDirection, const coreVector3 vOrientation)
{
    ASSERT(vDirection.IsNormalized() && vOrientation.IsNormalized())

    coreBool bNewPosition    = false;
    coreBool bNewVelocity    = false;
    coreBool bNewOrientation = false;

    // set properties of the listener
    if(m_vPosition      != vPosition)    {m_vPosition      = vPosition;    bNewPosition    = true;}
    if(m_vVelocity      != vVelocity)    {m_vVelocity      = vVelocity;    bNewVelocity    = true;}
    if(m_avDirection[0] != vDirection)   {m_avDirection[0] = vDirection;   bNewOrientation = true;}
    if(m_avDirection[1] != vOrientation) {m_avDirection[1] = vOrientation; bNewOrientation = true;}

    // update listener on demand
    if(bNewPosition || bNewVelocity || bNewOrientation)
    {
        this->DeferUpdates();
        {
            if(bNewPosition)    alListenerfv(AL_POSITION,    m_vPosition.ptr());
            if(bNewVelocity)    alListenerfv(AL_VELOCITY,    m_vVelocity.ptr());
            if(bNewOrientation) alListenerfv(AL_ORIENTATION, m_avDirection[0].ptr());
        }
        this->ProcessUpdates();
    }
}

void CoreAudio::SetListener(const coreFloat fSpeed, const coreInt8 iTimeID)
{
    // calculate velocity as relative camera movement
    const coreFloat   fTime     =  Core::System->GetTime(iTimeID);
    const coreVector3 vVelocity = (Core::Graphics->GetCamPosition() - m_vPosition) * (fTime ? (fSpeed / fTime) : 0.0f);

    // adjust listener with camera properties
    this->SetListener(Core::Graphics->GetCamPosition(),
                      vVelocity,
                      Core::Graphics->GetCamDirection(),
                      Core::Graphics->GetCamOrientation());
}


// ****************************************************************
/* control sound playback */
void CoreAudio::PauseSound(const coreUint8 iType)
{
    ALuint    aiSource[CORE_AUDIO_SOURCES_SOUND];
    coreUintW iNum = 0u;

    for(coreUintW i = CORE_AUDIO_SOURCES_MUSIC; i < CORE_AUDIO_SOURCES; ++i)
    {
        if(m_aSourceData[i].iBuffer && ((iType >= CORE_AUDIO_TYPES) || (m_aSourceData[i].iType == iType)))
        {
            // collect audio sources
            aiSource[iNum++] = m_aiSource[i];
        }
    }

    // pause audio sources
    if(iNum) alSourcePausev(iNum, aiSource);
}

void CoreAudio::ResumeSound(const coreUint8 iType)
{
    ALuint    aiSource[CORE_AUDIO_SOURCES_SOUND];
    coreUintW iNum = 0u;

    for(coreUintW i = CORE_AUDIO_SOURCES_MUSIC; i < CORE_AUDIO_SOURCES; ++i)
    {
        if(m_aSourceData[i].iBuffer && ((iType >= CORE_AUDIO_TYPES) || (m_aSourceData[i].iType == iType)))
        {
            // retrieve current status
            ALint iStatus;
            alGetSourcei(m_aiSource[i], AL_SOURCE_STATE, &iStatus);

            // collect paused audio sources
            if(iStatus == AL_PAUSED) aiSource[iNum++] = m_aiSource[i];
        }
    }

    // resume paused audio sources
    if(iNum) alSourcePlayv(iNum, aiSource);
}

void CoreAudio::CancelSound(const coreUint8 iType)
{
    ALuint    aiSource[CORE_AUDIO_SOURCES_SOUND];
    coreUintW iNum = 0u;

    for(coreUintW i = CORE_AUDIO_SOURCES_MUSIC; i < CORE_AUDIO_SOURCES; ++i)
    {
        if(m_aSourceData[i].iBuffer && ((iType >= CORE_AUDIO_TYPES) || (m_aSourceData[i].iType == iType)))
        {
            // collect audio sources
            aiSource[iNum++] = m_aiSource[i];

            // reset identification properties
            m_aSourceData[i].pRef    = NULL;
            m_aSourceData[i].iBuffer = 0u;
        }
    }

    if(iNum)
    {
        // stop audio sources
        alSourceStopv(iNum, aiSource);
        for(coreUintW i = 0u; i < iNum; ++i) alSourcei(aiSource[i], AL_BUFFER, 0);
    }
}


// ****************************************************************
/* retrieve next free audio source */
ALuint CoreAudio::NextSource(const void* pRef, const ALuint iBuffer, const coreFloat fVolume, const coreUint8 iType)
{
    ASSERT(iType < CORE_AUDIO_TYPES)

    // define search range
    const coreUintW iFrom = (iBuffer == CORE_AUDIO_MUSIC_BUFFER) ? 0u                       : CORE_AUDIO_SOURCES_MUSIC;
    const coreUintW iTo   = (iBuffer == CORE_AUDIO_MUSIC_BUFFER) ? CORE_AUDIO_SOURCES_MUSIC : CORE_AUDIO_SOURCES;

    // search for next free audio source
    for(coreUintW i = iFrom, ie = iTo; i < ie; ++i)
    {
        const ALuint iSource = m_aiSource[i];

        // retrieve current status
        ALint iStatus;
        alGetSourcei(iSource, AL_SOURCE_STATE, &iStatus);

        if((iStatus != AL_PLAYING) && (iStatus != AL_PAUSED))
        {
            // set current volume
            const coreFloat fBase = (iBuffer == CORE_AUDIO_MUSIC_BUFFER) ? m_afMusicVolume[1] : (m_afSoundVolume[1] * m_afTypeVolume[iType]);
            alSourcef(iSource, AL_GAIN, fVolume * fBase / CORE_AUDIO_MAX_GAIN);

            // save audio source data
            m_aSourceData[i].pRef    = pRef;
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

            // reset identification properties
            m_aSourceData[i].pRef    = NULL;
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
                alSourcef(iSource, AL_GAIN, fVolume * fBase / CORE_AUDIO_MAX_GAIN);

                // save new audio source data
                m_aSourceData[i].fVolume = fVolume;
            }
            break;
        }
    }
}


// ****************************************************************
/* check if audio source is still valid */
coreBool CoreAudio::CheckSource(const void* pRef, const ALuint iBuffer, const ALuint iSource)const
{
    for(coreUintW i = 0u; i < CORE_AUDIO_SOURCES; ++i)
    {
        if(m_aiSource[i] == iSource)
        {
            // test with identification properties
            return (m_aSourceData[i].pRef == pRef) && (m_aSourceData[i].iBuffer == iBuffer);
        }
    }
    return false;
}


// ****************************************************************
/* write OpenAL debug message */
void AL_APIENTRY WriteOpenAL(const ALenum iSource, const ALenum iType, const ALuint iID, const ALenum iSeverity, const ALsizei iLength, const ALchar* pcMessage, const void* pUserParam)
{
    // write debug message
    Core::Log->ListStartWarning("OpenAL Debug Message");
    {
        Core::Log->ListAdd(CORE_LOG_BOLD("ID:")           " %d", iID);
        Core::Log->ListAdd(CORE_LOG_BOLD("Source:")   " 0x%04X", iSource);
        Core::Log->ListAdd(CORE_LOG_BOLD("Type:")     " 0x%04X", iType);
        Core::Log->ListAdd(CORE_LOG_BOLD("Severity:") " 0x%04X", iSeverity);
        Core::Log->ListAdd(pcMessage);
    }
    Core::Log->ListEnd();

    WARN_IF(true) {}
}


// ****************************************************************
/* enable OpenAL debug output */
void CoreAudio::DebugOpenAL()
{
    if(!Core::Debug->IsEnabled()) return;

    if(CORE_AL_SUPPORT(EXT_debug))
    {
        // enable debug output
        alEnable(AL_DEBUG_OUTPUT_EXT);

        // set callback function and enable all messages
        alDebugMessageCallbackEXT(&WriteOpenAL, NULL);
        alDebugMessageControlEXT(AL_DONT_CARE_EXT, AL_DONT_CARE_EXT, AL_DONT_CARE_EXT, 0, NULL, true);
    }
}


// ****************************************************************
/* manually check for OpenAL errors */
void CoreAudio::CheckOpenAL()
{
    if(!Core::Debug->IsEnabled()) return;

    // get recent device error
    const ALCenum iErrorALC = alcGetError(m_pDevice);
    WARN_IF(iErrorALC != ALC_NO_ERROR)
    {
        Core::Log->Warning("OpenAL reported a device error (ALC Error Code: 0x%08X)", iErrorALC);
    }

    // get recent context error
    const ALenum iErrorAL = alGetError();
    WARN_IF(iErrorAL != AL_NO_ERROR)
    {
        Core::Log->Warning("OpenAL reported a context error (AL Error Code: 0x%08X)", iErrorAL);
    }
}


// ****************************************************************
/* label OpenAL object for debug tooling */
void CoreAudio::LabelOpenAL(const ALenum iType, const ALuint iIdentifier, const coreChar* pcLabel)
{
    if(!Core::Debug->IsEnabled()) return;

    if(CORE_AL_SUPPORT(EXT_debug))
    {
        // assign string to identifier
        alObjectLabelEXT(iType, iIdentifier, -1, pcLabel);
    }
}


// ****************************************************************
/* reconfigure audio interface */
void CoreAudio::Reconfigure()
{
    if(CORE_ALC_SUPPORT(SOFT_HRTF))
    {
        // reset audio device with different attributes
        alcResetDeviceSOFT(m_pDevice, this->__RetrieveAttributes());
    }

    // change resampler of all audio sources
    this->__ChangeResampler(Core::Config->GetInt(CORE_CONFIG_AUDIO_RESAMPLERINDEX));

    Core::Log->Info("Audio Interface reconfigured");
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
        alListenerf(AL_GAIN, m_afGlobalVolume[0] * CORE_AUDIO_MAX_GAIN);
    }

    // update music volume
    if(nUpdateVolumeFunc(m_afMusicVolume, CORE_CONFIG_AUDIO_MUSICVOLUME))
    {
        this->DeferUpdates();
        {
            for(coreUintW i = 0u; i < CORE_AUDIO_SOURCES_MUSIC; ++i)
            {
                alSourcef(m_aiSource[i], AL_GAIN, m_aSourceData[i].fVolume * m_afMusicVolume[0] / CORE_AUDIO_MAX_GAIN);
            }
        }
        this->ProcessUpdates();
    }

    // update sound volume
    if(nUpdateVolumeFunc(m_afSoundVolume, CORE_CONFIG_AUDIO_SOUNDVOLUME))
    {
        this->DeferUpdates();
        {
            for(coreUintW i = CORE_AUDIO_SOURCES_MUSIC; i < CORE_AUDIO_SOURCES; ++i)
            {
                alSourcef(m_aiSource[i], AL_GAIN, m_aSourceData[i].fVolume * m_afSoundVolume[0] * m_afTypeVolume[m_aSourceData[i].iType] / CORE_AUDIO_MAX_GAIN);
            }
        }
        this->ProcessUpdates();
    }
}


// ****************************************************************
/* update the audio device */
void CoreAudio::__UpdateDevice()
{
    // check for OpenAL errors
    this->CheckOpenAL();

    // periodically check for audio device disconnect
    if(m_bDeviceCheck)
    {
        // retrieve current status
        ALCint iConnected = INT32_MAX;
        alcGetIntegerv(m_pDevice, ALC_CONNECTED, 1, &iConnected);

        // check for disconnect
        WARN_IF(!iConnected)
        {
            Core::Log->Warning("Audio device disconnected");
            m_iDeviceFix.FetchMax(2u);
        }
    }

    // try to recover from audio device issues
    if(m_iDeviceFix.Exchange(0u))
    {
        if(CORE_ALC_SUPPORT(SOFT_reopen_device))
        {
            // reopen audio device
            if(alcReopenDeviceSOFT(m_pDevice, NULL, this->__RetrieveAttributes()))
            {
                Core::Log->Warning("Audio device (%s) reopened", alcGetString(m_pDevice, ALC_ALL_DEVICES_SPECIFIER));
            }
            else
            {
                Core::Log->Warning("Audio device could not be reopened (ALC Error Code: 0x%08X)", alcGetError(m_pDevice));
            }
        }
    }
}


// ****************************************************************
/* change resampler of all audio sources */
void CoreAudio::__ChangeResampler(const ALint iResampler)
{
    if(CORE_AL_SUPPORT(SOFT_source_resampler))
    {
        // retrieve number of resamplers
        const ALint iNum = alGetInteger(AL_NUM_RESAMPLERS_SOFT);
        if(iNum > 0)
        {
            // clamp and check for default resampler
            const ALint iIndex = (iResampler > -1) ? MIN(iResampler, iNum - 1) : alGetInteger(AL_DEFAULT_RESAMPLER_SOFT);

            // update resamplers
            this->DeferUpdates();
            {
                for(coreUintW i = 0u; i < CORE_AUDIO_SOURCES; ++i)
                {
                    alSourcei(m_aiSource[i], AL_SOURCE_RESAMPLER_SOFT, iIndex);
                }
            }
            this->ProcessUpdates();
        }
    }
}


// ****************************************************************
/* assemble OpenAL context attributes */
const ALint* CoreAudio::__RetrieveAttributes()
{
    coreUintW i = 0u;
    const auto nAttributeFunc = [&](const ALCint iAttribute, const ALCint iValue)
    {
        ASSERT(i + 3u < ARRAY_SIZE(m_aiAttributes))

        // add new attribute
        m_aiAttributes[i++] = iAttribute;
        m_aiAttributes[i++] = iValue;
    };

    // reset all attributes
    std::memset(m_aiAttributes, 0, sizeof(m_aiAttributes));

    // select audio configuration
    ALCint iOutputMode, iHRTF;
    switch(Core::Config->GetInt(CORE_CONFIG_AUDIO_MODE))
    {
    default:
    case CORE_AUDIO_MODE_AUTO:       iOutputMode = ALC_ANY_SOFT;         iHRTF = ALC_DONT_CARE_SOFT; break;
    case CORE_AUDIO_MODE_MONO:       iOutputMode = ALC_MONO_SOFT;        iHRTF = ALC_FALSE;          break;
    case CORE_AUDIO_MODE_SPEAKERS:   iOutputMode = ALC_ANY_SOFT;         iHRTF = ALC_FALSE;          break;
    case CORE_AUDIO_MODE_HEADPHONES: iOutputMode = ALC_STEREO_HRTF_SOFT; iHRTF = ALC_TRUE;           break;
    }

    // set standard attributes
    nAttributeFunc(ALC_MONO_SOURCES,   CORE_AUDIO_SOURCES);
    nAttributeFunc(ALC_STEREO_SOURCES, CORE_AUDIO_SOURCES);

    if(CORE_ALC_SUPPORT(SOFT_output_mode))
    {
        // set output mode
        nAttributeFunc(ALC_OUTPUT_MODE_SOFT, iOutputMode);
    }

    if(CORE_ALC_SUPPORT(SOFT_HRTF))
    {
        // set HRTF state
        nAttributeFunc(ALC_HRTF_SOFT, iHRTF);

        // retrieve number of HRTFs
        ALCint iNum = 0; alcGetIntegerv(m_pDevice, ALC_NUM_HRTF_SPECIFIERS_SOFT, 1, &iNum);
        if(iNum > 0)
        {
            // set HRTF index (if requested)
            const ALCint iIndex = MIN(Core::Config->GetInt(CORE_CONFIG_AUDIO_HRTFINDEX), iNum - 1);
            if(iIndex > -1) nAttributeFunc(ALC_HRTF_ID_SOFT, iIndex);
        }
    }

    if(CORE_ALC_SUPPORT(EXT_debug) && Core::Debug->IsEnabled())
    {
        // create debug context
        nAttributeFunc(ALC_CONTEXT_FLAGS_EXT, ALC_CONTEXT_DEBUG_BIT_EXT);
    }

    return m_aiAttributes;
}