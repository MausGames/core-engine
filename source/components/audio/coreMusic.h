//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MUSIC_H_
#define _CORE_GUARD_MUSIC_H_

// TODO: consider threaded music update (->coreMusicPlayer)
// TODO: implement global music volume change (music-player changes volume already)
// TODO: music which is inactive should not be in memory (maybe MusicPlayer?), too heavy with many files
// TODO: player: improve the representation of the current track while shuffled
// TODO: player: callback when music track changes (or return true is enough probably)
// TODO: player: own thread
// TODO: player: own (cached) parameters like volume, pitch - forwarded to music files
// TODO: player: reorder properties, SwitchBox is similar


// ****************************************************************
// music definitions
#define CORE_MUSIC_CHUNK (0x4000)   //!< size of a music stream chunk in bytes

enum coreMusicRepeat : coreUint8
{
    CORE_MUSIC_SINGLE_NOREPEAT = 0,
    CORE_MUSIC_SINGLE_REPEAT   = 1,
    CORE_MUSIC_ALL_NOREPEAT    = 2,
    CORE_MUSIC_ALL_REPEAT      = 3
};


// ****************************************************************
// music class
class coreMusic final
{
private:
    ALuint m_aiBuffer[2];         //!< sound buffers for streaming
    ALuint m_iSource;             //!< currently used sound source

    coreFile*      m_pFile;       //!< file object with streaming data
    OggVorbis_File m_Stream;      //!< music stream object

    vorbis_info*    m_pInfo;      //!< format of the music file
    vorbis_comment* m_pComment;   //!< meta-information
    coreDouble      m_dMaxTime;   //!< length of the music track in seconds

    coreFloat m_fVolume;          //!< current volume
    coreFloat m_fPitch;           //!< current playback speed
    coreBool  m_bLoop;            //!< loop status
    coreBool  m_bStatus;          //!< playback status


public:
    explicit coreMusic(const coreChar* pcPath)noexcept;
    explicit coreMusic(coreFile*       pFile)noexcept;
    ~coreMusic();

    DISABLE_COPY(coreMusic)

    //! update the music object
    //! @{
    coreBool Update();
    //! @}

    //! control playback
    //! @{
    coreStatus Play ();
    void       Stop ();
    void       Pause();
    inline const coreBool& IsPlaying()const {return m_bStatus;}
    //! @}

    //! set various sound source properties
    //! @{
    inline void SetVolume(const coreFloat& fVolume) {if(m_iSource && (m_fVolume != fVolume)) {m_fVolume = fVolume; alSourcef(m_iSource, AL_GAIN,  fVolume);} ASSERT(fVolume >= 0.0f)}
    inline void SetPitch (const coreFloat& fPitch)  {if(m_iSource && (m_fPitch  != fPitch))  {m_fPitch  = fPitch;  alSourcef(m_iSource, AL_PITCH, fPitch);}  ASSERT(fPitch  >= 0.0f)}
    inline void SetLoop  (const coreBool&  bLoop)   {m_bLoop = bLoop;}
    //! @}

    //! change and retrieve current music track position
    //! @{
    inline void SeekRaw   (const coreInt64& iBytes)    {ov_raw_seek_lap (&m_Stream, iBytes);}
    inline void SeekPcm   (const coreInt64& iSamples)  {ov_pcm_seek_lap (&m_Stream, iSamples);}
    inline void SeekTime  (const coreDouble& dSeconds) {ov_time_seek_lap(&m_Stream, dSeconds);}
    inline void SeekFactor(const coreDouble& dFactor)  {ov_time_seek_lap(&m_Stream, dFactor * m_dMaxTime);}
    inline coreInt64  TellRaw   ()                     {return ov_raw_tell (&m_Stream);}
    inline coreInt64  TellPcm   ()                     {return ov_pcm_tell (&m_Stream);}
    inline coreDouble TellTime  ()                     {return ov_time_tell(&m_Stream);}
    inline coreDouble TellFactor()                     {return ov_time_tell(&m_Stream) / m_dMaxTime;}
    //! @}

