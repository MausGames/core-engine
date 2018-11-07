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
    vec2 v2Alpha = coreTexture2D(1, v_av2TexCoord[0]).rg;
    gl_FragColor = vec4(vec3(0.15 + 0.85*v2Alpha.r), max(v2Alpha.r, v2Alpha.g)) * u_v4Color;
}