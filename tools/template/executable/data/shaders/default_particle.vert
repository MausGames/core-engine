///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#include "engine/data_transform_3d.glsl"


void VertexMain()
{
    gl_Position      = coreParticlePosition();
    v_av2TexCoord[0] = coreParticleTexCoord();

    v_v4VarColor    = u_v4Color;
    v_v4VarColor.a *= a_v1DivValue;
}