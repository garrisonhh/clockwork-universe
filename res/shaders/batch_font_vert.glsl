#version 330 core

vec2 quad[4] = vec2[](
	vec2(0.0, 1.0),
	vec2(1.0, 1.0),
	vec2(0.0, 0.0),
	vec2(1.0, 0.0)
);

// texture drawing stuff
layout (location = 0) in vec2 draw_pos;
layout (location = 1) in vec2 draw_size;
layout (location = 2) in vec2 atlas_pos;
layout (location = 3) in vec2 atlas_size;

// font attributes
layout (location = 4) in vec3 color;
layout (location = 5) in float italicize;
layout (location = 6) in float scale;
layout (location = 7) in float waviness;

out vec2 v_atlas_pos;
out vec3 v_color;

uniform vec2 camera;
uniform vec2 screen_size;
uniform float t;

/*
TODO this will generate mixels all over the place, so if proper pixelation is
desired the easiest thing to do would be to use a framebuffer and scale it up
*/

void main() {
	// italicization
	vec2 quad_pos = quad[gl_VertexID];

	quad_pos.x += (1.0 - quad_pos.y) * italicize;

	// pixel space position
	vec2 pos = draw_pos + (draw_size * quad_pos * scale) - camera;

	// waviness
	pos.y += sin((t * 4.0) + (pos.x / 20.0)) * waviness * scale;

	// pixel space -> screen space
	pos /= screen_size * 0.5;
	pos.y = -pos.y;

	gl_Position = vec4(pos, 0.0, 1.0);

	// varying vars
	v_atlas_pos = atlas_pos + (atlas_size * quad[gl_VertexID]);
    v_color = color;
}
