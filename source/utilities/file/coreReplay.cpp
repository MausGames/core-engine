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
coreReplay::coreReplay()noexcept
: m_sPath            ("")
, m_eMode            (CORE_REPLAY_MODE_DISABLED)
, m_pRecordingStream (NULL)
, m_pPlaybackFile    (NULL)
, m_pPlaybackCursor  (NULL)
{
}


// ****************************************************************
/* destructor */
coreReplay::~coreReplay()
{
    // close recording stream
    if(m_pRecordingStream) SDL_CloseIO(m_pRecordingStream);

    // delete playback file
    SAFE_DELETE(m_pPlaybackFile)
}


// ****************************************************************
/* start replay recording */
coreStatus coreReplay::StartRecording(const coreChar* pcPath)
{
    ASSERT(m_eMode == CORE_REPLAY_MODE_DISABLED)

    // save path
    m_sPath = pcPath;

    // open recording stream
    m_pRecordingStream = SDL_IOFromFile(m_sPath.c_str(), CORE_FILE_OPEN_WRITE);
    if(!m_pRecordingStream)
    {
        Core::Log->Warning("Replay Recording (%s) could not be started", m_sPath.c_str());
        return CORE_ERROR_FILE;
    }

    // save magic number and file version
    const coreUint32 aiHead[3] = {CORE_REPLAY_MAGIC, CORE_REPLAY_VERSION, UINT32_MAX};
    SDL_WriteIO(m_pRecordingStream, aiHead, sizeof(coreUint32) * 3u);

    // save additional information
    SDL_IOprintf(m_pRecordingStream, "%s.%s%c",  CoreApp::Settings::Identifier, CoreApp::Settings::Version, '\0');
    SDL_IOprintf(m_pRecordingStream, "SDL.%d%c", SDL_GetVersion(), '\0');

    // mark beginning of the replay
    const coreUint32 iOffset = SDL_TellIO(m_pRecordingStream);
    SDL_SeekIO (m_pRecordingStream, sizeof(coreUint32) * 2, SDL_IO_SEEK_SET);
    SDL_WriteIO(m_pRecordingStream, &iOffset, sizeof(coreUint32));
    SDL_SeekIO (m_pRecordingStream, 0, SDL_IO_SEEK_END);

    // enter recording mode
    m_eMode = CORE_REPLAY_MODE_RECORDING;

    Core::Log->Info("Replay Recording (%s) started", m_sPath.c_str());
    return CORE_OK;
}


// ****************************************************************
/* start replay playback */
coreStatus coreReplay::StartPlayback(const coreChar* pcPath)
{
    ASSERT(m_eMode == CORE_REPLAY_MODE_DISABLED)

    // save path
    m_sPath = pcPath;

    // open playback file
    m_pPlaybackFile = new coreFile(m_sPath.c_str());
    if(!m_pPlaybackFile->GetSize())
    {
        Core::Log->Warning("Replay Playback (%s) could not be started", m_sPath.c_str());
        return CORE_ERROR_FILE;
    }

    // handle custom compression
    if((*coreData::StrRight(m_sPath.c_str(), 1u)) == 'z')
    {
        m_pPlaybackFile->LoadData(true);
        m_pPlaybackFile->Decompress();
    }

    // read magic number and file version
    coreUint32 aiHead[3];
    std::memcpy(aiHead, m_pPlaybackFile->GetData(), sizeof(coreUint32) * 3u);

    // check magic number and file version
    if((aiHead[0] != CORE_REPLAY_MAGIC) || (aiHead[1] != CORE_REPLAY_VERSION) || (aiHead[2] >= m_pPlaybackFile->GetSize()))
    {
        Core::Log->Warning("Replay Playback (%s) is not a valid CRP-file", m_sPath.c_str());
        return CORE_ERROR_FILE;
    }

    // set cursor to the beginning of the replay
    m_pPlaybackCursor = m_pPlaybackFile->GetData() + aiHead[2];

    // disable interfering events not coming from the replay
    constexpr coreUint32 aiDisable[] = {SDL_EVENT_MOUSE_MOTION, SDL_EVENT_FINGER_MOTION};
    for(coreUintW i = 0u; i < ARRAY_SIZE(aiDisable); ++i) SDL_SetEventEnabled(aiDisable[i], false);

    // enter playback mode
    m_eMode = CORE_REPLAY_MODE_PLAYBACK;

    Core::Log->Info("Replay Playback (%s) started", m_sPath.c_str());
    return CORE_OK;
}


