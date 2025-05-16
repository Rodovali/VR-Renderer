// Metallic Roughness Physical Based Rendering Shader
// Rodolphe VALICON
// 2025

// Partly based on the learnopengl.com PBR chapter
// Enhanced with personal research

#version 460 core

#define MAX_SHADOW_CASTERS 4

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

struct PointLight {
    vec3 position;
    vec3 color;
    float power;
    float radius;
};

layout (std430, binding = 0) buffer DirectionalLighting {
    DirectionalLight[] gDirectionalLights;
};

layout (std430, binding = 1) buffer PointLighting {
    PointLight[] gPointLights;
};

#stage vertex
// === VERTEX SHADER ===============================================================================
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aTangent;
layout (location = 3) in vec2 aTexCoord;

out vec3 vPosition;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitangent;
out vec2 vUV;
out vec3 vLightPosition[MAX_SHADOW_CASTERS];

void main() {
    vPosition = vec3(uScene.ModelTransform * vec4(aPosition, 1.0));
    vNormal = mat3(uScene.NormalTransform) * aNormal;
    vTangent = mat3(uScene.NormalTransform) * aTangent.xyz;
    vBitangent = mat3(uScene.NormalTransform) * aTangent.w * cross(aNormal, aTangent.xyz);
    vUV = aTexCoord;

    for (uint i = 0; i < gDirectionalLights.length() && i < MAX_SHADOW_CASTERS; ++i) {
        vec4 lightCoords = gDirectionalLights[i].matrix * vec4(vPosition, 1.0);
        vLightPosition[i] = lightCoords.xyz / lightCoords.w;
    }
    
    gl_Position = uScene.ProjectionTransform * uScene.ViewTransform * vec4(vPosition, 1.0);
}

#stage fragment
// === FRAGMENT SHADER =============================================================================
#define PI 3.141592653589793

in vec3 vPosition;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vBitangent;
in vec2 vUV;
in vec3 vLightPosition[MAX_SHADOW_CASTERS];

out vec4 fColor;

layout (std140, binding = 2) uniform PBRMaterial {
    bool AlbedoMap;
    bool MetalRoughnessMap;
    bool NormalMap;
    bool OcclusionMap;
    bool EmissiveMap;
    float MetallicFactor;
    float RoughnessFactor;
    vec3 AlbedoFactor;
    vec3 EmissiveFactor;
    float AlphaMasking;
} uMaterial;

// -- Texture samplers --
layout (binding = 0) uniform samplerCube sEnvironment;
layout (binding = 1) uniform sampler2DArray sShadowMaps;
layout (binding = 2) uniform samplerCubeArray sShadowCubeMaps;
layout (binding = 3) uniform sampler2D sBRDF_LUT;
layout (binding = 4) uniform sampler2D sAlbedoMap;
layout (binding = 5) uniform sampler2D sMetalRoughnessMap;
layout (binding = 6) uniform sampler2D sOcclusionMap;
layout (binding = 7) uniform sampler2D sEmissiveMap;
layout (binding = 8) uniform sampler2D sNormalMap;

// Shadow maps offset for Cube Map PCF
const vec3 sampleOffsetDirections[20] = vec3[]
(
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);   

// Computes the irradiance map level corresponding to roughness.
float MipFromRoughness(float roughness) {
    return roughness * (textureQueryLevels(sEnvironment) - 1.0);
}

// Towbridge-Reitz GGX Microfacet Normal Distribution function.
float DistributionGGX(float cosTheta, float a) {
    float a2 = a * a;
    float cos2Theta = cosTheta * cosTheta;

    float nom = a2;
    float denom = cos2Theta * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return nom / denom;
}

// Microfacet shading/masking distribution function for direct lighting.
float GeometrySchlickGGX(float cosTheta, float a) {
    // Roughness remap for direct lighting
    float k = (a + 1.0) * (a + 1.0) / 8.0;

    float nom = cosTheta;
    float denom = cosTheta * (1.0 - k) + k;

    return nom / denom;
}

