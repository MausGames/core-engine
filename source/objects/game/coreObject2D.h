//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_OBJECT2D_H_
#define _CORE_GUARD_OBJECT2D_H_


// ****************************************************************
// 2d-object class
class coreObject2D : public coreObject
{
private:
    coreVector2 m_vPosition;    //!< position of the 2d-object (aspect ratio independent)
    coreVector2 m_vSize;        //!< size-factor of the 2d-object (independent)
    coreVector2 m_vDirection;   //!< direction for the rotation matrix (independent)
    coreVector2 m_vCenter;      //!< screen space origin (depending)
    coreVector2 m_vAlignment;   //!< offset factor (independent)


protected:
    static coreModel* s_pModel;   //!< global model object
    bool m_bFocused;              //!< interaction status

#if defined(_CORE_ANDROID_)
    int m_iFinger;                //!< separate finger interaction status (bitwise)
#endif


public:
    constexpr_obj coreObject2D()noexcept;
    virtual ~coreObject2D() {}
    friend class coreObjectManager;

    //! define the visual appearance
    //! @{
    void Undefine();
    //! @}

    //! render and move the 2d-object
    //! @{
    virtual void Render() {this->coreObject2D::Render(m_pProgram, true);}
    virtual void Render(const coreProgramShr& pProgram, const bool& bTextured) hot_func;
    virtual void Move() hot_func;
    //! @}

    //! interact with the 2d-object
    //! @{
    void Interact();
    bool IsClicked(const coreByte iButton = CORE_INPUT_LEFT, const coreInputType iType = CORE_INPUT_PRESS)const;
    inline const bool& IsFocused()const {return m_bFocused;}
    //! @}

    //! transform the whole object
    //! @{
    inline void FitToScreen() {m_iUpdate |= 3; m_vPosition = coreVector2(0.0f,0.0f); m_vSize = coreVector2(Core::System->GetResolution().AspectRatio(), 1.0f); m_vDirection = coreVector2(0.0f,1.0f); m_vCenter = coreVector2(0.0f,0.0f); m_vAlignment = coreVector2(0.0f,0.0f);}
    //! @}

    //! set object properties
    //! @{
    inline void SetPosition(const coreVector2& vPosition)   {if(m_vPosition  != vPosition)  {m_iUpdate |= 1; m_vPosition  = vPosition;}}
    inline void SetSize(const coreVector2& vSize)           {if(m_vSize      != vSize)      {m_iUpdate |= 1; m_vSize      = vSize;}}
    inline void SetDirection(const coreVector2& vDirection) {const coreVector2 vDirNorm = vDirection.Normalized(); if(m_vDirection != vDirNorm) {m_iUpdate |= 3; m_vDirection = vDirNorm;}}
    inline void SetCenter(const coreVector2& vCenter)       {if(m_vCenter    != vCenter)    {m_iUpdate |= 1; m_vCenter    = vCenter;}}
    inline void SetAlignment(const coreVector2& vAlignment) {if(m_vAlignment != vAlignment) {m_iUpdate |= 1; m_vAlignment = vAlignment;}}
    inline void SetFocus(const bool& bFocus)                {m_bFocused = bFocus;}
    //! @}

    //! get object properties
    //! @{
    inline const coreVector2& GetPosition()const  {return m_vPosition;}
    inline const coreVector2& GetSize()const      {return m_vSize;}
    inline const coreVector2& GetDirection()const {return m_vDirection;}
    inline const coreVector2& GetCenter()const    {return m_vCenter;}
    inline const coreVector2& GetAlignment()const {return m_vAlignment;}
    //! @}
};


// ****************************************************************
// constructor
constexpr_obj coreObject2D::coreObject2D()noexcept
: m_vPosition  (coreVector2(0.0f,0.0f))
, m_vSize      (coreVector2(1.0f,1.0f))
, m_vDirection (coreVector2(0.0f,1.0f))
, m_vCenter    (coreVector2(0.0f,0.0f))
, m_vAlignment (coreVector2(0.0f,0.0f))
, m_bFocused   (false)
#if defined(_CORE_ANDROID_)
, m_iFinger    (0)
#endif
{
}


#endif // _CORE_GUARD_OBJECT2D_H_