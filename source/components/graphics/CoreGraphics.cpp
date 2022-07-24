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
, m_aLight           {}
, m_TransformBuffer  ()
, m_AmbientBuffer    ()
, m_aTransformSync   {}
, m_aAmbientSync     {}
, m_iUniformUpdate   (0u)
, m_aiScissorData    {}
, m_iMemoryStart     (0u)
, m_iMaxSamples      (0u)
, m_iMaxAnisotropy   (0u)
, m_iMaxTextures     (0u)
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
    this->DebugOpenGL();

    // save version numbers
    m_fVersionOpenGL = coreData::StrVersion(r_cast<const coreChar*>(glGetString(GL_VERSION)));
    m_fVersionGLSL   = coreData::StrVersion(r_cast<const coreChar*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

    // get max anti aliasing level
    if(CORE_GL_SUPPORT(EXT_framebuffer_multisample))
    {
        GLint iValue = 0; glGetIntegerv(GL_MAX_SAMPLES, &iValue);
        m_iMaxSamples = MAX(iValue, 0);
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
    if(Core::Config->GetBool(CORE_CONFIG_SYSTEM_VSYNC))
    {
             if(!SDL_GL_SetSwapInterval(-1)) Core::Log->Info("Vertical synchronization enabled (adaptive)");
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
    if(CORE_GL_SUPPORT(V2_compatibility))           glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
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
        glMinSampleShading(1.0f);

    // enable parallel shader compilation
    if(CORE_GL_SUPPORT(ARB_parallel_shader_compile))
        glMaxShaderCompilerThreadsARB(0xFFFFFFFFu);

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
    {
        this->SetLight(i, coreVector4(0.0f,0.0f,0.0f,0.0f), coreVector4(0.0f,0.0f,-1.0f,1.0f), coreVector4(1.0f,1.0f,1.0f,1.0f));
    }

    // reset scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    SDL_GL_SwapWindow(Core::System->GetWindow());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if(Core::Config->GetBool(CORE_CONFIG_BASE_ASYNCMODE) && !DEFINED(_CORE_EMSCRIPTEN_))
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
/* destructor */
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

    if(m_TransformBuffer.IsValid())
    {
        const coreMatrix4 mViewProj = m_mCamera * m_mPerspective;

        // invalidate previous buffer range
        if(CORE_GL_SUPPORT(ARB_invalidate_subdata))
        {
            const coreUint32 iOldOffset = m_aTransformSync.index() * coreMath::CeilAlign(CORE_GRAPHICS_UNIFORM_TRANSFORM_SIZE, 256u);
            glInvalidateBufferSubData(m_TransformBuffer.GetIdentifier(), iOldOffset, coreMath::CeilAlign(CORE_GRAPHICS_UNIFORM_TRANSFORM_SIZE, 256u));
        }

        // synchronize and switch to next sync object
        m_aTransformSync.current().Create();
        m_aTransformSync.next();
        m_aTransformSync.current().Check(CORE_SYNC_WAIT_FOREVER, CORE_SYNC_CHECK_NORMAL);

        // bind next buffer range
        const coreUint32 iNewOffset = m_aTransformSync.index() * coreMath::CeilAlign(CORE_GRAPHICS_UNIFORM_TRANSFORM_SIZE, 256u);
        glBindBufferRange(GL_UNIFORM_BUFFER, CORE_SHADER_BUFFER_TRANSFORM_NUM, m_TransformBuffer.GetIdentifier(), iNewOffset, coreMath::CeilAlign(CORE_GRAPHICS_UNIFORM_TRANSFORM_SIZE, 16u));

        // map required area of the UBO
        coreByte* pRange = m_TransformBuffer.Map(iNewOffset, CORE_GRAPHICS_UNIFORM_TRANSFORM_SIZE, CORE_DATABUFFER_MAP_UNSYNCHRONIZED);

        // update transformation data
        std::memcpy(pRange,        &mViewProj,         sizeof(coreMatrix4));
        std::memcpy(pRange + 64u,  &m_mCamera,         sizeof(coreMatrix4));
        std::memcpy(pRange + 128u, &m_mPerspective,    sizeof(coreMatrix4));
        std::memcpy(pRange + 192u, &m_mOrtho,          sizeof(coreMatrix4));
        std::memcpy(pRange + 256u, &m_vViewResolution, sizeof(coreVector4));
        std::memcpy(pRange + 272u, &m_vCamPosition,    sizeof(coreVector3));
        m_TransformBuffer.Unmap();
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

    if(m_AmbientBuffer.IsValid())
    {
        // invalidate previous buffer range
        if(CORE_GL_SUPPORT(ARB_invalidate_subdata))
        {
            const coreUint32 iOldOffset = m_aAmbientSync.index() * coreMath::CeilAlign(CORE_GRAPHICS_UNIFORM_AMBIENT_SIZE, 256u);
            glInvalidateBufferSubData(m_AmbientBuffer.GetIdentifier(), iOldOffset, coreMath::CeilAlign(CORE_GRAPHICS_UNIFORM_AMBIENT_SIZE, 256u));
        }

        // synchronize and switch to next sync object
        m_aAmbientSync.current().Create();
        m_aAmbientSync.next();
        m_aAmbientSync.current().Check(CORE_SYNC_WAIT_FOREVER, CORE_SYNC_CHECK_NORMAL);

        // bind next buffer range
        const coreUint32 iNewOffset = m_aAmbientSync.index() * coreMath::CeilAlign(CORE_GRAPHICS_UNIFORM_AMBIENT_SIZE, 256u);
        glBindBufferRange(GL_UNIFORM_BUFFER, CORE_SHADER_BUFFER_AMBIENT_NUM, m_AmbientBuffer.GetIdentifier(), iNewOffset, coreMath::CeilAlign(CORE_GRAPHICS_UNIFORM_AMBIENT_SIZE, 16u));

        // map required area of the UBO
        coreByte* pRange = m_AmbientBuffer.Map(iNewOffset, CORE_GRAPHICS_UNIFORM_AMBIENT_SIZE, CORE_DATABUFFER_MAP_UNSYNCHRONIZED);

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

#if defined(_CORE_DEBUG_)

    // also show message box
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "OpenGL Error", pcMessage, NULL);
    WARN_IF(true) {}

#endif
}


// ****************************************************************
/* enable OpenGL debug output */
void CoreGraphics::DebugOpenGL()
{
    if(!Core::Config->GetBool(CORE_CONFIG_BASE_DEBUGMODE) && !DEFINED(_CORE_DEBUG_)) return;

    if(CORE_GL_SUPPORT(KHR_debug))
    {
        // enable synchronous debug output
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

        // set callback function and enable all messages
        glDebugMessageCallback(&WriteOpenGL, NULL);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true);

        // disable certain API messages
        constexpr GLuint aiID[] = {131169u, 131185u, 131204u, 131222u};
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER,              GL_DONT_CARE, ARRAY_SIZE(aiID), aiID, false);
        glDebugMessageControl(GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, GL_DONT_CARE, ARRAY_SIZE(aiID), aiID, false);

        // disable all shader compiler messages
        glDebugMessageControl(GL_DEBUG_SOURCE_SHADER_COMPILER, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 0, NULL, false);
    }

    // 131169: Framebuffer detailed info: The driver allocated multisample storage for renderbuffer #.
    // 131185: Buffer detailed info: Buffer object # (bound to #, usage hint is #) will use # memory as the source for buffer object operations.
    // 131204: Texture state usage warning: Texture # is base level inconsistent. Check texture size.
    // 131222: Program undefined behavior warning: Sampler object # is bound to non-depth texture #, yet it is used with a program that uses a shadow sampler. This is undefined behavior.

    // 1: Shader Stats (SGPRs, VGPRs, Code Size, LDS, Scratch, Max Waves, Spilled SGPRs, Spilled VGPRs, PrivMem VGPRs)
    // 2: LLVM Diagnostics (# instructions in function)
    // #: extension # unsupported in # shader
}


// ****************************************************************
/* manually check for OpenGL errors */
void CoreGraphics::CheckOpenGL()
{
    if(!Core::Config->GetBool(CORE_CONFIG_BASE_DEBUGMODE) && !DEFINED(_CORE_DEBUG_)) return;

    // loop through all recent errors
    GLenum iError;
    while((iError = glGetError()) != GL_NO_ERROR)
    {
        Core::Log->Warning("OpenGL reported an error (GL Error Code: 0x%08X)", iError);
        WARN_IF(true) {}
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
    if(piTotal)     (*piTotal)     = 1u;
    return false;
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
    glReadPixels(0, 0, iWidthSrc, iHeight, GL_RGB, GL_UNSIGNED_BYTE, pData);

    // copy path into another thread
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
        coreSurfaceScope pSurface = SDL_CreateRGBSurfaceFrom(pConvert, iWidthDst, iHeight, 24, iPitchDst, CORE_TEXTURE_MASK);
        if(pSurface)
        {
            const coreChar* pcFullPath = std::strcmp(coreData::StrExtension(sPathCopy.c_str()), "png") ? PRINT("%s.png", sPathCopy.c_str()) : sPathCopy.c_str();

            // create folder hierarchy
            coreData::FolderCreate(pcFullPath);

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

    // explicitly invalidate depth and stencil buffer
    if(CORE_GL_SUPPORT(ARB_invalidate_subdata))
    {
        constexpr GLenum aiAttachment[] = {GL_DEPTH, GL_STENCIL};
        glInvalidateFramebuffer(GL_FRAMEBUFFER, ARRAY_SIZE(aiAttachment), aiAttachment);
    }

    // swap color buffers
    Core::Debug->MeasureEnd  (CORE_DEBUG_OVERALL_NAME);
    SDL_GL_SwapWindow(Core::System->GetWindow());
    Core::Debug->MeasureStart(CORE_DEBUG_OVERALL_NAME);

#if !defined(_CORE_EMSCRIPTEN_)

    // clear color, depth and stencil buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

#endif
}


// ****************************************************************
/* update the Emscripten canvas */
void CoreGraphics::__UpdateEmscripten()
{
#if defined(_CORE_EMSCRIPTEN_)

    // clear color, depth and stencil buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

#endif
}