#version 450 core

in vec2 fUV;
in vec4 fColor;

out vec4 FragColor;

uniform sampler2D textAtlas;
uniform float outlineThickness;

void main() {
    vec4 outlineColor = vec4(0.0); // NOTE: Maybe will pass from CPU as uniform, idk
	float distance = texture(textAtlas, fUV).r;
    
    // Transform distance into world space
    float smoothing = fwidth(distance);
    
    // Calculate alpha for main part
    float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
    
    // Calculate alpha for outline
    float outlineDistance = 0.5 - outlineThickness;
    float outlineAlpha = smoothstep(outlineDistance - smoothing, outlineDistance + smoothing, distance);
    
    // Combine all
    vec4 textColor = fColor;
    vec4 finalColor = mix(outlineColor, textColor, alpha);
    finalColor.a = outlineAlpha;
    
    FragColor = finalColor;
}
