///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

#undef STB_VORBIS_HEADER_ONLY
#include <stb_vorbis.h>


// ****************************************************************
/* constructor */
coreMusic::coreMusic(const coreChar* pcPath)noexcept
: coreMusic (Core::Manager::Resource->RetrieveFile(pcPath))
{
}

coreMusic::coreMusic(coreFile* pFile)noexcept
: m_pFile      (NULL)
, m_pStream    (NULL)
, m_Info       {}
, m_Comment    {}
, m_iMaxSample (0u)
, m_fMaxTime   (0.0f)
, m_bLoop      (false)
{
    if(!pFile)            return;
    if(!pFile->GetSize()) return;   // do not load file data

    // copy file object as streaming source
    coreFile::InternalNew(&m_pFile, pFile);

    // open music stream
    coreInt32 iError;
    m_pStream = stb_vorbis_open_file_section(m_pFile->CreateReadStream(), 1, &iError, NULL, pFile->GetSize());
    if(!m_pStream)
    {
        Core::Log->Warning("Music (%s) is not a valid OGG-file (STB Error Code: 0x%08X)", pFile->GetPath(), iError);
        coreFile::InternalDelete(&m_pFile);

        return;
    }

    // retrieve music file information
    m_Info       = stb_vorbis_get_info                (m_pStream);
    m_Comment    = stb_vorbis_get_comment             (m_pStream);
    m_iMaxSample = stb_vorbis_stream_length_in_samples(m_pStream);
    m_fMaxTime   = stb_vorbis_stream_length_in_seconds(m_pStream);

    Core::Log->Info("Music (%s, %.1f seconds, %d channels, %d rate) loaded", pFile->GetPath(), m_fMaxTime, m_Info.channels, m_Info.sample_rate);
}


// ****************************************************************
/* destructor */
coreMusic::~coreMusic()
{
    // close music stream
    stb_vorbis_close(m_pStream);
    if(m_pFile) Core::Log->Info("Music (%s) unloaded", m_pFile->GetPath());

    // delete file object
    coreFile::InternalDelete(&m_pFile);
}


// ****************************************************************
/* get specific meta-information */
const coreChar* coreMusic::GetComment(const coreChar* pcName)const
{
    const coreUintW iLen = std::strlen(pcName);

    // loop through all comments
    for(coreUintW i = 0u, ie = m_Comment.comment_list_length; i < ie; ++i)
    {
        // check comment and extract meta-information
        if(!std::memcmp(pcName, m_Comment.comment_list[i], iLen))
            return m_Comment.comment_list[i] + iLen + 1u;
    }

    // specific meta-information not found
    return "";
}


// ****************************************************************
/* constructor */
coreMusicPlayer::coreMusicPlayer()noexcept
: coreThread  ("music_thread")
, m_aiBuffer  {UINT32_MAX}
, m_iSource   (0u)
, m_fVolume   (1.0f)
, m_fPitch    (1.0f)
, m_bStatus   (false)
, m_apMusic   {}
, m_eRepeat   (CORE_MUSIC_ALL_REPEAT)
, m_pCurMusic (NULL)
, m_iCurIndex (0u)
, m_Mutex     ()
{
    // configure music thread
    this->SetFrequency(60.0f);
}


// ****************************************************************
/* destructor */
coreMusicPlayer::~coreMusicPlayer()
{
    // kill music thread
    this->KillThread();

    // clear audio source and sound buffers
    this->Stop();
    if(m_aiBuffer[0] != UINT32_MAX) alDeleteBuffers(CORE_MUSIC_BUFFERS, m_aiBuffer);

    // remove all music objects
    this->ClearMusic();
}


// ****************************************************************
/* update the music-player */
coreBool coreMusicPlayer::Update()
{
    // process without music thread
    return this->GetActive() ? false : this->__ProcessQueue();
}


// ****************************************************************
/* play the music */
coreStatus coreMusicPlayer::Play()
{
    if(m_bStatus)    return CORE_BUSY;
    if(!m_pCurMusic) return CORE_INVALID_CALL;

    __CORE_MUSIC_LOCKER

    if(!m_iSource)
    {
        // retrieve next free audio source
        const ALuint iSource = Core::Audio->NextSource(CORE_AUDIO_MUSIC_BUFFER, m_fVolume, 0u);
        WARN_IF(!iSource) return CORE_ERROR_SYSTEM;

        // create sound buffers
        if(m_aiBuffer[0] == UINT32_MAX) alGenBuffers(CORE_MUSIC_BUFFERS, m_aiBuffer);

        // prepare sound buffers
        const coreUintW iUpdated = this->__StreamList(m_aiBuffer, CORE_MUSIC_BUFFERS);
        WARN_IF(!iUpdated) return CORE_INVALID_DATA;

        // queue sound buffers
        m_iSource = iSource;
        alSourceQueueBuffers(m_iSource, iUpdated, m_aiBuffer);

        Core::Audio->DeferUpdates();
        {
            // set initial audio source properties
            alSourcei(m_iSource, AL_SOURCE_RELATIVE, true);
            alSourcef(m_iSource, AL_PITCH,           m_fPitch);
            alSourcei(m_iSource, AL_LOOPING,         false);
        }
        Core::Audio->ProcessUpdates();
    }

    // start playback
    alSourcePlay(m_iSource);

    // set playback status
    m_bStatus = true;

    return CORE_OK;
}


