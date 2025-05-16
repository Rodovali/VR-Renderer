// Bloom Shader - Mix pass
// Rodolphe VALICON
// 2025

#version 460 core
#stage compute

layout (local_size_x = 8, local_size_y = 8) in;


layout (binding = 0) uniform sampler2D source;
layout (binding = 1) uniform sampler2D lens;
layout (rgba16f, binding = 0) uniform image2D self;
layout (rgba16f, binding = 1) uniform image2D result;

uniform float uBloomAmount = 0.5;

void main() {
    uvec2 id = gl_GlobalInvocationID.xy;
    ivec2 sz = imageSize(self);
    ivec2 szr = imageSize(result);
    vec2 uv = vec2(id) / sz;

    if (id.x >= szr.x || id.y >= szr.y ) return;

    vec2 sourceTexelSize = 1.0 / textureSize(source, 0);
    uv += 0.5 * sourceTexelSize;


    vec3 bloom = textureLod(source, uv, 0).rgb / textureQueryLevels(source);

    vec3 current = imageLoad(self, ivec2(id)).rgb;

    // Lerp rather than sum to preserve energy conservation.
    vec3 nColor = mix(current, bloom, uBloomAmount);

    // Lens dirt simulation. But its bad.
    //vec3 lens = texture(lens, uv).rgb;
    //nColor += 0.5 * bloom * lens;

    imageStore(result, ivec2(id), vec4(nColor, 1.0));
}