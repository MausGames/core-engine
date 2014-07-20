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


// ****************************************************************
// menu aggregation class
// TODO: allow custom animations
// TODO: remove wrong order while animating (double-objects)
// TODO: implement general relative behavior, submenu, hierarchy (menu/2d objects)
// TODO: optimize rendering with a frame buffer
// TODO: implement tab-switching and keyboard/gamepad control
// TODO: sized menu class ? (oh god, I forgot what this was)
// TODO: template class !
// TODO: handle individual transparency better
class coreMenu : public coreObject2D
{
private:
    std::vector<coreObject2D*>* m_papObject;     //!< surfaces with pointers to menu objects
    coreObject2D* m_pCurObject;                  //!< current object with input focus
                                                        
    coreByte m_iNumSurfaces;                     //!< number of surfaces
    coreByte m_iCurSurface;                      //!< current surface
    coreByte m_iOldSurface;                      //!< previous surface

    std::vector<coreObject2D*> m_aapRender[3];   //!< render-lists during a transition (0 = both | 1 = old surface | 2 = new surface)
    coreTimer m_Transition;                      //!< timer for a transition between two surfaces

    coreFrameBuffer* m_pFrameBuffer;             //!< frame buffer for optimized rendering
        

public:
    coreMenu(const coreByte& iNumSurfaces, const coreByte& iStartSurface)noexcept;
    virtual ~coreMenu();

    //! render and move the menu
    //! @{
    virtual void Render()override;
    virtual void Move  ()override;
    //! @}

    //! bind and unbind menu objects
    //! @{
    void BindObject  (const coreByte& iSurface, coreObject2D* pObject);
    void UnbindObject(const coreByte& iSurface, coreObject2D* pObject);
    //! @}

    //! control surfaces
    //! @{
    bool ChangeSurface(const coreByte& iNewSurface, const float& fSpeed);
    //! @}

    //! get object properties
    //! @{
    inline coreObject2D*    GetCurObject  ()const {return m_pCurObject;}
    inline const coreByte&  GetNumSurfaces()const {return m_iNumSurfaces;}
    inline const coreByte&  GetCurSurface ()const {return m_iCurSurface;}
    inline const coreByte&  GetOldSurface ()const {return m_iOldSurface;}
    inline const coreTimer& GetTransition ()const {return m_Transition;}
    //! @}


private:
    DISABLE_COPY(coreMenu)
};


#endif // _CORE_GUARD_MENU_H_