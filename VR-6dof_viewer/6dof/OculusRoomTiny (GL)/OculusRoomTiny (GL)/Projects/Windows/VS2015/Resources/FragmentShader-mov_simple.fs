#version 150
uniform sampler2D fgtext;
uniform sampler2D alphamask;
uniform sampler2D Fragfgdepth;
uniform float desat;
uniform vec3 spherecenter;
uniform vec3 eyeposition;
uniform vec3 headposition;
uniform float colored;

in vec4 oColor2;
in vec2 oTexCoord2;
in vec3 world_Position2;
in vec3 ViewDir2;
in vec3 CurrViewDir;
out vec4 FragColor;

#define M_E  2.7182818284590452353602874713526

///////////////////////////////////////////////
// Main
// Foreground layer
/////////////////////////////////////////////////
void main()
{
    float dist = 0.05+sqrt(pow(headposition.y-spherecenter.y,2) + pow(headposition.z-spherecenter.z,2) + pow(headposition.z-spherecenter.z,2));
    vec3 rgbcolor = texture2D(fgtext, oTexCoord2).rgb;
    float alpha = texture2D(alphamask, oTexCoord2).r;
    
    float k = 30.0;
    float c = 0.15;
    float Scurve = 1.0 / (1.0 + pow(M_E, -k*(dist-c)));
    
    float corrected  = abs((1-Scurve)*1.0 + alpha*(Scurve));
    
    FragColor = vec4(rgbcolor, corrected);
    if (colored != 0)
    {
        FragColor.r = 1.0;
    }
}