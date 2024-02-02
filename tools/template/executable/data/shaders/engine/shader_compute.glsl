///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////


#if defined(_CORE_COMPUTE_SHADER_)

    // main function
    void ComputeMain();
    void ShaderMain()
    {
        ComputeMain();
    }

#endif // _CORE_COMPUTE_SHADER_