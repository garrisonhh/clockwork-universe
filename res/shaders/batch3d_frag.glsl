#version 330 core

in vec2 v_tex_pos;
in vec2 v_depth_tex_pos;

layout (location = 0) out vec4 frag_color;

uniform sampler2D atlas;
uniform float render_dist;

void main() {
    // sample depth values from texture, flip, and scale
    float depth_tex = texture(atlas, v_depth_tex_pos).r;

    // depth texture represents near as 1.0 (white) and far as 0.0 (black),
    // while opengl is the opposite
    depth_tex = (1.0 - depth_tex) / render_dist;

    // pass frag color
    frag_color = texture(atlas, v_tex_pos);

    // pass depth values where texture alpha is not zero
    gl_FragDepth = mix(1.0, gl_FragCoord.z + depth_tex, frag_color.a);

#if 0
    // display depth for testing
    float depth = gl_FragDepth;

    depth = clamp(depth, 0.0, 1.0);

    frag_color = mix(vec4(vec3(0.1), 1.0), frag_color, 1.0 - depth);
#endif
}
