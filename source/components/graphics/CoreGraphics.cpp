///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
/* constructor */
CoreGraphics::CoreGraphics()noexcept
: m_pRenderContext    (NULL)
, m_pResourceContext  (NULL)
, m_fFOV              (0.0f)
, m_fNearClip         (0.0f)
, m_fFarClip          (0.0f)
, m_fAspectRatio      (0.0f)
, m_vCamPosition      (coreVector3(0.0f,0.0f,0.0f))
, m_vCamDirection     (coreVector3(0.0f,0.0f,0.0f))
, m_vCamOrientation   (coreVector3(0.0f,0.0f,0.0f))
, m_mCamera           (coreMatrix4::Identity())
, m_mPerspective      (coreMatrix4::Identity())
, m_mOrtho            (coreMatrix4::Identity())
, m_vViewResolution   (coreVector4(0.0f,0.0f,0.0f,0.0f))
, m_aLight            {}
, m_Transform3DBuffer ()
, m_Transform2DBuffer ()
, m_AmbientBuffer     ()
, m_iUniformUpdate    (0u)
, m_aiScissorData     {}
, m_iMemoryStart      (0u)
, m_iMaxSamples       (0u)
, m_aiMaxSamplesEQAA  {}
, m_aiMaxSamplesCSAA  {}
, m_iMaxAnisotropy    (0u)
, m_iMaxTextures      (0u)
, m_VersionOpenGL     (corePoint2U8(0u, 0u))
, m_VersionGLSL       (corePoint2U8(0u, 0u))
{
    Core::Log->Header("Graphics Interface");

    // create render context
    m_pRenderContext = SDL_GL_CreateContext(Core::System->GetWindow());
    if(!m_pRenderContext) Core::Log->Error("Render context could not be created (SDL: %s)", SDL_GetError());
                     else Core::Log->Info ("Render context created");

    // init OpenGL
    coreInitOpenGL();

    // enable OpenGL debug output
    this->DebugOpenGL();

    // save version numbers
    m_VersionOpenGL = coreData::StrVersion(r_cast<const coreChar*>(glGetString(GL_VERSION)));
    m_VersionGLSL   = coreData::StrVersion(r_cast<const coreChar*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

    // get max anti aliasing level
    if(CORE_GL_SUPPORT(EXT_framebuffer_multisample))
    {
        GLint iValue = 0; glGetIntegerv(GL_MAX_SAMPLES, &iValue);
        m_iMaxSamples = MAX(iValue, 0);

        // handle AMD extension (EQAA)
        if(CORE_GL_SUPPORT(AMD_framebuffer_multisample_advanced))
        {
            iValue = 0; glGetIntegerv(GL_MAX_COLOR_FRAMEBUFFER_SAMPLES_AMD,         &iValue); m_aiMaxSamplesEQAA[0] = MAX(iValue, 0);
            iValue = 0; glGetIntegerv(GL_MAX_DEPTH_STENCIL_FRAMEBUFFER_SAMPLES_AMD, &iValue); m_aiMaxSamplesEQAA[1] = MAX(iValue, 0);
            iValue = 0; glGetIntegerv(GL_MAX_COLOR_FRAMEBUFFER_STORAGE_SAMPLES_AMD, &iValue); m_aiMaxSamplesEQAA[2] = MAX(iValue, 0);
        }

        // handle Nvidia extension (CSAA)
        if(CORE_GL_SUPPORT(NV_framebuffer_multisample_coverage))
        {
            m_iMaxSamples = MIN(m_iMaxSamples, 16u);

            iValue = 0; glGetIntegerv(GL_MAX_MULTISAMPLE_COVERAGE_MODES_NV, &iValue);
            if(iValue > 0)
            {
                coreDataScope<corePoint2I32> pModeList = new corePoint2I32[iValue];

                glGetIntegerv(GL_MULTISAMPLE_COVERAGE_MODES_NV, r_cast<GLint*>(pModeList.Get()));
                std::sort(pModeList.Get(), pModeList.Get() + iValue, std::greater());

                m_aiMaxSamplesCSAA[0] = MAX(pModeList[0][0], 0);
                m_aiMaxSamplesCSAA[1] = MAX(pModeList[0][1], 0);
            }
        }
    }

    // get max texture filter level
    if(CORE_GL_SUPPORT(ARB_texture_filter_anisotropic))
    {
        GLfloat fValue = 0.0f; glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &fValue);
        m_iMaxAnisotropy = F_TO_UI(MAX(fValue, 1.0f));
    }

    // get max number of texture units (only for fragment shader)
    GLint iValue = 0; glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &iValue);
    m_iMaxTextures = MAX(iValue, 8);

    // log graphics device information
    Core::Log->ListStartInfo("Graphics Device Information");
    {
        coreUint64 iMemoryTotal;
        this->SystemGpuMemory(&m_iMemoryStart, &iMemoryTotal);

        const coreUint64 iMemoryUsed = iMemoryTotal - m_iMemoryStart;
        const coreDouble dMemoryPct  = 100.0 * (coreDouble(iMemoryUsed) / coreDouble(MAX(iMemoryTotal, 1u)));

        coreString sExtensions;
        coreExtensions(&sExtensions);

        coreString sPlatformExtensions;
        corePlatformExtensions(&sPlatformExtensions);

        GLint aiStatus[2] = {};
        glGetIntegerv(GL_CONTEXT_FLAGS,        &aiStatus[0]);
        glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &aiStatus[1]);

        Core::Log->ListAdd(CORE_LOG_BOLD("Vendor:")          " %s",                    glGetString(GL_VENDOR));
        Core::Log->ListAdd(CORE_LOG_BOLD("Renderer:")        " %s",                    glGetString(GL_RENDERER));
        Core::Log->ListAdd(CORE_LOG_BOLD("OpenGL Version:")  " %s",                    glGetString(GL_VERSION));
        Core::Log->ListAdd(CORE_LOG_BOLD("Shader Version:")  " %s",                    glGetString(GL_SHADING_LANGUAGE_VERSION));
        Core::Log->ListAdd(CORE_LOG_BOLD("Graphics Memory:") " %llu/%llu MB (%.1f%%)", iMemoryUsed / (1024u * 1024u), iMemoryTotal / (1024u * 1024u), dMemoryPct);
        Core::Log->ListAdd(sExtensions        .c_str());
        Core::Log->ListAdd(sPlatformExtensions.c_str());
        Core::Log->ListAdd("GL_MAX_SAMPLES (%u) GL_MAX_TEXTURE_MAX_ANISOTROPY (%u) GL_MAX_TEXTURE_IMAGE_UNITS (%u) GL_CONTEXT_FLAGS (0x%02X) GL_CONTEXT_PROFILE_MASK (0x%02X)", m_iMaxSamples, m_iMaxAnisotropy, m_iMaxTextures, aiStatus[0], aiStatus[1]);
    }
    Core::Log->ListEnd();

    // enable vertical synchronization
    const coreInt32 iVsync = Core::Config->GetInt(CORE_CONFIG_SYSTEM_VSYNC);
         if(SDL_GL_SetSwapInterval(iVsync)) Core::Log->Info("Vertical synchronization configured (interval %d)", iVsync);
    else if(SDL_GL_SetSwapInterval(1))      Core::Log->Info("Vertical synchronization configured (default)");
    else Core::Log->Warning("Vertical synchronization not directly supported (SDL: %s)", SDL_GetError());

    // setup texturing and packing
    if(CORE_GL_SUPPORT(CORE_gl2_compatibility) || DEFINED(_CORE_GLES_)) glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
    if(CORE_GL_SUPPORT(ARB_seamless_cube_map))                          glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);
    glPixelStorei(GL_PACK_ALIGNMENT,   4);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    // setup depth testing
    if(CORE_GL_SUPPORT(ARB_depth_clamp) && !DEFINED(_CORE_DEBUG_)) glEnable(GL_DEPTH_CLAMP);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(true);
    glClearDepth(1.0f);

    // setup stencil testing
    glDisable(GL_STENCIL_TEST);
    glStencilMask(0xFFu);
    glClearStencil(0);

    // setup culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // setup blending
    glEnable(GL_BLEND);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    // setup shading and rasterization
    if(CORE_GL_SUPPORT(CORE_gl2_compatibility))     glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    if(CORE_GL_SUPPORT(NV_multisample_filter_hint)) glHint(GL_MULTISAMPLE_FILTER_HINT_NV,  GL_NICEST);
    if(CORE_GL_SUPPORT(ARB_multisample))            glEnable(GL_MULTISAMPLE);
    if(CORE_GL_SUPPORT(ARB_framebuffer_sRGB))       glDisable(GL_FRAMEBUFFER_SRGB);
    glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);
    glDisable(GL_DITHER);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(true, true, true, true);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // enable maximum sample shading rate
    if(CORE_GL_SUPPORT(ARB_sample_shading))
    {
        glMinSampleShading(1.0f);
    }

    // enable parallel shader compilation
    if(CORE_GL_SUPPORT(ARB_parallel_shader_compile))
    {
        glMaxShaderCompilerThreadsARB(0xFFFFFFFFu);
    }

    // create uniform buffer objects
    m_Transform3DBuffer.Create(CORE_SHADER_BUFFER_TRANSFORM3D_NUM, CORE_GRAPHICS_UNIFORM_BUFFERS, CORE_GRAPHICS_UNIFORM_TRANSFORM3D_SIZE);
    m_Transform2DBuffer.Create(CORE_SHADER_BUFFER_TRANSFORM2D_NUM, CORE_GRAPHICS_UNIFORM_BUFFERS, CORE_GRAPHICS_UNIFORM_TRANSFORM2D_SIZE);
    m_AmbientBuffer    .Create(CORE_SHADER_BUFFER_AMBIENT_NUM,     CORE_GRAPHICS_UNIFORM_BUFFERS, CORE_GRAPHICS_UNIFORM_AMBIENT_SIZE);

    // reset camera and view frustum
    this->SetCamera(coreVector3(0.0f,0.0f,0.0f), coreVector3(0.0f,0.0f,-1.0f), coreVector3(0.0f,1.0f,0.0f));
    this->SetView  (coreVector2(0.0f,0.0f), PI*0.25f, 1.0f, 1000.0f, 0.0f);

    // reset ambient light
    for(coreUintW i = 0u; i < CORE_GRAPHICS_LIGHTS; ++i)
    {
        this->SetLight(i, coreVector4(0.0f,0.0f,0.0f,0.0f), coreVector4(0.0f,0.0f,-1.0f,1.0f), coreVector4(1.0f,1.0f,1.0f,1.0f));
    }

    // reset scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | (CoreApp::Settings::Graphics::StencilSize ? GL_STENCIL_BUFFER_BIT : 0u));
    SDL_GL_SwapWindow(Core::System->GetWindow());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | (CoreApp::Settings::Graphics::StencilSize ? GL_STENCIL_BUFFER_BIT : 0u));

    if(Core::Config->GetBool(CORE_CONFIG_BASE_ASYNCMODE) && !DEFINED(_CORE_EMSCRIPTEN_) && (SDL_GetNumLogicalCPUCores() > 1) && CORE_GL_SUPPORT(CORE_shared_context))
    {
        // create resource context (after reset, because of flickering on Windows with fullscreen)
        m_pResourceContext = SDL_GL_CreateContext(Core::System->GetWindow());
        if(!m_pResourceContext) Core::Log->Warning("Resource context could not be created (SDL: %s)", SDL_GetError());
                           else Core::Log->Info   ("Resource context created");

        // re-assign render context to main window
        SDL_GL_MakeCurrent(Core::System->GetWindow(), m_pRenderContext);
    }

    // load shader-cache
    coreProgram::LoadShaderCache();
}


