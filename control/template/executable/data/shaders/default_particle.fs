//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////


#if (__VERSION__) >= 300   // >= OpenGL 3.3 and OpenGL ES 3.0

    // shader input
    in vec4 v_v4Color;

#else

    // shader input
    varying vec4 v_v4Color;

#endif


void main()
{
    gl_FragColor = texture2D(u_as2Texture[0], v_sVertex.v_av2TexCoord[0]) * v_v4Color;
}
