// Shadow Cube Mapping Shader
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

struct PointLight {
    vec3 position;
    vec3 color;
    float power;
    float radius;
};

layout (std430, binding = 1) buffer PointLights {
    PointLight[] gPointLights;
};

uniform uint uLightIndex = 0;
uniform mat4 uLightViewProj;

layout (location = 0) in vec3 aPosition;

out vec3 vPosition;
out vec3 lightPosition;

void main() {
    vec4 position = uScene.ModelTransform * vec4(aPosition, 1.0);
    vPosition = position.xyz;
    lightPosition = gPointLights[uLightIndex].position;

    gl_Position = uLightViewProj * position;
}

#stage fragment
// === FRAGMENT SHADER =============================================================================
in vec3 vPosition;
in vec3 lightPosition;

void main() {
    if (!gl_FrontFacing) discard;

    // Linear depth
    float d = length(vPosition - lightPosition);

    // Map to [0.0, 1.0] range
    gl_FragDepth = d / 100.0;
}
