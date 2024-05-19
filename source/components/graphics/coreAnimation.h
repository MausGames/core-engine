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

// TODO 3: texture-array, texture-atlas


// ****************************************************************
/* animation class */
class coreAnimation final : public coreResource
{
private:
    coreTexturePtr m_pTextureFrom;   // proxy texture for current frame
    coreTexturePtr m_pTextureTo;     // proxy texture for next frame

    coreTexturePtr* m_ppFrame;       // frame textures
    coreUint16*     m_piTime;        // delays between frames (in milliseconds)

    coreUint16 m_iFrameCount;        // number of frames
    coreUint16 m_iTotalTime;         // total animation time (in milliseconds)

    coreFloat m_fLerp;               // current interpolation value between frames (0.0f <= X < 1.0f)

    coreTextureLoad m_eLoad;         // resource load configuration

    coreSync m_Sync;                 // sync object for asynchronous texture loading


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
    inline const coreTexturePtr& GetTextureFrom()const {return m_pTextureFrom;}
    inline const coreTexturePtr& GetTextureTo  ()const {return m_pTextureTo;}
    inline const coreUint16&     GetFrameCount ()const {return m_iFrameCount;}
    inline       coreFloat       GetTotalTime  ()const {return I_TO_F(m_iTotalTime) / 1000.0f;}
    inline const coreFloat&      GetLerp       ()const {return m_fLerp;}


private:
    /* change current animation state over time */
    void __ChangeTime(const coreUint16 iTime);
};


// ****************************************************************
/* animation resource access type */
using coreAnimationPtr = coreResourcePtr<coreAnimation>;


#endif /* _CORE_GUARD_ANIMATION_H_ */