///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_CHECKBOX_H_
#define _CORE_GUARD_CHECKBOX_H_


// ****************************************************************
// menu check-box class
class coreCheckBox : public coreButton
{
private:
    coreTexturePtr m_apUnchecked[2];   //!< unchecked background textures (0 = idle | 1 = busy)
    coreTexturePtr m_apChecked  [2];   //!< checked background textures   (0 = idle | 1 = busy)

    coreBool m_bCheck;                 //!< check status


public:
    coreCheckBox()noexcept;
    coreCheckBox(const coreHashString& sIdleUnchecked, const coreHashString& sBusyUnchecked, const coreHashString& sIdleChecked, const coreHashString& sBusyChecked)noexcept;
    coreCheckBox(const coreHashString& sIdleUnchecked, const coreHashString& sBusyUnchecked, const coreHashString& sIdleChecked, const coreHashString& sBusyChecked, const coreHashString& sFont, const coreUint8 iHeight, const coreUint8 iOutline)noexcept;
    virtual ~coreCheckBox()override = default;

    DISABLE_COPY(coreCheckBox)

    //! construct the check-box
    //! @{
    void Construct(const coreHashString& sIdleUnchecked, const coreHashString& sBusyUnchecked, const coreHashString& sIdleChecked, const coreHashString& sBusyChecked);
    void Construct(const coreHashString& sIdleUnchecked, const coreHashString& sBusyUnchecked, const coreHashString& sIdleChecked, const coreHashString& sBusyChecked, const coreHashString& sFont, const coreUint8 iHeight, const coreUint8 iOutline);
    //! @}

    //! move the check-box
    //! @{
    virtual void Move()override;
    //! @}

    //! set object properties
    //! @{
    void SetCheck(const coreBool bCheck);
    //! @}

    //! get object properties
    //! @{
    inline const coreBool& GetCheck()const {return m_bCheck;}
    //! @}


private:
    //! construct the check-box
    //! @{
    void __Construct(const coreHashString& sIdleChecked, const coreHashString& sBusyChecked);
    //! @}
};


#endif // _CORE_GUARD_CHECKBOX_H_