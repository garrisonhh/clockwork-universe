#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <ghh/utils.h>
#include <ghh/memcheck.h>

#include "noise.h"

// lookup tables and constants
static const uint8_t perm_table[] = {
    0xB6, 0xE8, 0x33, 0x0F, 0x37, 0x77, 0x07, 0x6B, 0xE6, 0xE3, 0x06, 0x22, 0xD8, 0x3D, 0xB7, 0x24,
    0x28, 0x86, 0x4A, 0x2D, 0x9D, 0x4E, 0x51, 0x72, 0x91, 0x09, 0xD1, 0xBD, 0x93, 0x3A, 0x7E, 0x00,
    0xF0, 0xA9, 0xE4, 0xEB, 0x43, 0xC6, 0x48, 0x40, 0x58, 0x62, 0x81, 0xC2, 0x63, 0x47, 0x1E, 0x7F,
    0x12, 0x96, 0x9B, 0xB3, 0x84, 0x3E, 0x74, 0xC8, 0xFB, 0xB2, 0x20, 0x8C, 0x82, 0x8B, 0xFA, 0x1A,
    0x97, 0xCB, 0x6A, 0x7B, 0x35, 0xFF, 0x4B, 0xFE, 0x56, 0xEA, 0xDF, 0x13, 0xC7, 0xF4, 0xF1, 0x01,
    0xAC, 0x46, 0x18, 0x61, 0xC4, 0x0A, 0x5A, 0xF6, 0xFC, 0x44, 0x54, 0xA1, 0xEC, 0xCD, 0x50, 0x5B,
    0xE9, 0xE1, 0xA4, 0xD9, 0xEF, 0xDC, 0x14, 0x2E, 0xCC, 0x23, 0x1F, 0xAF, 0x9A, 0x11, 0x85, 0x75,
    0x49, 0xE0, 0x7D, 0x41, 0x4D, 0xAD, 0x03, 0x02, 0xF2, 0xDD, 0x78, 0xDA, 0x38, 0xBE, 0xA6, 0x0B,
    0x8A, 0xD0, 0xE7, 0x32, 0x87, 0x6D, 0xD5, 0xBB, 0x98, 0xC9, 0x2F, 0xA8, 0xB9, 0xBA, 0xA7, 0xA5,
    0x66, 0x99, 0x9C, 0x31, 0xCA, 0x45, 0xC3, 0x5C, 0x15, 0xE5, 0x3F, 0x68, 0xC5, 0x88, 0x94, 0x5E,
    0xAB, 0x5D, 0x3B, 0x95, 0x17, 0x90, 0xA0, 0x39, 0x4C, 0x8D, 0x60, 0x9E, 0xA3, 0xDB, 0xED, 0x71,
    0xCE, 0xB5, 0x70, 0x6F, 0xBF, 0x89, 0xCF, 0xD7, 0x0D, 0x53, 0xEE, 0xF9, 0x64, 0x83, 0x76, 0xF3,
    0xA2, 0xF8, 0x2B, 0x42, 0xE2, 0x1B, 0xD3, 0x5F, 0xD6, 0x69, 0x6C, 0x65, 0xAA, 0x80, 0xD2, 0x57,
    0x26, 0x2C, 0xAE, 0xBC, 0xB0, 0x27, 0x0E, 0x8F, 0x9F, 0x10, 0x7C, 0xDE, 0x21, 0xF7, 0x25, 0xF5,
    0x08, 0x04, 0x16, 0x52, 0x6E, 0xB4, 0xB8, 0x0C, 0x19, 0x05, 0xC1, 0x29, 0x55, 0xB1, 0xC0, 0xFD,
    0x4F, 0x1D, 0x73, 0x67, 0x8E, 0x92, 0x34, 0x30, 0x59, 0x36, 0x79, 0xD4, 0x7A, 0x3C, 0x1C, 0x2A
};

static const vec3 gradients3[] = {
    { 0,  1,  1},
    { 0,  1, -1},
    { 0, -1,  1},
    { 0, -1, -1},
    { 1,  1,  0},
    { 1, -1,  0},
    {-1,  1,  0},
    {-1, -1,  0},
    { 1,  0,  1},
    { 1,  0, -1},
    {-1,  0,  1},
    {-1,  0, -1}
};

static const int corners3[8][3] = {
    {0, 0, 0},
    {0, 0, 1},
    {0, 1, 0},
    {0, 1, 1},
    {1, 0, 0},
    {1, 0, 1},
    {1, 1, 0},
    {1, 1, 1}
};

/*
static const vec2 gradients2[] = {
    { 0,  1},
    { 0, -1},
    { 1,  0},
    {-1,  0},
    { 1,  1},
    { 1, -1},
    {-1,  1},
    {-1, -1}
};
*/

double smootherstep(double edge0, double edge1, double x) {
    x = CLAMP((x - edge0) / (edge1 - edge0), 0.0, 1.0);

    return x * x * x * (x * (x * 6 - 15) + 10);
}

double perlin3(vec3 pos) {
    int i, j;

    // get coordinates of cell and within cell
    vec3 cell_pos;
    int grid_cell[3];

    for (i = 0; i < 3; ++i) {
        grid_cell[i] = (int)pos[i];
        cell_pos[i] = pos[i] - (double)grid_cell[i];
    }

    // calculate gradient indices using perm_table as a hash function
    int grid_indices[8];

    for (i = 0; i < 8; ++i) {
        grid_indices[i] = 0;

        for (j = 0; j < 3; ++j)
            grid_indices[i] += perm_table[grid_cell[j] + corners3[i][j]];

        grid_indices[i] %= 12;
    }

    // calculate noise dots from each corner
    double values[8];
    vec3 relative_pos;

    for (i = 0; i < 8; ++i) {
        for (j = 0; j < 3; ++j)
            relative_pos[j] = cell_pos[j] - (double)corners3[i][j];

        values[i] = glm_vec3_dot((float *)gradients3[grid_indices[i]], relative_pos);
    }

    // smooth and lerp
    for (i = 0; i < 3; ++i)
        cell_pos[i] = smootherstep(0.0, 1.0, cell_pos[i]);

    for (i = 0; i < 4; ++i)
        values[i] = LERP(cell_pos[0], values[i], values[i + 4]);

    for (i = 0; i < 2; ++i)
        values[i] = LERP(cell_pos[1], values[i], values[i + 2]);

    return LERP(cell_pos[2], values[0], values[1]);
}
