///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_ANIMATION_H_
#define _CORE_GUARD_ANIMATION_H_


// ****************************************************************
/* animation class */
class coreAnimation final : public coreResource
{
private:
    coreTexturePtr m_pTexture;     // grid texture
    corePoint2U8   m_Division;     // grid division
    coreUint16*    m_piTime;       // delays between frames (in milliseconds)

    coreUint16 m_iFrameCount;      // number of frames
    coreUint16 m_iTotalTime;       // total animation time (in milliseconds)

    coreVector2 m_vCurTexSize;     // texture size for a single frame
    coreVector4 m_vCurTexOffset;   // texture offset (from, to)
    coreUint16  m_iCurFrame;       // current base frame
    coreFloat   m_fCurLerp;        // current interpolation value between frames (0.0f <= X < 1.0f)

    coreTextureLoad m_eLoad;       // resource load configuration

    coreSync m_Sync;               // sync object for asynchronous texture loading


public:
    explicit coreAnimation(const coreTextureLoad eLoad = CORE_TEXTURE_LOAD_DEFAULT)noexcept;
    ~coreAnimation()final;

    DISABLE_COPY(coreAnimation)

    /* load and unload animation resource data */
    coreStatus Load(coreFile* pFile)final;
    coreStatus Unload()final;

    /* change current animation state */
    void     ChangeTimeLoop(const coreFloat  fTime);
    coreBool ChangeTimeOnce(const coreFloat  fTime);
    coreBool ChangeFrame   (const coreUint16 iFrame, const coreFloat fLerp);

    /* get object properties */
    inline const coreTexturePtr& GetTexture     ()const {return m_pTexture;}
    inline const coreUint16&     GetFrameCount  ()const {return m_iFrameCount;}
    inline       coreFloat       GetTotalTime   ()const {return I_TO_F(m_iTotalTime) / 1000.0f;}
    inline const coreVector2&    GetCurTexSize  ()const {return m_vCurTexSize;}
    inline const coreVector4&    GetCurTexOffset()const {return m_vCurTexOffset;}
    inline const coreUint16&     GetCurFrame    ()const {return m_iCurFrame;}
    inline const coreFloat&      GetCurLerp     ()const {return m_fCurLerp;}


private:
    /* change current animation state over time */
    void __ChangeTime(const coreUint16 iTime);
};


// ****************************************************************
/* animation resource access type */
using coreAnimationPtr = coreResourcePtr<coreAnimation>;


#endif /* _CORE_GUARD_ANIMATION_H_ */