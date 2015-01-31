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
    coreTexturePtr m_apChecked[2];     //!< checked background textures (0 = idle | 1 = busy)

    bool m_bCheck;                     //!< check status


public:
    constexpr_weak coreCheckBox()noexcept;
    coreCheckBox(const char* pcIdleUnchecked, const char* pcBusyUnchecked, const char* pcIdleChecked, const char* pcBusyChecked)noexcept;

    DISABLE_COPY(coreCheckBox)

    //! construct the check-box
    //! @{
    void Construct(const char* pcIdleUnchecked, const char* pcBusyUnchecked, const char* pcIdleChecked, const char* pcBusyChecked);
    //! @}

    //! move the check-box
    //! @{
    void Move()override;
    //! @}

    //! set object properties
    //! @{
    void SetCheck(const bool& bCheck);
    //! @}

    //! get object properties
    //! @{
    inline const bool& GetCheck()const {return m_bCheck;}
    //! @}
};


// ****************************************************************
// constructor
constexpr_weak coreCheckBox::coreCheckBox()noexcept
: m_bCheck (false)
{
}


#endif // _CORE_GUARD_CHECKBOX_H_