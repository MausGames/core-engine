uniform sampler2D u_s2Texture;

varying vec2 v_v2TexCoord;

void main()
{
    gl_FragColor = texture2D(u_s2Texture, v_v2TexCoord);
}