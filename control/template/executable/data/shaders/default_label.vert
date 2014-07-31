//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////


void main()
{
    vec3 v3Position = u_m3ScreenView * vec3(a_v3Position.xy, 1.0);
    gl_Position     = vec4(v3Position.xy, 0.0, v3Position.z);
    
    v_av2TexCoord[0] = a_v2Texture * u_v2TexSize;
}