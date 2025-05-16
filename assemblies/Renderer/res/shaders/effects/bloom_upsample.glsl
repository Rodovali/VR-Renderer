// Bloom Shader - Upsample pass
// Rodolphe VALICON
// 2025

#version 460 core
#stage compute

layout (local_size_x = 8, local_size_y = 8) in;

layout (binding = 0) uniform sampler2D source;
layout (rgba16f, binding = 0) uniform image2D self;
layout (rgba16f, binding = 1) uniform image2D result;

uniform int uMip = 0;
uniform float uFilterRadius = 0.08;

void main() {
    uvec2 id = gl_GlobalInvocationID.xy;
    ivec2 sz = imageSize(result);

    if (id.x > sz.x || id.y > sz.y) return;
    
    vec2 uv = vec2(id) / sz;



    float level = float(uMip);

    vec2 sourceTexelSize = 1.0 / textureSize(source, uMip);
    float x = sourceTexelSize.x;
    float y = sourceTexelSize.y;

    // Subtexel offset
    uv.x += 0.5 * x;
    uv.y += 0.5 * y;

    // Upsample gaussian blur filter from: https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
    vec3 a = textureLod(source, vec2(uv.x - x, uv.y + y), level).rgb;
    vec3 b = textureLod(source, vec2(uv.x,     uv.y + y), level).rgb;
    vec3 c = textureLod(source, vec2(uv.x + x, uv.y + y), level).rgb;

    vec3 d = textureLod(source, vec2(uv.x - x, uv.y), level).rgb;
    vec3 e = textureLod(source, vec2(uv.x,     uv.y), level).rgb;
    vec3 f = textureLod(source, vec2(uv.x + x, uv.y), level).rgb;

    vec3 g = textureLod(source, vec2(uv.x - x, uv.y - y), level).rgb;
    vec3 h = textureLod(source, vec2(uv.x,     uv.y - y), level).rgb;
    vec3 i = textureLod(source, vec2(uv.x + x, uv.y - y), level).rgb;

    vec3 upsample = e*4.0;
    upsample += (b+d+f+h)*2.0;
    upsample += (a+c+g+i);
    upsample *= 1.0 / 16.0;

    vec3 current = imageLoad(self, ivec2(id)).rgb;

    imageStore(result, ivec2(id), vec4(current + upsample, 1.0));
}