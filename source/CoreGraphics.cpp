//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"


// ******************************************************************
// constructor
CoreGraphics::CoreGraphics()
: m_fFOV            (Core::Config->GetFloat(CORE_CONFIG_GRAPHICS_FOV, 45.0f))
, m_fNearClip       (Core::Config->GetFloat(CORE_CONFIG_GRAPHICS_CLIP_NEAR, 0.1f))
, m_fFarClip        (Core::Config->GetFloat(CORE_CONFIG_GRAPHICS_CLIP_FAR, 1000.0f))
, m_vCamPosition    (coreVector3(0.0f,0.0f, 0.0f))
, m_vCamDirection   (coreVector3(0.0f,0.0f,-1.0f))
, m_vCamOrientation (coreVector3(0.0f,1.0f, 0.0f))
, m_mCurProjection  (coreMatrix::Identity())
, m_vCurResolution  (coreVector2(0.0f,0.0f))
{
    Core::Log->Header("Graphics Interface");

    // create OpenGL contexts
    m_RenderContext   = SDL_GL_CreateContext(Core::System->GetWindow());
    m_ResourceContext = SDL_GL_CreateContext(Core::System->GetWindow());
         if(!m_RenderContext)   Core::Log->Error(1, coreUtils::Print("Primary OpenGL context could not be created (SDL: %s)",   SDL_GetError()));
    else if(!m_ResourceContext) Core::Log->Error(1, coreUtils::Print("Secondary OpenGL context could not be created (SDL: %s)", SDL_GetError()));
    else Core::Log->Info("Primary and secondary OpenGL context created");

    // assign primary OpenGL context to main window
    if(SDL_GL_MakeCurrent(Core::System->GetWindow(), m_RenderContext))
        Core::Log->Error(1, coreUtils::Print("Primary OpenGL context could not be assigned to main window (SDL: %s)", SDL_GetError()));
    else Core::Log->Info("Primary OpenGL context assigned to main window");

    // init GLEW on primary OpenGL context
    const GLenum iError = glewInit();
    if(iError != GLEW_OK) Core::Log->Error(1, coreUtils::Print("GLEW could not be initialized on primary OpenGL context (GLEW: %s)", glewGetErrorString(iError)));
    else Core::Log->Info("GLEW initialized on primary OpenGL context");

    // log video card information
    Core::Log->ListStart("Video Card Information");
    Core::Log->ListEntry(coreUtils::Print("<b>Vendor:</b> %s",         glGetString(GL_VENDOR)));
    Core::Log->ListEntry(coreUtils::Print("<b>Renderer:</b> %s",       glGetString(GL_RENDERER)));
    Core::Log->ListEntry(coreUtils::Print("<b>OpenGL Version:</b> %s", glGetString(GL_VERSION)));
    Core::Log->ListEntry(coreUtils::Print("<b>Shader Version:</b> %s", glGetString(GL_SHADING_LANGUAGE_VERSION)));
    Core::Log->ListEntry((const char*)glGetString(GL_EXTENSIONS));
    Core::Log->ListEnd();

    // set numerical OpenGL version
    const float fForceOpenGL = Core::Config->GetFloat(CORE_CONFIG_GRAPHICS_FORCEOPENGL, 0.0f);
    if(fForceOpenGL) m_fOpenGL = fForceOpenGL;
    else
    {
             if(GLEW_VERSION_4_4) m_fOpenGL = 4.4f;
        else if(GLEW_VERSION_4_3) m_fOpenGL = 4.3f;
        else if(GLEW_VERSION_4_2) m_fOpenGL = 4.2f;
        else if(GLEW_VERSION_4_1) m_fOpenGL = 4.1f;
        else if(GLEW_VERSION_4_0) m_fOpenGL = 4.0f;
        else if(GLEW_VERSION_3_3) m_fOpenGL = 3.3f;
        else if(GLEW_VERSION_3_2) m_fOpenGL = 3.2f;
        else if(GLEW_VERSION_3_1) m_fOpenGL = 3.1f;
        else if(GLEW_VERSION_3_0) m_fOpenGL = 3.0f;
        else if(GLEW_VERSION_2_1) m_fOpenGL = 2.1f;
        else if(GLEW_VERSION_2_0) m_fOpenGL = 2.0f;
                             else m_fOpenGL = 0.0f;
    }    

    // check OpenGL version
    if(m_fOpenGL < 2.0f) Core::Log->Error(1, "Minimum system requirements are not met, video card with at least OpenGL 2.0 is required");

    // define base geometry attributes
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);    
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableVertexAttribArray(1);

    // enable and disable base features
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_MULTISAMPLE);
    glDisable(GL_DITHER);

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

    // improve texture perspective correction and mip mapping
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);

    // reset camera
    this->SetCamera(NULL, NULL, NULL);

    // reset view
    this->ResizeView(coreVector2(0.0f,0.0f));

    // reset scene
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    SDL_GL_SwapWindow(Core::System->GetWindow()); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


