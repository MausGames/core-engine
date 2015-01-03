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
    v_av2TexCoord[0] = vec2(0.5+a_v3RawPosition.x, 0.5-a_v3RawPosition.y) * u_v2TexSize;
}