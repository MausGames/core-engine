//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// callback functions for the music stream object
size_t WrapFread(void* pData, size_t iSize, size_t iCount, SDL_RWops* pFile) {return SDL_RWread(pFile, pData, iSize, iCount);}
int    WrapFseek(SDL_RWops* pFile, ogg_int64_t iOffset, int iWhence)         {return SDL_RWseek(pFile, iOffset, iWhence);}
int    WrapFclose(SDL_RWops* pFile)                                          {return SDL_RWclose(pFile);}
long   WrapFtell(SDL_RWops* pFile)                                           {return SDL_RWtell(pFile);}

ov_callbacks OV_CALLBACKS =
{
    (size_t (*)(void*, size_t, size_t, void*)) WrapFread,
    (int    (*)(void*, ogg_int64_t, int))      WrapFseek,
    (int    (*)(void*))                        WrapFclose,
    (long   (*)(void*))                        WrapFtell
};


// ****************************************************************
// constructor
coreMusic::coreMusic(const char* pcPath)
: m_iSource  (0)
, m_pFile    (NULL)
, m_pInfo    (NULL)
, m_pComment (NULL)
, m_dMaxTime (0.0)
, m_bLoop    (false)
, m_bStatus  (false)
{
    // reset memory
    memset(&m_aiBuffer, 0, sizeof(m_aiBuffer));
    memset(&m_Stream,   0, sizeof(m_Stream));

    // load from path
    coreFile File(pcPath);
    this->__Init(&File);
}

coreMusic::coreMusic(coreFile* pFile)
: m_iSource  (0)
, m_pFile    (NULL)
, m_pInfo    (NULL)
, m_pComment (NULL)
, m_dMaxTime (0.0)
, m_bLoop    (false)
, m_bStatus  (false)
{
    // reset memory
    memset(&m_aiBuffer, 0, sizeof(m_aiBuffer));
    memset(&m_Stream,   0, sizeof(m_Stream));

    // load from file
    this->__Init(pFile);
}


// ****************************************************************
// destructor
coreMusic::~coreMusic()
{
    // clear sound source and sound buffers
    this->Pause();
    alDeleteBuffers(2, m_aiBuffer);

    // close music stream
    ov_clear(&m_Stream);
    if(m_pFile) Core::Log->Info(coreUtils::Print("Music (%s) unloaded", m_pFile->GetPath()));

    // delete file object
    SAFE_DELETE(m_pFile)
}


// ****************************************************************
// reset the object with the resource manager
void coreMusic::Reset(const bool& bInit)
{
    if(bInit)
    {
        // create sound buffers
        alGenBuffers(2, m_aiBuffer);
    }
    else
    {
        if(m_bStatus)
        {
            // clear sound source
            this->Pause();
            m_bStatus = true;
        }

        // delete sound buffers
        alDeleteBuffers(2, m_aiBuffer);
    }
}


// ****************************************************************
// update the music object
bool coreMusic::Update()
{
    if(!m_bStatus) return false;

    // check for valid sound source
    if(!m_iSource)
    {
        this->Play();
        if(!m_iSource) return false;
    }

    // get number of processed sound buffers
    int iProcessed;
    alGetSourcei(m_iSource, AL_BUFFERS_PROCESSED, &iProcessed);

    if(iProcessed == 1)
    {
        ALuint iBuffer;

        // update the processed sound buffer
        alSourceUnqueueBuffers(m_iSource, 1, &iBuffer);
        this->__Stream(iBuffer);
        alSourceQueueBuffers(m_iSource, 1, &iBuffer);
    }
    else if(iProcessed == 2)
    {
        // check current track position
        if(this->TellTime() >= this->GetMaxTime())
        {
            // music is finished
            this->Stop();
            if(m_bLoop) this->Play();

            return true;
        }
        else
        {
            // music was interrupted
            this->Pause();
            this->Play();
        }
    }

    return false;
}