// ****************************************************************
/* destructor */
CoreGraphics::~CoreGraphics()
{
    // save and clear shader-cache
    coreProgram::SaveShaderCache();
    coreProgram::ClearShaderCache();

    // delete uniform buffer objects
    m_Transform3DBuffer.Delete();
    m_Transform2DBuffer.Delete();
    m_AmbientBuffer    .Delete();

    // exit OpenGL
    coreExitOpenGL();

    // disable vertical synchronization
    SDL_GL_SetSwapInterval(0);   // # AMD hotfix: prevent old crash on engine reset

    // dissociate render context from main window
    SDL_GL_MakeCurrent(Core::System->GetWindow(), NULL);

    // delete OpenGL contexts
    SDL_GL_DestroyContext(m_pResourceContext);
    SDL_GL_DestroyContext(m_pRenderContext);

    Core::Log->Info(CORE_LOG_BOLD("Graphics Interface shut down"));
}


// ****************************************************************
/* set camera and create camera matrix */
void CoreGraphics::SetCamera(const coreVector3 vPosition, const coreVector3 vDirection, const coreVector3 vOrientation)
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
/* set view frustum and create projection matrices */
void CoreGraphics::SetView(coreVector2 vResolution, const coreFloat fFOV, const coreFloat fNearClip, const coreFloat fFarClip, coreFloat fAspectRatio)
{
    coreBool bNewView = false;

    // retrieve window resolution
    if(!vResolution.x) vResolution.x = Core::System->GetResolution().x;
    if(!vResolution.y) vResolution.y = Core::System->GetResolution().y;

    // retrieve canonical aspect ratio
    if(!fAspectRatio) fAspectRatio = coreFloat(CoreApp::Settings::System::AspectRatio);

    // set properties of the view frustum
    if(m_vViewResolution.xy() != vResolution)
    {
        // save viewport resolution
        m_vViewResolution.xy(vResolution);
        m_vViewResolution.zw(coreVector2(1.0f / vResolution.x, 1.0f / vResolution.y));   // # normal division

        // set viewport
        glViewport(0, 0, F_TO_SI(vResolution.x), F_TO_SI(vResolution.y));
        bNewView = true;
    }
    if(m_fFOV         != fFOV)         {m_fFOV         = fFOV;         bNewView = true;}
    if(m_fNearClip    != fNearClip)    {m_fNearClip    = fNearClip;    bNewView = true;}
    if(m_fFarClip     != fFarClip)     {m_fFarClip     = fFarClip;     bNewView = true;}
    if(m_fAspectRatio != fAspectRatio) {m_fAspectRatio = fAspectRatio; bNewView = true;}

    if(bNewView)
    {
        // create projection matrices
        m_mPerspective = coreMatrix4::Perspective(vResolution, m_fFOV, m_fNearClip, m_fFarClip, m_fAspectRatio);
        m_mOrtho       = coreMatrix4::Ortho      (vResolution);

        // invoke transformation data update
        ADD_BIT(m_iUniformUpdate, 0u)
    }
}


