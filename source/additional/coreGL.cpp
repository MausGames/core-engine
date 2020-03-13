///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

#if defined(_CORE_WINDOWS_)
    #include <GL/wglew.h>
#elif defined(_CORE_LINUX_)
    #include <GL/glxew.h>
#endif

coreBool GLEW_V2_compatibility = false;


// ****************************************************************
/* pool definitions */
#define CORE_GL_POOL_SIZE (512u)   //!< number of resource names generated and pooled at the same time

#define CORE_GL_POOL_GENERATE(n,c,g)                                               \
{                                                                                  \
    ASSERT(iCount && pNames && (iCount < CORE_GL_POOL_SIZE))                       \
                                                                                   \
    const coreUintW iRemaining = CORE_GL_POOL_SIZE - (n).iNext;                    \
    if(iCount > iRemaining)                                                        \
    {                                                                              \
        std::memcpy(pNames, &(n).aiArray[(n).iNext], sizeof(GLuint) * iRemaining); \
        iCount -= iRemaining;                                                      \
        pNames += iRemaining;                                                      \
                                                                                   \
        if(CORE_GL_SUPPORT(ARB_direct_state_access))                               \
             (c)(CORE_GL_POOL_SIZE, (n).aiArray);                                  \
        else (g)(CORE_GL_POOL_SIZE, (n).aiArray);                                  \
        (n).iNext = 0u;                                                            \
    }                                                                              \
                                                                                   \
    std::memcpy(pNames, &(n).aiArray[(n).iNext], sizeof(GLuint) * iCount);         \
    (n).iNext += iCount;                                                           \
}

#define CORE_GL_POOL_RESET(n,d)                                                    \
{                                                                                  \
    const coreUintW iRemaining = CORE_GL_POOL_SIZE - (n).iNext;                    \
    (d)(iRemaining, &(n).aiArray[(n).iNext]);                                      \
                                                                                   \
    (n).iNext = CORE_GL_POOL_SIZE;                                                 \
}


// ****************************************************************
/* pool structure */
struct coreNamePool final
{
    GLuint       aiArray[CORE_GL_POOL_SIZE];   //!< actual pool holding all pre-generated resource names
    coreUintW    iNext = CORE_GL_POOL_SIZE;    //!< next unused resource name in the pool
    SDL_SpinLock iLock = 0;                    //!< spinlock to allow multiple threads accessing the pool
};

static coreNamePool g_PoolTextures2D;
static coreNamePool g_PoolBuffers;
static coreNamePool g_PoolVertexArrays;


// ****************************************************************
/* generate resource names */
void coreGenTextures2D(coreUintW iCount, GLuint* OUTPUT pNames)
{
    // wrap function for consistent interface
    const auto nCreateFunc = [](coreUintW iCount, GLuint* OUTPUT pNames) {glCreateTextures(GL_TEXTURE_2D, iCount, pNames);};

    // generate 2D texture names
    coreSpinLocker oLocker(&g_PoolTextures2D.iLock);
    CORE_GL_POOL_GENERATE(g_PoolTextures2D, nCreateFunc, glGenTextures)
}

void coreGenBuffers(coreUintW iCount, GLuint* OUTPUT pNames)
{
    // generate data buffer names
    coreSpinLocker oLocker(&g_PoolBuffers.iLock);
    CORE_GL_POOL_GENERATE(g_PoolBuffers, glCreateBuffers, glGenBuffers)
}

void coreGenVertexArrays(coreUintW iCount, GLuint* OUTPUT pNames)
{
    // generate vertex array names (without lock, because only executed on main-thread)
    CORE_GL_POOL_GENERATE(g_PoolVertexArrays, glCreateVertexArrays, glGenVertexArrays)
}


