//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
CoreAudio::CoreAudio()noexcept
: m_NumSource (Core::Config->GetInt(CORE_CONFIG_AUDIO_SOURCES))
, m_CurSource (0)
, m_fVolume   (-1.0f)
{
    Core::Log->Header("Audio Interface");

    // open audio device and create OpenAL context
    m_pDevice  = alcOpenDevice(NULL);
    m_pContext = alcCreateContext(m_pDevice, NULL);

    // activate OpenAL context
    if(!m_pDevice || !m_pContext || !alcMakeContextCurrent(m_pContext))
        Core::Log->Error(true, "OpenAL context could not be created (ALC Error Code: %d)", alcGetError(m_pDevice));
    else Core::Log->Info("OpenAL context created");

    // generate sound sources
    m_pSource = new ALuint[m_NumSource];
    alGenSources(m_NumSource, m_pSource);

    // log audio device information
    Core::Log->ListStart("Audio Device Information");
    Core::Log->ListEntry("<b>Device:</b> %s",   alcGetString(m_pDevice, ALC_DEVICE_SPECIFIER));
    Core::Log->ListEntry("<b>Vendor:</b> %s",   alGetString(AL_VENDOR));
    Core::Log->ListEntry("<b>Renderer:</b> %s", alGetString(AL_RENDERER));
    Core::Log->ListEntry("<b>Version:</b> %s",  alGetString(AL_VERSION));
    Core::Log->ListEntry(r_cast<const char*>(alGetString(AL_EXTENSIONS)));
    Core::Log->ListEnd();

    // reset listener
    const coreVector3 vInit = coreVector3(1.0f,1.0f,1.0f);
    this->SetListener(vInit, vInit, vInit, vInit);
    this->SetListener(0.0f);

    // reset volume
    this->SetVolume(Core::Config->GetFloat(CORE_CONFIG_AUDIO_VOLUME_GLOBAL));

    // check for errors
    const ALenum iError = alGetError();
    if(iError != AL_NO_ERROR) Core::Log->Error(false, "Error initializing Audio Interface (AL Error Code: %d)", iError);
}


// ****************************************************************
// destructor
CoreAudio::~CoreAudio()
{
    Core::Log->Info("Audio Interface shut down");

    // save global volume
    Core::Config->SetFloat(CORE_CONFIG_AUDIO_VOLUME_GLOBAL, m_fVolume);

    // clear memory
    m_aiBuffer.clear();

    // delete sound sources
    alDeleteSources(m_NumSource, m_pSource);
    SAFE_DELETE_ARRAY(m_pSource)

    // delete OpenAL context and close audio device
    alcMakeContextCurrent(NULL);
    alcDestroyContext(m_pContext);
    alcCloseDevice(m_pDevice);
}


// ****************************************************************
// control the listener
void CoreAudio::SetListener(const coreVector3& vPosition, const coreVector3& vVelocity, const coreVector3& vDirection, const coreVector3& vOrientation)
{
    bool bNewOrientation = false;

    // set and update attributes of the listener
    const coreVector3 vDirNorm = vDirection.Normalized();   
    const coreVector3 vOriNorm = vOrientation.Normalized(); 
    if(m_vPosition      != vPosition) {m_vPosition      = vPosition; alListenerfv(AL_POSITION, m_vPosition);}
    if(m_vVelocity      != vVelocity) {m_vVelocity      = vVelocity; alListenerfv(AL_VELOCITY, m_vVelocity);}
    if(m_avDirection[0] != vDirNorm)  {m_avDirection[0] = vDirNorm;  bNewOrientation = true;}
    if(m_avDirection[1] != vOriNorm)  {m_avDirection[1] = vOriNorm;  bNewOrientation = true;}

    // update direction and orientation
    if(bNewOrientation) alListenerfv(AL_ORIENTATION, m_avDirection[0]);
}

void CoreAudio::SetListener(const float& fSpeed, const int iTimeID)
{
    const coreVector3 vVelocity = (Core::Graphics->GetCamPosition() - m_vPosition) * fSpeed * Core::System->GetTime(iTimeID);

    // adjust listener with camera attributes
    this->SetListener(Core::Graphics->GetCamPosition(),
                      vVelocity,
                      Core::Graphics->GetCamDirection(),
                      Core::Graphics->GetCamOrientation());
}


// ****************************************************************
// retrieve next free sound source
ALuint CoreAudio::NextSource(const ALuint& iBuffer)
{
    // search for next free sound source
    for(int i = 0; i < m_NumSource; ++i)
    {
        if(++m_CurSource >= m_NumSource) m_CurSource = 0;

        // check status
        int iStatus;
        alGetSourcei(m_pSource[m_CurSource], AL_SOURCE_STATE, &iStatus);
        if(iStatus != AL_PLAYING)
        {
            const ALuint& pSource = m_pSource[m_CurSource];

            // return sound source
            m_aiBuffer[pSource] = iBuffer;
            return pSource;
        }
    }

    // no free sound source available
    return 0;
}


// ****************************************************************
// unbind sound buffer from all sound sources
void CoreAudio::ClearSources(const ALuint& iBuffer)
{
    FOR_EACH(it, m_aiBuffer)
    {
        if(it->second == iBuffer)
        {
#if defined(_CORE_DEBUG_)

            // check for loop property
            int iPlaying; alGetSourcei(it->first, AL_SOURCE_STATE, &iPlaying);
            int iLooping; alGetSourcei(it->first, AL_LOOPING,      &iLooping);
            SDL_assert(!iPlaying || !iLooping);

#endif
            // stop sound source
            alSourceStop(it->first);
            alSourcei(it->first, AL_BUFFER, 0);

            // reset sound buffer
            it->second = 0;
        }
    }
}