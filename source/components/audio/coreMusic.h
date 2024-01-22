///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MUSIC_H_
#define _CORE_GUARD_MUSIC_H_

// TODO 3: player: callback when music track changes (or return true is enough probably, not with threading)
// TODO 4: player: reorder properties, SwitchBox is similar
// TODO 4: split up: coreMusicFile.cpp, coreMusicPlayer.cpp
// TODO 5: <old comment style>
// TODO 3: on play, only stream first buffer, and move streaming other buffers to next iterations (queue them empty ?) (on update still keep loop, to handle possible catch-up, to not decay to single-buffering and stuttering if chunks-per-iteration > 1.0)
// TODO 3: in the end of a track, if sound-buffers are not re-queued anymore (on norepeat, last track), and then switching to a different track, buffering-chain is broken which can cause stuttering
// TODO 3: change std::recursive_mutex to SDL_Mutex ? (is also recursive) (needs many functions, maybe wrap into coreMutex)


// ****************************************************************
/* music definitions */
#define CORE_MUSIC_CHUNK   (DEFINED(_CORE_EMSCRIPTEN_) ? 0x2000u : 0x1000u)   // size of a music stream chunk in floats/shorts (on speed 1.0x)
#define CORE_MUSIC_BUFFERS (DEFINED(_CORE_EMSCRIPTEN_) ? 4u      : 3u)        // number of sound buffers (with chunks)

#define __CORE_MUSIC_LOCKER const std::lock_guard<std::recursive_mutex> oLocker(m_Mutex);

enum coreMusicRepeat : coreUint8
{
    CORE_MUSIC_SINGLE_NOREPEAT = 0u,
    CORE_MUSIC_SINGLE_REPEAT   = 1u,
    CORE_MUSIC_ALL_NOREPEAT    = 2u,
    CORE_MUSIC_ALL_REPEAT      = 3u
};


// ****************************************************************
/* music class */
class coreMusic final
{
private:
    coreFile*   m_pFile;               // file object with streaming data
    stb_vorbis* m_pStream;             // music stream object

    stb_vorbis_info    m_Info;         // format of the music file
    stb_vorbis_comment m_Comment;      // meta-information
    coreUint32         m_iMaxSample;   // length of the music track (in samples)
    coreFloat          m_fMaxTime;     // length of the music track (in seconds)

    coreBool m_bLoop;                  // individual loop status


public:
    explicit coreMusic(const coreChar* pcPath)noexcept;
    explicit coreMusic(coreFile*       pFile)noexcept;
    ~coreMusic();

    FRIEND_CLASS(coreMusicPlayer)
    DISABLE_COPY(coreMusic)

    /* change and retrieve current music track position */
    inline void Rewind    ()                          {stb_vorbis_seek_start(m_pStream);}
    inline void SeekSample(const coreUint32 iSamples) {stb_vorbis_seek(m_pStream, iSamples); ASSERT(iSamples <= m_iMaxSample)}
    inline void SeekFactor(const coreDouble dFactor)  {this->SeekSample(F_TO_UI   (dFactor   * coreDouble(m_iMaxSample)));}
    inline void SeekTime  (const coreFloat  fSeconds) {this->SeekFactor(coreDouble(fSeconds) / coreDouble(m_fMaxTime));}
    inline coreUint32 TellSample()const               {return stb_vorbis_get_sample_offset(m_pStream);}
    inline coreDouble TellFactor()const               {return coreDouble(this->TellSample()) / coreDouble(m_iMaxSample);}
    inline coreFloat  TellTime  ()const               {return coreFloat (this->TellFactor()  * coreDouble(m_fMaxTime));}

    /* get meta-information */
    const coreChar* GetComment(const coreChar* pcName)const;
    inline const coreChar* GetArtist()const {return this->GetComment("ARTIST");}
    inline const coreChar* GetTitle ()const {return this->GetComment("TITLE");}

    /* set object properties */
    inline void SetLoop(const coreBool bLoop) {m_bLoop = bLoop;}

