#version 460 core

#stage vertex
// === VERTEX SHADER ===============================================================================

layout (location = 0) in vec3 aPos;

layout (std140, binding = 0) uniform Matrices {
    mat4 modelTransform;
    mat4 normalTransform;
    mat4 viewTransform;
    mat4 projectionTransform;
    vec4 eyePosition;
};

out vec3 position;

void main() {
    position = aPos;
    gl_Position = (projectionTransform * vec4(mat3(viewTransform) * vec3(aPos), 1.0)).xyww;
}

#stage fragment
// === FRAGMENT SHADER =============================================================================

in vec3 position;
out vec4 FragColor;

uniform float uRoughness = 0.0;

layout (binding = 0) uniform samplerCube sEnvironment;

void main() {
    vec3 direction = normalize(position);
    float maxMip = textureQueryLevels(sEnvironment) - 1.0;

    vec3 color = textureLod(sEnvironment, direction, uRoughness * maxMip).rgb;

    FragColor = vec4(color, 1.0);
    // FragColor = vec4(1.0);
}
