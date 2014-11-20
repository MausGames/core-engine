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


// ****************************************************************
// music definitions
#define CORE_MUSIC_CHUNK   (0x4000)   //!< size of a music stream chunk in bytes

enum coreMusicRepeat
{
    CORE_MUSIC_SINGLE_NOREPEAT = 0,
    CORE_MUSIC_SINGLE_REPEAT   = 1,
    CORE_MUSIC_ALL_NOREPEAT    = 2,
    CORE_MUSIC_ALL_REPEAT      = 3
};


// ****************************************************************
// music class
// TODO: consider threaded music update (->coreMusicPlayer)
// TODO: implement global music volume change (music-player changes volume already)
// TODO: music which is inactive should not be in memory (maybe MusicPlayer?), too heavy with many files
class coreMusic final
{
private:
    ALuint m_aiBuffer[2];         //!< sound buffers for streaming
    ALuint m_iSource;             //!< currently used sound source

    coreFile*      m_pFile;       //!< file object with streaming data
    OggVorbis_File m_Stream;      //!< music stream object

    vorbis_info*    m_pInfo;      //!< format of the music file
    vorbis_comment* m_pComment;   //!< meta-information
    double m_dMaxTime;            //!< length of the music track in seconds

    float m_fPitch;               //!< current playback speed
    bool  m_bLoop;                //!< loop status
    bool  m_bStatus;              //!< playback status


public:
    explicit coreMusic(const char* pcPath)noexcept;
    explicit coreMusic(coreFile*   pFile)noexcept;
    ~coreMusic();

    //! update the music object
    //! @{
    bool Update();
    //! @}

    //! control playback
    //! @{
    coreError Play ();
    void      Stop ();
    void      Pause();
    inline const bool& IsPlaying()const {return m_bStatus;}
    //! @}

    //! set various sound source properties
    //! @{
    inline void SetVolume(const float& fVolume) {if(m_iSource) alSourcef(m_iSource, AL_GAIN, fVolume);}
    inline void SetPitch (const float& fPitch)  {if(m_iSource && m_fPitch != fPitch) {m_fPitch = fPitch; alSourcef(m_iSource, AL_PITCH, fPitch);}}
    inline void SetLoop  (const bool&  bLoop)   {m_bLoop = bLoop;}
    //! @}

    //! change and retrieve current music track position
    //! @{
    inline void SeekRaw   (const ogg_int64_t& iBytes)   {ov_raw_seek_lap (&m_Stream, iBytes);}
    inline void SeekPcm   (const ogg_int64_t& iSamples) {ov_pcm_seek_lap (&m_Stream, iSamples);}
    inline void SeekTime  (const double&      dSeconds) {ov_time_seek_lap(&m_Stream, dSeconds);}
    inline void SeekFactor(const double&      dFactor)  {ov_time_seek_lap(&m_Stream, dFactor * m_dMaxTime);}
    inline ogg_int64_t TellRaw   ()                     {return ov_raw_tell (&m_Stream);}
    inline ogg_int64_t TellPcm   ()                     {return ov_pcm_tell (&m_Stream);}
    inline double      TellTime  ()                     {return ov_time_tell(&m_Stream);}
    inline double      TellFactor()                     {return ov_time_tell(&m_Stream) / m_dMaxTime;}
    //! @}

    //! get meta-information
    //! @{
    const char* GetComment(const char* pcName)const;
    inline const char* GetArtist()const {return this->GetComment("ARTIST");}
    inline const char* GetTitle ()const {return this->GetComment("TITLE");}
    //! @}

    //! get object properties
    //! @{
    inline const char*        GetPath   ()const {return m_pFile->GetPath();}
    inline const vorbis_info* GetInfo   ()const {return m_pInfo;}
    inline const double&      GetMaxTime()const {return m_dMaxTime;}
    //! @}

    //! lap streams between two different music objects
    //! @{
    static bool CrossLap(coreMusic* pFirst, coreMusic* pSecond) {ASSERT(pFirst != pSecond) return ov_crosslap(&pFirst->m_Stream, &pSecond->m_Stream) ? false : true;}
    //! @}


private:
    //! read from music stream and update sound buffer
    //! @{
    bool __Stream(const ALuint& iBuffer)hot_func;
    //! @}
};


// ****************************************************************
// music-player class
// TODO: improve the representation of the current track while shuffled
// TODO: callback when music track changes (or return true is enough probably)
// TODO: own thread
// TODO: own (cached) parameters like volume, pitch - forwarded to music files
// TODO: reorder properties, SwitchBox is similar
class coreMusicPlayer final
{
private:
    std::vector<coreMusic*> m_apMusic;      //!< music objects
    coreMusic* m_pEmptyMusic;               //!< empty music object

    std::vector<coreMusic*> m_apSequence;   //!< playback sequence
    coreMusicRepeat m_iRepeat;              //!< repeat behavior

    coreMusic* m_pCurMusic;                 //!< current music object
    coreUint   m_iCurIndex;                 //!< sequence-index of the current music object

    coreTimer  m_FadeTimer;                 //!< timer for a transition between two music objects
    coreMusic* m_pFadePrevious;             //!< previous music object during a transition


public:
    coreMusicPlayer()noexcept;
    ~coreMusicPlayer();

    //! update the music-player
    //! @{
    bool Update();
    //! @}

    //! control playback behavior
    //! @{
    void Order();
    void Shuffle();
    //! @}

    //! manage music objects
    //! @{
    coreError AddMusicFile   (const char* pcPath);
    coreError AddMusicArchive(const char* pcPath, const char* pcFilter);
    coreError AddMusicFolder (const char* pcPath, const char* pcFilter);
    coreError DeleteMusic    (const coreUint& iIndex);
    void ClearMusic();
    //! @}

    //! switch current music object
    //! @{
    void Select(const coreUint& iIndex);
    bool Next    ();
    bool Previous();
    //! @}

    //! access music objects
    //! @{
    inline const coreMusic* GetMusic(const coreUint& iIndex)const {WARN_IF(iIndex >= m_apMusic.size()) return m_pEmptyMusic; return m_apMusic[iIndex];}
    inline coreMusic*       Control()const                        {ASSERT(m_pCurMusic != m_pEmptyMusic) return m_pCurMusic;}
    //! @}

    //! set object properties
    //! @{
    inline void SetRepeat(const coreMusicRepeat& iRepeat) {m_iRepeat = iRepeat;}
    inline void SetFade  (const float&           fTime)   {if(fTime) m_FadeTimer.SetSpeed(RCP(fTime)); else {m_FadeTimer.SetValue(1.0f); m_FadeTimer.SetSpeed(0.0f);}}
    //! @}

    //! get object properties
    //! @{
    inline coreUint               GetNumMusic()const {return m_apMusic.size();}
    inline const coreUint&        GetCurIndex()const {return m_iCurIndex;}
    inline const coreMusicRepeat& GetRepeat  ()const {return m_iRepeat;}
    //! @}


private:
    DISABLE_COPY(coreMusicPlayer)

    //! add music object
    //! @{
    coreError __AddMusic(coreFile* pFile);
    //! @}
};


#endif // _CORE_GUARD_MUSIC_H_