//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the README.md        |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////


void main()
{
    gl_FragColor = texture2D(u_as2Texture[0], v_av2TexCoord[0]);
    
    float fDetail = gl_FragColor.a;
    gl_FragColor *= vec4(fDetail, fDetail, fDetail, 1.0) * u_v4Color;
}