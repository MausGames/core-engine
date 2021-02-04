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
/* callback functions for the music stream object */
static const ov_callbacks OV_CALLBACKS =
{
    [](void* pData, coreUintW iSize, coreUintW iCount, void* pFile) {return coreUintW(SDL_RWread (s_cast<SDL_RWops*>(pFile), pData, iSize, iCount));},
    [](void* pFile, coreInt64 iOffset, coreInt32 iWhence)           {return coreInt32(SDL_RWseek (s_cast<SDL_RWops*>(pFile), iOffset, iWhence));},
    [](void* pFile)                                                 {return coreInt32(SDL_RWclose(s_cast<SDL_RWops*>(pFile)));},
    [](void* pFile)                                                 {return long     (SDL_RWtell (s_cast<SDL_RWops*>(pFile)));}
};


// ****************************************************************
/* constructor */
coreMusic::coreMusic(const coreChar* pcPath)noexcept
: coreMusic (Core::Manager::Resource->RetrieveFile(pcPath))
{
}

coreMusic::coreMusic(coreFile* pFile)noexcept
: m_aiBuffer {0u, 0u}
, m_iSource  (0u)
, m_fVolume  (1.0f)
, m_fPitch   (1.0f)
, m_bLoop    (false)
, m_bStatus  (false)
, m_pFile    (NULL)
, m_Stream   {}
, m_pInfo    (NULL)
, m_pComment (NULL)
, m_dMaxTime (0.0)
{
    if(!pFile)            return;
    if(!pFile->GetSize()) return;   // do not load file data

    // copy file object as streaming source
    coreFile::InternalNew(&m_pFile, pFile);

    // test file format and open music stream
    coreInt32   iError = ov_test_callbacks(m_pFile->CreateReadStream(), &m_Stream, NULL, 0, OV_CALLBACKS);
    if(!iError) iError = ov_test_open(&m_Stream);
    if( iError)
    {
        Core::Log->Warning("Music (%s) is not a valid OGG-file (OV Error Code: 0x%08X)", pFile->GetPath(), iError);
        ov_clear(&m_Stream);
        coreFile::InternalDelete(&m_pFile);

        return;
    }

    // create sound buffers
    alGenBuffers(2, m_aiBuffer);

    // retrieve music file information
    m_pInfo    = ov_info      (&m_Stream, -1);
    m_pComment = ov_comment   (&m_Stream, -1);
    m_dMaxTime = ov_time_total(&m_Stream, -1);

    Core::Log->Info("Music (%s, %.1f seconds) loaded", pFile->GetPath(), m_dMaxTime);
}


// ****************************************************************
/* destructor */
coreMusic::~coreMusic()
{
    // clear audio source and sound buffers
    this->Pause();
    alDeleteBuffers(2, m_aiBuffer);

    // close music stream
    ov_clear(&m_Stream);
    if(m_pFile) Core::Log->Info("Music (%s) unloaded", m_pFile->GetPath());

    // delete file object
    coreFile::InternalDelete(&m_pFile);
}


// ****************************************************************
/* update the music object */
coreBool coreMusic::Update()
{
    if(!m_bStatus) return false;

    // check for valid audio source
    if(!m_iSource)
    {
        if(this->Play() != CORE_OK)
            return false;
    }

    // get number of processed sound buffers
    ALint iProcessed;
    alGetSourcei(m_iSource, AL_BUFFERS_PROCESSED, &iProcessed);

    if(iProcessed)
    {
        ALuint iBuffer;

        // update the processed sound buffer
        alSourceUnqueueBuffers(m_iSource, 1, &iBuffer);
        if(!this->__Stream(iBuffer))
        {
            // music is finished
            this->Stop();
            if(m_bLoop) this->Play();

            return true;
        }
        alSourceQueueBuffers(m_iSource, 1, &iBuffer);

        // check for wrong status
        ALint iStatus;
        alGetSourcei(m_iSource, AL_SOURCE_STATE, &iStatus);
        if(iStatus != AL_PLAYING) alSourcePlay(m_iSource);
    }

    return false;
}


