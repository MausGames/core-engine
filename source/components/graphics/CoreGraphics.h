//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_GRAPHICS_H_
#define _CORE_GUARD_GRAPHICS_H_


// ****************************************************************
// graphics definitions
#define CORE_GRAPHICS_LIGHTS 4   //!< number of ambient lights


// ****************************************************************
// main graphics component
//! \ingroup component
class CoreGraphics final
{
private:
    //! light structure
    struct coreLight
    {
        coreVector3 vPosition;    //!< position of the light
        coreVector3 vDirection;   //!< direction of the light
        float fRange;             //!< range for attenuation calculations

        coreVector4 vValue;       //!< color and strength value
    };


private:
    SDL_GLContext m_RenderContext;              //!< primary OpenGL context for render operations
    SDL_GLContext m_ResourceContext;            //!< secondary OpenGL context for resource loading

    float m_fFOV;                               //!< field-of-view
    float m_fNearClip;                          //!< near clipping plane
    float m_fFarClip;                           //!< far clipping plane

    coreVector3 m_vCamPosition;                 //!< position of the camera
    coreVector3 m_vCamDirection;                //!< direction of the camera
    coreVector3 m_vCamOrientation;              //!< orientation of the camera
    coreMatrix4 m_mCamera;                      //!< camera matrix

    coreMatrix4 m_mPerspective;                 //!< perspective projection matrix
    coreMatrix4 m_mOrtho;                       //!< orthogonal projection matrix
    coreVector2 m_vCurResolution;               //!< current viewport resolution

    coreLight m_aLight[CORE_GRAPHICS_LIGHTS];   //!< global ambient lights

    GLuint m_iUniformBuffer;                    //!< uniform buffer object for global shader-data

    coreLookup<bool> m_abFeature;               //!< cached features of the video card
    float m_fOpenGL;                            //!< available OpenGL version
    float m_fGLSL;                              //!< available GLSL version


private:
    CoreGraphics()noexcept;
    ~CoreGraphics();
    friend class Core;


public:
    //! control camera and view
    //! @{
    void SetCamera(const coreVector3& vPosition, const coreVector3& vDirection, const coreVector3& vOrientation);
    void ResizeView(coreVector2 vResolution);
    //! @}

    //! control ambient
    //! @{
    void SetLight(const int& iID, const coreVector3& vPosition, const coreVector3& vDirection, const float& fRange, const coreVector4& vValue);
    //! @}

    //! take screenshot
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
    inline const coreMatrix4& GetCamera()const            {return m_mCamera;}
    inline const coreMatrix4& GetPerspective()const       {return m_mPerspective;}
    inline const coreMatrix4& GetOrtho()const             {return m_mOrtho;}
    inline const coreLight& GetLight(const int& iID)const {SDL_assert(iID < CORE_GRAPHICS_LIGHTS); return m_aLight[iID];}
    inline const GLuint& GetUniformBuffer()const          {return m_iUniformBuffer;}
    //! @}

    //! check hardware support
    //! @{
    inline const bool& SupportFeature(const char* pcFeature) {if(!m_abFeature.count(pcFeature)) m_abFeature[pcFeature] = (glewIsSupported(pcFeature) ? true : false); return m_abFeature.at(pcFeature);}
    inline const float& SupportOpenGL()const                 {return m_fOpenGL;}
    inline const float& SupportGLSL()const                   {return m_fGLSL;}
    //! @}


private:
    //! update the graphics scene
    //! @{
    void __UpdateScene();
    //! @}
};


#endif // _CORE_GUARD_GRAPHICS_H_