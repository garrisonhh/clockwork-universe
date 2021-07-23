#version 330 core

in vec2 v_atlas_pos;
in vec3 v_color;

layout (location = 0) out vec4 frag_color;

uniform sampler2D atlas;

void main() {
	frag_color = texture2D(atlas, v_atlas_pos);
    frag_color.xyz *= v_color;
}
