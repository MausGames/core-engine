//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_GRAPHICS_H_
#define _CORE_GUARD_GRAPHICS_H_

// TODO: deferred shading implementation delayed, but kept in mind
// TODO: FOV and clipping planes should be controlled differently
// TODO: implement post-processing pipeline (in CoreGraphics or own class?)
// TODO: implement conditional rendering (e.g. occlusion queries)
// TODO: persistend mapped buffers, multidrawindirect, array textures


// ****************************************************************
// graphics definitions
#define CORE_GRAPHICS_LIGHTS 4   //!< number of ambient lights


// ****************************************************************
// main graphics component
class CoreGraphics final
{
private:
    //! light structure
    struct coreLight
    {
        coreVector4 vPosition;    //!< position of the light
        coreVector4 vDirection;   //!< direction and range of the light
        coreVector4 vValue;       //!< color and strength value

        constexpr_func coreLight()noexcept;
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
    coreVector4 m_vCurResolution;               //!< current viewport resolution

    coreLight m_aLight[CORE_GRAPHICS_LIGHTS];   //!< global ambient lights

    coreDataBuffer m_iUniformBuffer;            //!< uniform buffer object for global shader-data

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
    void ResizeView(coreVector2 vResolution, const float& fFOV, const float& fNearClip, const float& fFarClip);
    //! @}

    //! control ambient
    //! @{
    void SetLight(const int& iID, const coreVector4& vPosition, const coreVector4& vDirection, const coreVector4& vValue);
    //! @}

    //! take screenshot
    //! @{
    void Screenshot(const char* pcPath)const;
    inline void Screenshot()const {this->Screenshot(coreData::DateTimePrint("screenshots/screenshot_%Y%m%d_%H%M%S"));}
    //! @}

    //! get component properties
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
    inline const coreVector4& GetResolution()const        {return m_vCurResolution;}
    inline const coreLight& GetLight(const int& iID)const {SDL_assert(iID < CORE_GRAPHICS_LIGHTS); return m_aLight[iID];}
    inline const GLuint& GetUniformBuffer()const          {return m_iUniformBuffer;}
    //! @}

    //! check OpenGL versions
    //! @{
    inline const float& VersionOpenGL()const {return m_fOpenGL;}
    inline const float& VersionGLSL()const   {return m_fGLSL;}
    //! @}


private:
    //! update the graphics scene
    //! @{
    void __UpdateScene();
    //! @}

    //! send transformation data to the global UBO
    //! @{
    void __SendTransformation();
    //! @}
};


// ****************************************************************
// constructor
constexpr_func CoreGraphics::coreLight::coreLight()noexcept
: vPosition  (coreVector4(0.0f,0.0f,0.0f,0.0f))
, vDirection (coreVector4(0.0f,0.0f,0.0f,0.0f))
, vValue     (coreVector4(1.0f,1.0f,1.0f,1.0f))
{
}


#endif // _CORE_GUARD_GRAPHICS_H_