    //! get meta-information
    //! @{
    const coreChar* GetComment(const coreChar* pcName)const;
    inline const coreChar* GetArtist()const {return this->GetComment("ARTIST");}
    inline const coreChar* GetTitle ()const {return this->GetComment("TITLE");}
    //! @}

    //! get object properties
    //! @{
    inline const coreChar*    GetPath   ()const {return m_pFile->GetPath();}
    inline const vorbis_info* GetInfo   ()const {return m_pInfo;}
    inline const coreDouble&  GetMaxTime()const {return m_dMaxTime;}
    //! @}

    //! lap streams between two different music objects
    //! @{
    static coreBool CrossLap(coreMusic* pFirst, coreMusic* pSecond) {ASSERT(pFirst != pSecond) return ov_crosslap(&pFirst->m_Stream, &pSecond->m_Stream) ? false : true;}
    //! @}


private:
    //! read from music stream and update sound buffer
    //! @{
    coreBool __Stream(const ALuint& iBuffer);
    //! @}
};


// ****************************************************************
// music-player class
class coreMusicPlayer final
{
private:
    std::vector<coreMusic*> m_apMusic;      //!< music objects
    coreMusic* m_pEmptyMusic;               //!< empty music object

    std::vector<coreMusic*> m_apSequence;   //!< playback sequence
    coreMusicRepeat m_iRepeat;              //!< repeat behavior

    coreMusic* m_pCurMusic;                 //!< current music object
    coreUintW  m_iCurIndex;                 //!< sequence-index of the current music object

    coreTimer  m_FadeTimer;                 //!< timer for a transition between two music objects
    coreMusic* m_pFadePrevious;             //!< previous music object during a transition


public:
    coreMusicPlayer()noexcept;
    ~coreMusicPlayer();

    DISABLE_COPY(coreMusicPlayer)

    //! update the music-player
    //! @{
    coreBool Update();
    //! @}

    //! control playback behavior
    //! @{
    void Order();
    void Shuffle();
    //! @}

    //! manage music objects
    //! @{
    coreStatus AddMusicFile   (const coreChar*  pcPath);
    coreStatus AddMusicArchive(const coreChar*  pcPath, const coreChar* pcFilter);
    coreStatus AddMusicFolder (const coreChar*  pcPath, const coreChar* pcFilter);
    coreStatus DeleteMusic    (const coreUintW& iIndex);
    void ClearMusic();
    //! @}

    //! switch current music object
    //! @{
    void Select(const coreUintW& iIndex);
    coreBool Next    ();
    coreBool Previous();
    //! @}

    //! access music objects
    //! @{
    inline const coreMusic* GetMusic(const coreUintW& iIndex)const {WARN_IF(iIndex >= m_apMusic.size()) return m_pEmptyMusic; return m_apMusic[iIndex];}
    inline       coreMusic* Control()const                         {ASSERT(m_pCurMusic != m_pEmptyMusic) return m_pCurMusic;}
    //! @}

    //! set object properties
    //! @{
    inline void SetRepeat(const coreMusicRepeat& iRepeat) {m_iRepeat = iRepeat;}
    inline void SetFade  (const coreFloat&       fTime)   {if(fTime) m_FadeTimer.SetSpeed(RCP(fTime)); else {m_FadeTimer.SetValue(1.0f); m_FadeTimer.SetSpeed(0.0f);}}
    //! @}

    //! get object properties
    //! @{
    inline       coreUintW        GetNumMusic()const {return m_apMusic.size();}
    inline const coreUintW&       GetCurIndex()const {return m_iCurIndex;}
    inline const coreMusicRepeat& GetRepeat  ()const {return m_iRepeat;}
    //! @}


private:
    //! add music object
    //! @{
    coreStatus __AddMusic(coreFile* pFile);
    //! @}
};


#endif // _CORE_GUARD_MUSIC_H_