// ****************************************************************
/* init OpenGL */
void __coreInitOpenGL()
{
    // init GLEW on current context
    const GLenum iError = glewInit();
    if(iError != GLEW_OK) Core::Log->Error("GLEW could not be initialized (GLEW: %s)", glewGetErrorString(iError));
                     else Core::Log->Info ("GLEW initialized (%s)",                    glewGetString(GLEW_VERSION));

    // handle support for deprecated features
    GLEW_V2_compatibility = !GLEW_VERSION_3_1;

    // improve frame buffer compatibility
    if(!GLEW_ARB_framebuffer_object && GLEW_EXT_framebuffer_object)
    {
        // remap GL_EXT_framebuffer_object
        glBindFramebuffer                     = glBindFramebufferEXT;
        glBindRenderbuffer                    = glBindRenderbufferEXT;
        glCheckFramebufferStatus              = glCheckFramebufferStatusEXT;
        glDeleteFramebuffers                  = glDeleteFramebuffersEXT;
        glDeleteRenderbuffers                 = glDeleteRenderbuffersEXT;
        glFramebufferRenderbuffer             = glFramebufferRenderbufferEXT;
        glFramebufferTexture1D                = glFramebufferTexture1DEXT;
        glFramebufferTexture2D                = glFramebufferTexture2DEXT;
        glFramebufferTexture3D                = glFramebufferTexture3DEXT;
        glGenFramebuffers                     = glGenFramebuffersEXT;
        glGenRenderbuffers                    = glGenRenderbuffersEXT;
        glGenerateMipmap                      = glGenerateMipmapEXT;
        glGetFramebufferAttachmentParameteriv = glGetFramebufferAttachmentParameterivEXT;
        glGetRenderbufferParameteriv          = glGetRenderbufferParameterivEXT;
        glIsFramebuffer                       = glIsFramebufferEXT;
        glIsRenderbuffer                      = glIsRenderbufferEXT;
        glRenderbufferStorage                 = glRenderbufferStorageEXT;

        // remap GL_EXT_framebuffer_blit
        glBlitFramebuffer = glBlitFramebufferEXT;

        // remap GL_EXT_framebuffer_multisample
        glRenderbufferStorageMultisample = glRenderbufferStorageMultisampleEXT;

        // remap GL_EXT_texture_array
        glFramebufferTextureLayer = glFramebufferTextureLayerEXT;
    }
    else if(GLEW_ARB_framebuffer_object)
    {
        // force extension status
        __GLEW_EXT_framebuffer_object      = true;
        __GLEW_EXT_framebuffer_blit        = true;
        __GLEW_EXT_framebuffer_multisample = true;
        __GLEW_EXT_texture_array           = true;
        __GLEW_EXT_packed_depth_stencil    = true;
    }

    // improve image unit compatibility
    if(!GLEW_ARB_shader_image_load_store && GLEW_EXT_shader_image_load_store)
    {
        // remap GL_EXT_shader_image_load_store
        glBindImageTexture = r_cast<PFNGLBINDIMAGETEXTUREPROC>(glBindImageTextureEXT);
        glMemoryBarrier    = glMemoryBarrierEXT;
    }
    else if(GLEW_ARB_shader_image_load_store)
    {
        // force extension status
        __GLEW_EXT_shader_image_load_store = true;
    }

    // improve texture storage compatibility
    if(!GLEW_ARB_texture_storage && GLEW_EXT_texture_storage)
    {
        // remap GL_EXT_texture_storage
        glTexStorage1D = glTexStorage1DEXT;
        glTexStorage2D = glTexStorage2DEXT;
        glTexStorage3D = glTexStorage3DEXT;
    }
    else if(GLEW_ARB_texture_storage)
    {
        // force extension status
        __GLEW_EXT_texture_storage = true;
    }

    // improve sample shading compatibility
    if(!GLEW_VERSION_4_0 && GLEW_ARB_sample_shading)
    {
        // remap GL_ARB_sample_shading
        glMinSampleShading = glMinSampleShadingARB;
    }
    else if(GLEW_VERSION_4_0)
    {
        // force extension status
        __GLEW_ARB_sample_shading = true;
    }

    // improve instancing compatibility
    if(!GLEW_VERSION_3_3 && GLEW_ARB_instanced_arrays)
    {
        // remap GL_ARB_instanced_arrays
        glDrawArraysInstanced   = glDrawArraysInstancedARB;
        glDrawElementsInstanced = glDrawElementsInstancedARB;
        glVertexAttribDivisor   = glVertexAttribDivisorARB;
    }
    else if(GLEW_VERSION_3_3)
    {
        // force extension status
        __GLEW_ARB_instanced_arrays = true;
    }

    // improve shader integer compatibility
    if(!GLEW_VERSION_3_0 && GL_EXT_gpu_shader4)
    {
        // remap GL_EXT_gpu_shader4
        glBindFragDataLocation = glBindFragDataLocationEXT;
        glUniform1ui           = glUniform1uiEXT;
        glUniform1uiv          = glUniform1uivEXT;
        glUniform2ui           = glUniform2uiEXT;
        glUniform2uiv          = glUniform2uivEXT;
        glUniform3ui           = glUniform3uiEXT;
        glUniform3uiv          = glUniform3uivEXT;
        glUniform4ui           = glUniform4uiEXT;
        glUniform4uiv          = glUniform4uivEXT;
        glVertexAttribIPointer = glVertexAttribIPointerEXT;
    }
    else if(GLEW_VERSION_3_0)
    {
        // force extension status
        __GLEW_EXT_gpu_shader4 = true;
    }

    // force additional extension status
    if(                     GLEW_EXT_framebuffer_sRGB)           __GLEW_ARB_framebuffer_sRGB           = true;
    if( GLEW_VERSION_1_3 || GLEW_EXT_multisample)                __GLEW_ARB_multisample                = true;
    if( GLEW_VERSION_1_4)                                        __GLEW_ARB_depth_texture              = true;
    if( GLEW_VERSION_2_1 || GLEW_EXT_pixel_buffer_object)        __GLEW_ARB_pixel_buffer_object        = true;
    if( GLEW_VERSION_3_2 || GLEW_EXT_geometry_shader4)           __GLEW_ARB_geometry_shader4           = true;
    if( GLEW_VERSION_4_6 || GLEW_ARB_texture_filter_anisotropic) __GLEW_EXT_texture_filter_anisotropic = true;
    if(!GLEW_VERSION_3_0 || Core::Config->GetBool(CORE_CONFIG_BASE_FALLBACKMODE))
        __GLEW_ARB_uniform_buffer_object = false;

    // change extension status through configuration file
    coreData::StrForEachToken(Core::Config->GetString(CORE_CONFIG_GRAPHICS_ENABLEEXTENSIONS),  " ,;", [](const coreChar* pcToken) {glewEnableExtension (pcToken);});
    coreData::StrForEachToken(Core::Config->GetString(CORE_CONFIG_GRAPHICS_DISABLEEXTENSIONS), " ,;", [](const coreChar* pcToken) {glewDisableExtension(pcToken);});

    // try to support old OpenGL versions
    if(!GLEW_VERSION_2_0)
    {
        // remap GL_ARB_vertex_program
        glDisableVertexAttribArray = glDisableVertexAttribArrayARB;
        glEnableVertexAttribArray  = glEnableVertexAttribArrayARB;
        glVertexAttribPointer      = glVertexAttribPointerARB;

        // remap GL_ARB_vertex_shader
        glBindAttribLocation = glBindAttribLocationARB;

        // remap GL_ARB_shader_objects
        glAttachShader       = glAttachObjectARB;
        glCompileShader      = glCompileShaderARB;
        glCreateProgram      = glCreateProgramObjectARB;
        glCreateShader       = glCreateShaderObjectARB;
        glDeleteShader       = glDeleteObjectARB;
        glDeleteProgram      = glDeleteObjectARB;
        glDetachShader       = glDetachObjectARB;
        glGetShaderInfoLog   = glGetInfoLogARB;
        glGetProgramInfoLog  = glGetInfoLogARB;
        glGetShaderiv        = glGetObjectParameterivARB;
        glGetProgramiv       = glGetObjectParameterivARB;
        glGetUniformLocation = glGetUniformLocationARB;
        glLinkProgram        = glLinkProgramARB;
        glShaderSource       = r_cast<PFNGLSHADERSOURCEPROC>(glShaderSourceARB);
        glUniform1f          = glUniform1fARB;
        glUniform1i          = glUniform1iARB;
        glUniform2fv         = glUniform2fvARB;
        glUniform3fv         = glUniform3fvARB;
        glUniform4fv         = glUniform4fvARB;
        glUniformMatrix3fv   = glUniformMatrix3fvARB;
        glUniformMatrix4fv   = glUniformMatrix4fvARB,
        glUseProgram         = glUseProgramObjectARB;
        glValidateProgram    = glValidateProgramARB;

        // remap GL_ARB_vertex_buffer_object
        glBindBuffer    = glBindBufferARB;
        glBufferData    = glBufferDataARB;
        glBufferSubData = glBufferSubDataARB;
        glDeleteBuffers = glDeleteBuffersARB;
        glGenBuffers    = glGenBuffersARB;

        // remap GL_EXT_draw_range_elements
        glDrawRangeElements = glDrawRangeElementsEXT;
    }

    // check for basic OpenGL support
    if((!GLEW_ARB_vertex_program           ||
        !GLEW_ARB_vertex_shader            ||
        !GLEW_ARB_fragment_shader          ||
        !GLEW_ARB_shader_objects           ||
        !GLEW_ARB_shading_language_100     ||
        !GLEW_ARB_vertex_buffer_object     ||
        !GLEW_ARB_texture_non_power_of_two ||
        !GLEW_EXT_draw_range_elements) && GLEW_V2_compatibility)
        Core::Log->Warning("Minimum OpenGL requirements not met, application may not work properly");

    // check for frame buffer object support
    if(!GLEW_EXT_framebuffer_object)
        Core::Log->Warning("Frame Buffer Objects not supported, application may not work properly");
}


