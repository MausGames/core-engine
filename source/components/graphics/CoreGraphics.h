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
// TODO: multidrawindirect, array textures
// TODO: wrap glDisable, glDepthMask, glCullFace, etc. within the class (reference-counter for nested calls ?)
// TODO: different names for camera, view, lights
// TODO: defer uniform-push to next shader-enable
// TODO: improve screenshot with pixel-pack-buffer


// ****************************************************************
// graphics definitions
#define CORE_GRAPHICS_LIGHTS                 (4u)                                              //!< number of ambient lights
#define CORE_GRAPHICS_UNIFORM_TRANSFORM_SIZE (4*sizeof(coreMatrix4) + 1*sizeof(coreVector4))   //!< transformation uniform data size (view-projection, camera, perspective, ortho, resolution)
#define CORE_GRAPHICS_UNIFORM_AMBIENT_SIZE   (CORE_GRAPHICS_LIGHTS * sizeof(coreLight))        //!< ambient uniform data size (light-positions, light-directions, light-values)
#define CORE_GRAPHICS_UNIFORM_BUFFERS        (24u)                                             //!< number of concurrent uniform buffer objects


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
    SDL_GLContext m_pRenderContext;                                                  //!< primary OpenGL context for render operations
    SDL_GLContext m_pResourceContext;                                                //!< secondary OpenGL context for resource loading

    coreFloat m_fFOV;                                                                //!< field-of-view
    coreFloat m_fNearClip;                                                           //!< near clipping plane
    coreFloat m_fFarClip;                                                            //!< far clipping plane

    coreVector3 m_vCamPosition;                                                      //!< position of the camera
    coreVector3 m_vCamDirection;                                                     //!< direction of the camera
    coreVector3 m_vCamOrientation;                                                   //!< orientation of the camera
    coreMatrix4 m_mCamera;                                                           //!< camera matrix

    coreMatrix4 m_mPerspective;                                                      //!< perspective projection matrix
    coreMatrix4 m_mOrtho;                                                            //!< orthographic projection matrix
    coreVector4 m_vViewResolution;                                                   //!< current viewport resolution (xy = normal, zw = reciprocal)

    coreLight m_aLight[CORE_GRAPHICS_LIGHTS];                                        //!< global ambient lights

    coreSelect<coreDataBuffer, CORE_GRAPHICS_UNIFORM_BUFFERS> m_aiTransformBuffer;   //!< uniform buffer objects for transformation data
    coreSelect<coreDataBuffer, CORE_GRAPHICS_UNIFORM_BUFFERS> m_aiAmbientBuffer;     //!< uniform buffer objects for ambient data
    coreUint8 m_iUniformUpdate;                                                      //!< update status for the UBOs (dirty flag)

    coreFloat m_fOpenGL;                                                             //!< available OpenGL version
    coreFloat m_fGLSL;                                                               //!< available GLSL version


private:
    CoreGraphics()noexcept;
    ~CoreGraphics();


public:
    FRIEND_CLASS(Core)
    DISABLE_COPY(CoreGraphics)

    //! control camera and view
    //! @{
    void SetCamera(const coreVector3& vPosition, const coreVector3& vDirection, const coreVector3& vOrientation);
    void SetView  (coreVector2 vResolution, const coreFloat& fFOV, const coreFloat& fNearClip, const coreFloat& fFarClip);
    //! @}

    //! control ambient
    //! @{
    void SetLight(const coreUintW& iIndex, const coreVector4& vPosition, const coreVector4& vDirection, const coreVector4& vValue);
    //! @}

    //! send data to the uniform buffer objects
    //! @{
    void SendTransformation();
    void SendAmbient();
    //! @}

    //! take screenshot
    //! @{
    void        Screenshot(const coreChar* pcPath)const;
    inline void Screenshot()const {this->Screenshot(coreData::DateTimePrint("screenshots/screenshot_%Y%m%d_%H%M%S"));}
    //! @}

    //! get component properties
    //! @{
    inline const SDL_GLContext&  GetRenderContext  ()const                        {return m_pRenderContext;}
    inline const SDL_GLContext&  GetResourceContext()const                        {return m_pResourceContext;}
    inline const coreFloat&      GetFOV            ()const                        {return m_fFOV;}
    inline const coreFloat&      GetNearClip       ()const                        {return m_fNearClip;}
    inline const coreFloat&      GetFarClip        ()const                        {return m_fFarClip;}
    inline const coreVector3&    GetCamPosition    ()const                        {return m_vCamPosition;}
    inline const coreVector3&    GetCamDirection   ()const                        {return m_vCamDirection;}
    inline const coreVector3&    GetCamOrientation ()const                        {return m_vCamOrientation;}
    inline const coreMatrix4&    GetCamera         ()const                        {return m_mCamera;}
    inline const coreMatrix4&    GetPerspective    ()const                        {return m_mPerspective;}
    inline const coreMatrix4&    GetOrtho          ()const                        {return m_mOrtho;}
    inline const coreVector4&    GetViewResolution ()const                        {return m_vViewResolution;}
    inline const coreLight&      GetLight          (const coreUintW& iIndex)const {ASSERT(iIndex < CORE_GRAPHICS_LIGHTS) return m_aLight[iIndex];}
    inline const coreDataBuffer* GetTransformBuffer()const                        {return m_aiTransformBuffer;}
    inline const coreDataBuffer* GetAmbientBuffer  ()const                        {return m_aiAmbientBuffer;}
    //! @}

    //! check OpenGL versions
    //! @{
    inline const coreFloat& VersionOpenGL()const {return m_fOpenGL;}
    inline const coreFloat& VersionGLSL  ()const {return m_fGLSL;}
    //! @}


private:
    //! update the graphics scene
    //! @{
    void __UpdateScene();
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