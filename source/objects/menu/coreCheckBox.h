//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
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
    coreCheckBox(const char* pcIdleUnchecked, const char* pcBusyUnchecked, const char* pcIdleChecked, const char* pcBusyChecked)noexcept;

    //! move the check-box
    //! @{
    void Move()override;
    //! @}

    //! set object attributes
    //! @{
    void SetCheck(const bool& bCheck);
    //! @}

    //! get object attributes
    //! @{
    inline const bool& GetCheck()const {return m_bCheck;}
    //! @}
};


#endif // _CORE_GUARD_CHECKBOX_H_