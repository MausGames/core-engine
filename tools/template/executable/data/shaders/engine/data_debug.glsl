///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////


#if defined(_CORE_DEBUG_)

    #if defined(CORE_GL_uniform_buffer_object)

        // debug uniforms
        layout(std140) uniform b_Debug
        {
            layoutEx(align = 16) highp vec4 u_av4DebugValue[CORE_NUM_DEBUGS];
        };

    #endif

#endif