// ****************************************************************
/* apply default command line */
void coreReplay::ApplyCommandLine()
{
    // record replay
    if(coreData::GetCommandLine("record"))
    {
        // retrieve command options
        const coreChar* pcOutput = coreData::GetCommandLine("output");
        if(!pcOutput)
        {
            std::puts("<output> parameter missing or invalid");
            return;
        }

        // start replay recording
        WARN_IF(this->StartRecording(pcOutput) != CORE_OK) {}
    }

    // play back replay
    if(coreData::GetCommandLine("playback"))
    {
        // retrieve command options
        const coreChar* pcInput = coreData::GetCommandLine("input");
        if(!pcInput)
        {
            std::puts("<input> parameter missing or invalid");
            return;
        }

        // start replay playback
        WARN_IF(this->StartPlayback(pcInput) != CORE_OK) {}
    }
}


// ****************************************************************
/* write event */
coreBool coreReplay::WriteEvent(const SDL_Event& oEvent)
{
    if(m_eMode == CORE_REPLAY_MODE_RECORDING)
    {
        // get event data
        ASSERT(!HAS_FLAG(Core::System->GetCurFrame(), CORE_REPLAY_EVENT_MARKER))
        const coreUint32 iCurFrame  = Core::System->GetCurFrame() | CORE_REPLAY_EVENT_MARKER;
        const coreUintW  iEventSize = coreReplay::__GetEventSize(oEvent.type);

        if(iEventSize)
        {
            // normalize event into portable format
            const SDL_Event& oNormalizedEvent = coreReplay::__NormalizeEvent(oEvent);

            // write event to the stream
            SDL_WriteIO(m_pRecordingStream, &iCurFrame,        sizeof(coreUint32));
            SDL_WriteIO(m_pRecordingStream, &oNormalizedEvent, iEventSize);

            // flush recording stream (including all frame times)
            SDL_FlushIO(m_pRecordingStream);

            return true;
        }
    }

    return false;
}


// ****************************************************************
/* read event */
coreBool coreReplay::ReadEvent(SDL_Event* OUTPUT pEvent)
{
    ASSERT(pEvent)

    if(m_eMode == CORE_REPLAY_MODE_PLAYBACK)
    {
        // check if playback is finished
        if(!this->__IsFinished())
        {
            const coreUint32 iCurFrame = (*r_cast<const coreUint32*>(m_pPlaybackCursor));

            // check for next valid event
            if(HAS_FLAG(iCurFrame, CORE_REPLAY_EVENT_MARKER) && ((iCurFrame & ~CORE_REPLAY_EVENT_MARKER) == Core::System->GetCurFrame()))
            {
                m_pPlaybackCursor += sizeof(coreUint32);

                // get event data
                const SDL_Event* pSource    = r_cast<const SDL_Event*>(m_pPlaybackCursor);
                const coreUintW  iEventSize = coreReplay::__GetEventSize(pSource->type);

                // read event from the file
                std::memcpy(pEvent, pSource, iEventSize);
                m_pPlaybackCursor += iEventSize;

                // denormalize event from portable format
                coreReplay::__DenormalizeEvent(pEvent);

                return true;
            }
        }
    }

    return false;
}


// ****************************************************************
/* write frame time */
coreBool coreReplay::WriteTime(const coreFloat& fTime)
{
    if(m_eMode == CORE_REPLAY_MODE_RECORDING)
    {
        // write frame time to the stream
        ASSERT(!HAS_FLAG(coreMath::FloatToBits(fTime), CORE_REPLAY_EVENT_MARKER))
        SDL_WriteIO(m_pRecordingStream, &fTime, sizeof(coreFloat));

        return true;
    }

    return false;
}


// ****************************************************************
/* read frame time */
coreBool coreReplay::ReadTime(coreFloat* OUTPUT pfTime)
{
    ASSERT(pfTime)

    if(m_eMode == CORE_REPLAY_MODE_PLAYBACK)
    {
        // check if playback is finished
        if(!this->__IsFinished())
        {
            // read frame time from the file
            std::memcpy(pfTime, m_pPlaybackCursor, sizeof(coreFloat));
            m_pPlaybackCursor += sizeof(coreFloat);

            return true;
        }
    }

    return false;
}


