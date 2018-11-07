///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
CoreGraphics::CoreGraphics()noexcept
: m_pRenderContext   (NULL)
, m_pResourceContext (NULL)
, m_fFOV             (0.0f)
, m_fNearClip        (0.0f)
, m_fFarClip         (0.0f)
, m_vCamPosition     (coreVector3(0.0f,0.0f,0.0f))
, m_vCamDirection    (coreVector3(0.0f,0.0f,0.0f))
, m_vCamOrientation  (coreVector3(0.0f,0.0f,0.0f))
, m_mCamera          (coreMatrix4::Identity())
, m_mPerspective     (coreMatrix4::Identity())
, m_mOrtho           (coreMatrix4::Identity())
, m_vViewResolution  (coreVector4(0.0f,0.0f,0.0f,0.0f))
, m_aLight           {{}}
, m_TransformBuffer  ()
, m_AmbientBuffer    ()
, m_aTransformSync   {}
, m_aAmbientSync     {}
, m_iUniformUpdate   (0u)
, m_iMaxSamples      (0u)
, m_iMaxAnisotropy   (0u)
, m_fVersionOpenGL   (0.0f)
, m_fVersionGLSL     (0.0f)
{
    Core::Log->Header("Graphics Interface");

    // create render context
    m_pRenderContext = SDL_GL_CreateContext(Core::System->GetWindow());
    if(!m_pRenderContext) Core::Log->Error("Render context could not be created (SDL: %s)", SDL_GetError());
                     else Core::Log->Info ("Render context created");

    // init OpenGL
    coreInitOpenGL();

    // enable OpenGL debug output
    Core::Log->DebugOpenGL();

    // save version numbers
    m_fVersionOpenGL = coreData::StrVersion(r_cast<const coreChar*>(glGetString(GL_VERSION)));
    m_fVersionGLSL   = coreData::StrVersion(r_cast<const coreChar*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

    // get max anti aliasing level
    if(CORE_GL_SUPPORT(EXT_framebuffer_multisample))
    {
        GLint iValue; glGetIntegerv(GL_MAX_SAMPLES, &iValue);
        m_iMaxSamples = MAX(iValue, 0);
    }

    // get max texture filter level
    if(CORE_GL_SUPPORT(EXT_texture_filter_anisotropic))
    {
        GLfloat fValue; glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fValue);
        m_iMaxAnisotropy = F_TO_UI(MAX(fValue, 1.0f));
    }

    // log video card information
    Core::Log->ListStartInfo("Video Card Information");
    {
        std::string sExtensions;
        coreExtensions(&sExtensions);

        std::string sPlatformExtensions;
        corePlatformExtensions(&sPlatformExtensions);

        Core::Log->ListAdd(CORE_LOG_BOLD("Vendor:")         " %s", glGetString(GL_VENDOR));
        Core::Log->ListAdd(CORE_LOG_BOLD("Renderer:")       " %s", glGetString(GL_RENDERER));
        Core::Log->ListAdd(CORE_LOG_BOLD("OpenGL Version:") " %s", glGetString(GL_VERSION));
        Core::Log->ListAdd(CORE_LOG_BOLD("Shader Version:") " %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
        Core::Log->ListAdd(sExtensions        .c_str());
        Core::Log->ListAdd(sPlatformExtensions.c_str());
    }
    Core::Log->ListEnd();

    // enable vertical synchronization
    if(Core::Config->GetBool(CORE_CONFIG_SYSTEM_VSYNC))
    {
             if(!SDL_GL_SetSwapInterval(-1)) Core::Log->Info("Vertical synchronization enabled (extended)");
        else if(!SDL_GL_SetSwapInterval( 1)) Core::Log->Info("Vertical synchronization enabled (normal)");
        else Core::Log->Warning("Vertical synchronization not directly supported (SDL: %s)", SDL_GetError());
    }

    // setup texturing and packing
    if(CORE_GL_SUPPORT(V2_compatibility) || DEFINED(_CORE_GLES_)) glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
    glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);
    glPixelStorei(GL_PACK_ALIGNMENT,   4);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    // setup depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(true);
    glClearDepth(1.0f);

    // setup culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // setup alpha blending
    glEnable(GL_BLEND);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    // setup shading and rasterization
    if(CORE_GL_SUPPORT(V2_compatibility))           glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    if(CORE_GL_SUPPORT(NV_multisample_filter_hint)) glHint(GL_MULTISAMPLE_FILTER_HINT_NV,  GL_NICEST);
    if(CORE_GL_SUPPORT(ARB_multisample))            glEnable(GL_MULTISAMPLE);
    if(CORE_GL_SUPPORT(ARB_framebuffer_sRGB))       glDisable(GL_FRAMEBUFFER_SRGB);
    glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);
    glDisable(GL_DITHER);
    glColorMask(true, true, true, true);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // enable maximum sample shading rate
    if(CORE_GL_SUPPORT(ARB_sample_shading))
        glMinSampleShading(1.0f);

    // disable parallel shader compilation
    if(CORE_GL_SUPPORT(ARB_parallel_shader_compile))
        glMaxShaderCompilerThreadsARB(0u);

    // create uniform buffer objects
    if(CORE_GL_SUPPORT(ARB_uniform_buffer_object))
    {
        m_TransformBuffer.Create(GL_UNIFORM_BUFFER, coreMath::CeilAlign(CORE_GRAPHICS_UNIFORM_TRANSFORM_SIZE, 256u) * CORE_GRAPHICS_UNIFORM_BUFFERS, NULL, CORE_DATABUFFER_STORAGE_DYNAMIC);
        m_AmbientBuffer  .Create(GL_UNIFORM_BUFFER, coreMath::CeilAlign(CORE_GRAPHICS_UNIFORM_AMBIENT_SIZE,   256u) * CORE_GRAPHICS_UNIFORM_BUFFERS, NULL, CORE_DATABUFFER_STORAGE_DYNAMIC);
    }

    // reset camera and view frustum
    this->SetCamera(coreVector3(0.0f,0.0f,0.0f), coreVector3(0.0f,0.0f,-1.0f), coreVector3(0.0f,1.0f,0.0f));
    this->SetView  (coreVector2(0.0f,0.0f), PI*0.25f, 1.0f, 1000.0f);

    // reset ambient light
    for(coreUintW i = 0u; i < CORE_GRAPHICS_LIGHTS; ++i)
        this->SetLight(i, coreVector4(0.0f,0.0f,0.0f,0.0f), coreVector4(0.0f,0.0f,-1.0f,1.0f), coreVector4(1.0f,1.0f,1.0f,1.0f));

    // reset scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SDL_GL_SwapWindow(Core::System->GetWindow());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(Core::Config->GetBool(CORE_CONFIG_BASE_ASYNCMODE) && DEFINED(_CORE_ASYNC_))
    {
        // create resource context (after reset, because of flickering on Windows with fullscreen)
        m_pResourceContext = SDL_GL_CreateContext(Core::System->GetWindow());
        if(!m_pResourceContext) Core::Log->Warning("Resource context could not be created (SDL: %s)", SDL_GetError());
                           else Core::Log->Info   ("Resource context created");

        // re-assign render context to main window
        SDL_GL_MakeCurrent(Core::System->GetWindow(), m_pRenderContext);
    }
}


