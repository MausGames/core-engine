//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

#define CORE_GRAPHICS_UNIFORM_OFFSET_LIGHT (4*sizeof(coreMatrix4) + 1*sizeof(coreVector4))
#define CORE_GRAPHICS_UNIFORM_SIZE         (CORE_GRAPHICS_UNIFORM_OFFSET_LIGHT + CORE_GRAPHICS_LIGHTS*sizeof(coreLight))


// ******************************************************************
// constructor
CoreGraphics::CoreGraphics()noexcept
: m_vCamPosition    (coreVector3(0.0f,0.0f,0.0f))
, m_vCamDirection   (coreVector3(0.0f,0.0f,0.0f))
, m_vCamOrientation (coreVector3(0.0f,0.0f,0.0f))
, m_vCurResolution  (coreVector4(0.0f,0.0f,0.0f,0.0f))
{
    Core::Log->Header("Graphics Interface");

    // create render context
    m_RenderContext = SDL_GL_CreateContext(Core::System->GetWindow());
    if(!m_RenderContext)
        Core::Log->Error("Render context could not be created (SDL: %s)", SDL_GetError());
    else Core::Log->Info("Render context created");

    if(Core::Config->GetBool(CORE_CONFIG_GRAPHICS_DUALCONTEXT) && (Core::System->SupportNumCores() >= 2))
    {
        // create resource context
        m_ResourceContext = SDL_GL_CreateContext(Core::System->GetWindow());
        if(!m_ResourceContext)
            Core::Log->Warning("Resource context could not be created (SDL: %s)", SDL_GetError());
        else Core::Log->Info("Resource context created");
    }
    else m_ResourceContext = NULL;

    // assign render context to main window
    if(SDL_GL_MakeCurrent(Core::System->GetWindow(), m_RenderContext))
        Core::Log->Error("Render context could not be assigned to main window (SDL: %s)", SDL_GetError());
    else Core::Log->Info("Render context assigned to main window");

    // init GLEW on render context
    const GLenum iError = glewInit();
    if(iError != GLEW_OK)
        Core::Log->Error("GLEW could not be initialized on render context (GLEW: %s)", glewGetErrorString(iError));
    else Core::Log->Info("GLEW initialized on render context (%s)", glewGetString(GLEW_VERSION));

    // enable OpenGL debug output
    Core::Log->DebugOpenGL();

    // log video card information
    Core::Log->ListStart("Video Card Information");
    {
        Core::Log->ListEntry(CORE_LOG_BOLD("Vendor:")         " %s", glGetString(GL_VENDOR));
        Core::Log->ListEntry(CORE_LOG_BOLD("Renderer:")       " %s", glGetString(GL_RENDERER));
        Core::Log->ListEntry(CORE_LOG_BOLD("OpenGL Version:") " %s", glGetString(GL_VERSION));
        Core::Log->ListEntry(CORE_LOG_BOLD("Shader Version:") " %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
        Core::Log->ListEntry(r_cast<const char*>(glGetString(GL_EXTENSIONS)));
    }
    Core::Log->ListEnd();

    // set numerical OpenGL version
    const float fForceOpenGL = Core::Config->GetFloat(CORE_CONFIG_GRAPHICS_FORCEOPENGL);
    m_fOpenGL = fForceOpenGL ? fForceOpenGL : coreData::StrVersion(r_cast<const char*>(glGetString(GL_VERSION)));
    m_fGLSL   = coreData::StrVersion(r_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));

    // check OpenGL version
    if(m_fOpenGL < 2.0f) Core::Log->Error("Minimum system requirements not met, video card with at least OpenGL 2.0 required");

    // enable vertical synchronization
    if(SDL_GL_SetSwapInterval(1)) Core::Log->Warning("Vertical Synchronization not directly supported (SDL: %s)", SDL_GetError());
    else Core::Log->Info("Vertical Synchronization enabled");

    // enable texturing
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DITHER);
    glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
    glPixelStorei(GL_PACK_ALIGNMENT,   4);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    // enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glPolygonOffset(1.1f, 4.0f);
    glClearDepth(1.0f);

    // enable culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // create uniform buffer object for global shader-data
    if(GLEW_ARB_uniform_buffer_object)
    {
        // generate and bind global UBO to a buffer target
        m_iUniformBuffer.Create(GL_UNIFORM_BUFFER, CORE_GRAPHICS_UNIFORM_SIZE, NULL, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, CORE_SHADER_BUFFER_GLOBAL_NUM, m_iUniformBuffer);
    }

    // reset camera and view
    this->SetCamera(coreVector3(0.0f,0.0f,0.0f), coreVector3(0.0f,0.0f,-1.0f), coreVector3(0.0f,1.0f,0.0f));
    this->ResizeView(coreVector2(0.0f,0.0f), PI*0.25f, 0.1f, 1000.0f);

    // reset ambient
    for(int i = 0; i < CORE_GRAPHICS_LIGHTS; ++i)
        this->SetLight(i, coreVector4(0.0f,0.0f,0.0f,0.0f), coreVector4(0.0f,0.0f,-1.0f,1.0f), coreVector4(1.0f,1.0f,1.0f,1.0f));

    // reset scene
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SDL_GL_SwapWindow(Core::System->GetWindow());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


// ******************************************************************
// destructor
CoreGraphics::~CoreGraphics()
{
    // delete global UBO
    m_iUniformBuffer.Delete();

    // dissociate render context from main window
    SDL_GL_MakeCurrent(Core::System->GetWindow(), NULL);

    // delete OpenGL contexts
    SDL_GL_DeleteContext(m_ResourceContext);
    SDL_GL_DeleteContext(m_RenderContext);

    Core::Log->Info("Graphics Interface shut down");
}


