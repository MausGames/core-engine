///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Released under the zlib License                     |//
//| More information available in the readme file       |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////


void FragmentMain()
{
    float v1Alpha = coreTexture2D(1, v_av2TexCoord[0]).r;
    gl_FragColor  = vec4(u_v4Color.rgb, u_v4Color.a * v1Alpha);
}