#include "Core.h"

// ****************************************************************    
// Konstruktor
CoreSound::CoreSound()
: m_NumSource (Core::Config->GetInt(CORE_CONFIG_SOUND_CHANNELS, 24))
, m_CurSource (0)
{
    Core::Log->Header("Sound Interface");

    // create OpenAL context
    if(!alutInit(NULL, NULL))
        Core::Log->Error(0, coreUtils::Print("OpenAL context could not be created (%s)", alutGetErrorString(alutGetError())));
    else Core::Log->Info("OpenAL context created");

    // retrieve sound channels
    m_pSource = new ALuint[m_NumSource];
    alGenSources(m_NumSource, m_pSource);

    // get context and device
    m_pContext = alcGetCurrentContext();
    m_pDevice  = alcGetContextsDevice(m_pContext);

    // log sound device information 
    Core::Log->ListStart("Sound Device Information");
    Core::Log->ListEntry(coreUtils::Print("<b>Device:</b> %s",     alcGetString(m_pDevice, ALC_DEVICE_SPECIFIER)));
    Core::Log->ListEntry(coreUtils::Print("<b>Vendor:</b> %s",     alGetString(AL_VENDOR)));
    Core::Log->ListEntry(coreUtils::Print("<b>Renderer:</b> %s",   alGetString(AL_RENDERER)));
    Core::Log->ListEntry(coreUtils::Print("<b>Version:</b> %s",    alGetString(AL_VERSION)));
    Core::Log->ListEntry((const char*)alGetString(AL_EXTENSIONS));
    Core::Log->ListEnd();

    // reset listener
    const coreVector3 vInit = coreVector3(1.0f,1.0f,1.0f);
    this->SetListener(&vInit, &vInit, &vInit, &vInit);
    this->SetListener(0.0f);

    // reset volume
    this->SetVolume(Core::Config->GetFloat(CORE_CONFIG_SOUND_VOLUME_GLOBAL, 0.5f));

    // check for errors
    const ALenum iError = alGetError();
    if(iError != AL_NO_ERROR) Core::Log->Error(0, coreUtils::Print("Error initializing sound interface (Code: %d)", iError));
}


// ****************************************************************
// Destruktor
CoreSound::~CoreSound()
{
    Core::Log->Info("Sound Interface Shut Down");

    // save global volume
    Core::Config->SetFloat(CORE_CONFIG_SOUND_VOLUME_GLOBAL, m_fVolume);

    // delete sound channels
    alDeleteSources(m_NumSource, m_pSource);
    SAFE_DELETE_ARRAY(m_pSource)

    // shut down OpenAL library
    alutExit();
}


// ****************************************************************
// control the listener
void CoreSound::SetListener(const coreVector3* pvPosition, const coreVector3* pvVelocity, const coreVector3* pvDirection, const coreVector3* pvOrientation)
{
    bool bNewOrientation = false;

    // set and update parameters of the listener
    if(pvPosition)    if(m_vPosition      != *pvPosition)    {m_vPosition      = *pvPosition;    alListenerfv(AL_POSITION, (float*)&m_vPosition);}
    if(pvVelocity)    if(m_vVelocity      != *pvVelocity)    {m_vVelocity      = *pvVelocity;    alListenerfv(AL_VELOCITY, (float*)&m_vVelocity);}
    if(pvDirection)   if(m_avDirection[0] != *pvDirection)   {m_avDirection[0] = *pvDirection;   bNewOrientation = true;}
    if(pvOrientation) if(m_avDirection[1] != *pvOrientation) {m_avDirection[1] = *pvOrientation; bNewOrientation = true;}

    if(bNewOrientation) alListenerfv(AL_ORIENTATION, (float*)m_avDirection);
}

void CoreSound::SetListener(const float& fSpeed)
{
    const coreVector3 vVelocity = (Core::Graphic->GetCamPosition() - m_vPosition) * fSpeed * Core::System->GetTime();

    // adjust listener with camera attributes
    this->SetListener(&Core::Graphic->GetCamPosition(),
                      &vVelocity,
                      &Core::Graphic->GetCamDirection(),
                      &Core::Graphic->GetCamOrientation());
}