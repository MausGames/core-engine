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
    constexpr_weak coreButton()noexcept;
    coreButton(const coreChar* pcIdle, const coreChar* pcBusy, const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iOutline, const coreUint8& iLength)noexcept;
    coreButton(const coreChar* pcIdle, const coreChar* pcBusy)noexcept;
    virtual ~coreButton();

    DISABLE_COPY(coreButton)

    //! construct the button
    //! @{
    void Construct(const coreChar* pcIdle, const coreChar* pcBusy, const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iOutline, const coreUint8& iLength);
    void Construct(const coreChar* pcIdle, const coreChar* pcBusy);
    //! @}

    //! render and move the button
    //! @{
    virtual void Render()override;
    virtual void Move  ()override;
    //! @}

    //! set object properties
    //! @{
    inline void SetOverride(const coreInt8& iOverride) {m_iOverride = iOverride;}
    //! @}

    //! get object properties
    //! @{
    inline       coreLabel* GetCaption ()const {return m_pCaption;}
    inline const coreInt8&  GetOverride()const {return m_iOverride;}
    //! @}
};


// ****************************************************************
// constructor
constexpr_weak coreButton::coreButton()noexcept
: m_pCaption  (NULL)
, m_bBusy     (false)
, m_iOverride (0)
{
}


#endif // _CORE_GUARD_BUTTON_H_