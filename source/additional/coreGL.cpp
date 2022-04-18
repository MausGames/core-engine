///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "Core.h"

#if defined(_CORE_WINDOWS_)
    #include <GL/wglew.h>
#elif defined(_CORE_LINUX_)
    #include <EGL/egl.h>
    #include <wayland-client.h>
    #include <GL/glx.h>
#endif

coreBool GLEW_V2_compatibility = false;


// ****************************************************************
/* pool definitions */
#define CORE_GL_POOL_SIZE (256u)   // number of resource names generated and pooled at the same time

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
    GLuint       aiArray[CORE_GL_POOL_SIZE];   // actual pool holding all pre-generated resource names
    coreUintW    iNext = CORE_GL_POOL_SIZE;    // next unused resource name in the pool
    coreSpinLock oLock = coreSpinLock();       // spinlock to allow multiple threads to access the pool
};

static coreNamePool s_PoolTextures2D;
static coreNamePool s_PoolBuffers;
static coreNamePool s_PoolVertexArrays;


// ****************************************************************
/* generate resource names */
void coreGenTextures2D(coreUintW iCount, GLuint* OUTPUT pNames)
{
    // wrap function for consistent interface
    const auto nCreateFunc = [](coreUintW iCount, GLuint* OUTPUT pNames) {glCreateTextures(GL_TEXTURE_2D, iCount, pNames);};

    // generate 2D texture names
    coreSpinLocker oLocker(&s_PoolTextures2D.oLock);
    CORE_GL_POOL_GENERATE(s_PoolTextures2D, nCreateFunc, glGenTextures)
}

void coreGenBuffers(coreUintW iCount, GLuint* OUTPUT pNames)
{
    // generate data buffer names
    coreSpinLocker oLocker(&s_PoolBuffers.oLock);
    CORE_GL_POOL_GENERATE(s_PoolBuffers, glCreateBuffers, glGenBuffers)
}

void coreGenVertexArrays(coreUintW iCount, GLuint* OUTPUT pNames)
{
    // generate vertex array names (without lock, because only executed on main-thread)
    CORE_GL_POOL_GENERATE(s_PoolVertexArrays, glCreateVertexArrays, glGenVertexArrays)
}


