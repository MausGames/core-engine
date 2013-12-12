//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_MENU_H_
#define _CORE_GUARD_MENU_H_


// ****************************************************************
// menu aggregation class
// TODO: allow custom animations
// TODO: remove wrong order while animating (double-objects)
// TODO: implement general relative behavior (menu/2d objects)
// TODO: optimize rendering with the frame buffer
class coreMenu : public coreObject2D
{
private:
    std::vector<coreObject2D*>* m_papObject;     //!< surfaces with pointers to menu objects
    coreObject2D* m_pCurObject;                  //!< current object with input focus
                                                        
    coreByte m_iNumSurface;                      //!< number of surfaces
    coreByte m_iCurSurface;                      //!< current surface
    coreByte m_iOldSurface;                      //!< previous surface

    std::vector<coreObject2D*> m_aapRender[3];   //!< render-lists during a transition (0 = both | 1 = old surface | 2 = new surface)
    coreTimer m_Transition;                      //!< timer for a transition between two surfaces

    coreFrameBuffer* m_pFrameBuffer;             //!< frame buffer for optimized rendering
        

public:
    coreMenu(const coreByte& iNumSurfaces)noexcept;
    virtual ~coreMenu();

    //! render and move the menu
    //! @{
    virtual void Render()override;
    virtual void Move()override;
    //! @}

    //! manage menu objects
    //! @{
    void AddObject(const coreByte& iSurface, coreObject2D* pObject);
    void RemoveObject(const coreByte& iSurface, coreObject2D* pObject);
    void ClearObjects();
    //! @}

    //! control the current surface
    //! @{
    bool ChangeSurface(const coreByte& iNewSurface, const float& fSpeed);
    //! @}

    //! get object attributes
    //! @{
    inline coreObject2D* GetCurObject()const    {return m_pCurObject;}
    inline const coreByte& GetCurSurface()const {return m_iCurSurface;}
    inline const coreByte& GetOldSurface()const {return m_iOldSurface;}
    //! @}


private:
    DISABLE_COPY(coreMenu)
};


#endif // _CORE_GUARD_MENU_H_