///////////////////////////////////////////////////////////
//*-----------------------------------------------------*//
//| Part of the Core Engine (https://www.maus-games.at) |//
//*-----------------------------------------------------*//
//| Copyright (c) 2013 Martin Mauersics                 |//
//| Released under the zlib License                     |//
//*-----------------------------------------------------*//
///////////////////////////////////////////////////////////
#if defined(_DEBUG) || (defined(__GNUC__) && !defined(__OPTIMIZE__))


// ****************************************************************
/* set compiler warnings */
#if defined(_MSC_VER)

    #pragma warning(push, 3)

    #pragma warning(disable : 4355)   // 'this' used in base member initializer list
    #pragma warning(disable : 4514)   // unreferenced inline function removed
    #pragma warning(disable : 5039)   // potentially throwing function passed to extern C function
    #pragma warning(disable : 5045)   // possible Spectre vulnerability
    #pragma warning(disable : 5219)   // implicit conversion between different types
    #pragma warning(disable : 5262)   // implicit fall-through

#endif


// ****************************************************************
/* use own OpenGL loader */
#if defined(__ANGLE__)

    #define IMGUI_IMPL_OPENGL_ES3

#elif !defined(__EMSCRIPTEN__)

    #define IMGUI_IMPL_OPENGL_LOADER_CUSTOM

    #define GLEW_NO_GLU
    #define GLEW_STATIC

    #include <GL/glew.h>

#endif


// ****************************************************************
/* include library code */
#include <imgui/imgui.cpp>
#include <imgui/imgui_draw.cpp>
#include <imgui/imgui_tables.cpp>
#include <imgui/imgui_widgets.cpp>
#include <imgui/imgui_impl_sdl3.cpp>
#include <imgui/imgui_impl_opengl3.cpp>


// ****************************************************************
/* reset compiler warnings */
#if defined(_MSC_VER)

    #pragma warning(pop)

#endif


#endif