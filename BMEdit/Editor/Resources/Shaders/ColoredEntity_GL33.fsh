#version 330 core
//
// This file is a part of BMEdit project
// Description: Basic shader to render color filled entity & gizmo
//

uniform vec4 i_Color;

// Out
out vec4 o_FragColor;

void main()
{
    o_FragColor = i_Color;
}