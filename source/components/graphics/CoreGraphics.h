//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_GRAPHICS_H_
#define _CORE_GUARD_GRAPHICS_H_


// ****************************************************************
// main graphics component
//! \ingroup component
class CoreGraphics final
{
private:
    SDL_GLContext m_RenderContext;               //!< primary OpenGL context for render operations
    SDL_GLContext m_ResourceContext;             //!< secondary OpenGL context for resource loading

    float m_fFOV;                                //!< field-of-view
    float m_fNearClip;                           //!< near clipping plane
    float m_fFarClip;                            //!< far clipping plane

    coreVector3 m_vCamPosition;                  //!< position of the camera
    coreVector3 m_vCamDirection;                 //!< direction of the camera
    coreVector3 m_vCamOrientation;               //!< orientation of the camera
    coreMatrix m_mCamera;                        //!< camera matrix

    coreMatrix m_mPerspective;                   //!< perspective projection matrix
    coreMatrix m_mOrtho;                         //!< orthogonal projection matrix
    coreVector2 m_vCurResolution;                //!< current viewport resolution

    std::u_map<std::string, bool> m_abFeature;   //!< cached features of the video card
    float m_fOpenGL;                             //!< available OpenGL version
    float m_fGLSL;                               //!< available GLSL version


private:
    CoreGraphics()noexcept;
    ~CoreGraphics();
    friend class Core;

    //! update the graphics scene
    //! @{
    void __UpdateScene();


public:
    //! control view and camera
    //! @{
    void SetCamera(const coreVector3* pvPosition, const coreVector3* pvDirection, const coreVector3* pvOrientation);
    void ResizeView(coreVector2 vResolution);
    //! @}

    //! create a screenshot
    //! @{
    void Screenshot(const char* pcPath);
    void Screenshot();
    //! @}

    //! get component attributes
    //! @{
    inline const SDL_GLContext& GetRenderContext()const   {return m_RenderContext;}
    inline const SDL_GLContext& GetResourceContext()const {return m_ResourceContext;}
    inline const float& GetFOV()const                     {return m_fFOV;}
    inline const float& GetNearClip()const                {return m_fNearClip;}
    inline const float& GetFarClip()const                 {return m_fFarClip;}
    inline const coreVector3& GetCamPosition()const       {return m_vCamPosition;}
    inline const coreVector3& GetCamDirection()const      {return m_vCamDirection;}
    inline const coreVector3& GetCamOrientation()const    {return m_vCamOrientation;}
    inline const coreMatrix& GetCamera()const             {return m_mCamera;}
    inline const coreMatrix& GetPerspective()const        {return m_mPerspective;}
    inline const coreMatrix& GetOrtho()const              {return m_mOrtho;}
    //! @}

    //! check hardware support
    //! @{
    inline const bool& SupportFeature(const char* pcFeature) {if(!m_abFeature.count(pcFeature)) m_abFeature[pcFeature] = (glewIsSupported(pcFeature) ? true : false); return m_abFeature.at(pcFeature);}
    inline const float& SupportOpenGL()const                 {return m_fOpenGL;}
    inline const float& SupportGLSL()const                   {return m_fGLSL;}
    //! @}
};


#endif // _CORE_GUARD_GRAPHICS_H_