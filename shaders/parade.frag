#version 430 core

out vec4 FragColor;

in vec2 texCoord;

// uniform sampler2D tex;

layout(binding=0, rgba8ui) uniform uimage2D uRGBParade;

void main()
{
    //FragColor = texture(tex, texCoord) + vec4(0.5f, 0.0f, 0.0f, 1.0f);

    imageStore(uRGBParade, ivec2(texCoord), uvec4(255.0, 0.0, 0.0, 255.0));
}