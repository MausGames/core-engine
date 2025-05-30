///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#pragma once
#ifndef _CORE_GUARD_GRAPHICS_H_
#define _CORE_GUARD_GRAPHICS_H_

// TODO 3: implement conditional rendering (e.g. occlusion queries)
// TODO 3: array textures
// TODO 3: wrap glDisable, glDepthMask, glCullFace, glClearColor, etc. within the class (reference-counter for nested calls ?)
// TODO 3: if(CORE_GL_SUPPORT(ARB_clip_control)) glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE); -> improves depth-precision, breaks depth-dependent rendering (water, shadow) in Project One
// TODO 3: async glReadPixels, improve screenshot with pixel-pack-buffer (currently cannot move-only into lambda, and engine-reset can be problematic)
// TODO 5: <old comment style>
// TODO 3: check for max number of draw buffers and color attachments, max render buffer size and view-port dimension, max number of vertex attributes, max texture size


// ****************************************************************
/* graphics definitions */
#define CORE_GRAPHICS_LIGHTS                   (1u)                                         // number of ambient lights
#define CORE_GRAPHICS_UNIFORM_TRANSFORM3D_SIZE (204u)                                       // 3d-transformation uniform data size (view-projection (64), camera matrix (64), perspective (64), camera position (12))
#define CORE_GRAPHICS_UNIFORM_TRANSFORM2D_SIZE (80u)                                        // 2d-transformation uniform data size (ortho (64), resolution (16))
#define CORE_GRAPHICS_UNIFORM_AMBIENT_SIZE     (CORE_GRAPHICS_LIGHTS * sizeof(coreLight))   // ambient uniform data size (light-positions (N*16), light-directions (N*16), light-values (N*16))
#define CORE_GRAPHICS_UNIFORM_BUFFERS          (32u)                                        // number of concurrent uniform buffer objects


// ****************************************************************
/* main graphics component */
class CoreGraphics final
{
private:
    /* light structure */
    struct coreLight final
    {
        coreVector4 vPosition;    // position of the light
        coreVector4 vDirection;   // direction and range of the light
        coreVector4 vValue;       // color and strength value
    };


private:
    SDL_GLContext m_pRenderContext;             // primary OpenGL context for render operations
    SDL_GLContext m_pResourceContext;           // secondary OpenGL context for resource loading

    coreFloat m_fFOV;                           // field-of-view
    coreFloat m_fNearClip;                      // near clipping plane
    coreFloat m_fFarClip;                       // far clipping plane
    coreFloat m_fAspectRatio;                   // aspect ratio

    coreVector3 m_vCamPosition;                 // position of the camera
    coreVector3 m_vCamDirection;                // direction of the camera
    coreVector3 m_vCamOrientation;              // orientation of the camera
    coreMatrix4 m_mCamera;                      // camera matrix

    coreMatrix4 m_mPerspective;                 // perspective projection matrix
    coreMatrix4 m_mOrtho;                       // orthographic projection matrix
    coreVector4 m_vViewResolution;              // current viewport resolution (xy = normal, zw = reciprocal)

    coreLight m_aLight[CORE_GRAPHICS_LIGHTS];   // global ambient lights

    coreUniformBuffer m_Transform3DBuffer;      // uniform buffer object for 3d-transformation data
    coreUniformBuffer m_Transform2DBuffer;      // uniform buffer object for 2d-transformation data
    coreUniformBuffer m_AmbientBuffer;          // uniform buffer object for ambient data
    coreUint8         m_iUniformUpdate;         // update status for the UBOs (dirty flag)

    coreUint32 m_aiScissorData[4];              // current scissor test properties

    coreUint64   m_iMemoryStart;                // available graphics memory at the start of the application (in bytes)
    coreUint8    m_iMaxSamples;                 // max multisample anti aliasing level
    coreUint8    m_aiMaxSamplesEQAA[3];         // max enhanced quality anti aliasing levels (color, depth, storage)
    coreUint8    m_aiMaxSamplesCSAA[2];         // max coverage sampling anti aliasing levels (coverage, color)
    coreUint8    m_iMaxAnisotropy;              // max anisotropic texture filter level
    coreUint8    m_iMaxTextures;                // max number of texture units (only for fragment shader)
    corePoint2U8 m_VersionOpenGL;               // available OpenGL version
    corePoint2U8 m_VersionGLSL;                 // available GLSL version


private:
    CoreGraphics()noexcept;
    ~CoreGraphics();


public:
    FRIEND_CLASS(Core)
    DISABLE_COPY(CoreGraphics)

    /* control camera and view frustum */
    void SetCamera(const coreVector3 vPosition, const coreVector3 vDirection, const coreVector3 vOrientation);
    void SetView  (coreVector2 vResolution, const coreFloat fFOV, const coreFloat fNearClip, const coreFloat fFarClip, coreFloat fAspectRatio);

