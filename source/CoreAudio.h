//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once


// ****************************************************************
// main audio interface
class CoreAudio
{
private:
    ALCcontext* m_pContext;         // OpenAL context
    ALCdevice* m_pDevice;           // OpenAL device

    coreVector3 m_vPosition;        // position of the listener
    coreVector3 m_vVelocity;        // velocity of the listener
    coreVector3 m_avDirection[2];   // direction and orientation of the listener

    ALuint* m_pSource;              // sound channels
    coreByte m_NumSource;           // number of sound channels
    coreByte m_CurSource;           // current sound channel

    float m_fVolume;                // global volume


private:
    CoreAudio();
    ~CoreAudio();
    friend class Core;


public:
    // control the listener
    void SetListener(const coreVector3* pvPosition, const coreVector3* pvVelocity, const coreVector3* pvDirection, const coreVector3* pvOrientation);
    void SetListener(const float& fSpeed);

    // get next sound channel
    inline const ALuint& GetNextSource() {if(++m_CurSource >= m_NumSource) m_CurSource = 0; return m_pSource[m_CurSource];}

    // set global volume
    inline void SetVolume(const float& fVolume) {if(m_fVolume != fVolume) {m_fVolume = fVolume; alListenerf(AL_GAIN, m_fVolume);}}
};