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
    gl_FragColor = vec4(mix(vec3(0.1), u_v4Color.rgb, v2Alpha.r), u_v4Color.a * max(v2Alpha.r, v2Alpha.g));
}