// ****************************************************************
// play the music
coreError coreMusic::Play()
{
    if(m_iSource) return CORE_INVALID_CALL;
    if(!m_pFile)  return CORE_INVALID_DATA;

    // set playback status
    m_bStatus = true;

    // retrieve next free sound source
    m_iSource = Core::Audio->NextSource(this);
    if(m_iSource)
    {
        // prepare and queue sound buffers
        this->__Stream(m_aiBuffer[0]);
        this->__Stream(m_aiBuffer[1]);
        alSourceQueueBuffers(m_iSource, 2, m_aiBuffer);

        // set initial sound source properties
        alSourcei(m_iSource, AL_SOURCE_RELATIVE, true);
        alSourcef(m_iSource, AL_GAIN,            1.0f * Core::Config->GetFloat(CORE_CONFIG_AUDIO_VOLUME_MUSIC));
        alSourcef(m_iSource, AL_PITCH,           1.0f);
        alSourcei(m_iSource, AL_LOOPING,         false);

        // start playback
        alSourcePlay(m_iSource);
        return CORE_OK;
    }
    return CORE_BUSY;
}


// ****************************************************************
// stop the music
void coreMusic::Stop()
{
    // pause and rewind the music stream
    this->Pause();
    this->SeekTime(0.0);
}


// ****************************************************************
// pause the music
void coreMusic::Pause()
{
    if(m_iSource)
    {
        ALuint aiBuffer[2];

        // stop and clear sound source
        alSourceStop(m_iSource);
        alSourceUnqueueBuffers(m_iSource, 2, aiBuffer);
        m_iSource = 0;

        // reset playback status
        m_bStatus = false;
    }
}


// ****************************************************************
// change the sound source volume
void coreMusic::SetVolume(const float& fVolume)
{
    if(m_iSource) alSourcef(m_iSource, AL_GAIN, fVolume * Core::Config->GetFloat(CORE_CONFIG_AUDIO_VOLUME_MUSIC));
}


// ****************************************************************
// get specific meta-information
const char* coreMusic::GetComment(const char* pcName)const
{
    if(m_pComment)
    {
        const coreUint iLen = strlen(pcName);

        // traverse all comments
        for(int i = 0; i < m_pComment->comments; ++i)
        {
            // check comment and extract meta-information
            if(!strncmp(pcName, m_pComment->user_comments[i], iLen))
                return m_pComment->user_comments[i] + iLen+1;
        }
    }

    // specific meta-information not found
    return "";
}


// ****************************************************************
// init the music object
coreError coreMusic::__Init(coreFile* pFile)
{
    if(!pFile)            return CORE_INVALID_INPUT;
    if(!pFile->GetData()) return CORE_FILE_ERROR;

    // create virtual file as streaming source
    m_pFile = new coreFile(pFile->GetPath(), pFile->MoveData(), pFile->GetSize());
    SDL_RWops* pSource = SDL_RWFromConstMem(m_pFile->GetData(), m_pFile->GetSize());

    // test file format and open music stream
            int iError = ov_test_callbacks(pSource, &m_Stream, NULL, 0, OV_CALLBACKS);
    if(!iError) iError = ov_test_open(&m_Stream);
    if( iError)
    {
        Core::Log->Error(0, coreUtils::Print("Music (%s) is not a valid OGG-file (OV Error Code: %d)", pFile->GetPath(), iError));
        ov_clear(&m_Stream);
        SAFE_DELETE(m_pFile)

        return CORE_INVALID_DATA;
    }

    // create sound buffers
    alGenBuffers(2, m_aiBuffer);

    // retrieve music file information
    m_pInfo    = ov_info(&m_Stream, -1);
    m_pComment = ov_comment(&m_Stream, -1);
    m_dMaxTime = ov_time_total(&m_Stream, -1);

    Core::Log->Info(coreUtils::Print("Music (%s) loaded", pFile->GetPath()));
    return CORE_OK;
}


