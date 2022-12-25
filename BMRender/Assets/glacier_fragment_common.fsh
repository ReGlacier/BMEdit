#version 330 core

// Input
in vec2 g_TexCoord;  /// Unused for now

// Result
out vec4 o_FragColor;

// Uniforms

void main()
{
    o_FragColor = vec4(1.0, 1.0, 0.0, 1.0);
}