// ****************************************************************
/* set and update ambient light */
void CoreGraphics::SetLight(const coreUintW iIndex, const coreVector4 vPosition, const coreVector4 vDirection, const coreVector4 vValue)
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
/* update transformation data for the uniform buffer objects */
void CoreGraphics::UpdateTransformation()
{
    // check update status
    if(!HAS_BIT(m_iUniformUpdate, 0u)) return;
    REMOVE_BIT(m_iUniformUpdate, 0u)

    if(CORE_GL_SUPPORT(ARB_uniform_buffer_object))
    {
        const coreMatrix4 mViewProj = m_mCamera * m_mPerspective;

        // map buffer range
        coreByte* pRange3D = m_Transform3DBuffer.MapWriteNext();

        // update 3d-transformation data
        std::memcpy(pRange3D,        &mViewProj,      sizeof(coreMatrix4));
        std::memcpy(pRange3D + 64u,  &m_mCamera,      sizeof(coreMatrix4));
        std::memcpy(pRange3D + 128u, &m_mPerspective, sizeof(coreMatrix4));
        std::memcpy(pRange3D + 192u, &m_vCamPosition, sizeof(coreVector3));
        m_Transform3DBuffer.Unmap();

        // map buffer range
        coreByte* pRange2D = m_Transform2DBuffer.MapWriteNext();

        // update 2d-transformation data
        std::memcpy(pRange2D,       &m_mOrtho,          sizeof(coreMatrix4));
        std::memcpy(pRange2D + 64u, &m_vViewResolution, sizeof(coreVector4));
        m_Transform2DBuffer.Unmap();
    }
    else
    {
        // invoke manual data forwarding
        coreProgram::Disable(false);
    }
}


