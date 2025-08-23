#version 450 core
in vec2 fUV;
in vec4 fColor;
out vec4 FragColor;
uniform sampler2D textAtlas;
uniform float outlineThickness;
void main()
{
    float sdf = texture(textAtlas, fUV).r;
    float width = fwidth(sdf);
    float alpha = smoothstep(0.5 - width, 0.5 + width, sdf);
    float outline = smoothstep(0.5 - outlineThickness*width, 0.5 - width, sdf);
    vec3 color = mix(vec3(0.0), fColor.rgb, alpha);
    FragColor = vec4(mix(color, vec3(0.0), outline), max(alpha, outline));
}
