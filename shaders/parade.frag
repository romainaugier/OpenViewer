#version 430 core

in vec2 texCoord;

uniform sampler2D tex;

uniform layout(binding=1, rgba32f) image2D uRGBParade;

void main()
{
    vec4 imageColor = texture(tex, vec2(gl_FragCoord.x / 1000, gl_FragCoord.y / 500));
 
    ivec2 coordsR = ivec2(gl_FragCoord.x, (1.0 - imageColor.x) * 500);
    vec4 colorR = imageLoad(uRGBParade, coordsR);
    colorR += vec4(1.0, 0.0, 0.0, 1.0);
    imageStore(uRGBParade, coordsR, colorR);
    
    ivec2 coordsG = ivec2(gl_FragCoord.x, (1.0 - imageColor.y) * 500);
    vec4 colorG = imageLoad(uRGBParade, coordsG);
    colorG += vec4(0.0, 1.0, 0.0, 1.0);
    imageStore(uRGBParade, coordsG, colorG);
    
    ivec2 coordsB = ivec2(gl_FragCoord.x,(1.0 - imageColor.z) * 500);
    vec4 colorB = imageLoad(uRGBParade, coordsB);
    colorB += vec4(0.0, 0.0, 1.0, 1.0);
    imageStore(uRGBParade, coordsB, colorB);
}