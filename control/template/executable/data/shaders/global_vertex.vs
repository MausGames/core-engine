//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////


#if (__VERSION__) >= 140   // >= OpenGL 3.1

    // vertex attributes
    in vec3 a_v3Position;
    in vec2 a_v2Texture;
    in vec3 a_v3Normal;
    in vec4 a_v4Tangent;

    // shader output
    out b_Varying {coreVertex v_sVertex;};

#else

    // vertex attributes
    attribute vec3 a_v3Position;
    attribute vec2 a_v2Texture;
    attribute vec3 a_v3Normal;
    attribute vec4 a_v4Tangent;

    // shader output
    varying coreVertex v_sVertex;

#endif


#line 1