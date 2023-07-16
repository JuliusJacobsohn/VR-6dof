#version 150
uniform mat4 matWVP;
uniform sampler2D depthbg;
uniform float radius;
in      vec4 Position;
in      vec4 Color;
in      vec2 TexCoord;
out     vec2 oTexCoord;
out     vec4 oColor;
///////////////////////////////////////////////
// Probably bounding sphere for user limits
/////////////////////////////////////////////////
void main()
{
//float dbbg = texture2D(depthbg, TexCoord).r;
//float imgdepth = 0.299999999999999999f / (dbbg + 0.001f);
float imgdepth = radius*10;
vec4 pos = vec4( Position.x * imgdepth, Position.y * imgdepth, Position.z * imgdepth, 1 );
gl_Position = (matWVP * pos);
oTexCoord   = TexCoord;
oColor.rgb = vec3(imgdepth);
oColor.a    = 1.0f;
}