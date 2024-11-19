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
coreOpusStream::coreOpusStream(coreFile* pFile)noexcept
: pContext     (pFile->CreateReadStream())
, aiSeekOffset {}
, aiSeekType   {}
{
    const coreInt64 iBase = SDL_RWtell(pContext);

    // pre-calculate seek parameters
    aiSeekOffset[0] = iBase;                    aiSeekType[0] = RW_SEEK_SET;
    aiSeekOffset[1] = 0u;                       aiSeekType[1] = RW_SEEK_CUR;
    aiSeekOffset[2] = iBase + pFile->GetSize(); aiSeekType[2] = RW_SEEK_SET;   // # emulate END
}


// ****************************************************************
/* destructor */
coreOpusStream::~coreOpusStream()
{
    // close stream
    SDL_RWclose(pContext);
}


// ****************************************************************
/* callback functions for Opus streams */
static coreInt32 OPUS_CALL coreRead (const coreOpusStream* pStream, coreByte* OUTPUT pData, const coreInt32 iSize)    {return (SDL_RWread(pStream->pContext, pData, 1u, iSize));}
static coreInt32 OPUS_CALL coreSeek (const coreOpusStream* pStream, const coreInt64 iOffset, const coreInt32 iWhence) {return (SDL_RWseek(pStream->pContext, iOffset + pStream->aiSeekOffset[iWhence], pStream->aiSeekType[iWhence]) < 0);}
static coreInt64 OPUS_CALL coreTell (const coreOpusStream* pStream)                                                   {return (SDL_RWtell(pStream->pContext) - pStream->aiSeekOffset[0]);}
static coreInt32 OPUS_CALL coreClose(const coreOpusStream* pStream)                                                   {SAFE_DELETE(pStream) return 0;}

const OpusFileCallbacks g_OpusCallbacks =
{
    r_cast<op_read_func> (coreRead),
    r_cast<op_seek_func> (coreSeek),
    r_cast<op_tell_func> (coreTell),
    r_cast<op_close_func>(coreClose)
};


// ****************************************************************
/* constructor */
coreMusic::coreMusic(const coreChar* pcPath)noexcept
: coreMusic (Core::Manager::Resource->RetrieveFile(pcPath))
{
}

coreMusic::coreMusic(coreFile* pFile)noexcept
: m_pFile         (NULL)
, m_pOpusStream   (NULL)
, m_pOpusHead     (NULL)
, m_pOpusTags     (NULL)
, m_pVorbisStream (NULL)
, m_VorbisInfo    {}
, m_VorbisComment {}
, m_iMaxSample    (0u)
, m_fMaxTime      (0.0f)
, m_bOpus         (false)
, m_bLoop         (false)
{
    if(!pFile)            return;
    if(!pFile->GetSize()) return;   // do not load file data

    // copy file object as streaming source
    coreFile::InternalNew(&m_pFile, pFile);

    // extract file extension
    const coreChar* pcExtension = coreData::StrToLower(coreData::StrExtension(pFile->GetPath()));

    // determine audio format
    if(!std::memcmp(pcExtension, "opus", 4u))
    {
        coreInt32 iError;

        // open music stream (Opus)
        m_pOpusStream = op_open_callbacks(new coreOpusStream(m_pFile), &g_OpusCallbacks, NULL, 0u, &iError);
        if(!m_pOpusStream)
        {
            Core::Log->Warning("Music (%s) is not a valid OPUS-file (OP Error Code: %d)", pFile->GetPath(), iError);
            coreFile::InternalDelete(&m_pFile);

            return;
        }

        ASSERT(op_seekable(m_pOpusStream))

        // retrieve music file information
        m_pOpusHead  = op_head     (m_pOpusStream, -1);
        m_pOpusTags  = op_tags     (m_pOpusStream, -1);
        m_iMaxSample = op_pcm_total(m_pOpusStream, -1);
        m_fMaxTime   = coreFloat(coreDouble(m_iMaxSample) / coreDouble(CORE_MUSIC_OPUS_RATE));
        m_bOpus      = true;

        Core::Log->Info("Music (%s, %.2f seconds, %.1f KB/s, %d channels, %d rate) loaded", pFile->GetPath(), m_fMaxTime, I_TO_F(op_bitrate(m_pOpusStream, -1)) / 1000.0f, m_pOpusHead->channel_count, m_pOpusHead->input_sample_rate);
    }
    else if(!std::memcmp(pcExtension, "ogg", 3u))
    {
        coreInt32 iError;

        // open music stream (Vorbis)
        m_pVorbisStream = stb_vorbis_open_file_section(m_pFile->CreateReadStream(), 1, &iError, NULL, pFile->GetSize());
        if(!m_pVorbisStream)
        {
            Core::Log->Warning("Music (%s) is not a valid VORBIS-file (STB Error Code: %d)", pFile->GetPath(), iError);
            coreFile::InternalDelete(&m_pFile);

            return;
        }

        // retrieve music file information
        m_VorbisInfo    = stb_vorbis_get_info                (m_pVorbisStream);
        m_VorbisComment = stb_vorbis_get_comment             (m_pVorbisStream);
        m_iMaxSample    = stb_vorbis_stream_length_in_samples(m_pVorbisStream);
        m_fMaxTime      = stb_vorbis_stream_length_in_seconds(m_pVorbisStream);
        m_bOpus         = false;

        Core::Log->Info("Music (%s, %.2f seconds, %.1f KB/s, %d channels, %d rate) loaded", pFile->GetPath(), m_fMaxTime, I_TO_F(m_VorbisInfo.bit_rate) / 1000.0f, m_VorbisInfo.channels, m_VorbisInfo.sample_rate);
    }
    else
    {
        Core::Log->Warning("Music (%s) could not be identified (valid extensions: opus, ogg)", pFile->GetPath());
        coreFile::InternalDelete(&m_pFile);
    }
}


