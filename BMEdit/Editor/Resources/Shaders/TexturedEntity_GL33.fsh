#version 460 core

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64  : require

// SSBO textures
layout(std430, binding = 1) buffer TexturesBuffer {
    sampler2D textures[];
};

// Texture parameter
in vec2 g_TexCoord;
in flat uint g_TexId;

// Out
out vec4 o_FragColor;

void main()
{
    if (g_TexId != 0)
    {
        // DronCode: In our case, we should reserve a few textures for error & unsupported materials view
        //
        o_FragColor = texture(textures[g_TexId - 1], g_TexCoord);
    }
    else discard;
    //o_FragColor = vec4(g_TexCoord, 0.0, 1.0);
}