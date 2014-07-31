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
        gl_FragColor = texture2D(u_as2Texture[0], v_av2TexCoord[0]) * v_v4VarColor;
    }

#else

    void main()
    {
        gl_FragColor = texture2D(u_as2Texture[0], v_av2TexCoord[0]) * u_v4Color;
    }

#endif
