// Textured Material
// Rodolphe VALICON
// 2024

#version 460 core

layout (std140, binding = 0) uniform Matrices {
    mat4 modelTransform;
    mat4 normalTransform;
    mat4 viewTransform;
    mat4 projectionTransform;
    vec3 eyePosition;
};

#stage vertex
// ==== VERTEX SHADER ==============================================================================

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoords;

out vec2 UV;

void main() {
    vec3 position = vec3(modelTransform * vec4(aPosition, 1.0));
    UV = aTexCoords;
    gl_Position = projectionTransform * viewTransform * vec4(aPosition, 1.0);
}

#stage fragment
// === FRAGMENT SHADER =============================================================================

in vec2 UV;

out vec4 FragColor;

layout (binding = 1) uniform sampler2D sTexture;

void main() {
    vec3 color = texture(sTexture, UV).rgb;
    
    FragColor = vec4(color, 1.0);
}
