#version 150
uniform sampler2D bgtext;
uniform float desat;
uniform float colored;
in vec4 oColor;
in vec2 oTexCoord;
out vec4 FragColor;


///////////////////////////////////////////////
// Inpainting layer
/////////////////////////////////////////////////
void main()
{
    vec3 rgbcolor = texture2D(bgtext, oTexCoord).rgb;
    FragColor = vec4(rgbcolor, 1.0);
    if (colored != 0)
    {
        FragColor.b = 1.0;
    }

}