// ****************************************************************
/* stop the music */
void coreMusicPlayer::Stop()
{
    __CORE_MUSIC_LOCKER

    if(m_iSource)
    {
        // stop audio source
        alSourceStop(m_iSource);

        // remove remaining sound buffers
        alSourcei(m_iSource, AL_BUFFER, 0);   // after stop
        m_iSource = 0u;

        // rewind the music stream
        m_pCurMusic->Rewind();
    }

    // reset playback status
    m_bStatus = false;
}


// ****************************************************************
/* pause the music */
void coreMusicPlayer::Pause()
{
    __CORE_MUSIC_LOCKER

    if(m_iSource)
    {
        // pause audio source
        alSourcePause(m_iSource);
    }

    // reset playback status
    m_bStatus = false;
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
    coreList<coreString> asFolder;
    coreData::FolderScan(pcPath, pcFilter, &asFolder);

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

    __CORE_MUSIC_LOCKER

    // stop the music (if currently selected)
    if(iIndex == m_iCurIndex) this->Stop();

    // delete music object
    MANAGED_DELETE(m_apMusic[iIndex])

    // remove music object
    m_apMusic.erase(iIndex);

    // check and switch the current music object
    if(m_apMusic.empty())
    {
        m_pCurMusic = NULL;
        m_iCurIndex = 0u;
    }
    else if(iIndex <  m_iCurIndex) m_iCurIndex = m_iCurIndex - 1u;
    else if(iIndex == m_iCurIndex) this->Select(0u);

    return CORE_OK;
}


// ****************************************************************
/* remove all music objects */
void coreMusicPlayer::ClearMusic()
{
    __CORE_MUSIC_LOCKER

    // stop the music
    this->Stop();

    // delete music objects
    FOR_EACH(it, m_apMusic)
        MANAGED_DELETE(*it)

    // clear memory
    m_apMusic.clear();

    // reset current music object
    m_pCurMusic = NULL;
    m_iCurIndex = 0u;
}


// ****************************************************************
/* switch to specific music object */
void coreMusicPlayer::Select(const coreUintW iIndex)
{
    WARN_IF(iIndex >= m_apMusic.size())  return;
    if(m_pCurMusic == m_apMusic[iIndex]) return;

    __CORE_MUSIC_LOCKER

    // stop the music (if currently paused)
    if(m_iSource && !m_bStatus) this->Stop();

    // rewind the old music stream
    if(m_bStatus) m_pCurMusic->Rewind();

    // set new music object
    m_pCurMusic = m_apMusic[iIndex];
    m_iCurIndex = iIndex;
}

void coreMusicPlayer::SelectName(const coreHashString& sName)
{
    // search for file name
    const coreUintW iIndex = m_apMusic.index(sName);
    WARN_IF(iIndex >= m_apMusic.size()) return;

    // go to selected music object
    this->Select(iIndex);
}