// ****************************************************************
/* update ambient data for the uniform buffer objects */
void CoreGraphics::UpdateAmbient()
{
    // check update status
    if(!HAS_BIT(m_iUniformUpdate, 1u)) return;
    REMOVE_BIT(m_iUniformUpdate, 1u)

    if(CORE_GL_SUPPORT(ARB_uniform_buffer_object))
    {
        // map buffer range
        coreByte* pRange = m_AmbientBuffer.MapWriteNext();

        // update ambient data
        std::memcpy(pRange, m_aLight, CORE_GRAPHICS_UNIFORM_AMBIENT_SIZE);
        m_AmbientBuffer.Unmap();
    }
    else
    {
        // invoke manual data forwarding
        coreProgram::Disable(false);
    }
}


// ****************************************************************
/* handle stencil testing */
void CoreGraphics::WriteStencilTest(const GLenum iBackZFail, const GLenum iBackZPass, const GLenum iFrontZFail, const GLenum iFrontZPass)
{
    // reset test function
    glStencilFunc(GL_ALWAYS, 0, 0xFFu);

    // set change operation
    if((iBackZFail == iFrontZFail) && (iBackZPass == iFrontZPass))
    {
        glStencilOp(GL_KEEP, iBackZFail, iBackZPass);
    }
    else
    {
        glStencilOpSeparate(GL_BACK,  GL_KEEP, iBackZFail,  iBackZPass);
        glStencilOpSeparate(GL_FRONT, GL_KEEP, iFrontZFail, iFrontZPass);
    }

    // enable stencil testing
    glEnable(GL_STENCIL_TEST);
}