// ****************************************************************
/* destructor */
coreMusic::~coreMusic()
{
    // close music stream
    op_free         (m_pOpusStream);
    stb_vorbis_close(m_pVorbisStream);
    if(m_pFile) Core::Log->Info("Music (%s) unloaded", m_pFile->GetPath());

    // delete file object
    coreFile::InternalDelete(&m_pFile);
}


// ****************************************************************
/* get specific meta-information */
const coreChar* coreMusic::GetComment(const coreChar* pcName)const
{
    const coreUintW iLen = std::strlen(pcName);

    // select audio format
    const coreUintW  iCount     = m_bOpus ? m_pOpusTags->comments      : m_VorbisComment.comment_list_length;
    coreChar** const ppcComment = m_bOpus ? m_pOpusTags->user_comments : m_VorbisComment.comment_list;

    // loop through all comments
    for(coreUintW i = 0u; i < iCount; ++i)
    {
        // check comment and extract meta-information
        if(!std::memcmp(pcName, ppcComment[i], iLen))
            return ppcComment[i] + iLen + 1u;
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
, m_Lock      ()
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
        const ALuint iSource = Core::Audio->NextSource(NULL, CORE_AUDIO_MUSIC_BUFFER, m_fVolume, 0u);
        WARN_IF(!iSource) return CORE_ERROR_SYSTEM;

        // create sound buffers
        if(m_aiBuffer[0] == UINT32_MAX) alGenBuffers(CORE_MUSIC_BUFFERS, m_aiBuffer);

        // add debug labels
        Core::Audio->LabelOpenAL(AL_SOURCE_EXT, iSource, "music.source");
        for(coreUintW i = 0u; i < CORE_MUSIC_BUFFERS; ++i)
        {
            Core::Audio->LabelOpenAL(AL_BUFFER_EXT, m_aiBuffer[i], "music.buffer");
        }

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
        alSourcei(m_iSource, AL_BUFFER, 0);   // # after stop
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
    Core::Manager::Resource->FolderScan(pcPath, pcFilter, &asFolder);

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

    // clear sound buffers
    if(m_aiBuffer[0] != UINT32_MAX) alDeleteBuffers(CORE_MUSIC_BUFFERS, m_aiBuffer);
    m_aiBuffer[0] = UINT32_MAX;

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
    alignas(ALIGNMENT_PAGE) BIG_STATIC coreByte s_aData[F_TO_UI(CORE_AUDIO_MAX_PITCH) * CORE_MUSIC_CHUNK * sizeof(coreFloat)];

    const coreInt32 iChunkSize = MIN(F_TO_UI(m_fPitch * I_TO_F(CORE_MUSIC_CHUNK)), F_TO_UI(CORE_AUDIO_MAX_PITCH) * CORE_MUSIC_CHUNK);
    coreInt32       iReadSize  = 0;

    // select audio format
    const coreBool  bOpus     = m_pCurMusic->m_bOpus;
    const coreInt32 iChannels = bOpus ? m_pCurMusic->m_pOpusHead->channel_count : m_pCurMusic->m_VorbisInfo.channels;
    const coreInt32 iRate     = bOpus ? CORE_MUSIC_OPUS_RATE                    : m_pCurMusic->m_VorbisInfo.sample_rate;

    do
    {
        coreInt32 iResult;

        // read and decode data from the music stream
        if(bOpus)
        {
            if(Core::Audio->GetSupportFloat()) iResult = op_read_float(m_pCurMusic->m_pOpusStream, r_cast<coreFloat*>(s_aData) + iReadSize, iChunkSize - iReadSize, NULL) * iChannels;
                                          else iResult = op_read      (m_pCurMusic->m_pOpusStream, r_cast<coreInt16*>(s_aData) + iReadSize, iChunkSize - iReadSize, NULL) * iChannels;
        }
        else
        {
            if(Core::Audio->GetSupportFloat()) iResult = stb_vorbis_get_samples_float_interleaved(m_pCurMusic->m_pVorbisStream, iChannels, r_cast<coreFloat*>(s_aData) + iReadSize, iChunkSize - iReadSize) * iChannels;
                                          else iResult = stb_vorbis_get_samples_short_interleaved(m_pCurMusic->m_pVorbisStream, iChannels, r_cast<coreInt16*>(s_aData) + iReadSize, iChunkSize - iReadSize) * iChannels;
        }

        WARN_IF(iResult <  0) break;
             if(iResult == 0) break;
        iReadSize += iResult;
    }
    while(iReadSize < iChunkSize);

    // music stream finished
    if(iReadSize == 0) return false;

    // write decoded data to sound buffer
    if(Core::Audio->GetSupportFloat()) alBufferData(iBuffer, (iChannels == 1) ? AL_FORMAT_MONO_FLOAT32 : AL_FORMAT_STEREO_FLOAT32, s_aData, iReadSize * sizeof(coreFloat), iRate);
                                  else alBufferData(iBuffer, (iChannels == 1) ? AL_FORMAT_MONO16       : AL_FORMAT_STEREO16,       s_aData, iReadSize * sizeof(coreInt16), iRate);
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
                else if(m_eRepeat == CORE_MUSIC_ALL_REPEAT)      {this->Next(); if(m_apMusic.size() == 1u) m_pCurMusic->Rewind();}
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
    const coreChar* pcName = coreData::StrFilename(pFile->GetPath(), false);
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