// ****************************************************************
/* init OpenGL */
void __coreInitOpenGL()
{
    // init GLEW on current context
    const GLenum iError = glewInit();
    if(iError != GLEW_OK) Core::Log->Error("GLEW could not be initialized (GLEW: %s)", glewGetErrorString(iError));
                     else Core::Log->Info ("GLEW initialized (%s)",                    glewGetString(GLEW_VERSION));

    // improve extension support
    #define __IMPROVE(x,y) {coreString* T = &A; if(!(x)) {(__ ## x) = (y); T = (y) ? &C : &B;} T->append(" GL").append(&(#x[4]));}
    {
        coreString A, B, C;

        __IMPROVE(GLEW_AMD_conservative_depth,               false)                                                  // shader extension
        __IMPROVE(GLEW_AMD_framebuffer_multisample_advanced, false)
        __IMPROVE(GLEW_AMD_gpu_shader_half_float,            false)                                                  // shader extension
        __IMPROVE(GLEW_AMD_shader_trinary_minmax,            false)                                                  // shader extension
        __IMPROVE(GLEW_ARB_buffer_storage,                   GLEW_VERSION_4_4)
        __IMPROVE(GLEW_ARB_clear_buffer_object,              GLEW_VERSION_4_3)
        __IMPROVE(GLEW_ARB_clear_texture,                    GLEW_VERSION_4_4)
        __IMPROVE(GLEW_ARB_compute_shader,                   GLEW_VERSION_4_3)
        __IMPROVE(GLEW_ARB_conservative_depth,               GLEW_VERSION_4_2)                                       // shader extension
        __IMPROVE(GLEW_ARB_copy_buffer,                      GLEW_VERSION_3_1)
        __IMPROVE(GLEW_ARB_copy_image,                       GLEW_VERSION_4_3 || GLEW_NV_copy_image)
        __IMPROVE(GLEW_ARB_depth_buffer_float,               GLEW_VERSION_3_0)
        __IMPROVE(GLEW_ARB_depth_texture,                    GLEW_VERSION_1_4)                                       // below minimum
        __IMPROVE(GLEW_ARB_direct_state_access,              GLEW_VERSION_4_5)
        __IMPROVE(GLEW_ARB_enhanced_layouts,                 GLEW_VERSION_4_4)                                       // shader extension
        __IMPROVE(GLEW_ARB_framebuffer_sRGB,                 GLEW_VERSION_3_0 || GLEW_EXT_framebuffer_sRGB)
        __IMPROVE(GLEW_ARB_geometry_shader4,                 GLEW_VERSION_3_2 || GLEW_EXT_geometry_shader4)
        __IMPROVE(GLEW_ARB_gpu_shader5,                      GLEW_VERSION_4_0)                                       // shader extension
        __IMPROVE(GLEW_ARB_half_float_vertex,                GLEW_VERSION_3_0)
        __IMPROVE(GLEW_ARB_instanced_arrays,                 GLEW_VERSION_3_3)
        __IMPROVE(GLEW_ARB_invalidate_subdata,               GLEW_VERSION_4_3)
        __IMPROVE(GLEW_ARB_map_buffer_range,                 GLEW_VERSION_3_0)
        __IMPROVE(GLEW_ARB_multi_bind,                       GLEW_VERSION_4_4)
        __IMPROVE(GLEW_ARB_multisample,                      GLEW_VERSION_1_3)                                       // below minimum
        __IMPROVE(GLEW_ARB_parallel_shader_compile,          false            || GLEW_KHR_parallel_shader_compile)
        __IMPROVE(GLEW_ARB_pipeline_statistics_query,        GLEW_VERSION_4_6)
        __IMPROVE(GLEW_ARB_pixel_buffer_object,              GLEW_VERSION_2_1 || GLEW_EXT_pixel_buffer_object)
        __IMPROVE(GLEW_ARB_program_interface_query,          GLEW_VERSION_4_3)
        __IMPROVE(GLEW_ARB_sample_shading,                   GLEW_VERSION_4_0)                                       // shader extension (also)
        __IMPROVE(GLEW_ARB_shader_group_vote,                GLEW_VERSION_4_6)                                       // shader extension
        __IMPROVE(GLEW_ARB_shader_image_load_store,          GLEW_VERSION_4_2 || GLEW_EXT_shader_image_load_store)   // shader extension (also)
        __IMPROVE(GLEW_ARB_shading_language_packing,         GLEW_VERSION_4_2)                                       // shader extension
        __IMPROVE(GLEW_ARB_sync,                             GLEW_VERSION_3_2)
        __IMPROVE(GLEW_ARB_tessellation_shader,              GLEW_VERSION_4_0)
        __IMPROVE(GLEW_ARB_texture_compression_rgtc,         GLEW_VERSION_3_0 || GLEW_EXT_texture_compression_rgtc)
        __IMPROVE(GLEW_ARB_texture_filter_anisotropic,       GLEW_VERSION_4_6 || GLEW_EXT_texture_filter_anisotropic)
        __IMPROVE(GLEW_ARB_texture_float,                    GLEW_VERSION_3_0)
        __IMPROVE(GLEW_ARB_texture_rg,                       GLEW_VERSION_3_0)
        __IMPROVE(GLEW_ARB_texture_stencil8,                 GLEW_VERSION_4_4)
        __IMPROVE(GLEW_ARB_texture_storage,                  GLEW_VERSION_4_2 || GLEW_EXT_texture_storage)
        __IMPROVE(GLEW_ARB_timer_query,                      GLEW_VERSION_3_2)
        __IMPROVE(GLEW_ARB_uniform_buffer_object,            GLEW_VERSION_3_1)                                       // shader extension (also)
        __IMPROVE(GLEW_ARB_vertex_array_object,              GLEW_VERSION_3_0)
        __IMPROVE(GLEW_ARB_vertex_attrib_binding,            GLEW_VERSION_4_3)
        __IMPROVE(GLEW_ARB_vertex_type_2_10_10_10_rev,       GLEW_VERSION_3_3)
        __IMPROVE(GLEW_EXT_demote_to_helper_invocation,      false)                                                  // shader extension
        __IMPROVE(GLEW_EXT_direct_state_access,              false)
        __IMPROVE(GLEW_EXT_framebuffer_blit,                 GLEW_VERSION_3_0 || GLEW_ARB_framebuffer_object)        // split up
        __IMPROVE(GLEW_EXT_framebuffer_multisample,          GLEW_VERSION_3_0 || GLEW_ARB_framebuffer_object)        // split up
        __IMPROVE(GLEW_EXT_framebuffer_object,               GLEW_VERSION_3_0 || GLEW_ARB_framebuffer_object)        // split up
        __IMPROVE(GLEW_EXT_gpu_shader4,                      GLEW_VERSION_3_0)                                       // shader extension (also)
        __IMPROVE(GLEW_EXT_packed_depth_stencil,             GLEW_VERSION_3_0 || GLEW_ARB_framebuffer_object)        // split up
        __IMPROVE(GLEW_EXT_packed_float,                     GLEW_VERSION_3_0)
        __IMPROVE(GLEW_EXT_texture_compression_s3tc,         false)
        __IMPROVE(GLEW_KHR_debug,                            GLEW_VERSION_4_3)
        __IMPROVE(GLEW_KHR_no_error,                         false)                                                  // indirectly used
        __IMPROVE(GLEW_NVX_gpu_memory_info,                  false)
        __IMPROVE(GLEW_NV_framebuffer_multisample_coverage,  false)
        __IMPROVE(GLEW_NV_gpu_shader5,                       false)                                                  // shader extension
        __IMPROVE(GLEW_NV_multisample_filter_hint,           false)
        __IMPROVE(GLEW_NV_shader_buffer_load,                false)

        Core::Log->ListStartInfo("Extensions loaded");
        {
            Core::Log->ListAdd(CORE_LOG_BOLD("Detected:")     "%s", A.c_str());
            Core::Log->ListAdd(CORE_LOG_BOLD("Not detected:") "%s", B.c_str());
            Core::Log->ListAdd(CORE_LOG_BOLD("Overridden:")   "%s", C.c_str());
        }
        Core::Log->ListEnd();
    }
    #undef __IMPROVE

    // remap GL_ARB_instanced_arrays to OpenGL 3.1 and 3.3
    if(!glDrawArraysInstanced)   glDrawArraysInstanced   = glDrawArraysInstancedARB;
    if(!glDrawElementsInstanced) glDrawElementsInstanced = glDrawElementsInstancedARB;
    if(!glVertexAttribDivisor)   glVertexAttribDivisor   = glVertexAttribDivisorARB;

    // remap GL_ARB_sample_shading to OpenGL 4.0
    if(!glMinSampleShading) glMinSampleShading = glMinSampleShadingARB;

    // remap GL_EXT_framebuffer_blit to GL_ARB_framebuffer_object
    if(!glBlitFramebuffer) glBlitFramebuffer = glBlitFramebufferEXT;

    // remap GL_EXT_framebuffer_multisample to GL_ARB_framebuffer_object
    if(!glRenderbufferStorageMultisample) glRenderbufferStorageMultisample = glRenderbufferStorageMultisampleEXT;

    // remap GL_EXT_framebuffer_object to GL_ARB_framebuffer_object
    if(!glBindFramebuffer)                     glBindFramebuffer                     = glBindFramebufferEXT;
    if(!glBindRenderbuffer)                    glBindRenderbuffer                    = glBindRenderbufferEXT;
    if(!glCheckFramebufferStatus)              glCheckFramebufferStatus              = glCheckFramebufferStatusEXT;
    if(!glDeleteFramebuffers)                  glDeleteFramebuffers                  = glDeleteFramebuffersEXT;
    if(!glDeleteRenderbuffers)                 glDeleteRenderbuffers                 = glDeleteRenderbuffersEXT;
    if(!glFramebufferRenderbuffer)             glFramebufferRenderbuffer             = glFramebufferRenderbufferEXT;
    if(!glFramebufferTexture1D)                glFramebufferTexture1D                = glFramebufferTexture1DEXT;
    if(!glFramebufferTexture2D)                glFramebufferTexture2D                = glFramebufferTexture2DEXT;
    if(!glFramebufferTexture3D)                glFramebufferTexture3D                = glFramebufferTexture3DEXT;
    if(!glGenFramebuffers)                     glGenFramebuffers                     = glGenFramebuffersEXT;
    if(!glGenRenderbuffers)                    glGenRenderbuffers                    = glGenRenderbuffersEXT;
    if(!glGenerateMipmap)                      glGenerateMipmap                      = glGenerateMipmapEXT;
    if(!glGetFramebufferAttachmentParameteriv) glGetFramebufferAttachmentParameteriv = glGetFramebufferAttachmentParameterivEXT;
    if(!glGetRenderbufferParameteriv)          glGetRenderbufferParameteriv          = glGetRenderbufferParameterivEXT;
    if(!glIsFramebuffer)                       glIsFramebuffer                       = glIsFramebufferEXT;
    if(!glIsRenderbuffer)                      glIsRenderbuffer                      = glIsRenderbufferEXT;
    if(!glRenderbufferStorage)                 glRenderbufferStorage                 = glRenderbufferStorageEXT;

    // remap GL_EXT_gpu_shader4 to OpenGL 3.0
    if(!glBindFragDataLocation) glBindFragDataLocation = glBindFragDataLocationEXT;
    if(!glUniform1ui)           glUniform1ui           = glUniform1uiEXT;
    if(!glUniform1uiv)          glUniform1uiv          = glUniform1uivEXT;
    if(!glUniform2ui)           glUniform2ui           = glUniform2uiEXT;
    if(!glUniform2uiv)          glUniform2uiv          = glUniform2uivEXT;
    if(!glUniform3ui)           glUniform3ui           = glUniform3uiEXT;
    if(!glUniform3uiv)          glUniform3uiv          = glUniform3uivEXT;
    if(!glUniform4ui)           glUniform4ui           = glUniform4uiEXT;
    if(!glUniform4uiv)          glUniform4uiv          = glUniform4uivEXT;
    if(!glVertexAttribIPointer) glVertexAttribIPointer = glVertexAttribIPointerEXT;

    // remap GL_EXT_shader_image_load_store to GL_ARB_shader_image_load_store
    if(!glBindImageTexture) glBindImageTexture = r_cast<PFNGLBINDIMAGETEXTUREPROC>(glBindImageTextureEXT);
    if(!glMemoryBarrier)    glMemoryBarrier    = glMemoryBarrierEXT;

    // remap GL_EXT_texture_storage to GL_ARB_texture_storage
    if(!glTexStorage1D) glTexStorage1D = glTexStorage1DEXT;
    if(!glTexStorage2D) glTexStorage2D = glTexStorage2DEXT;
    if(!glTexStorage3D) glTexStorage3D = glTexStorage3DEXT;

    // remap GL_KHR_parallel_shader_compile to GL_ARB_parallel_shader_compile
    if(!glMaxShaderCompilerThreadsARB) glMaxShaderCompilerThreadsARB = glMaxShaderCompilerThreadsKHR;

    // remap GL_NV_copy_image to GL_ARB_copy_image
    if(!glCopyImageSubData) glCopyImageSubData = glCopyImageSubDataNV;

    // handle support for deprecated features
    GLEW_V2_compatibility = !GLEW_VERSION_3_1;

    // disable all features related to uniform buffer objects
    if(!GLEW_VERSION_3_0 || Core::Config->GetBool(CORE_CONFIG_BASE_FALLBACKMODE))
        __GLEW_ARB_uniform_buffer_object = false;

    // change extension status through configuration file
    coreData::StrForEachToken(Core::Config->GetString(CORE_CONFIG_GRAPHICS_ENABLEEXTENSIONS),  " ,;", [](const coreChar* pcToken) {glewEnableExtension (pcToken);});
    coreData::StrForEachToken(Core::Config->GetString(CORE_CONFIG_GRAPHICS_DISABLEEXTENSIONS), " ,;", [](const coreChar* pcToken) {glewDisableExtension(pcToken);});

    // check for minimum OpenGL support
    if(!GLEW_VERSION_2_0)            Core::Log->Warning("OpenGL 2.0 or higher not detected, application may not work properly");
    if(!GLEW_EXT_framebuffer_object) Core::Log->Warning("Frame Buffer Object support not detected, application may not work properly");
}


