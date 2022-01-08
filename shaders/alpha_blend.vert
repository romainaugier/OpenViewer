#version 430 core

uniform int width;
uniform int height;
uniform int mode;

uniform sampler2D tex;

uniform layout(binding=1, rgba32f) image2D img;

vec3 checker(vec2 coords, vec2 size)
{
    float chk = sin(coords.x * size.x) * sin(coords.y * size.y);

    if (chk > 0.0) return vec3(0.2, 0.2, 0.2);
    else return vec3(0.3, 0.3, 0.3);
}

void main()
{
    ivec2 i = ivec2(gl_VertexID % width, gl_VertexID / width);
    vec2 uv = vec2(i) * vec2(1.0 / float(width), 1.0 / float(height));

    vec4 texColor = texture2D(tex, uv);

    vec3 colorToBlend = vec3(0.0, 0.0, 0.0);

    if (mode == 1) colorToBlend = vec3(0.25, 0.25, 0.25);
    else if (mode == 2) colorToBlend = checker(uv, vec2(width * 0.05, height * 0.05));

    vec3 color = texColor.xyz + colorToBlend * (1.0 - texColor.w);
    
    vec4 outColor = vec4(color.x, color.y, color.z, 1.0);

    imageStore(img, i, outColor);
}