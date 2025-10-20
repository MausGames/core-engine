///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////


// ****************************************************************
/* set compiler warnings */
#if defined(_MSC_VER)

    #pragma warning(push, 3)

    #pragma warning(disable : 4191)   // unsafe conversion between function types
    #pragma warning(disable : 4711)   // function automatically inlined
    #pragma warning(disable : 5045)   // possible Spectre vulnerability

#endif


// ****************************************************************
/* include library code */
#define GLEW_NO_GLU
#define GLEW_STATIC

#include <GL/glew.c>


// ****************************************************************
/* reset compiler warnings */
#if defined(_MSC_VER)

    #pragma warning(pop)

#endif
