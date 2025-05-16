// Cook-Torrance Irradiance convolution Shader
// Rodolphe VALICON
// 2024

// Partly base on learnopengl.com IBL chapter
// Enhanced with importance sampling to remove fireflies artifacts for high-frequency details.

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

uniform float uRoughness = 0.0;

const float PI = 3.14159265359;
const vec3 W_UP = vec3(0.0, 1.0, 0.0);
const int SAMPLE_COUNT = 2048;

float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}  

vec3 ImportanceSampleGGX(vec2 Xi, vec3 N, float roughness)
{
    float a = roughness*roughness;
	
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
	
    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);
	
    vec3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

float DistributionGGX(float NdotH, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}


void main() {
    vec3 N = normalize(LocalPos);
    vec3 R = N;
    vec3 V = R;

    vec3 irradiance = vec3(0.0);

    if (uRoughness == 0.0) {
        irradiance = textureLod(sEnvironment, N, 0.0).rgb;
    } else {
        // Importance Sampling Monte Carlo Estimator
        float totalWeight = 0.0;
        for (int i = 0; i < SAMPLE_COUNT; ++i) {
            vec2 Xi = Hammersley(i, SAMPLE_COUNT);
            vec3 H = ImportanceSampleGGX(Xi, N, uRoughness);
            vec3 L  = normalize(2.0 * dot(V, H) * H - V);

            float NdotL = max(dot(N, L), 0.0);
            if (NdotL > 0.0) {
                float NdotH = dot(N, H);
                float HdotV = dot(H, V);
                float D   = DistributionGGX(NdotH, uRoughness);

                float pdf = (D * NdotH / (4.0 * HdotV)) + 0.0001; 

                float resolution = 2048.0; // resolution of source cubemap (per face)
                float saTexel  = 4.0 * PI / (6.0 * resolution * resolution);
                float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);

                float mip = 0.5 * log2(saSample / saTexel); 

                irradiance += textureLod(sEnvironment, L, mip).rgb * NdotL;
                totalWeight += NdotL;
            }
        }
        irradiance = irradiance / totalWeight;
    }

    FragColor = vec4(irradiance, 1.0);
}