void CoreGraphics::WriteStencilTest(const GLenum iZFail, const GLenum iZPass)
{
    this->WriteStencilTest(iZFail, iZPass, iZFail, iZPass);
}

void CoreGraphics::ReadStencilTest(const GLenum iBackFunc, const coreUint8 iBackRef, const coreUint8 iBackMask, const GLenum iFrontFunc, const coreUint8 iFrontRef, const coreUint8 iFrontMask)
{
    // set test function
    if((iBackFunc == iFrontFunc) && (iBackRef == iFrontRef) && (iBackMask == iFrontMask))
    {
        glStencilFunc(iBackFunc, iBackRef, iBackMask);
    }
    else
    {
        glStencilFuncSeparate(GL_BACK,  iBackFunc,  iBackRef,  iBackMask);
        glStencilFuncSeparate(GL_FRONT, iFrontFunc, iFrontRef, iFrontMask);
    }

    // reset change operation
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    // enable stencil testing
    glEnable(GL_STENCIL_TEST);
}

void CoreGraphics::ReadStencilTest(const GLenum iFunc, const coreUint8 iRef, const coreUint8 iMask)
{
    this->ReadStencilTest(iFunc, iRef, iMask, iFunc, iRef, iMask);
}

void CoreGraphics::EndStencilTest()
{
    // disable stencil testing
    glDisable(GL_STENCIL_TEST);
}


// ****************************************************************
/* handle scissor testing */
void CoreGraphics::StartScissorTest(const coreVector2 vLowerLeft, const coreVector2 vUpperRight)
{
    ASSERT(vLowerLeft <= vUpperRight)

    // convert center coordinates to window coordinates
    const coreVector2 vRealLowerLeft  = (vLowerLeft  + 0.5f).Processed(CLAMP, 0.0f, 1.0f);
    const coreVector2 vRealUpperRight = (vUpperRight + 0.5f).Processed(CLAMP, 0.0f, 1.0f);
    const coreVector2 vRealSize       = vRealUpperRight - vRealLowerLeft;

    // calculate scissor properties
    const coreUint32 iNewScissorData[] =
    {
        F_TO_UI(ROUND(vRealLowerLeft.x * m_vViewResolution.x)),
        F_TO_UI(ROUND(vRealLowerLeft.y * m_vViewResolution.y)),
        F_TO_UI(ROUND(vRealSize     .x * m_vViewResolution.x)),
        F_TO_UI(ROUND(vRealSize     .y * m_vViewResolution.y))
    };
    STATIC_ASSERT(sizeof(m_aiScissorData) == sizeof(iNewScissorData))

    // update scissor properties
    if(std::memcmp(m_aiScissorData, iNewScissorData, sizeof(m_aiScissorData)))
    {
        std::memcpy(m_aiScissorData, iNewScissorData, sizeof(m_aiScissorData));
        glScissor(iNewScissorData[0], iNewScissorData[1], iNewScissorData[2], iNewScissorData[3]);
    }

    // enable scissor testing
    glEnable(GL_SCISSOR_TEST);
}

