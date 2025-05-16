// Bloom Shader - Copy pass
// Rodolphe VALICON
// 2025

#version 460 core
#stage compute

layout (local_size_x = 8, local_size_y = 8) in;

layout (binding = 0) uniform sampler2D source;
layout (rgba16f, binding = 1) uniform image2D result;

uniform int uMip = 0;

void main() {
    uvec2 id = gl_GlobalInvocationID.xy;
    ivec2 sz = textureSize(source, 0);
    vec2 uv = vec2(id) / sz;

    vec2 sourceTexelSize = 1.0 / textureSize(source, uMip);
    uv += 0.5 * sourceTexelSize;

    vec3 color = textureLod(source, uv, 0.0).rgb;

    imageStore(result, ivec2(id), vec4(color, 1.0));
}