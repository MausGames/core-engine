uniform mat4 u_mPerspective;
uniform mat4 u_mOrtho;
uniform mat4 u_mCamera;
uniform mat4 u_mTransform;

attribute vec3 a_v3Position;
attribute vec2 a_v2Texture;
attribute vec3 a_v3Normal;
attribute vec4 a_v4Tangent;

varying vec2 v_v2TexCoord;

void main()
{
    gl_Position  = u_mPerspective * u_mCamera * u_mTransform * vec4(a_v3Position, 1.0);
    v_v2TexCoord = a_v2Texture;
}