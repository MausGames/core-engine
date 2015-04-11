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

// TODO: add interaction for rotated objects (ABS is for 180 degrees)
// TODO: Interact depends on Move, and Move of some menu objects depend on Interact
// TODO: on IsClicked: make right mouse button on Android a longer push ?
// TODO: on IsClicked: consider finger number


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
    coreMatrix2 m_mRotation;        //!< separate rotation matrix
    coreMatrix3 m_mTransform;       //!< transformation matrix

    coreBool    m_bFocused;         //!< interaction status
    coreVector2 m_vFocusModifier;   //!< size-modifier for interaction handling

#if defined(_CORE_ANDROID_)
    coreUint32 m_iFinger;           //!< separate finger interaction status (bitwise)
#endif


public:
    constexpr_weak coreObject2D()noexcept;
    virtual ~coreObject2D() {}

    ENABLE_COPY(coreObject2D)

    //! define the visual appearance
    //! @{
    void Undefine();
    //! @}

    //! render and move the 2d-object
    //! @{
    virtual void Render(const coreProgramPtr& pProgram);
    virtual void Render();
    virtual void Move  ();
    //! @}

    //! interact with the 2d-object
    //! @{
    void Interact();
    coreBool IsClicked(const coreUint8 iButton = CORE_INPUT_LEFT, const coreInputType iType = CORE_INPUT_PRESS)const;
    inline const coreBool& IsFocused()const {return m_bFocused;}
    //! @}

    //! transform the whole object
    //! @{
    inline void FitToScreen() {m_iUpdate = CORE_OBJECT_UPDATE_ALL; m_vPosition = coreVector2(0.0f,0.0f); m_vSize = Core::System->GetResolution() / Core::System->GetResolution().Min(); m_vDirection = coreVector2(0.0f,1.0f); m_vCenter = coreVector2(0.0f,0.0f); m_vAlignment = coreVector2(0.0f,0.0f);}
    //! @}

    //! set object properties
    //! @{
    inline void SetPosition     (const coreVector2& vPosition)      {if(m_vPosition  != vPosition)  {ADD_VALUE(m_iUpdate, CORE_OBJECT_UPDATE_TRANSFORM) m_vPosition  = vPosition;}}
    inline void SetSize         (const coreVector2& vSize)          {if(m_vSize      != vSize)      {ADD_VALUE(m_iUpdate, CORE_OBJECT_UPDATE_TRANSFORM) m_vSize      = vSize;}}
    inline void SetDirection    (const coreVector2& vDirection)     {if(m_vDirection != vDirection) {ADD_VALUE(m_iUpdate, CORE_OBJECT_UPDATE_ALL)       m_vDirection = vDirection;} ASSERT(vDirection.IsNormalized())}
    inline void SetCenter       (const coreVector2& vCenter)        {if(m_vCenter    != vCenter)    {ADD_VALUE(m_iUpdate, CORE_OBJECT_UPDATE_TRANSFORM) m_vCenter    = vCenter;}}
    inline void SetAlignment    (const coreVector2& vAlignment)     {if(m_vAlignment != vAlignment) {ADD_VALUE(m_iUpdate, CORE_OBJECT_UPDATE_TRANSFORM) m_vAlignment = vAlignment;}}
    inline void SetFocus        (const coreBool&    bFocus)         {m_bFocused       = bFocus;}
    inline void SetFocusModifier(const coreVector2& vFocusModifier) {m_vFocusModifier = vFocusModifier;}
    //! @}

    //! get object properties
    //! @{
    inline const coreVector2& GetPosition ()const {return m_vPosition;}
    inline const coreVector2& GetSize     ()const {return m_vSize;}
    inline const coreVector2& GetDirection()const {return m_vDirection;}
    inline const coreVector2& GetCenter   ()const {return m_vCenter;}
    inline const coreVector2& GetAlignment()const {return m_vAlignment;}
    inline const coreMatrix2& GetRotation ()const {return m_mRotation;}
    inline const coreMatrix3& GetTransform()const {return m_mTransform;}
    //! @}
};


// ****************************************************************
// constructor
constexpr_weak coreObject2D::coreObject2D()noexcept
: m_vPosition      (coreVector2(0.0f,0.0f))
, m_vSize          (coreVector2(0.0f,0.0f))
, m_vDirection     (coreVector2(0.0f,1.0f))
, m_vCenter        (coreVector2(0.0f,0.0f))
, m_vAlignment     (coreVector2(0.0f,0.0f))
, m_mRotation      (coreMatrix2::Identity())
, m_mTransform     (coreMatrix3::Identity())
, m_bFocused       (false)
, m_vFocusModifier (coreVector2(1.0f,1.0f))
#if defined(_CORE_ANDROID_)
, m_iFinger        (0u)
#endif
{
}


#endif // _CORE_GUARD_OBJECT2D_H_