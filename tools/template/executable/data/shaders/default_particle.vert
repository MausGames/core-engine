///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////


void VertexMain()
{
    gl_Position      = coreParticlePosition();
    v_av2TexCoord[0] = coreParticleTexCoord();

    v_v4VarColor    = u_v4Color;
    v_v4VarColor.a *= a_v1DivValue;
}