// ****************************************************************
// read from music stream and update sound buffer
bool coreMusic::__Stream(const ALuint& iBuffer)
{
    char acData[CORE_MUSIC_CHUNK];
    int iSize = 0;

    // process the defined music stream chunk size
    while(iSize < CORE_MUSIC_CHUNK)
    {
        // read and decode data from the music track
        const int iResult = ov_read(&m_Stream, acData + iSize, CORE_MUSIC_CHUNK - iSize, (SDL_BYTEORDER == SDL_BIG_ENDIAN) ? 1 : 0, 2, 1, NULL);

        if(iResult > 0) iSize += iResult;
        else break;
    }

    // music track finished
    if(iSize == 0) return false;

    // write decoded data to sound buffer
    alBufferData(iBuffer, (m_pInfo->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, acData, iSize, m_pInfo->rate);
    return true;
}


// ****************************************************************
// constructor
coreMusicPlayer::coreMusicPlayer()
: m_iRepeat       (CORE_MUSIC_ALL_REPEAT)
, m_iCurIndex     (0)
, m_FadeTimer     (1.0f, 0.0f, 1)
, m_pFadePrevious (NULL)
{
    // reserve memory for music objects
    m_apMusic.reserve(16);
    m_apSequence.reserve(16);

    // create NULL music object
    m_pNullMusic = new coreMusic((coreFile*)NULL);
    m_pCurMusic  = m_pNullMusic;
}


// ****************************************************************
// destructor
coreMusicPlayer::~coreMusicPlayer()
{
    this->ClearFiles();

    // delete NULL music object
    SAFE_DELETE(m_pNullMusic)
}


// ****************************************************************
// update the music player
bool coreMusicPlayer::Update()
{
    if(m_apMusic.empty()) return false;
    SDL_assert(m_pCurMusic != m_pNullMusic);

    // update transition between two music objects
    if(m_FadeTimer.GetStatus())
    {
        if(m_FadeTimer.Update())
            m_pFadePrevious->Stop();

        // adjust their volume
        m_pFadePrevious->SetVolume(m_FadeTimer.GetCurrent(true));
        m_pCurMusic->SetVolume(m_FadeTimer.GetCurrent(false));

        // update the previous music object
        m_pFadePrevious->Update();
    }

    // update the current music object
    if(m_pCurMusic->Update())
    {
        // handle unnecessary loop
        if((m_iRepeat != CORE_MUSIC_SINGLE_REPEAT) && m_pCurMusic->IsPlaying())
            m_pCurMusic->Stop();

        // repeat, switch or stop as defined
        switch(m_iRepeat)
        {
        case CORE_MUSIC_ALL_NOREPEAT:    if(m_iCurIndex+1 >= m_apMusic.size()) break;
        case CORE_MUSIC_ALL_REPEAT:      this->Next();
        case CORE_MUSIC_SINGLE_REPEAT:   m_pCurMusic->Play();
        case CORE_MUSIC_SINGLE_NOREPEAT: break;
        }

        return true;
    }
    return false;
}


// ****************************************************************
// order the playback sequence
void coreMusicPlayer::Order()
{
    // reset playback sequence
    m_apSequence.clear();
    m_apSequence = m_apMusic;

    // switch to first music object
    this->Goto(0);
}


// ****************************************************************
// shuffle the playback sequence
void coreMusicPlayer::Shuffle()
{
    // reset playback sequence
    m_apSequence.clear();
    m_apSequence = m_apMusic;

    // shuffle the list
    std::random_shuffle(m_apSequence.begin(), m_apSequence.end());

    // switch to first music object
    this->Goto(0);
}


// ****************************************************************
// add music object from file
coreError coreMusicPlayer::AddFile(const char* pcPath)
{
    // load from path
    coreFile File(pcPath);
    return this->AddFile(&File);
}

coreError coreMusicPlayer::AddFile(coreFile* pFile)
{
    // create new music object
    coreMusic* pNewMusic = new coreMusic(pFile);
    if(!pNewMusic->GetInfo())
    {
        // remove invalid file
        SAFE_DELETE(pNewMusic)
        return CORE_INVALID_INPUT;
    }

    // add music object to the music player
    m_apMusic.push_back(pNewMusic);
    m_apSequence.push_back(pNewMusic);

    // init the access pointer
    if(m_pCurMusic == m_pNullMusic) m_pCurMusic = pNewMusic;

    return CORE_OK;
}


// ****************************************************************
// add music object from archive
coreError coreMusicPlayer::AddArchive(const char* pcPath)
{
    // open the archive
    coreArchive Archive(pcPath);

    // try to add all files to the music player
    bool bStatus = false;
    for(coreUint i = 0; i < Archive.GetSize(); ++i)
    {
        if(this->AddFile(Archive.GetFile(i)) == CORE_OK)
            bStatus = true;
    }

    return bStatus ? CORE_OK : CORE_INVALID_INPUT;
}


// ****************************************************************
// add music object from folder
coreError coreMusicPlayer::AddFolder(const char* pcPath, const char* pcFilter)
{
    // get files from the folder
    std::vector<std::string> asFolder;
    coreFile::SearchFolder(pcPath, pcFilter, &asFolder);

    // try to add all files to the music player
    bool bStatus = false;
    for(auto it = asFolder.begin(); it != asFolder.end(); ++it)
    {
        if(this->AddFile((*it).c_str()) == CORE_OK)
            bStatus = true;
    }

    return bStatus ? CORE_OK : CORE_INVALID_INPUT;
}


// ****************************************************************
// remove music object
coreError coreMusicPlayer::DeleteFile(const coreUint& iIndex)
{
    SDL_assert(iIndex < m_apMusic.size());
    if(iIndex >= m_apMusic.size()) return CORE_INVALID_INPUT;

    coreMusic* pMusic = m_apMusic[iIndex];

    // remove music object
    m_apMusic.erase(m_apMusic.begin()+iIndex);
    m_apSequence.erase(std::find(m_apSequence.begin(), m_apSequence.end(), pMusic));

    // check and switch the current music object
    if(m_apMusic.empty()) m_pCurMusic = m_pNullMusic;
    else this->Goto(0);

    // delete music object
    SAFE_DELETE(pMusic)

    return CORE_OK;
}


// ****************************************************************
// remove all files
void coreMusicPlayer::ClearFiles()
{
    // delete music objects
    for(auto it = m_apMusic.begin(); it != m_apMusic.end(); ++it)
        SAFE_DELETE(*it)

    // clear memory
    m_apMusic.clear();
    m_apSequence.clear();

    // reset current music object
    m_pCurMusic = m_pNullMusic;
}


// ****************************************************************
// switch to specific music object
void coreMusicPlayer::Goto(const coreUint& iIndex)
{
    SDL_assert(iIndex < m_apMusic.size());
    if(iIndex >= m_apMusic.size()) return;
    if(m_pCurMusic == m_apSequence[iIndex]) return;

    // get playback status
    const bool bStatus = m_pCurMusic->IsPlaying();

    if(m_FadeTimer.GetSpeed() && bStatus)
    {
        // start transition to new music object
        m_FadeTimer.Play(true);
        m_pFadePrevious = m_pCurMusic;
    }
    else
    {
        // stop old music object
        m_pCurMusic->Stop();
        coreMusic::CrossLap(m_pCurMusic, m_apSequence[iIndex]);
    }

    // set new music object
    m_iCurIndex = iIndex;
    m_pCurMusic = m_apSequence[iIndex];

    // adjust the attributes
    m_pCurMusic->SetVolume(1.0f);
    if(bStatus) m_pCurMusic->Play();
}


// ****************************************************************
// switch to next music object
bool coreMusicPlayer::Next()
{
    if(m_iCurIndex+1 >= m_apMusic.size())
    {
        // back to the beginning
        this->Goto(0);
        return true;
    }

    // go to next music object
    this->Goto(m_iCurIndex+1);
    return false;
}


// ****************************************************************
// switch to previous music object
bool coreMusicPlayer::Previous()
{
    if(m_iCurIndex == 0)
    {
        // back to the end
        this->Goto(m_apMusic.size()-1);
        return true;
    }

    // go to previous music object
    this->Goto(m_iCurIndex-1);
    return false;
}