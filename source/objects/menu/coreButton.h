//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_BUTTON_H_
#define _CORE_GUARD_BUTTON_H_


// ****************************************************************
// menu button class
class coreButton : public coreObject2D
{
protected:
    coreTexturePtr m_apBackground[2];   //!< background textures (0 = idle | 1 = busy)
    coreLabel* m_pCaption;              //!< label displayed inside of the button

    coreBool m_bBusy;                   //!< current background status
    coreInt8 m_iOverride;               //!< override current background status (0 = normal | 1 = always busy | -1 = always idle)


public:
    coreButton()noexcept;
    coreButton(const coreHashString& sIdle, const coreHashString& sBusy, const coreHashString& sFont, const coreUint8 iHeight, const coreUint8 iOutline, const coreUint8 iLength)noexcept;
    coreButton(const coreHashString& sIdle, const coreHashString& sBusy)noexcept;
    virtual ~coreButton()override;

    DISABLE_COPY(coreButton)

    //! construct the button
    //! @{
    void Construct(const coreHashString& sIdle, const coreHashString& sBusy, const coreHashString& sFont, const coreUint8 iHeight, const coreUint8 iOutline, const coreUint8 iLength);
    void Construct(const coreHashString& sIdle, const coreHashString& sBusy);
    //! @}

    //! render and move the button
    //! @{
    void         Render()final;
    virtual void Move  ()override;
    //! @}

    //! set object properties
    //! @{
    inline void SetOverride(const coreInt8 iOverride) {m_iOverride = iOverride;}
    //! @}

    //! get object properties
    //! @{
    inline       coreLabel* GetCaption ()const {return m_pCaption;}
    inline const coreInt8&  GetOverride()const {return m_iOverride;}
    //! @}
};


#endif // _CORE_GUARD_BUTTON_H_