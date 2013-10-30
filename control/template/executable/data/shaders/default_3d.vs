//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
// #version defined by application

void main()
{
    gl_Position  = u_mPerspective * u_mCamera * u_mTransform * vec4(a_v3Position, 1.0);
    v_v2TexCoord = a_v2Texture;
}