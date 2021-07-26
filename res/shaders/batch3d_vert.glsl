#version 330 core

// size of a voxel image
#define VOXEL_IMG_WIDTH     32.0
#define VOXEL_IMG_HEIGHT    36.0

// pixel projection constants based on voxel image size
const vec3 PROJECT_PIXELS = vec3(
    (VOXEL_IMG_WIDTH / 2.0),
    (VOXEL_IMG_WIDTH / 4.0),
    VOXEL_IMG_HEIGHT - (VOXEL_IMG_WIDTH / 2.0)
);

const vec3 VOXEL_SUBPIXELS = PROJECT_PIXELS.xxz;

const vec2 quad[] = vec2[](
	vec2(0.0, 1.0),
	vec2(1.0, 1.0),
	vec2(0.0, 0.0),
	vec2(1.0, 0.0)
);

layout (location = 0) in vec3 draw_pos; // 3d position
layout (location = 1) in vec2 draw_size; // pixels
layout (location = 2) in vec2 draw_offset; // pixels

// tex coords
layout (location = 3) in vec2 tex_pos;
layout (location = 4) in vec2 depth_tex_pos;
layout (location = 5) in vec2 normal_tex_pos;
layout (location = 6) in vec2 tex_size;

out vec2 v_tex_pos;
out vec2 v_depth_tex_pos;
out vec2 v_normal_tex_pos;

uniform vec2 camera;
// TODO uniform vec3 camera_dir ?
uniform vec2 screen_size;
uniform float scale;
uniform float render_dist;

vec2 project(vec3 pos) {
    return vec2(
        (pos.x - pos.y) * PROJECT_PIXELS.x,
        (pos.x + pos.y) * PROJECT_PIXELS.y - pos.z * PROJECT_PIXELS.z
    );
}

void main() {
	// get pixel position
	vec2 pos = project(draw_pos) + (draw_size * quad[gl_VertexID]) + draw_offset - camera;

	// scale pixel coords to opengl coords
	pos /= screen_size * 0.5;
	pos.y = -pos.y;
    pos *= scale;

    // find depth
    float depth = (render_dist - (draw_pos.x + draw_pos.y + draw_pos.z)) / render_dist;

	gl_Position = vec4(pos, depth, 1.0);

	// atlas quad positions
    vec2 quad_pos = tex_size * quad[gl_VertexID];

    v_tex_pos = tex_pos + quad_pos;
	v_depth_tex_pos = depth_tex_pos + quad_pos;
    v_normal_tex_pos = normal_tex_pos + quad_pos;
}
