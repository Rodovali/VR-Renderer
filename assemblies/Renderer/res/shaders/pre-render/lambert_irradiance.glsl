// Lambert Irradiance convolution Shader
// Rodolphe VALICON
// 2024

#version 460 core

#stage vertex
// === VERTEX SHADER ===============================================================================
layout(location = 0) in vec3 aPos;

out vec3 LocalPos;

uniform mat4 uProjection;
uniform mat4 uView;

void main() {
    LocalPos = aPos;
    gl_Position = uProjection * uView * vec4(LocalPos, 1.0);
}

#stage fragment
// === FRAGMENT SHADER =============================================================================
in vec3 LocalPos;

out vec4 FragColor;

layout(binding = 0) uniform samplerCube sEnvironment;

const float PI = 3.14159265359;
const vec3 W_UP = vec3(0.0, 1.0, 0.0);
const float SAMPLE_DELTA = 0.025;
const float MIP = 5.0;

void main() {
    vec3 normal = normalize(LocalPos);
    vec3 right = normalize(cross(W_UP, normal));
    vec3 up = normalize(cross(normal, right));

    vec3 irradiance = vec3(0.0);

    // Uniform Monte Carlo Estimator
    float nrSamples = 0.0; 
    for(float phi = 0.0; phi < 2.0 * PI; phi += SAMPLE_DELTA) {
        for(float theta = 0.0; theta < 0.5 * PI; theta += SAMPLE_DELTA) {
            // Spherical to cartesian (in tangent space)
            vec3 tangentVec = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));
            // Tangent space to world space
            vec3 sampleVec = tangentVec.x * right + tangentVec.y * up + tangentVec.z * normal; 

            // TODO: (Anti-Aliasing) Calculate needed mip level depending on SAMPLE_DELTA and 
            // cubemap resolution to achieve Nyquist criterion
            vec3 color = textureLod(sEnvironment, sampleVec, MIP).rgb;

            irradiance += color * cos(theta) * sin(theta);
            nrSamples++;
        }
    }
    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    FragColor = vec4(irradiance, 1.0);
}