#version 330 core
//
// This file is a part of BMEdit project
// Description: Basic shader to render textured entity
//
in vec2 g_TexCoord;

struct Material
{
    // See Common.fx for details
    // Common uniforms
    vec4 v4DiffuseColor;
    vec4 gm_vZBiasOffset;
    vec4 v4Opacity;
    vec4 v4Bias;
    float fAlphaREF;

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
    o_FragColor = texture(i_uMaterial.mapDiffuse, g_TexCoord);
}