    /* control ambient */
    void SetLight(const coreUintW iIndex, const coreVector4 vPosition, const coreVector4 vDirection, const coreVector4 vValue);

    /* update data for the uniform buffer objects */
    void UpdateTransformation();
    void UpdateAmbient();

    /* handle stencil testing */
    void WriteStencilTest(const GLenum iBackZFail, const GLenum iBackZPass, const GLenum iFrontZFail, const GLenum iFrontZPass);
    void WriteStencilTest(const GLenum iZFail,     const GLenum iZPass);
    void ReadStencilTest (const GLenum iBackFunc, const coreUint8 iBackRef, const coreUint8 iBackMask, const GLenum iFrontFunc, const coreUint8 iFrontRef, const coreUint8 iFrontMask);
    void ReadStencilTest (const GLenum iFunc,     const coreUint8 iRef,     const coreUint8 iMask);
    void EndStencilTest  ();

    /* handle scissor testing */
    void StartScissorTest(const coreVector2 vLowerLeft, const coreVector2 vUpperRight);
    void EndScissorTest  ();

    /* handle conservative rasterization */
    coreStatus StartConservativeRaster();
    void       EndConservativeRaster();

    /* handle OpenGL debug output */
    friend void GL_APIENTRY WriteOpenGL(const GLenum iSource, const GLenum iType, const GLuint iID, const GLenum iSeverity, const GLsizei iLength, const GLchar* pcMessage, const void* pUserParam);
    void DebugOpenGL();
    void CheckOpenGL();
    void LabelOpenGL(const GLenum iType, const GLuint iIdentifier, const coreChar* pcLabel);

    /* retrieve graphics properties */
    coreUint64         ProcessGpuMemory()const;
    coreBool           SystemGpuMemory (coreUint64* OUTPUT piAvailable, coreUint64* OUTPUT piTotal)const;
    const coreGpuType& SystemGpuType   ()const;

    /* take screenshot */
    void        TakeScreenshot(const coreChar* pcPath)const;
    inline void TakeScreenshot()const {this->TakeScreenshot(coreData::UserFolderPrivate(coreData::DateTimePrint("screenshots/screenshot_%Y%m%d_%H%M%S")));}

    /* get component properties */
    inline const SDL_GLContext& GetRenderContext  ()const                       {return m_pRenderContext;}
    inline const SDL_GLContext& GetResourceContext()const                       {return m_pResourceContext;}
    inline const coreFloat&     GetFOV            ()const                       {return m_fFOV;}
    inline const coreFloat&     GetNearClip       ()const                       {return m_fNearClip;}
    inline const coreFloat&     GetFarClip        ()const                       {return m_fFarClip;}
    inline const coreFloat&     GetAspectRatio    ()const                       {return m_fAspectRatio;}
    inline const coreVector3&   GetCamPosition    ()const                       {return m_vCamPosition;}
    inline const coreVector3&   GetCamDirection   ()const                       {return m_vCamDirection;}
    inline const coreVector3&   GetCamOrientation ()const                       {return m_vCamOrientation;}
    inline const coreMatrix4&   GetCamera         ()const                       {return m_mCamera;}
    inline const coreMatrix4&   GetPerspective    ()const                       {return m_mPerspective;}
    inline const coreMatrix4&   GetOrtho          ()const                       {return m_mOrtho;}
    inline const coreVector4&   GetViewResolution ()const                       {return m_vViewResolution;}
    inline const coreLight&     GetLight          (const coreUintW iIndex)const {ASSERT(iIndex < CORE_GRAPHICS_LIGHTS) return m_aLight[iIndex];}

    /* check OpenGL properties */
    inline const coreUint8&    GetMaxSamples    ()const                       {return m_iMaxSamples;}
    inline const coreUint8&    GetMaxSamplesEQAA(const coreUintW iIndex)const {ASSERT(iIndex < ARRAY_SIZE(m_aiMaxSamplesEQAA)) return m_aiMaxSamplesEQAA[iIndex];}
    inline const coreUint8&    GetMaxSamplesCSAA(const coreUintW iIndex)const {ASSERT(iIndex < ARRAY_SIZE(m_aiMaxSamplesCSAA)) return m_aiMaxSamplesCSAA[iIndex];}
    inline const coreUint8&    GetMaxAnisotropy ()const                       {return m_iMaxAnisotropy;}
    inline const coreUint8&    GetMaxTextures   ()const                       {return m_iMaxTextures;}
    inline const corePoint2U8& GetVersionOpenGL ()const                       {return m_VersionOpenGL;}
    inline const corePoint2U8& GetVersionGLSL   ()const                       {return m_VersionGLSL;}


private:
    /* update the graphics scene */
    void __UpdateScene();

    /* update the Emscripten canvas */
    void __UpdateEmscripten();
};


#endif /* _CORE_GUARD_GRAPHICS_H_ */