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

// Layout
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in uint aInTexId;

// Uniforms
uniform mat4 cameraProjView;

// Out
out vec2 g_TexCoord;
out flat uint g_TexId;

void main()
{
    vec4 vOut = cameraProjView * objectTransformDescriptions[gl_BaseInstance].Matrix * vec4(aPos.x, aPos.y, aPos.z, 1.0);

    gl_Position = vOut;
    g_TexCoord = aUV;
    g_TexId = aInTexId;
}