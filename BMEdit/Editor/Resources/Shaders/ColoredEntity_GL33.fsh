#version 330 core
//
// This file is a part of BMEdit project
// Description: Basic shader to render color filled entity & gizmo
//

struct Material
{
    // See Common.fx for details
    // Common uniforms
    vec4 v4DiffuseColor;
    vec4 gm_vZBiasOffset;
    vec4 v4Opacity;
    vec4 v4Bias;
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

// Out
out vec4 o_FragColor;

void main()
{
    o_FragColor = i_uMaterial.v4DiffuseColor;
}