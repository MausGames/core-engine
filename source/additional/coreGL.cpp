//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
#include "Core.h"

thread_local GLEWContext g_GlewContext; // = 0;


// ****************************************************************
/* init OpenGL */
void __coreInitOpenGL()
{
    // init GLEW on current context
    const GLenum iError = glewInit();
    if(iError != GLEW_OK) Core::Log->Error("GLEW could not be initialized (GLEW: %s)", glewGetErrorString(iError));
                     else Core::Log->Info ("GLEW initialized (%s)",                    glewGetString(GLEW_VERSION));

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
        GLEW_EXT_framebuffer_object      = true;
        GLEW_EXT_framebuffer_blit        = true;
        GLEW_EXT_framebuffer_multisample = true;
        GLEW_EXT_texture_array           = true;
        GLEW_EXT_packed_depth_stencil    = true;
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
        GLEW_ARB_instanced_arrays = true;
    }

    // force additional extension status
    if( GLEW_VERSION_2_1 || GLEW_EXT_pixel_buffer_object) GLEW_ARB_pixel_buffer_object = true;
    if( GLEW_VERSION_3_2 || GLEW_EXT_geometry_shader4)    GLEW_ARB_geometry_shader4    = true;
    if(!GLEW_VERSION_3_0 || Core::Config->GetBool(CORE_CONFIG_GRAPHICS_FALLBACKMODE))
        GLEW_ARB_uniform_buffer_object = false;

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
        glShaderSource       = (PFNGLSHADERSOURCEPROC)glShaderSourceARB;
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
    if(!GLEW_ARB_vertex_program           ||
       !GLEW_ARB_vertex_shader            ||
       !GLEW_ARB_fragment_shader          ||
       !GLEW_ARB_shader_objects           ||
       !GLEW_ARB_shading_language_100     ||
       !GLEW_ARB_vertex_buffer_object     ||
       !GLEW_ARB_texture_non_power_of_two ||
       !GLEW_EXT_draw_range_elements)
        Core::Log->Warning("Minimum OpenGL requirements not met, application may not work properly");

    // check for frame buffer object support
    if(!GLEW_EXT_framebuffer_object)
        Core::Log->Warning("Frame Buffer Objects not supported, application may not work properly");
}