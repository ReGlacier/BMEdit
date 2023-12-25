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

// Uniforms
uniform Camera i_uCamera;
uniform Transform i_uTransform;

// Out
out vec2 g_TexCoord;

void main()
{
    gl_Position = i_uCamera.proj * i_uCamera.view * i_uTransform.model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    g_TexCoord = aUV;
}