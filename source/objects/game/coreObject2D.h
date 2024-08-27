///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_OBJECT2D_H_
#define _CORE_GUARD_OBJECT2D_H_

// TODO 2: Interact depends on Move, and Move of some menu objects depend on Interact
// TODO 3: on IsClicked: make right mouse button on mobile a longer push or multiple fingers ?
// TODO 3: on IsClicked: consider finger number
// TODO 5: <old comment style>


// ****************************************************************
/* 2d-object definitions */
#define CORE_OBJECT2D_DEPTH_DEFAULT (100u)
#define CORE_OBJECT2D_DEPTH_MAX     (255u)

enum coreObject2DStyle : coreUint8
{
    CORE_OBJECT2D_STYLE_NOTHING   = 0x00u,   // override nothing
    CORE_OBJECT2D_STYLE_VIEWDIR   = 0x01u,   // react to global 2d-object rotation
    CORE_OBJECT2D_STYLE_ALTCENTER = 0x02u    // react to global 2d-object screen space resolution
};
ENABLE_BITWISE(coreObject2DStyle)


// ****************************************************************
/* 2d-object class */
class coreObject2D : public coreObject
{
private:
    coreVector2 m_vPosition;    // position of the 2d-object (aspect ratio independent)
    coreVector2 m_vSize;        // size-factor of the 2d-object (independent)
    coreVector2 m_vDirection;   // direction of the 2d-object (independent)
    coreVector2 m_vCenter;      // screen space origin (depending)
    coreVector2 m_vAlignment;   // offset factor (independent)


protected:
    coreVector2 m_vScreenPosition;    // transformed position
    coreVector2 m_vScreenSize;        // transformed size-factor (without rotation)
    coreVector2 m_vScreenDirection;   // transformed direction

    coreObject2DStyle m_eStyle;       // style overrides
    coreUint8         m_iDepth;       // depth order (higher = farther away)

    coreUint8   m_iFocused;           // interaction status (0 = hold, 1 = enter, 2 = leave)
    coreBool    m_bFocusable;         // enabled interaction handling
    coreVector2 m_vFocusModifier;     // size-modifier for interaction handling

#if defined(_CORE_MOBILE_)
    coreUint32 m_iFinger;             // separate finger interaction status (bitwise)
#endif


public:
    coreObject2D()noexcept;
    coreObject2D(const coreObject2D& c)noexcept;
    coreObject2D(coreObject2D&&      m)noexcept;
    virtual ~coreObject2D();

    /* assignment operations */
    coreObject2D& operator = (const coreObject2D& c)noexcept;
    coreObject2D& operator = (coreObject2D&&      m)noexcept;

    /* define the visual appearance */
    void Undefine();

    /* render and move the 2d-object */
    coreBool     Prepare(const coreProgramPtr& pProgram);
    coreBool     Prepare();
    virtual void Render (const coreProgramPtr& pProgram);
    virtual void Render ();
    virtual void Move   ();

    /* interact with the 2d-object */
    void Interact();
    coreBool IsClicked(const coreUint8 iButton = CORE_INPUT_LEFT, const coreInputType eType = CORE_INPUT_PRESS)const;
    inline       coreBool  IsFocused     ()const {return HAS_BIT(m_iFocused, 0u);}
    inline       coreBool  IsFocusedEnter()const {return HAS_BIT(m_iFocused, 1u);}
    inline       coreBool  IsFocusedLeave()const {return HAS_BIT(m_iFocused, 2u);}
    inline const coreBool& IsFocusable   ()const {return m_bFocusable;}

    /* retrieve transformation components */
    inline const coreVector2& GetScreenPosition ()const {return m_vScreenPosition;}
    inline const coreVector2& GetScreenDirection()const {return m_vScreenDirection;}
    inline       coreVector2  GetScreenBound    ()const {return m_vScreenDirection.x ? coreVector2::Bound(m_vScreenSize, m_vScreenDirection) : m_vScreenSize;}
    inline       coreVector2  GetScreenBound90  ()const {return m_vScreenDirection.x ? m_vScreenSize.yx()                                    : m_vScreenSize;}

    /* set object properties */
    inline void SetPosition     (const coreVector2       vPosition)      {if(m_vPosition  != vPosition)  {ADD_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_TRANSFORM) m_vPosition  = vPosition;}}
    inline void SetSize         (const coreVector2       vSize)          {if(m_vSize      != vSize)      {ADD_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_TRANSFORM) m_vSize      = vSize;}}
    inline void SetDirection    (const coreVector2       vDirection)     {if(m_vDirection != vDirection) {ADD_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_TRANSFORM) m_vDirection = vDirection;} ASSERT(vDirection.IsNormalized())}
    inline void SetCenter       (const coreVector2       vCenter)        {if(m_vCenter    != vCenter)    {ADD_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_TRANSFORM) m_vCenter    = vCenter;}}
    inline void SetAlignment    (const coreVector2       vAlignment)     {if(m_vAlignment != vAlignment) {ADD_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_TRANSFORM) m_vAlignment = vAlignment;}}
    inline void SetStyle        (const coreObject2DStyle eStyle)         {if(m_eStyle     != eStyle)     {ADD_FLAG(m_eUpdate, CORE_OBJECT_UPDATE_TRANSFORM) m_eStyle     = eStyle;}}
    inline void SetDepth        (const coreUint8         iDepth)         {m_iDepth         = iDepth;}
    inline void SetFocused      (const coreBool          bFocused)       {SET_BIT(m_iFocused, 0u, bFocused)}
    inline void SetFocusable    (const coreBool          bFocusable)     {m_bFocusable     = bFocusable;}
    inline void SetFocusModifier(const coreVector2       vFocusModifier) {m_vFocusModifier = vFocusModifier;}

    /* get object properties */
    inline const coreVector2&       GetPosition ()const {return m_vPosition;}
    inline const coreVector2&       GetSize     ()const {return m_vSize;}
    inline const coreVector2&       GetDirection()const {return m_vDirection;}
    inline const coreVector2&       GetCenter   ()const {return m_vCenter;}
    inline const coreVector2&       GetAlignment()const {return m_vAlignment;}
    inline const coreObject2DStyle& GetStyle    ()const {return m_eStyle;}
    inline const coreUint8&         GetDepth    ()const {return m_iDepth;}
};


// ****************************************************************
/* fullscreen-object class */
class coreFullscreen : public coreObject2D
{
public:
    coreFullscreen() = default;
    virtual ~coreFullscreen()override = default;

    ENABLE_COPY(coreFullscreen)

    /* render the fullscreen-object */
    virtual void Render(const coreProgramPtr& pProgram)override;
    virtual void Render()override;
};


#endif /* _CORE_GUARD_OBJECT2D_H_ */