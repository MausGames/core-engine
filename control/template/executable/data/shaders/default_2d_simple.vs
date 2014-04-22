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
    gl_Position      = u_m4ScreenView * vec4(a_v3Position.xy, 1.0, 1.0);
    v_av2TexCoord[0] = a_v2Texture;
}