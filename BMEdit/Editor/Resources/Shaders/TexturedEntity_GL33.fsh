#version 330 core
//
// This file is a part of BMEdit project
// Description: Basic shader to render textured entity
//

uniform sampler2D i_ActiveTexture;
in vec2 g_TexCoord;

// Out
out vec4 o_FragColor;

void main()
{
    o_FragColor = texture(i_ActiveTexture, g_TexCoord);
}