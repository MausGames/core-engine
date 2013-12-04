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
    gl_Position      = u_m4ModelViewProj * vec4(a_v3Position, 1.0);
    v_av2TexCoord[0] = a_v2Texture * u_v2TexSize + u_v2TexOffset;
}