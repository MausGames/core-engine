//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MENU_H_
#define _CORE_GUARD_MENU_H_

// TODO: allow custom animations
// TODO: remove wrong order while animating (double-objects)
// TODO: implement general relative behavior, submenu, hierarchy (menu/2d objects)
// TODO: optimize rendering with a frame buffer ?
// TODO: implement tab-switching and keyboard/gamepad control
// TODO: sized menu class ? (# I forgot what it was)
// TODO: template class !
// TODO: handle individual transparency better
// TODO: depth-pass
// TODO: surface-vector-vector ?


// ****************************************************************
// menu aggregation class
class coreMenu : public coreObject2D
{
private:
    std::vector<coreObject2D*>* m_papObject;     //!< surfaces with pointers to menu objects
    coreObject2D* m_pCurObject;                  //!< current object with input focus

    coreUint8 m_iNumSurfaces;                    //!< number of surfaces
    coreUint8 m_iCurSurface;                     //!< current surface
    coreUint8 m_iOldSurface;                     //!< previous surface

    coreTimer m_Transition;                      //!< timer for a transition between two surfaces
    std::vector<coreObject2D*> m_aapRender[3];   //!< render-lists during a transition (0 = both | 1 = old surface | 2 = new surface)


public:
    coreMenu(const coreUint8 iNumSurfaces, const coreUint8 iStartSurface)noexcept;
    virtual ~coreMenu()override;

    DISABLE_COPY(coreMenu)

    //! render and move the menu
    //! @{
    virtual void Render()override;
    virtual void Move  ()override;
    //! @}

    //! bind and unbind menu objects
    //! @{
    void BindObject  (const coreUintW iSurface, coreObject2D* pObject);
    void UnbindObject(const coreUintW iSurface, coreObject2D* pObject);
    //! @}

    //! control surfaces
    //! @{
    coreBool ChangeSurface(const coreUint8 iNewSurface, const coreFloat fSpeed);
    //! @}

    //! get object properties
    //! @{
    inline coreObject2D*    GetCurObject  ()const {return m_pCurObject;}
    inline const coreUint8& GetNumSurfaces()const {return m_iNumSurfaces;}
    inline const coreUint8& GetCurSurface ()const {return m_iCurSurface;}
    inline const coreUint8& GetOldSurface ()const {return m_iOldSurface;}
    inline const coreTimer& GetTransition ()const {return m_Transition;}
    //! @}
};


#endif // _CORE_GUARD_MENU_H_