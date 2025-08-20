#version 460 core

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64  : require

// Material
#define TEXTURE_DIFFUSE 1
#define TEXTURE_DIFFUSE_MASK 2
#define TEXTURE_NORMAL 3
#define TEXTURE_NORMAL_DETAIL 4
#define MAX_MATERIAL_TEXTURES 12

struct MaterialDescription {
    // #0 - usage mask (uint32):  textures[MAX_MATERIAL_TEXTURES] & (1 << TEXTURE_DIFFUSE) to check that normal texture presented
    uint textures[MAX_MATERIAL_TEXTURES];
    vec4 zBiasOffset; // x - is enabled, y - offset, z - classid (u32), w - unused
};

// SSBO textures
layout(std430, binding = 1) buffer TexturesBuffer {
    sampler2D textures[];
};

layout(std430, binding = 2) buffer MaterialsBuffer {
    MaterialDescription materials[];
};

// Texture parameter
in vec2 g_TexCoord;
in flat uint g_MaterialID;

// Out
out vec4 o_FragColor;


void main()
{
    uint materialClass = 0;

    if (g_MaterialID == 0)
    {
        discard;
    }

    // get mat class
    materialClass = uint(materials[g_MaterialID - 1].zBiasOffset.z) & 0x0000FFFFu;

    // Generic texture
    if (bool(materials[g_MaterialID - 1].textures[0] & (1 << TEXTURE_DIFFUSE)))
    {
        vec4 color = texture(textures[materials[g_MaterialID - 1].textures[TEXTURE_DIFFUSE]], g_TexCoord);
        o_FragColor = color;
    }
    else discard;
}