// ****************************************************************
// destructor
CoreGraphics::~CoreGraphics()
{
    // delete uniform buffer objects
    m_TransformBuffer.Delete();
    m_AmbientBuffer  .Delete();

    // exit OpenGL
    coreExitOpenGL();

    // disable vertical synchronization
    SDL_GL_SetSwapInterval(0);

    // dissociate render context from main window
    SDL_GL_MakeCurrent(Core::System->GetWindow(), NULL);

    // delete OpenGL contexts
    SDL_GL_DeleteContext(m_pResourceContext);
    SDL_GL_DeleteContext(m_pRenderContext);

    Core::Log->Info(CORE_LOG_BOLD("Graphics Interface shut down"));
}


// ****************************************************************
// set camera and create camera matrix
void CoreGraphics::SetCamera(const coreVector3& vPosition, const coreVector3& vDirection, const coreVector3& vOrientation)
{
    coreBool bNewCamera = false;

    // set properties of the camera
    ASSERT(vDirection.IsNormalized() && vOrientation.IsNormalized())
    if(m_vCamPosition    != vPosition)    {m_vCamPosition    = vPosition;    bNewCamera = true;}
    if(m_vCamDirection   != vDirection)   {m_vCamDirection   = vDirection;   bNewCamera = true;}
    if(m_vCamOrientation != vOrientation) {m_vCamOrientation = vOrientation; bNewCamera = true;}

    if(bNewCamera)
    {
        // create camera matrix
        m_mCamera = coreMatrix4::Camera(m_vCamPosition, m_vCamDirection, m_vCamOrientation);

        // invoke transformation data update
        ADD_BIT(m_iUniformUpdate, 0u)
    }
}


