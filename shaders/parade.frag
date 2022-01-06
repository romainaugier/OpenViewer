#version 430 core

in vec2 texCoord;

uniform sampler2D tex;

uniform layout(binding=1, rgba32f) image2D uRGBParade;

void main()
{
    ivec2 coords = ivec2(gl_FragCoord.xy);
 
    vec4 color = texture(tex, texCoord);

    imageStore(uRGBParade, coords, color);
}