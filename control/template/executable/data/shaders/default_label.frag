//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////


void FragmentMain()
{
    float v1Alpha = coreTexture2D(0, v_av2TexCoord[0]).r;
    gl_FragColor  = u_v4Color * v1Alpha;
}