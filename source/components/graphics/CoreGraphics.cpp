//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ****************************************************************
// constructor
CoreGraphics::CoreGraphics()noexcept
: m_vCamPosition    (coreVector3(0.0f,0.0f,0.0f))
, m_vCamDirection   (coreVector3(0.0f,0.0f,0.0f))
, m_vCamOrientation (coreVector3(0.0f,0.0f,0.0f))
, m_vViewResolution (coreVector4(0.0f,0.0f,0.0f,0.0f))
, m_iUniformUpdate  (0)
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
    m_fOpenGL = coreData::StrVersion(r_cast<const char*>(glGetString(GL_VERSION)));
    m_fGLSL   = coreData::StrVersion(r_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

    // log video card information
    Core::Log->ListStartInfo("Video Card Information");
    {
        Core::Log->ListAdd(CORE_LOG_BOLD("Vendor:")         " %s", glGetString(GL_VENDOR));
        Core::Log->ListAdd(CORE_LOG_BOLD("Renderer:")       " %s", glGetString(GL_RENDERER));
        Core::Log->ListAdd(CORE_LOG_BOLD("OpenGL Version:") " %s", glGetString(GL_VERSION));
        Core::Log->ListAdd(CORE_LOG_BOLD("Shader Version:") " %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
        Core::Log->ListAdd(r_cast<const char*>(glGetString(GL_EXTENSIONS)));
    }
    Core::Log->ListEnd();

    // enable vertical synchronization
    if(Core::Config->GetBool(CORE_CONFIG_SYSTEM_VSYNC))
    {
             if(!SDL_GL_SetSwapInterval(-1)) Core::Log->Info("Vertical synchronization enabled (extended)");
        else if(!SDL_GL_SetSwapInterval( 1)) Core::Log->Info("Vertical synchronization enabled (normal)");
        else Core::Log->Warning("Vertical synchronization not directly supported (SDL: %s)", SDL_GetError());
    }

    // setup texturing
    glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
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
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    // setup rasterization
    glEnable(GL_MULTISAMPLE);
    glDisable(GL_DITHER);
    glColorMask(true, true, true, true);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // create uniform buffer objects
    if(CORE_GL_SUPPORT(ARB_uniform_buffer_object))
    {
        FOR_EACH(it, *m_aiTransformBuffer.List()) it->Create(GL_UNIFORM_BUFFER, CORE_GRAPHICS_UNIFORM_TRANSFORM_SIZE, NULL, CORE_DATABUFFER_STORAGE_PERSISTENT | CORE_DATABUFFER_STORAGE_FENCED);
        FOR_EACH(it, *m_aiAmbientBuffer  .List()) it->Create(GL_UNIFORM_BUFFER, CORE_GRAPHICS_UNIFORM_AMBIENT_SIZE,   NULL, CORE_DATABUFFER_STORAGE_PERSISTENT | CORE_DATABUFFER_STORAGE_FENCED);
    }

    // reset camera and view
    this->SetCamera(coreVector3(0.0f,0.0f,0.0f), coreVector3(0.0f,0.0f,-1.0f), coreVector3(0.0f,1.0f,0.0f));
    this->SetView  (coreVector2(0.0f,0.0f), PI*0.25f, 0.1f, 1000.0f);

    // reset ambient
    for(coreByte i = 0; i < CORE_GRAPHICS_LIGHTS; ++i)
        this->SetLight(i, coreVector4(0.0f,0.0f,0.0f,0.0f), coreVector4(0.0f,0.0f,-1.0f,1.0f), coreVector4(1.0f,1.0f,1.0f,1.0f));

    // reset scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SDL_GL_SwapWindow(Core::System->GetWindow());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(Core::Config->GetBool(CORE_CONFIG_GRAPHICS_RESOURCECONTEXT))
    {
        // create resource context (after reset, because of flickering on Windows with fullscreen)
        m_pResourceContext = SDL_GL_CreateContext(Core::System->GetWindow());
        if(!m_pResourceContext) Core::Log->Warning("Resource context could not be created (SDL: %s)", SDL_GetError());
                           else Core::Log->Info   ("Resource context created");

        // re-assign render context to main window
        SDL_GL_MakeCurrent(Core::System->GetWindow(), m_pRenderContext);
    }
    else m_pResourceContext = NULL;
}


// ****************************************************************
// destructor
CoreGraphics::~CoreGraphics()
{
    // delete uniform buffer objects
    FOR_EACH(it, *m_aiTransformBuffer.List()) it->Delete();
    FOR_EACH(it, *m_aiAmbientBuffer  .List()) it->Delete();

    // dissociate render context from main window
    SDL_GL_MakeCurrent(Core::System->GetWindow(), NULL);

    // delete OpenGL contexts
    SDL_GL_DeleteContext(m_pResourceContext);
    SDL_GL_DeleteContext(m_pRenderContext);

    Core::Log->Info("Graphics Interface shut down");
}


// ****************************************************************
// set camera and create camera matrix
void CoreGraphics::SetCamera(const coreVector3& vPosition, const coreVector3& vDirection, const coreVector3& vOrientation)
{
    bool bNewCamera = false;

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
        BIT_SET(m_iUniformUpdate, 1)
    }
}


// ****************************************************************
// set view and create projection matrices
void CoreGraphics::SetView(coreVector2 vResolution, const float& fFOV, const float& fNearClip, const float& fFarClip)
{
    bool bNewView = false;

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
        BIT_SET(m_iUniformUpdate, 1)
    }
}


