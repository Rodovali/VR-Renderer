// Equirectangular to Cubemap Shader
// Rodolphe VALICON
// 2024

#version 460 core

#stage vertex
// === VERTEX SHADER ===============================================================================

layout (location = 0) in vec3 aPos;

out vec3 Position;

uniform mat4 uProjection;
uniform mat4 uView;

void main() {
    Position = aPos;
    gl_Position = uProjection * uView * vec4(Position, 1.0);
}

#stage fragment
// === FRAGMENT SHADER =============================================================================

in vec3 Position;

out vec4 FragColor;

layout (binding = 0) uniform sampler2D sEquirectangular;

uniform float uExposureCorrection = 1.0;

const vec2 inv2Pi_Pi = vec2(0.1591, 0.3183);
vec2 DirectionToSphericalUV(vec3 dir) {
    vec2 uv = vec2(atan(dir.z, dir.x), asin(dir.y)); // Get direction angles
    uv *= inv2Pi_Pi; // Map angles to [-0.5, 0.5]
    uv += 0.5; // Map uv to [0, 1]

    return uv;
}

void main() {
    vec3 V = normalize(Position);
    vec2 uv = DirectionToSphericalUV(V);
    vec3 color = texture(sEquirectangular, uv).rgb;

    vec3 correctedColor = color * uExposureCorrection; 

    FragColor = vec4(correctedColor, 1.0);
}