// ****************************************************************
// set view frustum and create projection matrices
void CoreGraphics::SetView(coreVector2 vResolution, const coreFloat fFOV, const coreFloat fNearClip, const coreFloat fFarClip)
{
    coreBool bNewView = false;

    // retrieve window resolution
    if(!vResolution.x) vResolution.x = Core::System->GetResolution().x;
    if(!vResolution.y) vResolution.y = Core::System->GetResolution().y;

    // set properties of the view frustum
    if(m_vViewResolution.xy() != vResolution)
    {
        // save viewport resolution
        m_vViewResolution.xy(vResolution);
        m_vViewResolution.zw(coreVector2(1.0f,1.0f) / vResolution);

        // set viewport
        glViewport(0, 0, F_TO_SI(vResolution.x), F_TO_SI(vResolution.y));
        bNewView = true;
    }
    if(m_fFOV      != fFOV)      {m_fFOV      = fFOV;      bNewView = true;}
    if(m_fNearClip != fNearClip) {m_fNearClip = fNearClip; bNewView = true;}
    if(m_fFarClip  != fFarClip)  {m_fFarClip  = fFarClip;  bNewView = true;}

    if(bNewView)
    {
        // create projection matrices
        m_mPerspective = coreMatrix4::Perspective(vResolution, m_fFOV, m_fNearClip, m_fFarClip);
        m_mOrtho       = coreMatrix4::Ortho(vResolution);

        // invoke transformation data update
        ADD_BIT(m_iUniformUpdate, 0u)
    }
}


// ****************************************************************
// set and update ambient light
void CoreGraphics::SetLight(const coreUintW iIndex, const coreVector4& vPosition, const coreVector4& vDirection, const coreVector4& vValue)
{
    coreBool bNewLight = false;

    // get requested ambient light
    ASSERT(iIndex < CORE_GRAPHICS_LIGHTS)
    coreLight& oCurLight = m_aLight[iIndex];

    // set properties of the ambient light
    ASSERT(vDirection.xyz().IsNormalized())
    if(oCurLight.vPosition  != vPosition)  {oCurLight.vPosition  = vPosition;  bNewLight = true;}
    if(oCurLight.vDirection != vDirection) {oCurLight.vDirection = vDirection; bNewLight = true;}
    if(oCurLight.vValue     != vValue)     {oCurLight.vValue     = vValue;     bNewLight = true;}

    if(bNewLight)
    {
        // invoke ambient data update
        ADD_BIT(m_iUniformUpdate, 1u)
    }
}


// ****************************************************************
// update transformation data for the uniform buffer objects
void CoreGraphics::UpdateTransformation()
{
    // check update status
    if(!CONTAINS_BIT(m_iUniformUpdate, 0u)) return;
    REMOVE_BIT(m_iUniformUpdate, 0u)

    if(m_TransformBuffer.IsValid())
    {
        const coreMatrix4 mViewProj = m_mCamera * m_mPerspective;

        // switch and check next available sync object
        m_aTransformSync.next();
        m_aTransformSync.current().Check(GL_TIMEOUT_IGNORED, CORE_SYNC_CHECK_NORMAL);
        const coreUint32 iOffset = m_aTransformSync.index() * coreMath::CeilAlign(CORE_GRAPHICS_UNIFORM_TRANSFORM_SIZE, 256u);

        // bind and map required area of the UBO
        glBindBufferRange(GL_UNIFORM_BUFFER, CORE_SHADER_BUFFER_TRANSFORM_NUM, m_TransformBuffer.GetIdentifier(), iOffset, CORE_GRAPHICS_UNIFORM_TRANSFORM_SIZE);
        coreByte* pRange = m_TransformBuffer.Map(iOffset, CORE_GRAPHICS_UNIFORM_TRANSFORM_SIZE, CORE_DATABUFFER_MAP_UNSYNCHRONIZED);

        // update transformation data
        std::memcpy(pRange,                                                 &mViewProj,         sizeof(coreMatrix4));
        std::memcpy(pRange + 1u*sizeof(coreMatrix4),                        &m_mCamera,         sizeof(coreMatrix4));
        std::memcpy(pRange + 2u*sizeof(coreMatrix4),                        &m_mPerspective,    sizeof(coreMatrix4));
        std::memcpy(pRange + 3u*sizeof(coreMatrix4),                        &m_mOrtho,          sizeof(coreMatrix4));
        std::memcpy(pRange + 4u*sizeof(coreMatrix4),                        &m_vViewResolution, sizeof(coreVector4));
        std::memcpy(pRange + 4u*sizeof(coreMatrix4) + 4u*sizeof(coreFloat), &m_vCamPosition,    sizeof(coreVector3));
        m_TransformBuffer.Unmap(pRange);

        // create sync object
        m_aTransformSync.current().Create();
    }
    else
    {
        // invoke manual data forwarding
        coreProgram::Disable(false);
    }
}