void CoreGraphics::EndScissorTest()
{
    // disable scissor testing
    glDisable(GL_SCISSOR_TEST);
}


// ****************************************************************
/* handle conservative rasterization */
coreStatus CoreGraphics::StartConservativeRaster()
{
    if(CORE_GL_SUPPORT(INTEL_conservative_rasterization))
    {
        // enable Intel extension
        glEnable(GL_CONSERVATIVE_RASTERIZATION_INTEL);
        return CORE_OK;
    }
    else if(CORE_GL_SUPPORT(NV_conservative_raster))
    {
        // enable Nvidia extension
        glEnable(GL_CONSERVATIVE_RASTERIZATION_NV);
        return CORE_OK;
    }

    return CORE_ERROR_SUPPORT;
}

void CoreGraphics::EndConservativeRaster()
{
    if(CORE_GL_SUPPORT(INTEL_conservative_rasterization))
    {
        // disable Intel extension
        glDisable(GL_CONSERVATIVE_RASTERIZATION_INTEL);
    }
    else if(CORE_GL_SUPPORT(NV_conservative_raster))
    {
        // disable Nvidia extension
        glDisable(GL_CONSERVATIVE_RASTERIZATION_NV);
    }
}


// ****************************************************************
/* write OpenGL debug message */
void GL_APIENTRY WriteOpenGL(const GLenum iSource, const GLenum iType, const GLuint iID, const GLenum iSeverity, const GLsizei iLength, const GLchar* pcMessage, const void* pUserParam)
{
    // write debug message
    Core::Log->ListStartWarning("OpenGL Debug Message");
    {
        Core::Log->ListAdd(CORE_LOG_BOLD("ID:")           " %d", iID);
        Core::Log->ListAdd(CORE_LOG_BOLD("Source:")   " 0x%04X", iSource);
        Core::Log->ListAdd(CORE_LOG_BOLD("Type:")     " 0x%04X", iType);
        Core::Log->ListAdd(CORE_LOG_BOLD("Severity:") " 0x%04X", iSeverity);
        Core::Log->ListAdd(pcMessage);
    }
    Core::Log->ListEnd();

    // also display shader compiler message
    if(iSource == GL_DEBUG_SOURCE_SHADER_COMPILER)
    {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Shader Compiler", pcMessage, NULL);
    }

    WARN_IF(true) {}
}


// ****************************************************************
/* enable OpenGL debug output */
void CoreGraphics::DebugOpenGL()
{
    if(!Core::Debug->IsEnabled()) return;

    if(CORE_GL_SUPPORT(KHR_debug))
    {
        // enable synchronous debug output
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        // set callback function and enable all messages
        glDebugMessageCallback(&WriteOpenGL, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);

        // disable certain API messages
        constexpr GLuint aiID[] = {131154u, 131169u, 131185u, 131204u, 131222u};
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, GL_DONT_CARE, ARRAY_SIZE(aiID), aiID, false);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_PERFORMANCE,        GL_DONT_CARE, ARRAY_SIZE(aiID), aiID, false);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER,              GL_DONT_CARE, ARRAY_SIZE(aiID), aiID, false);

        // disable all shader compiler diagnostics and warnings
        glDebugMessageControl(GL_DEBUG_SOURCE_SHADER_COMPILER, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 0, NULL, false);
    }

    // 131154: Pixel-path performance warning: Pixel transfer is synchronized with 3D rendering.
    // 131169: Framebuffer detailed info: The driver allocated multisample storage for renderbuffer #.
    // 131185: Buffer detailed info: Buffer object # (bound to #, usage hint is #) will use # memory as the source for buffer object operations.
    // 131204: Texture state usage warning: Texture # is base level inconsistent. Check texture size.
    // 131222: Program undefined behavior warning: Sampler object # is bound to non-depth texture #, yet it is used with a program that uses a shadow sampler. This is undefined behavior.

    // 1:      Shader Stats (SGPRs, VGPRs, Code Size, LDS, Scratch, Max Waves, Spilled SGPRs, Spilled VGPRs, PrivMem VGPRs)
    // 2:      LLVM Diagnostics (# instructions in function)
    // #:      extension # unsupported in # shader
}