    /* get object properties */
    inline const coreChar*        GetPath     ()const {return m_pFile ? m_pFile->GetPath() : "";}
    inline const stb_vorbis_info& GetInfo     ()const {return m_Info;}
    inline const coreUint32&      GetMaxSample()const {return m_iMaxSample;}
    inline const coreFloat&       GetMaxTime  ()const {return m_fMaxTime;}
    inline const coreBool&        GetLoop     ()const {return m_bLoop;}
};


// ****************************************************************
/* music-player class */
class coreMusicPlayer final : public coreThread
{
private:
    ALuint m_aiBuffer[CORE_MUSIC_BUFFERS];   // sound buffers for streaming
    ALuint m_iSource;                        // currently used audio source

    coreFloat m_fVolume;                     // current volume
    coreFloat m_fPitch;                      // current playback speed
    coreBool  m_bStatus;                     // playback status

    coreMapStr<coreMusic*> m_apMusic;        // music objects

    coreMusicRepeat m_eRepeat;               // repeat behavior

    coreMusic* m_pCurMusic;                  // current music object
    coreUintW  m_iCurIndex;                  // index of the current music object

    std::recursive_mutex m_Mutex;            // recursive mutex for asynchronous streaming


public:
    coreMusicPlayer()noexcept;
    ~coreMusicPlayer()final;

    DISABLE_COPY(coreMusicPlayer)

    /* update the music-player */
    coreBool Update();

    /* control playback */
    coreStatus Play ();
    void       Stop ();
    void       Pause();
    inline const coreBool& IsPlaying()const {return m_bStatus;}

    /* set various audio source properties */
    inline void SetVolume(const coreFloat fVolume) {__CORE_MUSIC_LOCKER if(m_iSource && (m_fVolume != fVolume)) Core::Audio->UpdateSource(m_iSource, fVolume); m_fVolume = fVolume; ASSERT((fVolume >= 0.0f) && (fVolume <= CORE_AUDIO_MAX_GAIN))}
    inline void SetPitch (const coreFloat fPitch)  {__CORE_MUSIC_LOCKER if(m_iSource && (m_fPitch  != fPitch))  alSourcef(m_iSource, AL_PITCH, fPitch);        m_fPitch  = fPitch;  ASSERT((fPitch  >= 0.5f) && (fPitch  <= 2.0f))}

    /* manage music objects */
    coreStatus AddMusicFile   (const coreChar* pcPath);
    coreStatus AddMusicArchive(const coreChar* pcPath, const coreChar* pcFilter);
    coreStatus AddMusicFolder (const coreChar* pcPath, const coreChar* pcFilter);
    coreStatus DeleteMusic    (const coreUintW iIndex);
    void ClearMusic();

    /* switch current music object */
    void Select    (const coreUintW       iIndex);
    void SelectName(const coreHashString& sName);
    coreBool Next    ();
    coreBool Previous();

    /* access music objects */
    inline coreMusic* GetMusic    (const coreUintW       iIndex)const {return (iIndex < m_apMusic.size()) ? m_apMusic[iIndex] : NULL;}
    inline coreMusic* GetMusicName(const coreHashString& sName)const  {return this->GetMusic(m_apMusic.index(sName));}

    /* set object properties */
    inline void SetRepeat(const coreMusicRepeat eRepeat) {__CORE_MUSIC_LOCKER m_eRepeat = eRepeat;}

    /* get object properties */
    inline       coreUintW        GetNumMusic()const {return m_apMusic.size();}
    inline       coreMusic*       GetCurMusic()const {return m_pCurMusic;}
    inline const coreUintW&       GetCurIndex()const {return m_iCurIndex;}
    inline const coreMusicRepeat& GetRepeat  ()const {return m_eRepeat;}


private:
    /* music thread implementations */
    coreStatus __InitThread()final;
    coreStatus __RunThread ()final;
    void       __ExitThread()final;

    /* process sound queue */
    coreBool __ProcessQueue();

    /* read from music stream and update sound buffer */
    coreBool  __Stream    (const ALuint  iBuffer);
    coreUintW __StreamList(const ALuint* piBuffer, const coreUintW iCount);

    /* add music object */
    coreStatus __AddMusic(coreFile* pFile);
};


#endif /* _CORE_GUARD_MUSIC_H_ */