// ****************************************************************
// update ambient data for the uniform buffer objects
void CoreGraphics::UpdateAmbient()
{
    // check update status
    if(!CONTAINS_BIT(m_iUniformUpdate, 1u)) return;
    REMOVE_BIT(m_iUniformUpdate, 1u)

    if(m_AmbientBuffer.IsValid())
    {
        // switch and check next available sync object
        m_aAmbientSync.next();
        m_aAmbientSync.current().Check(GL_TIMEOUT_IGNORED, CORE_SYNC_CHECK_NORMAL);
        const coreUint32 iOffset = m_aAmbientSync.index() * coreMath::CeilAlign(CORE_GRAPHICS_UNIFORM_AMBIENT_SIZE, 256u);

        // bind and map required area of the UBO
        glBindBufferRange(GL_UNIFORM_BUFFER, CORE_SHADER_BUFFER_AMBIENT_NUM, m_AmbientBuffer.GetIdentifier(), iOffset, CORE_GRAPHICS_UNIFORM_AMBIENT_SIZE);
        coreByte* pRange = m_AmbientBuffer.Map(iOffset, CORE_GRAPHICS_UNIFORM_AMBIENT_SIZE, CORE_DATABUFFER_MAP_UNSYNCHRONIZED);

        // update ambient data
        std::memcpy(pRange, m_aLight, CORE_GRAPHICS_UNIFORM_AMBIENT_SIZE);
        m_AmbientBuffer.Unmap(pRange);

        // create sync object
        m_aAmbientSync.current().Create();
    }
    else
    {
        // invoke manual data forwarding
        coreProgram::Disable(false);
    }
}


// ****************************************************************
// take screenshot
void CoreGraphics::TakeScreenshot(const coreChar* pcPath)const
{
    const coreUintW iWidthSrc = coreMath::CeilAlign(F_TO_UI(Core::System->GetResolution().x), 4u);
    const coreUintW iWidthDst = F_TO_UI(Core::System->GetResolution().x);
    const coreUintW iHeight   = F_TO_UI(Core::System->GetResolution().y);
    const coreUintW iPitchSrc = iWidthSrc * 3u;
    const coreUintW iPitchDst = iWidthDst * 3u;
    const coreUintW iSize     = iHeight * iPitchSrc;
    ASSERT(iPitchDst <= iPitchSrc)

    // read pixel data from the frame buffer
    coreByte* pData = new coreByte[iSize * 2u];
    glReadPixels(0, 0, iWidthSrc, iHeight, GL_RGB, GL_UNSIGNED_BYTE, pData);

    // copy path into another thread
    const std::string sPathCopy = pcPath;

    Core::Manager::Resource->AttachFunction([=, sPathCopy = std::move(sPathCopy)]()
    {
        // flip pixel data vertically
        coreByte* pConvert = pData + iSize;
        for(coreUintW i = 0u; i < iHeight; ++i)
            std::memcpy(pConvert + (iHeight - i - 1u) * iPitchDst, pData + i * iPitchSrc, iPitchDst);

        // create SDL surface
        coreSurfaceScope pSurface = SDL_CreateRGBSurfaceFrom(pConvert, iWidthDst, iHeight, 24, iPitchDst, CORE_TEXTURE_MASK);
        if(pSurface)
        {
            // create folder hierarchy
            const coreChar* pcFullPath = std::strcmp(coreData::StrExtension(sPathCopy.c_str()), "png") ? PRINT("%s.png", sPathCopy.c_str()) : sPathCopy.c_str();
            coreData::CreateFolder(pcFullPath);

            // save the surface as PNG image
            IMG_SavePNG(pSurface, pcFullPath);
        }

        // delete pixel data
        delete[] pData;
        return CORE_OK;
    });
}


// ****************************************************************
// update the graphics scene
void CoreGraphics::__UpdateScene()
{
    // take screenshot
    if(Core::Input->GetKeyboardButton(CORE_INPUT_KEY(PRINTSCREEN), CORE_INPUT_PRESS))
        this->TakeScreenshot();

    // disable last model, textures and shader-program
    coreModel  ::Disable(true);
    coreTexture::DisableAll();
    coreProgram::Disable(true);

    // explicitly invalidate depth buffer
    if(CORE_GL_SUPPORT(ARB_invalidate_subdata))
    {
        constexpr GLenum aiAttachment[] = {GL_DEPTH};
        glInvalidateFramebuffer(GL_FRAMEBUFFER, ARRAY_SIZE(aiAttachment), aiAttachment);
    }

    // swap color buffers
    Core::Debug->MeasureEnd  (CORE_DEBUG_OVERALL_NAME);
    SDL_GL_SwapWindow(Core::System->GetWindow());
    Core::Debug->MeasureStart(CORE_DEBUG_OVERALL_NAME);

    // clear color and depth buffer
#if defined(_CORE_DEBUG_) || defined(_CORE_GLES_)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
    glClear(GL_DEPTH_BUFFER_BIT);
#endif
}