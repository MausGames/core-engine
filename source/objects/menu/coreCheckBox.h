//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_CHECKBOX_H_
#define _CORE_GUARD_CHECKBOX_H_


// ****************************************************************
// menu check-box class
class coreCheckBox final : public coreButton
{
private:
    coreTexturePtr m_apUnchecked[2];   //!< unchecked background textures (0 = idle | 1 = busy)
    coreTexturePtr m_apChecked  [2];   //!< checked background textures   (0 = idle | 1 = busy)

    coreBool m_bCheck;                 //!< check status


public:
    constexpr_weak coreCheckBox()noexcept;
    coreCheckBox(const coreChar* pcIdleUnchecked, const coreChar* pcBusyUnchecked, const coreChar* pcIdleChecked, const coreChar* pcBusyChecked)noexcept;
    coreCheckBox(const coreChar* pcIdleUnchecked, const coreChar* pcBusyUnchecked, const coreChar* pcIdleChecked, const coreChar* pcBusyChecked, const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iOutline)noexcept;

    DISABLE_COPY(coreCheckBox)

    //! construct the check-box
    //! @{
    void Construct(const coreChar* pcIdleUnchecked, const coreChar* pcBusyUnchecked, const coreChar* pcIdleChecked, const coreChar* pcBusyChecked);
    void Construct(const coreChar* pcIdleUnchecked, const coreChar* pcBusyUnchecked, const coreChar* pcIdleChecked, const coreChar* pcBusyChecked, const coreChar* pcFont, const coreUint8& iHeight, const coreUint8& iOutline);
    //! @}

    //! move the check-box
    //! @{
    void Move()override;
    //! @}

    //! set object properties
    //! @{
    void SetCheck(const coreBool& bCheck);
    //! @}

    //! get object properties
    //! @{
    inline const coreBool& GetCheck()const {return m_bCheck;}
    //! @}


private:
    //! construct the check-box
    //! @{
    void __Construct(const coreChar* pcIdleChecked, const coreChar* pcBusyChecked);
    //! @}
};


// ****************************************************************
// constructor
constexpr_weak coreCheckBox::coreCheckBox()noexcept
: m_bCheck (false)
{
}


#endif // _CORE_GUARD_CHECKBOX_H_