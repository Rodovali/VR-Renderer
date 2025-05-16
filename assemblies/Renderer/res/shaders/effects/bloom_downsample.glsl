// Bloom Shader - Downsample pass
// Rodolphe VALICON
// 2025

#version 460 core
#stage compute

layout (local_size_x = 8, local_size_y = 8) in;

layout (binding = 0) uniform sampler2D source;
layout (rgba16f, binding = 0) uniform image2D result;

uniform int uMip = 0;

void main() {
    uvec2 id = gl_GlobalInvocationID.xy;
    ivec2 sz = imageSize(result);
    vec2 uv = vec2(id) / sz;

    float level = float(uMip);

    vec2 sourceTexelSize = 1.0 / textureSize(source, uMip);
    float x = sourceTexelSize.x;
    float y = sourceTexelSize.y;

    // Subtexel offset to sample exactly between 4 texels.
    uv.x += 0.5 * x;
    uv.y += 0.5 * y;

    // Downsample Filter from https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
    vec3 a = textureLod(source, vec2(uv.x - 2*x, uv.y + 2*y), level).rgb;
    vec3 b = textureLod(source, vec2(uv.x,       uv.y + 2*y), level).rgb;
    vec3 c = textureLod(source, vec2(uv.x + 2*x, uv.y + 2*y), level).rgb;

    vec3 d = textureLod(source, vec2(uv.x - 2*x, uv.y), level).rgb;
    vec3 e = textureLod(source, vec2(uv.x,       uv.y), level).rgb;
    vec3 f = textureLod(source, vec2(uv.x + 2*x, uv.y), level).rgb;

    vec3 g = textureLod(source, vec2(uv.x - 2*x, uv.y - 2*y), level).rgb;
    vec3 h = textureLod(source, vec2(uv.x,       uv.y - 2*y), level).rgb;
    vec3 i = textureLod(source, vec2(uv.x + 2*x, uv.y - 2*y), level).rgb;

    vec3 j = textureLod(source, vec2(uv.x - x, uv.y + y), level).rgb;
    vec3 k = textureLod(source, vec2(uv.x + x, uv.y + y), level).rgb;
    vec3 l = textureLod(source, vec2(uv.x - x, uv.y - y), level).rgb;
    vec3 m = textureLod(source, vec2(uv.x + x, uv.y - y), level).rgb;

    vec3 downsample = e*0.125;
    downsample += (a+c+g+i)*0.03125;
    downsample += (b+d+f+h)*0.0625;
    downsample += (j+k+l+m)*0.125;
    
    // Eliminate NaN issue
    downsample = max(downsample, 0.0);

    imageStore(result, ivec2(id), vec4(downsample, 1.0));
}