// ****************************************************************
/* normalize event into portable format */
const SDL_Event& coreReplay::__NormalizeEvent(const SDL_Event& oEvent)
{
    // copy event into work buffer
    static SDL_Event s_WorkEvent = {};
    std::memcpy(&s_WorkEvent, &oEvent, coreReplay::__GetEventSize(oEvent.type));

    // remove window coordinate size
    switch(oEvent.type)
    {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        s_WorkEvent.button.x /= Core::System->GetCoordSize().x;
        s_WorkEvent.button.y /= Core::System->GetCoordSize().y;
        break;

    case SDL_EVENT_MOUSE_WHEEL:
        s_WorkEvent.wheel.mouse_x /= Core::System->GetCoordSize().x;
        s_WorkEvent.wheel.mouse_y /= Core::System->GetCoordSize().y;
        break;

    case SDL_EVENT_FINGER_DOWN:
    case SDL_EVENT_FINGER_UP:
    case SDL_EVENT_FINGER_CANCELED:
        s_WorkEvent.tfinger.x /= Core::System->GetCoordSize().x;
        s_WorkEvent.tfinger.y /= Core::System->GetCoordSize().y;
        break;
    }

    return s_WorkEvent;
}


// ****************************************************************
/* denormalize event from portable format */
void coreReplay::__DenormalizeEvent(SDL_Event* OUTPUT pEvent)
{
    // add window coordinate size
    switch(pEvent->type)
    {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        pEvent->button.x *= Core::System->GetCoordSize().x;
        pEvent->button.y *= Core::System->GetCoordSize().y;
        break;

    case SDL_EVENT_MOUSE_WHEEL:
        pEvent->wheel.mouse_x *= Core::System->GetCoordSize().x;
        pEvent->wheel.mouse_y *= Core::System->GetCoordSize().y;
        break;

    case SDL_EVENT_FINGER_DOWN:
    case SDL_EVENT_FINGER_UP:
    case SDL_EVENT_FINGER_CANCELED:
        pEvent->tfinger.x *= Core::System->GetCoordSize().x;
        pEvent->tfinger.y *= Core::System->GetCoordSize().y;
        break;
    }
}


// ****************************************************************
/* get event size */
coreUintW coreReplay::__GetEventSize(const coreUint32 iEventType)
{
    // select specific event size (smaller than full event size)
    switch(iEventType)
    {
    case SDL_EVENT_KEY_DOWN:             return sizeof(SDL_KeyboardEvent);
    case SDL_EVENT_KEY_UP:               return sizeof(SDL_KeyboardEvent);
    case SDL_EVENT_MOUSE_BUTTON_DOWN:    return sizeof(SDL_MouseButtonEvent);
    case SDL_EVENT_MOUSE_BUTTON_UP:      return sizeof(SDL_MouseButtonEvent);
    case SDL_EVENT_MOUSE_WHEEL:          return sizeof(SDL_MouseWheelEvent);
    case SDL_EVENT_JOYSTICK_AXIS_MOTION: return sizeof(SDL_JoyAxisEvent);
    case SDL_EVENT_JOYSTICK_HAT_MOTION:  return sizeof(SDL_JoyHatEvent);
    case SDL_EVENT_JOYSTICK_BUTTON_DOWN: return sizeof(SDL_JoyButtonEvent);
    case SDL_EVENT_JOYSTICK_BUTTON_UP:   return sizeof(SDL_JoyButtonEvent);
    case SDL_EVENT_GAMEPAD_AXIS_MOTION:  return sizeof(SDL_GamepadAxisEvent);
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:  return sizeof(SDL_GamepadButtonEvent);
    case SDL_EVENT_GAMEPAD_BUTTON_UP:    return sizeof(SDL_GamepadButtonEvent);
    case SDL_EVENT_FINGER_DOWN:          return sizeof(SDL_TouchFingerEvent);
    case SDL_EVENT_FINGER_UP:            return sizeof(SDL_TouchFingerEvent);
    case SDL_EVENT_FINGER_CANCELED:      return sizeof(SDL_TouchFingerEvent);
    }

    // ignore event
    return 0u;
}