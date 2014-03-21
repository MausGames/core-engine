//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////


// precision qualifier
#ifdef GL_ES
    precision mediump float;
#endif


#if (__VERSION__) >= 140   // >= OpenGL 3.1

    // shader input
    in b_Varying {coreVertex v_sVertex;};

    // shader output
    out vec4 o_av4Color[CORE_SHADER_OUTPUT_COLORS];

#else

    // shader input
    varying coreVertex v_sVertex;

#endif


#line 1