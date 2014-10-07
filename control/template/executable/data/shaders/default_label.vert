//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////


void VertexMain()
{
    gl_Position      = coreObject2DPosition();
    v_av2TexCoord[0] = a_v2Texture * u_v2TexSize;
}