// ****************************************************************
// set and update ambient light
void CoreGraphics::SetLight(const coreByte& iID, const coreVector4& vPosition, const coreVector4& vDirection, const coreVector4& vValue)
{
    bool bNewLight = false;

    // get requested ambient light
    ASSERT(iID < CORE_GRAPHICS_LIGHTS)
    coreLight& CurLight = m_aLight[iID];

    // set properties of the ambient light
    ASSERT(vDirection.xyz().IsNormalized())
    if(CurLight.vPosition  != vPosition)  {CurLight.vPosition  = vPosition;  bNewLight = true;}
    if(CurLight.vDirection != vDirection) {CurLight.vDirection = vDirection; bNewLight = true;}
    if(CurLight.vValue     != vValue)     {CurLight.vValue     = vValue;     bNewLight = true;}

    if(bNewLight)
    {
        // invoke ambient data update
        BIT_SET(m_iUniformUpdate, 2)
    }
}


// ****************************************************************
// send transformation data to the uniform buffer objects
void CoreGraphics::SendTransformation()
{
    // check update status
    if(!(m_iUniformUpdate & BIT(1))) return;
    BIT_RESET(m_iUniformUpdate, 1)

    if(m_aiTransformBuffer[0])
    {
        const coreMatrix4 mViewProj = m_mCamera * m_mPerspective;

        // switch to next available buffer
        m_aiTransformBuffer.Next();
        glBindBufferBase(GL_UNIFORM_BUFFER, CORE_SHADER_BUFFER_TRANSFORM_NUM, m_aiTransformBuffer.Current());

        // map required area of the UBO
        coreByte* pRange = m_aiTransformBuffer.Current().Map<coreByte>(0, CORE_GRAPHICS_UNIFORM_TRANSFORM_SIZE, CORE_DATABUFFER_MAP_UNSYNCHRONIZED);

        // update transformation data
        std::memcpy(pRange,                         &mViewProj,         sizeof(coreMatrix4));
        std::memcpy(pRange + 1*sizeof(coreMatrix4), &m_mCamera,         sizeof(coreMatrix4));
        std::memcpy(pRange + 2*sizeof(coreMatrix4), &m_mPerspective,    sizeof(coreMatrix4));
        std::memcpy(pRange + 3*sizeof(coreMatrix4), &m_mOrtho,          sizeof(coreMatrix4));
        std::memcpy(pRange + 4*sizeof(coreMatrix4), &m_vViewResolution, sizeof(coreVector4));
        m_aiTransformBuffer.Current().Unmap(pRange);
    }
    else
    {
        // invoke manual data forwarding
        coreProgram::Disable(false);
    }
}


// ****************************************************************
// send ambient data to the uniform buffer objects
void CoreGraphics::SendAmbient()
{
    // check update status
    if(!(m_iUniformUpdate & BIT(2))) return;
    BIT_RESET(m_iUniformUpdate, 2)

    if(m_aiAmbientBuffer[0])
    {
        // switch to next available buffer
        m_aiAmbientBuffer.Next();
        glBindBufferBase(GL_UNIFORM_BUFFER, CORE_SHADER_BUFFER_AMBIENT_NUM, m_aiAmbientBuffer.Current());

        // map required area of the UBO
        coreByte* pRange = m_aiAmbientBuffer.Current().Map<coreByte>(0, CORE_GRAPHICS_UNIFORM_AMBIENT_SIZE, CORE_DATABUFFER_MAP_UNSYNCHRONIZED);

        // update ambient data
        std::memcpy(pRange, m_aLight, CORE_GRAPHICS_UNIFORM_AMBIENT_SIZE);
        m_aiAmbientBuffer.Current().Unmap(pRange);
    }
    else
    {
        // invoke manual data forwarding
        coreProgram::Disable(false);
    }
}


// ****************************************************************
// take screenshot
void CoreGraphics::Screenshot(const char* pcPath)const
{
    const int iWidth  = F_TO_SI(Core::System->GetResolution().x);
    const int iHeight = F_TO_SI(Core::System->GetResolution().y);
    const int iPitch  = iWidth*3;
    const int iSize   = iHeight*iPitch;

    // read pixel data from the frame buffer
    coreByte* pData = new coreByte[iSize*2];
    glReadPixels(0, 0, iWidth, iHeight, GL_RGB, GL_UNSIGNED_BYTE, pData);

    Core::Manager::Resource->AttachFunction([=]()
    {
        // flip pixel data vertically
        coreByte* pConvert = pData + iSize;
        for(int i = 0; i < iHeight; ++i)
            std::memcpy(pConvert + (iHeight-i-1)*iPitch, pData + i*iPitch, iPitch);

        // create SDL surface
        SDL_Surface* pSurface = SDL_CreateRGBSurfaceFrom(pConvert, iWidth, iHeight, 24, iPitch, CORE_TEXTURE_MASK);
        if(pSurface)
        {
            // create folder hierarchy
            const char* pcFullPath = PRINT(std::strcmp(coreData::StrExtension(pcPath), "png") ? "%s.png" : "%s", pcPath);
            coreData::CreateFolder(pcFullPath);

            // save the surface as PNG image
            IMG_SavePNG(pSurface, pcFullPath);
            SDL_FreeSurface(pSurface);
        }

        // delete pixel data
        delete pData;
        return CORE_OK;
    });
}


// ****************************************************************
// update the graphics scene
void CoreGraphics::__UpdateScene()
{
    // take screenshot
    if(Core::Input->GetKeyboardButton(CORE_INPUT_KEY(PRINTSCREEN), CORE_INPUT_PRESS))
        this->Screenshot();

    // disable last model, textures and shader-program
    coreModel  ::Disable(true);
    coreTexture::DisableAll();
    coreProgram::Disable(true);

    // explicitly invalidate depth buffer
    if(CORE_GL_SUPPORT(ARB_invalidate_subdata))
    {
        constexpr_var GLenum aiAttachment[1] = {GL_DEPTH};
        glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, aiAttachment);
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