// ****************************************************************
/* manually check for OpenGL errors */
void CoreGraphics::CheckOpenGL()
{
    if(!Core::Debug->IsEnabled()) return;

    // loop through all recent errors
    GLenum iError;
    while((iError = glGetError()) != GL_NO_ERROR)
    {
        Core::Log->Warning("OpenGL reported an error (GL Error Code: 0x%08X)", iError);
        WARN_IF(true) {}
    }
}


// ****************************************************************
/* label OpenGL object for debug tooling */
void CoreGraphics::LabelOpenGL(const GLenum iType, const GLuint iIdentifier, const coreChar* pcLabel)
{
    if(!Core::Debug->IsEnabled()) return;

    if(CORE_GL_SUPPORT(KHR_debug))
    {
        // assign string to identifier
        if(pcLabel && pcLabel[0]) glObjectLabel(iType, iIdentifier, -1, pcLabel);
                             else glObjectLabel(iType, iIdentifier,  0, NULL);
    }
}


// ****************************************************************
/* get amount of graphics memory assigned to the application (approximation) */
coreUint64 CoreGraphics::ProcessGpuMemory()const
{
    if(CORE_GL_SUPPORT(NVX_gpu_memory_info))
    {
        GLint iAvailable;

        // retrieve GPU memory info (in KB)
        glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &iAvailable);

        // convert to bytes and return
        return (m_iMemoryStart - coreUint64(iAvailable) * 1024u);
    }

    return 0u;
}


// ****************************************************************
/* get dedicated graphics memory */
coreBool CoreGraphics::SystemGpuMemory(coreUint64* OUTPUT piAvailable, coreUint64* OUTPUT piTotal)const
{
    if(CORE_GL_SUPPORT(NVX_gpu_memory_info))
    {
        GLint iAvailable, iTotal;

        // retrieve GPU memory info (in KB)
        glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &iAvailable);
        glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX,         &iTotal);

        // convert to bytes and return
        if(piAvailable) (*piAvailable) = coreUint64(iAvailable) * 1024u;
        if(piTotal)     (*piTotal)     = coreUint64(iTotal)     * 1024u;
        return true;
    }

    // could not get dedicated graphics memory
    if(piAvailable) (*piAvailable) = 0u;
    if(piTotal)     (*piTotal)     = 0u;
    return false;
}


// ****************************************************************
/* get GPU vendor identifier */
const coreGpuType& CoreGraphics::SystemGpuType()const
{
    static const coreGpuType s_eGpuType = []()
    {
        // retrieve vendor string
        const coreChar* pcVendor = coreData::StrToLower(PRINT("%s", glGetString(GL_VENDOR)));   // to handle NULL

        // determine GPU vendor
        if(std::strstr(pcVendor, "amd"))    return CORE_GPU_TYPE_AMD;
        if(std::strstr(pcVendor, "ati"))    return CORE_GPU_TYPE_AMD;
        if(std::strstr(pcVendor, "nvidia")) return CORE_GPU_TYPE_NVIDIA;
        if(std::strstr(pcVendor, "intel"))  return CORE_GPU_TYPE_INTEL;
        if(std::strstr(pcVendor, "apple"))  return CORE_GPU_TYPE_APPLE;

        WARN_IF(true) {}
        return CORE_GPU_TYPE_UNKNOWN;
    }();

    return s_eGpuType;
}


