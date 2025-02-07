#version 460 core

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64  : require


// SSBO transform data
// Keep sync with same struct at BMEdit/Editor/Source/Widgets/SceneRenderWidget.cpp
struct ObjectTransformDescription {
    mat4 Matrix;    // CPU: Write, GPU: Read
    vec4 BoundsMin; // CPU: Write, GPU: Read
    vec4 BoundsMax; // CPU: Write, GPU: Read
    vec4 Status;    // CPU: Read,  GPU: Write
};

layout(std430, binding = 0) buffer TransformBuffer {
    ObjectTransformDescription objectTransformDescriptions[];
};

// Materials
#define MAX_MATERIAL_TEXTURES 12

struct MaterialDescription {
    uint textures[MAX_MATERIAL_TEXTURES];
    vec4 zBiasOffset; // x - is enabled, y - offset, z - classid (u32), w - unused
};

layout(std430, binding = 2) buffer MaterialsBuffer {
    MaterialDescription materials[];
};

// Layout
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in uint aInMaterialID;

// Uniforms
uniform mat4 cameraProjView;

// Out
out vec2 g_TexCoord;
out flat uint g_MaterialID;

void main()
{
    // Send vars to FSH
    g_TexCoord = aUV;
    g_MaterialID = aInMaterialID;

    gl_Position = cameraProjView * objectTransformDescriptions[gl_BaseInstance].Matrix * vec4(aPos.xyz, 1.0);

    if (g_MaterialID != 0)
    {
        gl_Position.z -= materials[g_MaterialID - 1].zBiasOffset.y;
    }
}