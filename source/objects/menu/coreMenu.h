///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MENU_H_
#define _CORE_GUARD_MENU_H_

// TODO 3: allow custom animations
// TODO 2: remove wrong order while animating (double-objects)
// TODO 3: implement general relative behavior, submenu, hierarchy (menu/2d objects)
// TODO 5: optimize rendering with a frame buffer ?
// TODO 5: implement tab-switching and keyboard/gamepad control
// TODO 2: handle individual transparency better
// TODO 5: depth-pass
// TODO 5: surface-vector-vector instead of static size ?
// TODO 5: <old comment style>


// ****************************************************************
/* menu aggregation class */
class coreMenu : public coreObject2D
{
private:
    coreSet<coreObject2D*>* m_papObject;      // surfaces with pointers to menu objects
    coreObject2D* m_pCurObject;               // current object with input focus

    coreUint8 m_iNumSurfaces;                 // number of surfaces
    coreUint8 m_iCurSurface;                  // current surface
    coreUint8 m_iOldSurface;                  // previous surface

    coreTimer m_Transition;                   // timer for a transition between two surfaces
    coreList<coreObject2D*> m_aapRender[3];   // render-lists during a transition (0 = both | 1 = old surface | 2 = new surface)


public:
    coreMenu(const coreUint8 iNumSurfaces, const coreUint8 iStartSurface)noexcept;
    virtual ~coreMenu()override;

    DISABLE_COPY(coreMenu)

    /* render and move the menu */
    virtual void Render()override;
    virtual void Move  ()override;

    /* bind and unbind menu objects */
    void BindObject  (const coreUintW iSurface, coreObject2D* pObject);
    void UnbindObject(const coreUintW iSurface, coreObject2D* pObject);

    /* control surfaces */
    coreBool ChangeSurface(const coreUint8 iNewSurface, const coreFloat fSpeed);

    /* access menu object list directly */
    inline const coreSet<coreObject2D*>* List(const coreUintW iSurface)const {ASSERT(iSurface < m_iNumSurfaces) return &m_papObject[iSurface];}

    /* get object properties */
    inline coreObject2D*    GetCurObject  ()const {return m_pCurObject;}
    inline const coreUint8& GetNumSurfaces()const {return m_iNumSurfaces;}
    inline const coreUint8& GetCurSurface ()const {return m_iCurSurface;}
    inline const coreUint8& GetOldSurface ()const {return m_iOldSurface;}
    inline const coreTimer& GetTransition ()const {return m_Transition;}
};


#endif /* _CORE_GUARD_MENU_H_ */