// ****************************************************************
/* take screenshot */
void CoreGraphics::TakeScreenshot(const coreChar* pcPath)const
{
#if !defined(_CORE_EMSCRIPTEN_)

    const coreUintW iWidthSrc = coreMath::CeilAlign(F_TO_UI(m_vViewResolution.x), 4u);
    const coreUintW iWidthDst = F_TO_UI(m_vViewResolution.x);
    const coreUintW iHeight   = F_TO_UI(m_vViewResolution.y);
    const coreUintW iPitchSrc = iWidthSrc * 3u;
    const coreUintW iPitchDst = iWidthDst * 3u;
    const coreUintW iSize     = iHeight * iPitchSrc;
    ASSERT(iPitchDst <= iPitchSrc)

    // read pixel data from the frame buffer
    coreByte* pData = new coreByte[iSize * 2u];
    if(CORE_GL_SUPPORT(ARB_robustness)) glReadnPixels(0, 0, iWidthSrc, iHeight, GL_RGB, GL_UNSIGNED_BYTE, iSize, pData);
                                   else glReadPixels (0, 0, iWidthSrc, iHeight, GL_RGB, GL_UNSIGNED_BYTE,        pData);

    // copy path into lambda
    coreString sPathCopy = pcPath;
    ASSERT(!sPathCopy.empty())

    Core::Manager::Resource->AttachFunction([=, sPathCopy = std::move(sPathCopy)]()
    {
        // flip pixel data vertically
        coreByte* pConvert = pData + iSize;
        for(coreUintW i = 0u; i < iHeight; ++i)
        {
            std::memcpy(pConvert + (iHeight - i - 1u) * iPitchDst, pData + i * iPitchSrc, iPitchDst);
        }

        // create SDL surface
        coreSurfaceScope pSurface = SDL_CreateSurfaceFrom(iWidthDst, iHeight, SDL_PIXELFORMAT_RGB24, pConvert, iPitchDst);
        if(pSurface)
        {
            const coreChar* pcFullPath = std::strcmp(coreData::StrExtension(sPathCopy.c_str()), "png") ? PRINT("%s.png", sPathCopy.c_str()) : sPathCopy.c_str();

            // create directory hierarchy
            coreData::DirectoryCreate(coreData::StrDirectory(pcFullPath));

            // save the surface as PNG image
            IMG_SavePNG(pSurface, pcFullPath);
        }

        // delete pixel data
        delete[] pData;
        return CORE_OK;
    });

#endif
}


// ****************************************************************
/* update the graphics scene */
void CoreGraphics::__UpdateScene()
{
    // take screenshot
    if(Core::Input->GetKeyboardButton(CORE_INPUT_KEY(PRINTSCREEN), CORE_INPUT_PRESS))
        this->TakeScreenshot();

    // check for OpenGL errors
    this->CheckOpenGL();

    // disable last model, textures and shader-program
    coreModel  ::Disable(true);
    coreTexture::DisableAll();
    coreProgram::Disable(true);

#if defined(_CORE_ANGLE_)

    // # Steam hotfix: overlay injection invalidates UBO content
    ADD_BIT(m_iUniformUpdate, 0u)
    ADD_BIT(m_iUniformUpdate, 1u)

#endif

    // explicitly invalidate depth and stencil buffer
    if(CORE_GL_SUPPORT(ARB_invalidate_subdata))
    {
        constexpr GLenum aiAttachment[] = {GL_DEPTH, GL_STENCIL};
        glInvalidateFramebuffer(GL_FRAMEBUFFER, ARRAY_SIZE(aiAttachment), aiAttachment);
    }

    // explicitly flush command buffers
    if(!CoreApp::Settings::Graphics::DoubleBuffer)
    {
        coreSync::Flush();
    }

    // swap color buffers
    SDL_GL_SwapWindow(Core::System->GetWindow());

    // measure overall performance
    Core::Debug->MeasureEnd  (CORE_DEBUG_OVERALL_NAME);
    Core::Debug->MeasureStart(CORE_DEBUG_OVERALL_NAME);

    // reset engine on OpenGL context loss
    if(CORE_GL_SUPPORT(ARB_robustness))
    {
        if(glGetGraphicsResetStatus() != GL_NO_ERROR) Core::Reset();
    }

#if !defined(_CORE_EMSCRIPTEN_)

    // clear color, depth and stencil buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | (CoreApp::Settings::Graphics::StencilSize ? GL_STENCIL_BUFFER_BIT : 0u));

#endif
}


// ****************************************************************
/* update the Emscripten canvas */
void CoreGraphics::__UpdateEmscripten()
{
#if defined(_CORE_EMSCRIPTEN_)

    // clear color, depth and stencil buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | (CoreApp::Settings::Graphics::StencilSize ? GL_STENCIL_BUFFER_BIT : 0u));

#endif
}