// ****************************************************************
/* exit OpenGL */
void __coreExitOpenGL()
{
    // delete remaining resource names from the pools
    CORE_GL_POOL_RESET(g_PoolTextures2D,   glDeleteTextures)
    CORE_GL_POOL_RESET(g_PoolBuffers,      glDeleteBuffers)
    CORE_GL_POOL_RESET(g_PoolVertexArrays, glDeleteTextures)
}


// ****************************************************************
/* get extension string */
void coreExtensions(std::string* OUTPUT psOutput)
{
    if(GLEW_VERSION_3_0)
    {
        // get number of available extensions
        GLint iNumExtensions;
        glGetIntegerv(GL_NUM_EXTENSIONS, &iNumExtensions);

        // reserve some memory
        psOutput->clear();
        psOutput->reserve(iNumExtensions * 32u);

        // concatenate all extensions to a single string
        for(coreUintW i = 0u, ie = iNumExtensions; i < ie; ++i)
        {
            (*psOutput) += r_cast<const coreChar*>(glGetStringi(GL_EXTENSIONS, i));
            (*psOutput) += ' ';
        }

        // reduce output size
        psOutput->pop_back();
        psOutput->shrink_to_fit();
    }
    else
    {
        // get full extension string
        (*psOutput) = r_cast<const coreChar*>(glGetString(GL_EXTENSIONS));
    }
}


// ****************************************************************
/* get platform-specific extension string */
void corePlatformExtensions(std::string* OUTPUT psOutput)
{
    // clear memory
    psOutput->clear();

#if defined(_CORE_WINDOWS_)

    // get device context of current screen
    const HDC pDC = GetDC(NULL);
    if(pDC)
    {
        // get full extension string
        (*psOutput) = wglGetExtensionsStringARB(pDC);

        // release device context
        ReleaseDC(NULL, pDC);
    }

#elif defined(_CORE_LINUX_)

    // open connection to default display
    Display* pDisplay = XOpenDisplay(NULL);
    if(pDisplay)
    {
        // get full extension string
        (*psOutput) = glXQueryExtensionsString(pDisplay, DefaultScreen(pDisplay));

        // close connection
        XCloseDisplay(pDisplay);
    }

#endif
}