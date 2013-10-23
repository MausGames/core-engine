//////////////////////////////////////////////////////////
//*----------------------------------------------------*//
//| Part of the Core Engine (http://www.maus-games.at) |//
//*----------------------------------------------------*//
//| Released under zlib License                        |//
//| More Information in the README.md and LICENSE.txt  |//
//*----------------------------------------------------*//
//////////////////////////////////////////////////////////
// #version defined by application

#if (__VERSION__) >= 140 // >= OpenGL 3.1

    layout(std140) uniform b_Global
    {
        // view uniforms
        mat4 u_mPerspective;
        mat4 u_mOrtho;

        // camera uniforms
        mat4 u_mCamera;
        vec3 u_v3CamDirection;

        // ambient uniforms
        vec3 u_v3LightDirection;
        vec4 u_v4LightValue;
    };

    // shader input
    in vec2 v_v2TexCoord;

    // shader output
    out vec4 o_v4Color0;
    out vec4 o_v4Color1;
    out vec4 o_v4Color2;
    out vec4 o_v4Color3;

#else

    // view uniforms
    uniform mat4 u_mPerspective;
    uniform mat4 u_mOrtho;

    // camera uniforms
    uniform mat4 u_mCamera;
    uniform vec3 u_v3CamDirection;

    // ambient uniforms
    uniform vec3 u_v3LightDirection;
    uniform vec4 u_v4LightValue;

    // shader input
    varying vec2 v_v2TexCoord;

    // shader output
    vec4 o_v4Color0;
    vec4 o_v4Color1;
    vec4 o_v4Color2;
    vec4 o_v4Color3;

#endif

// object uniforms
uniform mat4 u_mTransform;
uniform vec4 u_v4Color;
uniform vec2 u_v2TexSize;
uniform vec2 u_v2TexOffset;

// texture uniforms
uniform sampler2D u_s2Texture;