// ******************************************************************
// destructor
CoreGraphics::~CoreGraphics()
{
    Core::Log->Info("Graphics Interface shut down");

    // clear memory
    m_abFeature.clear();

    // dissociate primary OpenGL context from main window
    SDL_GL_MakeCurrent(Core::System->GetWindow(), NULL);

    // delete OpenGL contexts
    SDL_GL_DeleteContext(m_ResourceContext);
    SDL_GL_DeleteContext(m_RenderContext);
}


// ******************************************************************
// update the graphics scene
void CoreGraphics::__UpdateScene()
{
    // swap main frame buffers
    SDL_GL_SwapWindow(Core::System->GetWindow()); 

    // reset depth buffer
#if defined(_DEBUG)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#else
    glClear(GL_DEPTH_BUFFER_BIT);
#endif
}


// ******************************************************************
// set camera and create camera matrix
void CoreGraphics::SetCamera(const coreVector3* pvPosition, const coreVector3* pvDirection, const coreVector3* pvOrientation)
{
    bool bNewCamera = false;

    // set attributes of the camera
    if(pvPosition)    {                                                          if(m_vCamPosition    != *pvPosition) {m_vCamPosition    = *pvPosition; bNewCamera = true;}}
    if(pvDirection)   {const coreVector3 vDirNorm = pvDirection->Normalized();   if(m_vCamDirection   != vDirNorm)    {m_vCamDirection   = vDirNorm;    bNewCamera = true;}}
    if(pvOrientation) {const coreVector3 vOriNorm = pvOrientation->Normalized(); if(m_vCamOrientation != vOriNorm)    {m_vCamOrientation = vOriNorm;    bNewCamera = true;}}

    // create camera matrix
    if(bNewCamera) m_mCamera = coreMatrix::Camera(m_vCamPosition, m_vCamDirection, m_vCamOrientation);
}


// ******************************************************************
// resize view and create projection matrices
void CoreGraphics::ResizeView(coreVector2 vResolution)
{
    if(!vResolution.x) vResolution.x = Core::System->GetResolution().x;
    if(!vResolution.y) vResolution.y = Core::System->GetResolution().y;
    if(m_vCurResolution == vResolution) return;

    // set viewport
    glViewport(0, 0, (int)vResolution.x, (int)vResolution.y);

    // generate projection matrices
    m_mPerspective = coreMatrix::Perspective(vResolution, DEG_TO_RAD(m_fFOV), m_fNearClip, m_fFarClip);
    m_mOrtho       = coreMatrix::Ortho(vResolution);
    this->EnablePerspective();

    m_vCurResolution = vResolution;
}


// ******************************************************************
// enable perspective projection matrix
void CoreGraphics::EnablePerspective()
{
    if(m_mCurProjection == m_mPerspective) return;

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_mPerspective);
    glMatrixMode(GL_MODELVIEW);

    m_mCurProjection = m_mPerspective;
}


// ******************************************************************
// enable orthogonal projection matrix
void CoreGraphics::EnableOrtho()
{
    if(m_mCurProjection == m_mOrtho) return;

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_mOrtho);
    glMatrixMode(GL_MODELVIEW);

    m_mCurProjection = m_mOrtho;
}

// ******************************************************************
// create a screenshot
void CoreGraphics::Screenshot(const char* pcPath)
{
    // TODO: implement function
    // ? extern DECLSPEC int SDLCALL IMG_SavePNG(SDL_Surface *surface, const char *file);
    // ? extern DECLSPEC int SDLCALL IMG_SavePNG_RW(SDL_Surface *surface, SDL_RWops *dst, int freedst);
}

void CoreGraphics::Screenshot()
{
    // TODO: implement function
    this->Screenshot("");
}