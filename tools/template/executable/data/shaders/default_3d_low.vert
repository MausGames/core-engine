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
    gl_Position      = coreObject3DPositionLow();
    v_av2TexCoord[0] = coreObject3DTexCoordLow();
}