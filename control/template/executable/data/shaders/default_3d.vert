//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under the zlib License                    |//
//| More information available in the readme file      |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////


#ifdef _CORE_OPTION_INSTANCING_

    void main()
    {
        gl_Position  = u_m4Perspective * a_m4DivModelView * vec4(a_v3Position, 1.0);
        v_v4VarColor = coreUnpackUnorm4x8(a_iDivColor);
        
    #ifdef _CORE_OPTION_NO_TEXTURE_TRANSFORM_
        v_av2TexCoord[0] = a_v2Texture;
    #else
        v_av2TexCoord[0] = a_v2Texture * a_v2DivTexSize + a_v2DivTexOffset;
    #endif
    }

#else

    void main()
    {
        gl_Position = u_m4ModelViewProj * vec4(a_v3Position, 1.0);
        
    #ifdef _CORE_OPTION_NO_TEXTURE_TRANSFORM_
        v_av2TexCoord[0] = a_v2Texture;
    #else
        v_av2TexCoord[0] = a_v2Texture * u_v2TexSize + u_v2TexOffset;
    #endif
    }

#endif