// ****************************************************************
/* play the music */
coreStatus coreMusic::Play()
{
    if(m_iSource) return CORE_INVALID_CALL;
    if(!m_pFile)  return CORE_INVALID_DATA;

    // retrieve next free audio source
    m_iSource = Core::Audio->NextSource(CORE_AUDIO_MUSIC_BUFFER, m_fVolume, 0u);
    if(m_iSource)
    {
        // prepare sound buffers
        this->__Stream(m_aiBuffer[0]);
        this->__Stream(m_aiBuffer[1]);

        // remove old and queue new sound buffers
        alSourcei(m_iSource, AL_BUFFER, 0);
        alSourceQueueBuffers(m_iSource, 2, m_aiBuffer);

        // set initial audio source properties
        alSourcei(m_iSource, AL_SOURCE_RELATIVE, true);
        alSourcef(m_iSource, AL_PITCH,           m_fPitch);
        alSourcei(m_iSource, AL_LOOPING,         false);

        // start playback
        alSourcePlay(m_iSource);
    }

    // set playback status (after init)
    m_bStatus = true;

    return m_iSource ? CORE_OK : CORE_BUSY;
}


// ****************************************************************
/* stop the music */
void coreMusic::Stop()
{
    // pause and rewind the music stream
    this->Pause();
    ov_time_seek_page_lap(&m_Stream, 0.0);
}


// ****************************************************************
/* pause the music */
void coreMusic::Pause()
{
    if(m_iSource)
    {
        ALuint iBuffer;

        // stop and clear audio source
        alSourceStop(m_iSource);
        alSourceUnqueueBuffers(m_iSource, 1, &iBuffer);
        alSourceUnqueueBuffers(m_iSource, 1, &iBuffer);
        m_iSource = 0;

        // reset playback status
        m_bStatus = false;
    }
}


// ****************************************************************
/* get specific meta-information */
const coreChar* coreMusic::GetComment(const coreChar* pcName)const
{
    if(m_pComment)
    {
        const coreUintW iLen = std::strlen(pcName);

        // loop through all comments
        for(coreUintW i = 0u, ie = m_pComment->comments; i < ie; ++i)
        {
            // check comment and extract meta-information
            if(!std::memcmp(pcName, m_pComment->user_comments[i], iLen))
                return m_pComment->user_comments[i] + iLen + 1;
        }
    }

    // specific meta-information not found
    return "";
}


