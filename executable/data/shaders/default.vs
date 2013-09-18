#version 130

uniform mat4 u_mPerspective;
uniform mat4 u_mOrtho;
uniform mat4 u_mCamera;

in vec3 in_v3Position;
in vec2 in_v2Texture;
in vec3 in_v3Normal;
in vec4 in_v4Tangent;


void main()
{
    gl_Position = u_mPerspective * u_mCamera * vec4(in_v3Position, 1.0);
}


