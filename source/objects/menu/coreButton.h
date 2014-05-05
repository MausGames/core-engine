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

    bool m_bBusy;                       //!< current background status
    int m_iOverride;                    //!< override current background status (0 = normal | 1 = always busy | -1 = always idle)


public:
    constexpr_obj coreButton()noexcept;
    coreButton(const char* pcIdle, const char* pcBusy, const char* pcFont, const int& iHeight, const coreUint& iLength)noexcept;
    coreButton(const char* pcIdle, const char* pcBusy)noexcept;
    virtual ~coreButton();

    //! construct the button
    //! @{
    void Construct(const char* pcIdle, const char* pcBusy, const char* pcFont, const int& iHeight, const coreUint& iLength);
    void Construct(const char* pcIdle, const char* pcBusy);
    //! @}

    //! render and move the button
    //! @{
    virtual void Render()override;
    virtual void Move()override;
    //! @}

    //! set object properties
    //! @{
    inline void SetOverride(const int& iOverride) {m_iOverride = iOverride;}
    //! @}

    //! get object properties
    //! @{
    inline coreLabel* GetCaption()const  {return m_pCaption;}
    inline const int& GetOverride()const {return m_iOverride;}
    //! @}


private:
    DISABLE_COPY(coreButton)
};


// ****************************************************************    
// constructor
constexpr_obj coreButton::coreButton()noexcept
: m_pCaption  (NULL)
, m_bBusy     (false)
, m_iOverride (0)
{
}


#endif // _CORE_GUARD_BUTTON_H_