// ****************************************************************
/* switch to next music object */
coreBool coreMusicPlayer::Next()
{
    __CORE_MUSIC_LOCKER

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
    __CORE_MUSIC_LOCKER

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
/* init music thread */
coreStatus coreMusicPlayer::__InitThread()
{
    return CORE_OK;
}


// ****************************************************************
/* run music thread */
coreStatus coreMusicPlayer::__RunThread()
{
    __CORE_MUSIC_LOCKER

    // process with music thread
    this->__ProcessQueue();

    return CORE_OK;
}


// ****************************************************************
/* exit music thread */
void coreMusicPlayer::__ExitThread()
{
}


// ****************************************************************
/* process sound queue */
coreBool coreMusicPlayer::__ProcessQueue()
{
    if(!m_bStatus) return false;
    ASSERT(m_pCurMusic)

    // get number of processed sound buffers
    ALint iProcessed;
    alGetSourcei(m_iSource, AL_BUFFERS_PROCESSED, &iProcessed);

    if(iProcessed)
    {
        ALuint aiBuffer[CORE_MUSIC_BUFFERS];

        // retrieve processed sound buffers
        alSourceUnqueueBuffers(m_iSource, iProcessed, aiBuffer);

        // update processed sound buffers
        const coreUintW iUpdated = this->__StreamList(aiBuffer, iProcessed);

        if(iUpdated)
        {
            // queue updated sound buffers
            alSourceQueueBuffers(m_iSource, iUpdated, aiBuffer);
        }
        else
        {
            // get number of queued sound buffers
            ALint iQueued;
            alGetSourcei(m_iSource, AL_BUFFERS_QUEUED, &iQueued);

            if(!iQueued)
            {
                // music is finished
                this->Stop();
                return true;
            }
        }

        // keep music playing
        ALint iStatus;
        alGetSourcei(m_iSource, AL_SOURCE_STATE, &iStatus);
        if(iStatus != AL_PLAYING) alSourcePlay(m_iSource);
    }

    return false;
}


// ****************************************************************
/* read from music stream and update sound buffer */
coreBool coreMusicPlayer::__Stream(const ALuint iBuffer)
{
    alignas(ALIGNMENT_PAGE) BIG_STATIC coreByte s_aData[4u * CORE_MUSIC_CHUNK * sizeof(coreFloat)];

    const coreInt32 iChunkSize = MIN(F_TO_UI(m_fPitch * I_TO_F(CORE_MUSIC_CHUNK)), 4u * CORE_MUSIC_CHUNK);
    coreInt32       iReadSize  = 0;

    // process the defined music stream chunk size
    while(iReadSize < iChunkSize)
    {
        coreInt32 iResult;

        // read and decode data from the music track
        if(Core::Audio->GetSupportFloat()) iResult = stb_vorbis_get_samples_float_interleaved(m_pCurMusic->m_pStream, m_pCurMusic->m_Info.channels, r_cast<coreFloat*>(s_aData) + iReadSize, iChunkSize - iReadSize) * m_pCurMusic->m_Info.channels;
                                      else iResult = stb_vorbis_get_samples_short_interleaved(m_pCurMusic->m_pStream, m_pCurMusic->m_Info.channels, r_cast<coreInt16*>(s_aData) + iReadSize, iChunkSize - iReadSize) * m_pCurMusic->m_Info.channels;

        if(iResult <= 0) break;
        iReadSize += iResult;
    }

    // music track finished
    if(iReadSize == 0) return false;

    // write decoded data to sound buffer
    if(Core::Audio->GetSupportFloat()) alBufferData(iBuffer, (m_pCurMusic->m_Info.channels == 1) ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32, s_aData, iReadSize * sizeof(coreFloat), m_pCurMusic->m_Info.sample_rate);
                                  else alBufferData(iBuffer, (m_pCurMusic->m_Info.channels == 1) ? AL_FORMAT_MONO16       : AL_FORMAT_STEREO16,       s_aData, iReadSize * sizeof(coreInt16), m_pCurMusic->m_Info.sample_rate);
    return true;
}


// ****************************************************************
/* update multiple sound buffers */
coreUintW coreMusicPlayer::__StreamList(const ALuint* piBuffer, const coreUintW iCount)
{
    coreUintW iUpdated = 0u;

    // try to update all provided sound buffers
    while(iUpdated < iCount)
    {
        if(!this->__Stream(piBuffer[iUpdated]))
        {
            if(m_pCurMusic->m_bLoop)
            {
                // rewind the music stream
                m_pCurMusic->Rewind();
            }
            else
            {
                // repeat, switch or stop as defined
                     if(m_eRepeat == CORE_MUSIC_SINGLE_NOREPEAT) {break;}
                else if(m_eRepeat == CORE_MUSIC_SINGLE_REPEAT)   {m_pCurMusic->Rewind();}
                else if(m_eRepeat == CORE_MUSIC_ALL_NOREPEAT)    {if((m_iCurIndex + 1u) >= m_apMusic.size()) break; this->Next();}
                else if(m_eRepeat == CORE_MUSIC_ALL_REPEAT)      {this->Next();}
            }

            // cancel on further failure
            WARN_IF(!this->__Stream(piBuffer[iUpdated])) break;
        }
        iUpdated += 1u;
    }

    return iUpdated;
}


// ****************************************************************
/* add music object */
coreStatus coreMusicPlayer::__AddMusic(coreFile* pFile)
{
    // create new music object
    coreMusic* pNewMusic = MANAGED_NEW(coreMusic, pFile);
    if(!pNewMusic->GetMaxSample())   // invalid file
    {
        MANAGED_DELETE(pNewMusic)
        return CORE_ERROR_FILE;
    }

    __CORE_MUSIC_LOCKER

    // extract and store file name
    const coreChar* pcName = coreData::StrFilename(pFile->GetPath());
    WARN_IF(m_apMusic.count(pcName))
    {
        MANAGED_DELETE(pNewMusic)
        return CORE_INVALID_INPUT;
    }

    // add music object to the music-player
    m_apMusic.emplace(pcName, pNewMusic);

    // init the access pointer
    if(!m_pCurMusic) m_pCurMusic = pNewMusic;

    return CORE_OK;
}