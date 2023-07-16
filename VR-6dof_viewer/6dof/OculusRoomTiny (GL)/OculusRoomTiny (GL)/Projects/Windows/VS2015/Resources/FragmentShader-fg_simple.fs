#version 150
uniform sampler2D fgtext;
uniform float desat;
uniform float colored;
in vec4 oColor2;
in vec2 oTexCoord2;
out vec4 FragColor;
uniform sampler2D alphamask;

///////////////////////////////////////////////
// Background layer
/////////////////////////////////////////////////
void main()
{
    vec3 rgbcolor = texture2D(fgtext, oTexCoord2).rgb;
    float mask = texture2D(alphamask, oTexCoord2).r;
    FragColor = vec4(rgbcolor, mask);
    if (colored != 0)
    {
        FragColor.g = 1.0;
    }

}