// ******************************************************************
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

        // send transformation data to the global UBO
        this->__SendTransformation();
    }
}


// ******************************************************************
// resize view and create projection matrices
void CoreGraphics::ResizeView(coreVector2 vResolution, const float& fFOV, const float& fNearClip, const float& fFarClip)
{
    bool bNewView = false;

    // retrieve window resolution
    if(!vResolution.x) vResolution.x = Core::System->GetResolution().x;
    if(!vResolution.y) vResolution.y = Core::System->GetResolution().y;

    // set properties of the view frustum
    if(m_vCurResolution.xy() != vResolution)
    {
        // save viewport resolution
        m_vCurResolution.xy(vResolution);
        m_vCurResolution.zw(coreVector2(1.0f,1.0f) / vResolution);

        // set viewport
        glViewport(0, 0, (int)vResolution.x, (int)vResolution.y);
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

        // send transformation data to the global UBO
        this->__SendTransformation();
    }
}


// ******************************************************************
// set and update ambient light
void CoreGraphics::SetLight(const int& iID, const coreVector4& vPosition, const coreVector4& vDirection, const coreVector4& vValue)
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

    if(bNewLight && m_iUniformBuffer)
    {
        // map required area of the global UBO
        coreByte* pRange = m_iUniformBuffer.Map<coreByte>(CORE_GRAPHICS_UNIFORM_OFFSET_LIGHT + iID*sizeof(coreLight), sizeof(coreLight), true);

        // update specific light data
        std::memcpy(pRange, &CurLight, sizeof(coreLight));
        m_iUniformBuffer.Unmap(pRange);
    }
}


// ******************************************************************
// take screenshot
// TODO: improve with pixel-pack-buffer
void CoreGraphics::Screenshot(const char* pcPath)const
{
    const coreUint iWidth  = (coreUint)Core::System->GetResolution().x;
    const coreUint iHeight = (coreUint)Core::System->GetResolution().y;
    const coreUint iPitch  = iWidth*3;
    const coreUint iSize   = iHeight*iPitch;

    // create folder hierarchy
    const char* pcFullPath = PRINT(std::strcmp(coreData::StrRight(pcPath, 4), ".png") ? "%s.png" : "%s", pcPath);
    coreData::CreateFolder(pcFullPath);

    // read pixel data from the frame buffer
    coreByte* pData = new coreByte[iSize];
    glReadPixels(0, 0, iWidth, iHeight, GL_RGB, GL_UNSIGNED_BYTE, pData);

    // flip pixel data vertically
    coreByte* pConvert = new coreByte[iSize];
    for(coreUint i = 0; i < iHeight; ++i)
        std::memcpy(pConvert + (iHeight-i-1)*iPitch, pData + i*iPitch, iPitch);

    // create an SDL surface
    SDL_Surface* pSurface = SDL_CreateRGBSurfaceFrom(pConvert, iWidth, iHeight, 24, iPitch, CORE_TEXTURE_MASK);
    if(pSurface)
    {
        // save the surface as PNG image
        IMG_SavePNG(pSurface, pcFullPath);
        SDL_FreeSurface(pSurface);
    }

    SAFE_DELETE_ARRAY(pData)
    SAFE_DELETE_ARRAY(pConvert)
}


// ******************************************************************
// update the graphics scene
void CoreGraphics::__UpdateScene()
{
    // take screenshot
    if(Core::Input->GetKeyboardButton(KEY(PRINTSCREEN), CORE_INPUT_PRESS))
        this->Screenshot();

    // disable last model, textures and shader-program
    coreModel::Disable(true);
    coreTexture::DisableAll();
    coreProgram::Disable(true);

    // explicitly invalidate depth buffer 
    if(GLEW_ARB_invalidate_subdata)
    {
        constexpr_var GLenum aiAttachment[1] = {GL_DEPTH};
        glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, aiAttachment);
    }

    // swap color buffers
    SDL_GL_SwapWindow(Core::System->GetWindow());

    // clear color and depth buffer
#if defined(_CORE_DEBUG_) || defined(_CORE_GLES_)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
    glClear(GL_DEPTH_BUFFER_BIT);
#endif
}


// ******************************************************************
// send transformation data to the global UBO
void CoreGraphics::__SendTransformation()
{
    if(!m_iUniformBuffer) return;

    // calculate view-projection matrix
    const coreMatrix4 mViewProj = m_mCamera * m_mPerspective;

    // map required area of the global UBO
    coreByte* pRange = m_iUniformBuffer.Map<coreByte>(0, 4*sizeof(coreMatrix4) + sizeof(coreVector4), true);

    // update transformation matrices
    std::memcpy(pRange,                         &mViewProj,        sizeof(coreMatrix4));
    std::memcpy(pRange + 1*sizeof(coreMatrix4), &m_mCamera,        sizeof(coreMatrix4));
    std::memcpy(pRange + 2*sizeof(coreMatrix4), &m_mPerspective,   sizeof(coreMatrix4));
    std::memcpy(pRange + 3*sizeof(coreMatrix4), &m_mOrtho,         sizeof(coreMatrix4));
    std::memcpy(pRange + 4*sizeof(coreMatrix4), &m_vCurResolution, sizeof(coreVector4));
    m_iUniformBuffer.Unmap(pRange);
}