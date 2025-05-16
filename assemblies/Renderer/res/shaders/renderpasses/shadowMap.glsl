// Shadow Mapping Shader
// Rodolphe VALICON
// 2025

#version 460 core

#stage vertex
// === VERTEX SHADER ===============================================================================
layout (std140, binding = 0) uniform Scene {
    mat4 ModelTransform;
    mat4 NormalTransform;
    mat4 ViewTransform;
    mat4 ProjectionTransform;
    vec3 EyePosition;
} uScene;

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float power;
    mat4 matrix;
};

uniform uint uLightIndex = 0;

layout (std430, binding = 0) buffer DirectionalLights {
    DirectionalLight[] gDirectionalLights;
};

layout (location = 0) in vec3 aPosition;

void main() {
    gl_Position = gDirectionalLights[uLightIndex].matrix * uScene.ModelTransform * vec4(aPosition, 1.0);
}

#stage fragment
// === FRAGMENT SHADER =============================================================================
void main() {}
