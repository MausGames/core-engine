///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_BUTTON_H_
#define _CORE_GUARD_BUTTON_H_

// TODO 5: <old comment style>


// ****************************************************************
/* menu button class */
class coreButton : public coreObject2D
{
protected:
    coreTexturePtr m_apBackground[2];   // background textures (0 = idle | 1 = busy)
    coreLabel* m_pCaption;              // label displayed inside of the button

    coreBool m_bBusy;                   // current background status
    coreInt8 m_iOverride;               // override current background status (0 = normal | 1 = always busy | -1 = always idle)


public:
    coreButton()noexcept;
    coreButton(const coreHashString& sIdle, const coreHashString& sBusy, const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline)noexcept;
    coreButton(const coreHashString& sIdle, const coreHashString& sBusy)noexcept;
    virtual ~coreButton()override;

    DISABLE_COPY(coreButton)

    /* construct the button */
    void Construct(const coreHashString& sIdle, const coreHashString& sBusy, const coreHashString& sFont, const coreUint16 iHeight, const coreUint8 iOutline);
    void Construct(const coreHashString& sIdle, const coreHashString& sBusy);

    /* render and move the button */
    virtual void Render()override;
    virtual void Move  ()override;

    /* set object properties */
    inline void SetOverride(const coreInt8 iOverride) {m_iOverride = iOverride;}

    /* get object properties */
    inline       coreLabel* GetCaption ()const {return m_pCaption;}
    inline const coreInt8&  GetOverride()const {return m_iOverride;}
};


#endif /* _CORE_GUARD_BUTTON_H_ */