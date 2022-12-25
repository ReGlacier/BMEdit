#version 330 core

// Attributes
#if defined(GLACIER_USE_FORMAT_10)
layout(location = 0) in vec3 aPos; // X,Y,Z position
layout(location = 1) in int aUnk1; // Some unknown data, maybe color ?
#elif defined(GLACIER_USE_FORMAT_24)
layout(location = 0) in vec3  aPos; // X,Y,Z position
layout(location = 1) in vec2  aUV;  // U, V position
layout(location = 2) in ivec2 unk1; // Unknown data (int[2])
layout(location = 3) in vec2  aUV2; // U1, V1 position
#elif defined(GLACIER_USE_FORMAT_28)
layout(location = 0) in vec3  aPos; // X,Y,Z position
layout(location = 1) in ivec2 unk1; // Unknown data (int[2])
layout(location = 2) in vec2  aUV;  // U, V position
layout(location = 3) in ivec3 unk2; // Unknown data (int[3])
#elif defined(GLACIER_USE_FORMAT_34)
layout(location = 0) in vec3  aPos; // X,Y,Z position
layout(location = 1) in vec3  unk1; // Unknown data (float[3])
layout(location = 2) in ivec3 unk2; // Unknown data (int[3])
layout(location = 3) in vec2  aUV;  // U, V
layout(location = 4) in ivec2 unk3; // Unknown data (int[2])
#else
#error "UNKNOWN VERTEX FORMAT!"
#endif

// MVP
uniform mat4 i_Model;
uniform mat4 i_ProjView;

// Output
out vec2 g_TexCoord;

void main() {
    gl_Position = i_ProjView * i_Model * vec4(aPos, 1.0);
    g_TexCoord = vec2(0.0);
}
