#version 330 core

// size of a voxel image
#define VOXEL_IMG_WIDTH     32.0
#define VOXEL_IMG_HEIGHT    34.0

// pixel projection constants based on voxel image size
const vec3 VOXEL_PIXELS = vec3(
    VOXEL_IMG_WIDTH / 2.0,
    VOXEL_IMG_WIDTH / 4.0,
    VOXEL_IMG_HEIGHT - (VOXEL_IMG_WIDTH / 2.0)
);

const vec2 quad[] = vec2[](
	vec2(0.0, 1.0),
	vec2(1.0, 1.0),
	vec2(0.0, 0.0),
	vec2(1.0, 0.0)
);

layout (location = 0) in vec3 draw_pos; // 3d position
layout (location = 1) in vec2 draw_size; // pixels
layout (location = 2) in vec2 draw_offset; // pixels
layout (location = 3) in vec2 atlas_pos; // tex coords
layout (location = 4) in vec2 atlas_size; // tex coords

out vec2 v_atlas_pos;

uniform vec2 camera;
// TODO uniform vec3 camera_dir ?
uniform vec2 screen_size;
uniform float scale;

vec2 project(vec3 pos) {
    return vec2(
        (pos.x - pos.y) * VOXEL_PIXELS.x,
        (pos.x + pos.y) * VOXEL_PIXELS.y - pos.z * VOXEL_PIXELS.z
    );
}

void main() {
	// get pixel position
	vec2 pos = project(draw_pos) + (draw_size * quad[gl_VertexID]) + draw_offset - camera;

	// scale pixel coords to opengl coords
	pos /= screen_size * 0.5;
	pos.y = -pos.y;
    pos *= scale;

	gl_Position = vec4(pos, 0.0, 1.0);

	// atlas quad position
	v_atlas_pos = atlas_pos + (atlas_size * quad[gl_VertexID]);
}
