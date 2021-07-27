#version 330 core

#define M_PI 3.1415

in vec2 v_tex_pos;
in vec2 v_depth_tex_pos;
in vec2 v_normal_tex_pos;

layout (location = 0) out vec4 frag_color;

uniform sampler2D atlas;
uniform float render_dist;
uniform vec3 light_pos;

void main() {
    // sample depth values from texture, flip, and scale
    float depth_tex = texture(atlas, v_depth_tex_pos).r;

    // depth texture represents near as 1.0 (white) and far as 0.0 (black),
    // while opengl is the opposite
    depth_tex = (1.0 - depth_tex) / render_dist;

    // pass frag color
    frag_color = texture(atlas, v_tex_pos);

    // find lighting value from normal and light_pos
    vec3 normal = normalize(texture(atlas, v_normal_tex_pos).rgb * 2.0 - 1.0);
    float light = dot(normal, normalize(light_pos));

    frag_color.rgb *= light;

    // pass depth values where texture alpha is not zero
    gl_FragDepth = mix(1.0, gl_FragCoord.z + depth_tex, frag_color.a);

#if 0
    // display depth for debugging
    frag_color = mix(vec4(vec3(gl_FragDepth), 1.0), frag_color, 0.0);
#endif
#if 0
    // display lighting for debugging
    frag_color = vec4(vec3(light), 1.0);
#endif
}
