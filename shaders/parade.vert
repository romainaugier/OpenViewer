#version 430 core

uniform int width;
uniform int height;
uniform int imageWidth;
uniform int imageHeight;

uniform sampler2D tex;

uniform layout(binding=1, rgba32f) image2D uRGBParade;

void main()
{
    ivec2 i = ivec2(gl_VertexID % width, gl_VertexID / width);
    vec2 uv = vec2(i) * vec2(1.0 / float(width), 1.0 / float(height));

    vec4 imageColor = texture(tex, uv);
 
    ivec2 coordsR = ivec2(i.x, (1.0 - imageColor.x) * 500);
    vec4 colorR = imageLoad(uRGBParade, coordsR);
    colorR += vec4(0.1, 0.0, 0.0, 1.0);
    imageStore(uRGBParade, coordsR, colorR);
    
    ivec2 coordsG = ivec2(i.x, (1.0 - imageColor.y) * 500);
    vec4 colorG = imageLoad(uRGBParade, coordsG);
    colorG += vec4(0.0, 0.1, 0.0, 1.0);
    imageStore(uRGBParade, coordsG, colorG);
    
    ivec2 coordsB = ivec2(i.x,(1.0 - imageColor.z) * 500);
    vec4 colorB = imageLoad(uRGBParade, coordsB);
    colorB += vec4(0.0, 0.0, 0.1, 1.0);
    imageStore(uRGBParade, coordsB, colorB);
}