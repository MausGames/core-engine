//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
// #version defined by application

#if (__VERSION__) <= 130 // <= OpenGL 3.0

    // shader input
    varying vec2 v_v2TexCoord;

    // shader output
    vec4 o_v4Color0;

#else

    // shader input
    in vec2 v_v2TexCoord;

    // shader output
    out vec4 o_v4Color0;

#endif

// texture uniforms
uniform sampler2D u_s2Texture;