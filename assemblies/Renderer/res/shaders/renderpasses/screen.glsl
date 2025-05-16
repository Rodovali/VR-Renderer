// HDR to LDR (Tone mapping)
// & LDR effects (Saturation, Luminosity, Chromatic Abberation)
// & Gamma Correction
// Rodolphe VALICON
// 2025


#version 460 core

#stage vertex
// === Vertex Shader ===============================================================================

layout(location = 0) in vec2 aPosition;
layout(location = 3) in vec2 aTextureCoordinates;

out vec2 TexCoords;

void main() {
    gl_Position = vec4(aPosition, 0.0, 1.0);
    TexCoords = aTextureCoordinates;
}

#stage fragment
// === Fragment Shader =============================================================================
const uint ACES_FITTED = 0;
const uint ACES_FILMIC = 1;
const uint REINHARD = 2;
const uint SIMPLE_EXPOSURE = 3;

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;

uniform float uGamma = 2.2;
uniform float uExposure = 1.0;
uniform float uChromaticAbberation = 0.0;
uniform int uTonemapper = 0;
uniform float uSaturation = 1.0;
uniform float uLuminosity = 1.0;


// --- Modified from: https://64.github.io/tonemapping/
const mat3 ACES_INPUT = transpose(mat3(
    vec3(0.59719f, 0.35458f, 0.04823f),
    vec3(0.07600f, 0.90834f, 0.01566f),
    vec3(0.02840f, 0.13383f, 0.83777f)
));

const mat3 ACES_OUTPUT = transpose(mat3(
    vec3( 1.60475f, -0.53108f, -0.07367f),
    vec3(-0.10208f,  1.10813f, -0.00605f),
    vec3(-0.00327f, -0.07276f,  1.07602f)
));

vec3 rtt_and_odt_fit(vec3 v) {
    vec3 a = v * (v + 0.0245786f) - 0.000090537f;
    vec3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

vec3 aces_fitted(vec3 v) {
    v = ACES_INPUT * v;
    v = rtt_and_odt_fit(v);
    return ACES_OUTPUT * v;
}
// ---------------------------------------------------

// From: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
vec3 aces_filmic(vec3 x) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

vec3 reinhard(vec3 v) {
    return v / (v + 1.0);
}

vec3 simple_exposure(vec3 v) {
    return vec3(1.0) - exp(-v);
}

// HSV - RGB --- From: https://gamedev.stackexchange.com/questions/59797/glsl-shader-change-hue-saturation-brightness
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
// ----------------------------------------------


vec3 chromaticAbberatedSample(vec2 UV) {
    float r = texture(screenTexture, UV + uChromaticAbberation * 0.01 * vec2(cos(1 * 2 * 3.14/3), sin(1 * 2 * 3.14/3))).r;
    float g = texture(screenTexture, UV + uChromaticAbberation * 0.01 * vec2(cos(2 * 2 * 3.14/3), sin(2 * 2 * 3.14/3))).g;
    float b = texture(screenTexture, UV + uChromaticAbberation * 0.01 * vec2(cos(3 * 2 * 3.14/3), sin(3 * 3.14/3))).b;

    return vec3(r, g, b);
}


void main() {
    vec3 color = chromaticAbberatedSample(TexCoords) * uExposure;
    
    // Tone mapping
    vec3 mapped = color;
    if (uTonemapper == ACES_FITTED) {
        mapped = aces_fitted(color);
    } else if (uTonemapper == ACES_FILMIC) {
        mapped = aces_filmic(color);
    } else if (uTonemapper == REINHARD) {
        mapped = reinhard(color);
    } else if (uTonemapper == SIMPLE_EXPOSURE) {
        mapped = simple_exposure(color);
    }

    vec3 saturated = rgb2hsv(mapped);
    saturated.y *= uSaturation;
    saturated.z *= uLuminosity;
    saturated = hsv2rgb(saturated);

    // Gamma correction
    vec3 correctedColor = pow(saturated, vec3(1.0/uGamma));

    

    FragColor = vec4(correctedColor * 1.0, 1.0);
}