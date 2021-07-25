#version 330 core

#define M_PI 3.1415

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
    // TODO this stuff has shown that the VOXEL_PIXEL z value is off, plz fix!!!
    int idepth = int(gl_FragDepth * float(1 << 24));
    vec3 dcolor = vec3(
        (idepth & 0xFF0000) >> 0x10,
        (idepth & 0x00FF00) >> 0x08,
        (idepth & 0x0000FF) >> 0x00
    ) / 255.0;

    dcolor = cos(dcolor * M_PI); // make it wavy

    frag_color = mix(vec4(dcolor, 1.0), frag_color, 0.0);
#endif
}
