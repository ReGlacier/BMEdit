#version 330 core
//
// This file is a part of BMEdit project
// Description: Basic shader to render textured entity
//

// Layout
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;

// Common
struct Camera
{
    mat4  proj;
    mat4  view;
    ivec2 resolution;
};

struct Transform
{
    mat4 model;
};

struct Material
{
    // See Common.fx for details
    // Common uniforms
    vec4 v4DiffuseColor;
    vec4 gm_vZBiasOffset;
    vec4 v4Opacity;
    vec4 v4Bias;
    float fZOffset;
    int alphaREF;

    // Textures
    sampler2D mapDiffuse;
    sampler2D mapSpecularMask;
    sampler2D mapEnvironment;
    sampler2D mapReflectionMask;
    sampler2D mapReflectionFallOff;
    sampler2D mapIllumination;
    sampler2D mapTranslucency;
};

uniform Material i_uMaterial;

// Uniforms
uniform Camera i_uCamera;
uniform Transform i_uTransform;

// Out
out vec2 g_TexCoord;

void main()
{
    vec4 vOut = i_uCamera.proj * i_uCamera.view * i_uTransform.model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    vOut -= i_uMaterial.gm_vZBiasOffset;
    vOut.z -= i_uMaterial.fZOffset;

    gl_Position = vOut;
    g_TexCoord = aUV;
}