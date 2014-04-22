//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////


void main()
{
    gl_Position      = u_m4ModelViewProj * vec4(a_v3Position, 1.0);
    v_av2TexCoord[0] = a_v2Texture;
}