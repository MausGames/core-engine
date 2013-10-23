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

    // vertex attributes
    in vec3 a_v3Position;
    in vec2 a_v2Texture;
    in vec3 a_v3Normal;
    in vec4 a_v4Tangent;

    // shader output
    out vec2 v_v2TexCoord;

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

    // vertex attributes
    attribute vec3 a_v3Position;
    attribute vec2 a_v2Texture;
    attribute vec3 a_v3Normal;
    attribute vec4 a_v4Tangent;

    // shader output
    varying vec2 v_v2TexCoord;

#endif

// object uniforms
uniform mat4 u_mTransform;
uniform vec4 u_v4Color;
uniform vec2 u_v2TexSize;
uniform vec2 u_v2TexOffset;