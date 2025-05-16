// Pure Uniform Lambertian Material PBR Shader
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

layout (std140, binding = 2) uniform Material {
    vec3 diffuse;
};

#stage vertex
// ==== VERTEX SHADER ==============================================================================

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;

void main() {
    vec3 position = vec3(modelTransform * vec4(aPosition, 1.0));
    Normal = vec3(normalTransform * vec4(aNormal, 0.0));
    gl_Position = projectionTransform * viewTransform * vec4(position, 1.0);
}

#stage fragment
// === FRAGMENT SHADER =============================================================================

in vec3 Normal;

out vec4 FragColor;

layout (binding = 0) uniform samplerCube sEnvironment;

void main() {
    vec3 N = normalize(Normal);
    float maxMip = textureQueryLevels(sEnvironment) - 1.0;
    vec3 irradiance = textureLod(sEnvironment, N, maxMip).rgb;
    vec3 Lo = diffuse * irradiance;

    FragColor = vec4(Lo, 1.0);
}