// ****************************************************************
/* exit OpenGL */
void __coreExitOpenGL()
{
    // delete remaining resource names from the pools
    CORE_GL_POOL_RESET(s_PoolTextures2D,   glDeleteTextures)
    CORE_GL_POOL_RESET(s_PoolBuffers,      glDeleteBuffers)
    CORE_GL_POOL_RESET(s_PoolVertexArrays, glDeleteTextures)
}


// ****************************************************************
/* get extension string */
void coreExtensions(coreString* OUTPUT psOutput)
{
    if(GLEW_VERSION_3_0)
    {
        // get number of available extensions
        GLint iNumExtensions = 0;
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
        if(!psOutput->empty()) psOutput->pop_back();
    }
    else
    {
        // get full extension string
        (*psOutput) = r_cast<const coreChar*>(glGetString(GL_EXTENSIONS));
    }
}


// ****************************************************************
/* get platform-specific extension string */
void corePlatformExtensions(coreString* OUTPUT psOutput)
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

    #define __LOAD_FUNCTION(x,y) decltype(x)* __ ## x = r_cast<decltype(x)*>(coreData::GetAddress(y, #x));
    {
        // open EGL library
        void* pLibraryEGL = coreData::OpenLibrary("libEGL.so");
        if(pLibraryEGL)
        {
            __LOAD_FUNCTION(eglGetCurrentContext, pLibraryEGL)
            __LOAD_FUNCTION(eglGetDisplay,        pLibraryEGL)
            __LOAD_FUNCTION(eglQueryString,       pLibraryEGL)

            if(__eglGetCurrentContext && __eglGetDisplay && __eglQueryString && (__eglGetCurrentContext() != EGL_NO_CONTEXT))
            {
                // open Wayland library
                void* pLibraryWL = coreData::OpenLibrary("libwayland-client.so");
                if(pLibraryWL)
                {
                    __LOAD_FUNCTION(wl_display_connect,    pLibraryWL)
                    __LOAD_FUNCTION(wl_display_disconnect, pLibraryWL)

                    if(__wl_display_connect && __wl_display_disconnect)
                    {
                        // open connection to default display
                        wl_display* pDisplay = __wl_display_connect(NULL);
                        if(pDisplay)
                        {
                            // get full extension string (EGL)
                            (*psOutput) = __eglQueryString(__eglGetDisplay(pDisplay), EGL_EXTENSIONS);

                            // close connection
                            __wl_display_disconnect(pDisplay);
                        }
                    }

                    // close Wayland library
                    coreData::CloseLibrary(pLibraryWL);
                }
            }
            else
            {
                // open X11 library
                void* pLibraryX = coreData::OpenLibrary("libX11.so");
                if(pLibraryX)
                {
                    __LOAD_FUNCTION(XOpenDisplay,  pLibraryX)
                    __LOAD_FUNCTION(XCloseDisplay, pLibraryX)

                    if(__XOpenDisplay && __XCloseDisplay)
                    {
                        // open connection to default display
                        Display* pDisplay = __XOpenDisplay(NULL);
                        if(pDisplay)
                        {
                            // get full extension string (GLX)
                            (*psOutput) = glXQueryExtensionsString(pDisplay, DefaultScreen(pDisplay));

                            // close connection
                            __XCloseDisplay(pDisplay);
                        }
                    }

                    // close X11 library
                    coreData::CloseLibrary(pLibraryX);
                }
            }

            // close EGL library
            coreData::CloseLibrary(pLibraryEGL);
        }
    }
    #undef __LOAD_FUNCTION

#else

    // return nothing
    (*psOutput) = "Unknown Platform Extensions";

#endif
}