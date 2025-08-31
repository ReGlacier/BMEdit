#version 450 core
layout(location = 0) in vec2 vPos;
layout(location = 1) in vec2 vUV;
layout(location = 2) in vec4 vColor;
out vec2 fUV;
out vec4 fColor;
void main()
{
    gl_Position = vec4(vPos, 0.0, 1.0);
    fUV = vUV;
    fColor = vColor;
}
