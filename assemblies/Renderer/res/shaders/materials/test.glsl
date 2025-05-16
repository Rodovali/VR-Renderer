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

layout (location = 0) in vec2 aPosition;

void main() {
    vec3 position = vec3(aPosition, 0.0);
    gl_Position = projectionTransform * viewTransform * vec4(position, 1.0);
}

#stage fragment
// === FRAGMENT SHADER =============================================================================


out vec4 FragColor;

void main() {
    if (gl_FrontFacing) {
        FragColor = vec4(1.0);
    } else {
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
}
