///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_REPLAY_H_
#define _CORE_GUARD_REPLAY_H_

// TODO 3: save and override coreConfig
// TODO 3: save and override coreRand
// TODO 3: EndRecording, EndPlayback ?


// ****************************************************************
/* replay definitions */
#define CORE_REPLAY_EXTENSION    "crp"                    // default file extension of core-replays
#define CORE_REPLAY_MAGIC        (UINT_LITERAL("CRP0"))   // magic number of core-replays
#define CORE_REPLAY_VERSION      (0x00000001u)            // current file version of core-replays
#define CORE_REPLAY_EVENT_MARKER (BIT(31u))               // mark next data as event

enum coreReplayMode : coreUint8
{
    CORE_REPLAY_MODE_DISABLED  = 0u,   // replay disabled
    CORE_REPLAY_MODE_RECORDING = 1u,   // currently recording
    CORE_REPLAY_MODE_PLAYBACK  = 2u    // currently playing back
};


// ****************************************************************
/* replay class */
class coreReplay final
{
private:
    coreString     m_sPath;              // relative path of the file
    coreReplayMode m_eMode;              // replay mode

    SDL_IOStream* m_pRecordingStream;    // recording stream (for continuous writing)

    coreFile*       m_pPlaybackFile;     // playback file
    const coreByte* m_pPlaybackCursor;   // current playback file location


public:
    coreReplay()noexcept;
    ~coreReplay();

    DISABLE_COPY(coreReplay)

    /* start recording or playback */
    coreStatus StartRecording(const coreChar* pcPath);
    coreStatus StartPlayback (const coreChar* pcPath);

    /* apply default command line */
    void ApplyCommandLine();

    /* handle events */
    coreBool WriteEvent(const SDL_Event&  oEvent);
    coreBool ReadEvent (SDL_Event* OUTPUT pEvent);

    /* handle frame times */
    coreBool WriteTime(const coreFloat&  fTime);
    coreBool ReadTime (coreFloat* OUTPUT pfTime);

    /* get object properties */
    inline const coreChar*       GetPath()const {return m_sPath.c_str();}
    inline const coreReplayMode& GetMode()const {return m_eMode;}


private:
    /* check if playback is finished */
    inline coreBool __IsFinished()const {ASSERT(m_pPlaybackFile) return (m_pPlaybackCursor >= m_pPlaybackFile->GetData() + m_pPlaybackFile->GetSize());}

    /* process events with portable format */
    static const SDL_Event& __NormalizeEvent  (const SDL_Event&  oEvent);
    static void             __DenormalizeEvent(SDL_Event* OUTPUT pEvent);

    /* get event size */
    static coreUintW __GetEventSize(const coreUint32 iEventType);
};


#endif /* _CORE_GUARD_REPLAY_H_ */