// Computes the combined microfacet shading and masking factor using Smith Method. 
float GeometrySmith(float cosThetaO, float cosThetaI, float a) {
    float ggx1 = GeometrySchlickGGX(cosThetaO, a);
    float ggx2 = GeometrySchlickGGX(cosThetaI, a);

    return ggx1 * ggx2;
}

// Schlick's approximation of Fresnel reflection equation.
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Shlick's approximation of Fresnel reflection equation with roughness > 0.0.
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float ComputeShadow(in DirectionalLight light, in vec3 lightCoords, in uint mapLayer, in float cosThetaL) {
    const int shadowMapLayers = textureSize(sShadowMaps, 0).z;
    const vec2 shadowTexelSize = 1.0 / textureSize(sShadowMaps, 0).xy;
    const float actualLayer = max(0, min(shadowMapLayers - 1, floor(mapLayer + 0.5)));
    
    float shadow = 0.0;

    // Check if fragment is inside light clip space.
    if (lightCoords.z <= 1.0) {
        // Remap light coordinates to texture coordinates.
        const vec3 lightUVs = (lightCoords + 1.0) * 0.5;

        // Compute a bias depending on light angle to avoid shadow acnee.
        const float bias = max(2e-4 * (1.0 - cosThetaL), 1e-4);    
        
        // PCF: Filter shadow map using a moving uniform averaging kernel.
        const int KERNEL_SZ = 2;
        for (int x = -KERNEL_SZ; x <= KERNEL_SZ; ++x) {
            for (int y = -KERNEL_SZ; y <= KERNEL_SZ; ++y) {
                float closestDepth = texture(sShadowMaps, vec3(lightUVs.xy + vec2(x, y) * shadowTexelSize.xy, actualLayer)).r;
                shadow += lightUVs.z - bias > closestDepth ? 1.0 : 0.0;
            }
        }
        shadow /= ((KERNEL_SZ * 2) + 1) * ((KERNEL_SZ * 2) + 1);
    }

    return shadow;
}

float ComputeCubeShadow(in PointLight light, in vec3 Lvec, in uint mapLayer, in float cosThetaL) {
    const int shadowMapLayers = textureSize(sShadowCubeMaps, 0).z;
    const float actualLayer = max(0, min(shadowMapLayers - 1, floor(mapLayer + 0.5)));

    float shadow = 0.0;
    float currentDepth = length(Lvec);


    // Compute a bias depending on light angle to avoid shadow acnee.
    const float bias = max(4e-2 * (1.0 - cosThetaL), 3e-2);  

    // PCF
    for (int i = 0; i < 20; ++i) {
        float closestDepth = texture(sShadowCubeMaps, vec4(-Lvec + sampleOffsetDirections[i] * light.radius, actualLayer)).r * 100.0;
        shadow += currentDepth - bias > closestDepth ? 1.0 : 0.0;
    } 
    shadow /= 20.0;

    return shadow;
}

