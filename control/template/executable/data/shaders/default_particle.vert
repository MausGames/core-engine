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
#ifdef _CORE_OPTION_NO_PARTICLE_ROTATION_
    
    vec3 v3Range = u_m3Normal * vec3(a_v3Position.xy * a_fDivScale, 0.0);
    
#else

    float fSin   = sin(a_fDivAngle);
    float fCos   = cos(a_fDivAngle);
    vec3 v3Range = u_m3Normal * vec3(mat2(fCos, fSin, -fSin, fCos) * (a_v3Position.xy * a_fDivScale), 0.0);

#endif
    
    gl_Position      = u_m4ViewProj * vec4(v3Range + a_v3DivPosition, 1.0);
    v_av2TexCoord[0] = a_v2Texture;

    v_v4VarColor    = coreUnpackUnorm4x8(a_iDivColor);
    v_v4VarColor.a *= a_fDivValue;
}
