//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
CoreAudio::CoreAudio()noexcept
: m_iNumSources (Core::Config->GetInt(CORE_CONFIG_AUDIO_SOURCES))
, m_iCurSource  (0)
, m_fVolume     (-1.0f)
{
    Core::Log->Header("Audio Interface");

    // open audio device and create OpenAL context
    m_pDevice  = alcOpenDevice(NULL);
    m_pContext = alcCreateContext(m_pDevice, NULL);

    // activate OpenAL context
    if(!m_pDevice || !m_pContext || !alcMakeContextCurrent(m_pContext))
        Core::Log->Error("OpenAL context could not be created (ALC Error Code: 0x%04X)", alcGetError(m_pDevice));
    else Core::Log->Info("OpenAL context created");

    // generate sound sources
    m_pSource = new ALuint[m_iNumSources];
    alGenSources(m_iNumSources, m_pSource);

    // log audio device information
    Core::Log->ListStartInfo("Audio Device Information");
    {
        Core::Log->ListAdd(CORE_LOG_BOLD("Device:")   " %s", alcGetString(m_pDevice, ALC_DEVICE_SPECIFIER));
        Core::Log->ListAdd(CORE_LOG_BOLD("Vendor:")   " %s", alGetString(AL_VENDOR));
        Core::Log->ListAdd(CORE_LOG_BOLD("Renderer:") " %s", alGetString(AL_RENDERER));
        Core::Log->ListAdd(CORE_LOG_BOLD("Version:")  " %s", alGetString(AL_VERSION));
        Core::Log->ListAdd(r_cast<const char*>(alGetString(AL_EXTENSIONS)));
    }
    Core::Log->ListEnd();

    // reset listener
    constexpr_var coreVector3 vInit = coreVector3(1.0f,0.0f,0.0f);
    this->SetListener(vInit, vInit, vInit, vInit);
    this->SetListener(0.0f);

    // reset volume
    this->SetVolume(Core::Config->GetFloat(CORE_CONFIG_AUDIO_GLOBALVOLUME));

    // check for errors
    const ALenum iError = alGetError();
    if(iError != AL_NO_ERROR) Core::Log->Warning("Error initializing Audio Interface (AL Error Code: 0x%04X)", iError);
}


// ****************************************************************
// destructor
CoreAudio::~CoreAudio()
{
    // clear memory
    m_aiBuffer.clear();

    // delete sound sources
    alDeleteSources(m_iNumSources, m_pSource);
    SAFE_DELETE_ARRAY(m_pSource)

    // delete OpenAL context and close audio device
    alcMakeContextCurrent(NULL);
    alcDestroyContext(m_pContext);
    alcCloseDevice(m_pDevice);

    Core::Log->Info("Audio Interface shut down");
}


// ****************************************************************
// control the listener
void CoreAudio::SetListener(const coreVector3& vPosition, const coreVector3& vVelocity, const coreVector3& vDirection, const coreVector3& vOrientation)
{
    bool bNewOrientation = false;

    // set and update properties of the listener
    ASSERT(vDirection.IsNormalized() && vOrientation.IsNormalized())
    if(m_vPosition      != vPosition)    {m_vPosition      = vPosition;    alListenerfv(AL_POSITION, m_vPosition);}
    if(m_vVelocity      != vVelocity)    {m_vVelocity      = vVelocity;    alListenerfv(AL_VELOCITY, m_vVelocity);}
    if(m_avDirection[0] != vDirection)   {m_avDirection[0] = vDirection;   bNewOrientation = true;}
    if(m_avDirection[1] != vOrientation) {m_avDirection[1] = vOrientation; bNewOrientation = true;}

    // update direction and orientation
    if(bNewOrientation) alListenerfv(AL_ORIENTATION, m_avDirection[0]);
}

void CoreAudio::SetListener(const float& fSpeed, const int iTimeID)
{
    // calculate velocity as relative camera movement
    const float fTime = Core::System->GetTime(iTimeID);
    const coreVector3 vVelocity = (Core::Graphics->GetCamPosition() - m_vPosition) * fSpeed * (fTime ? RCP(fTime) : 0.0f);

    // adjust listener with camera properties
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
    for(int i = 0; i < m_iNumSources; ++i)
    {
        if(++m_iCurSource >= m_iNumSources) m_iCurSource = 0;

        // check status
        int iStatus;
        alGetSourcei(m_pSource[m_iCurSource], AL_SOURCE_STATE, &iStatus);
        if(iStatus != AL_PLAYING)
        {
            const ALuint& pSource = m_pSource[m_iCurSource];

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
            ASSERT(!iPlaying || !iLooping)

#endif
            // stop sound source
            alSourceStop(it->first);
            alSourcei(it->first, AL_BUFFER, 0);

            // reset sound buffer
            it->second = 0;
        }
    }
}