void main() {
    // Fetch material parameters from textures
    vec3 albedo = vec3(1.0);
    float alpha = 1.0;
    if (uMaterial.AlbedoMap) {
        vec4 albedo_alpha= texture(sAlbedoMap, vUV).rgba;
        if (albedo_alpha.a < uMaterial.AlphaMasking) discard;
        albedo = albedo_alpha.rgb;
        alpha = albedo_alpha.a;
    }

    albedo *= uMaterial.AlbedoFactor;
    
    float metallic = 1.0;
    float roughness = 1.0;
    if (uMaterial.MetalRoughnessMap) {
        vec3 sampl = texture(sMetalRoughnessMap, vUV).rgb;
        metallic = sampl.b;
        roughness = sampl.g;
    }
    metallic *= uMaterial.MetallicFactor;
    roughness *= uMaterial.RoughnessFactor;

    float ao = 1.0;
    if (uMaterial.OcclusionMap) {
        ao = texture(sOcclusionMap, vUV).r;
    }

    vec3 emissive = vec3(1.0);
    if (uMaterial.EmissiveMap) {
        emissive = texture(sEmissiveMap, vUV).rgb;
    }
    emissive *= uMaterial.EmissiveFactor;

    // Compute normal vector
    vec3 N = normalize(vNormal);
    if (uMaterial.NormalMap) {
        vec3 normal = texture(sNormalMap, vUV).rgb * 2.0 - 1.0;
        N = normalize(normal.x * vTangent + normal.y * vBitangent + normal.z * vNormal);
        //N = normalize(normal);
    }
    // If material is double-faced, the normal needs to be inverted on the back-face.
    if (!gl_FrontFacing) {
        N = -N;
    }

    // Precompute view and reflected vectors
    vec3 V = normalize(uScene.EyePosition - vPosition);
    vec3 R = reflect(-V, N);
    float cosThetaO = max(dot(N, V), 0.0);

    // Compute fragment reflectivity
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    
    // Compute reflected/refracted light ratio.
    vec3 F = FresnelSchlickRoughness(cosThetaO, F0, roughness);
    vec3 kD = max(1.0 - F, 0.0);

    // Direct diffuse component (Lambert)
    vec3 diffuseDirect = kD * albedo / PI;

    // Compute direct lighting
    vec3 Ld = vec3(0.0);

    // Directional Lights
    for (uint i = 0; i < gDirectionalLights.length(); ++i) {
        DirectionalLight light = gDirectionalLights[i];

        // Light & Halfway vector
        vec3 L = normalize(-light.direction);
        vec3 H = normalize(L + V);
        float cosThetaL = max(dot(N, L), 0.0);
        float cosThetaH = max(dot(N, H), 0.0);
       
        // Compute shadow
        float shadow = (i < MAX_SHADOW_CASTERS) ? ComputeShadow(light, vLightPosition[i], i, cosThetaL) : 0.0;

        // Direct specular component (Cook-Torrance)
        vec3 numCT = DistributionGGX(cosThetaH, roughness) * F * GeometrySmith(cosThetaO, cosThetaL, roughness);
        float denCT = max(4 * cosThetaO * cosThetaL, 0.01);
        vec3 specular = numCT / denCT;

        Ld += (1.0 - shadow) * (diffuseDirect + specular) * light.color * light.power * cosThetaL;
    }

    // Point Lights
    for (uint i = 0; i < gPointLights.length(); ++i) {
        PointLight light = gPointLights[i];

        vec3 Lvec = light.position - vPosition;

        // Light & Halfway vector
        vec3 L = normalize(Lvec);
        vec3 H = normalize(L + V);
        float cosThetaL = max(dot(N, L), 0.0);
        float cosThetaH = max(dot(N, H), 0.0);

        // Compute shadow
        float shadow = (i < MAX_SHADOW_CASTERS) ? ComputeCubeShadow(light, Lvec, i, cosThetaL) : 0.0;

        // Attenuation
        float attenuation = 1.0 / dot(Lvec, Lvec);

        // Direct specular component (Cook-Torrance)
        vec3 numCT = DistributionGGX(cosThetaH, roughness) * F * GeometrySmith(cosThetaO, cosThetaL, roughness);
        float denCT = max(4 * cosThetaO * cosThetaL, 0.01);
        vec3 specular = numCT / denCT;

        Ld += (1.0 - shadow) * (diffuseDirect + specular) * light.color * light.power * attenuation * cosThetaL;
       
    }

    // Compute indirect lighting
    // Indirect diffuse light (Lambert)
    vec3 diffuseIrradiance = textureLod(sEnvironment, N, MipFromRoughness(1.0)).rgb;
    vec3 diffuse = kD * albedo * diffuseIrradiance;

    // Indirect specular light (Cook-Torrance)
    vec2 specLUT = texture(sBRDF_LUT, vec2(cosThetaO, roughness)).rg;
    vec3 specBRDF = (F0 * specLUT.x + specLUT.y);
    vec3 specIrradiance = textureLod(sEnvironment, R, MipFromRoughness(roughness)).rgb;
    vec3 specular = specBRDF * specIrradiance;

    vec3 Li = diffuse + specular;

    // Apply ambiant occlusion and self-emitted light.
    vec3 color = (Ld + Li) * ao + emissive;

    fColor = vec4(color, alpha);   
}