// ****************************************************************
/* read from music stream and update sound buffer */
coreBool coreMusic::__Stream(const ALuint iBuffer)
{
    alignas(ALIGNMENT_PAGE) BIG_STATIC coreChar acData[4u * CORE_MUSIC_CHUNK];

    const coreInt32 iChunkSize = MIN(F_TO_UI(m_fPitch * I_TO_F(CORE_MUSIC_CHUNK)), 4u * CORE_MUSIC_CHUNK);
    coreInt32       iReadSize  = 0;

    // process the defined music stream chunk size
    while(iReadSize < iChunkSize)
    {
        // read and decode data from the music track
        const coreInt32 iResult = ov_read(&m_Stream, acData + iReadSize, iChunkSize - iReadSize, (SDL_BYTEORDER == SDL_BIG_ENDIAN) ? 1 : 0, 2, 1, NULL);

        if(iResult > 0) iReadSize += iResult;
        else break;
    }

    // music track finished
    if(iReadSize == 0) return false;

    // write decoded data to sound buffer
    alBufferData(iBuffer, (m_pInfo->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, acData, iReadSize, m_pInfo->rate);
    return true;
}


// ****************************************************************
/* constructor */
coreMusicPlayer::coreMusicPlayer()noexcept
: m_apMusic       {}
, m_pEmptyMusic   (NULL)
, m_apSequence    {}
, m_eRepeat       (CORE_MUSIC_ALL_REPEAT)
, m_FadeTimer     (coreTimer(1.0f, 0.0f, 1u))
, m_pFadePrevious (NULL)
, m_pCurMusic     (NULL)
, m_iCurIndex     (0u)
{
    // create empty music object
    m_pEmptyMusic = new coreMusic(r_cast<coreFile*>(0));
    m_pCurMusic   = m_pEmptyMusic;
}


// ****************************************************************
/* destructor */
coreMusicPlayer::~coreMusicPlayer()
{
    // remove all music objects
    this->ClearMusic();

    // delete empty music object
    SAFE_DELETE(m_pEmptyMusic)
}


// ****************************************************************
/* update the music-player */
coreBool coreMusicPlayer::Update()
{
    if(m_apMusic.empty()) return false;
    ASSERT(m_pCurMusic != m_pEmptyMusic)

    // update transition between two music objects
    if(m_FadeTimer.GetStatus())
    {
        if(m_FadeTimer.Update(1.0f))
            m_pFadePrevious->Stop();

        // adjust their volume
        m_pFadePrevious->SetVolume(m_FadeTimer.GetValue(CORE_TIMER_GET_REVERSED));
        m_pCurMusic    ->SetVolume(m_FadeTimer.GetValue(CORE_TIMER_GET_NORMAL));

        // update the previous music object
        m_pFadePrevious->Update();
    }

    // update the current music object
    if(m_pCurMusic->Update())
    {
        // handle unnecessary loop
        if((m_eRepeat != CORE_MUSIC_SINGLE_REPEAT))
            m_pCurMusic->Stop();

        // repeat, switch or stop as defined
        switch(m_eRepeat)
        {
        default: ASSERT(false)
        case CORE_MUSIC_ALL_NOREPEAT:    if((m_iCurIndex + 1u) >= m_apMusic.size()) break; FALLTHROUGH
        case CORE_MUSIC_ALL_REPEAT:      this->Next();                                     FALLTHROUGH
        case CORE_MUSIC_SINGLE_REPEAT:   m_pCurMusic->Play();                              FALLTHROUGH
        case CORE_MUSIC_SINGLE_NOREPEAT: break;
        }
        return true;
    }
    return false;
}


// ****************************************************************
/* order the playback sequence */
void coreMusicPlayer::Order()
{
    // reset playback sequence
    m_apSequence.clear();
    m_apSequence = m_apMusic;

    // switch to first music object
    this->Select(0u);
}


// ****************************************************************
/* shuffle the playback sequence */
void coreMusicPlayer::Shuffle()
{
    // reset playback sequence
    m_apSequence.clear();
    m_apSequence = m_apMusic;

    // shuffle the list
    coreData::Shuffle(m_apSequence.begin(), m_apSequence.end());

    // switch to first music object
    this->Select(0u);
}


// ****************************************************************
/* add music object from file */
coreStatus coreMusicPlayer::AddMusicFile(const coreChar* pcPath)
{
    // load from path
    return this->__AddMusic(Core::Manager::Resource->RetrieveFile(pcPath));
}


// ****************************************************************
/* add music objects from archive */
coreStatus coreMusicPlayer::AddMusicArchive(const coreChar* pcPath, const coreChar* pcFilter)
{
    coreBool bStatus = false;

    // retrieve archive with resource files
    coreArchive* pArchive = Core::Manager::Resource->RetrieveArchive(pcPath);

    // try to add all files to the music-player
    for(coreUintW i = 0u, ie = pArchive->GetNumFiles(); i < ie; ++i)
    {
        // check path and use only specific files
        if(coreData::StrCmpLike(pArchive->GetFile(i)->GetPath(), pcFilter))
        {
            if(this->__AddMusic(pArchive->GetFile(i)) == CORE_OK)
                bStatus = true;
        }
    }

    return bStatus ? CORE_OK : CORE_INVALID_INPUT;
}


// ****************************************************************
/* add music objects from folder */
coreStatus coreMusicPlayer::AddMusicFolder(const coreChar* pcPath, const coreChar* pcFilter)
{
    coreBool bStatus = false;

    // get specific files from the folder
    std::vector<std::string> asFolder;
    coreData::ScanFolder(pcPath, pcFilter, &asFolder);

    // try to add all files to the music-player
    FOR_EACH(it, asFolder)
    {
        if(this->AddMusicFile(it->c_str()) == CORE_OK)
            bStatus = true;
    }

    return bStatus ? CORE_OK : CORE_INVALID_INPUT;
}


// ****************************************************************
/* remove music object */
coreStatus coreMusicPlayer::DeleteMusic(const coreUintW iIndex)
{
    WARN_IF(iIndex >= m_apMusic.size()) return CORE_INVALID_INPUT;

    coreMusic* pMusic = m_apMusic[iIndex];

    // remove music object
    m_apMusic   .erase(m_apMusic.begin() + iIndex);
    m_apSequence.erase(std::find(m_apSequence.begin(), m_apSequence.end(), pMusic));

    // delete music object
    SAFE_DELETE(pMusic)

    // check and switch the current music object
    if(m_apMusic.empty()) m_pCurMusic = m_pEmptyMusic;
    else this->Select(0u);

    return CORE_OK;
}


// ****************************************************************
/* remove all music objects */
void coreMusicPlayer::ClearMusic()
{
    // delete music objects
    FOR_EACH(it, m_apMusic)
        SAFE_DELETE(*it)

    // clear memory
    m_apMusic   .clear();
    m_apSequence.clear();

    // reset current music object
    m_pCurMusic = m_pEmptyMusic;
}


// ****************************************************************
/* switch to specific music object */
void coreMusicPlayer::Select(const coreUintW iIndex)
{
    WARN_IF(iIndex >= m_apMusic.size())     return;
    if(m_pCurMusic == m_apSequence[iIndex]) return;

    // get playback status
    const coreBool bStatus = m_pCurMusic->IsPlaying();

    if(bStatus)
    {
        if(m_FadeTimer.GetSpeed())
        {
            // start transition to new music object
            m_FadeTimer.Play(CORE_TIMER_PLAY_RESET);
            m_pFadePrevious = m_pCurMusic;
        }
        else
        {
            // stop old music object
            m_pCurMusic->Stop();
            coreMusic::CrossLap(m_pCurMusic, m_apSequence[iIndex]);
        }
    }

    // set new music object
    m_iCurIndex = iIndex;
    m_pCurMusic = m_apSequence[iIndex];

    // adjust volume and status
    m_pCurMusic->SetVolume(m_FadeTimer.GetStatus() ? 0.0f : 1.0f);
    if(bStatus) m_pCurMusic->Play();
}


// ****************************************************************
/* switch to next music object */
coreBool coreMusicPlayer::Next()
{
    if((m_iCurIndex + 1u) >= m_apMusic.size())
    {
        // back to the beginning
        this->Select(0u);
        return true;
    }

    // go to next music object
    this->Select(m_iCurIndex + 1u);
    return false;
}


// ****************************************************************
/* switch to previous music object */
coreBool coreMusicPlayer::Previous()
{
    if(m_iCurIndex == 0u)
    {
        // back to the end
        this->Select(m_apMusic.size() - 1u);
        return true;
    }

    // go to previous music object
    this->Select(m_iCurIndex - 1u);
    return false;
}


// ****************************************************************
/* add music object */
coreStatus coreMusicPlayer::__AddMusic(coreFile* pFile)
{
    // create new music object
    coreMusic* pNewMusic = new coreMusic(pFile);
    if(!pNewMusic->GetInfo())
    {
        // remove invalid file
        SAFE_DELETE(pNewMusic)
        return CORE_INVALID_INPUT;
    }

    // add music object to the music-player
    m_apMusic   .push_back(pNewMusic);
    m_apSequence.push_back(pNewMusic);

    // init the access pointer
    if(m_pCurMusic == m_pEmptyMusic) m_pCurMusic = pNewMusic;

    return CORE_OK;
}