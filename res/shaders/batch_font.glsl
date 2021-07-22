#version 330 core

vec2 quad[4] = vec2[](
	vec2(0.0, 1.0),
	vec2(1.0, 1.0),
	vec2(0.0, 0.0),
	vec2(1.0, 0.0)
);

layout (location = 0) in vec2 draw_pos;
layout (location = 1) in vec2 draw_size;
layout (location = 2) in vec2 atlas_pos;
layout (location = 3) in vec2 atlas_size;

out vec2 v_atlas_pos;

uniform vec2 camera;
uniform vec2 screen_size;

void main() {
	// get pixel position
	vec2 pos = draw_pos + (draw_size * quad[gl_VertexID]) - camera;

	// scale to opengl coords
	pos /= screen_size * 0.5;
	pos.y = -pos.y;

	gl_Position = vec4(pos, 0.0, 1.0);

	// atlas quad position
	v_atlas_pos = atlas_pos + (atlas_size * quad[gl_VertexID]);
}
