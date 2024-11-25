/*
 * Modified from template.c
 *
 *  Created on: September 10, 2024
 *      Author: Thumrongsak Kosiyatrakul
 */


#ifdef __APPLE__  // include Mac OS X verions of headers

#include <OpenGL/OpenGL.h>
#include <GLUT/glut.h>

#else // non-Mac OS X operating systems

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>

#endif  // __APPLE__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include "initShader.h"
#include "myLib.h"
#include "maze_algorithms.h"

#define IDENTITY_M4 {{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}}
#define MICROSECONDS_PER_SECOND 1000000

typedef struct {
    vec2 x_pos;
    vec2 x_neg;
    vec2 y_pos;
    vec2 y_neg;
    vec2 z_pos;
    vec2 z_neg;
} Block;

typedef struct Coordinate {
    int x;
    int y;
    struct Coordinate *next;
} Coordinate;

typedef struct {
    vec4 eye, at, up;
} view_position;

vec2 TEXTURE_GRASS_TOP = { 0, 0 };
vec2 TEXTURE_STONE_BRICKS = { 0, 1 };
vec2 TEXTURE_POLISHED_GRANITE = { 0, 2 };
vec2 TEXTURE_CACTUS = { 0, 3 };
vec2 TEXTURE_GRAVEL = { 1, 0 };
vec2 TEXTURE_CRACKED_STONE_BRICKS = { 1, 1 };
vec2 TEXTURE_GRANITE = { 1, 2 };
vec2 TEXTURE_BLOCK_OF_BAMBOO = { 1, 3 };
vec2 TEXTURE_COBBLESTONE = { 2, 0 };
vec2 TEXTURE_MOSSY_STONE_BRICKS = { 2, 1 };
vec2 TEXTURE_SANDSTONE = { 2, 2 };
vec2 TEXTURE_GRASS_SIDE = { 2, 3 };
vec2 TEXTURE_MOSSY_COBBLESTONE = { 3, 0 };
vec2 TEXTURE_BRICKS = { 3, 1 };
vec2 TEXTURE_BIRCH_PLANKS = { 3, 2 };
vec2 TEXTURE_DIRT = { 3, 3 };

Block BLOCK_GRASS;
Block BLOCK_DIRT;
Block BLOCK_BIRCH_PLANKS;
Block BLOCK_BRICKS;
Block BLOCK_STONE_BRICKS;

#define get_left_direction(direction) (direction == 0 ? 3 : direction - 1)
#define get_right_direction(direction) (direction == 3 ? 0 : direction + 1)
#define get_behind_direction(direction) (direction < 2 ? direction + 2 : direction - 2)

// Generation parameters
#define ISLAND_PADDING 6
#define CELL_SIZE 3
#define CELL_SIZE_WITH_WALLS 4
#define WALL_HEIGHT 5
#define REMOVE_DIST 2

// Texture constants
float TEX_SIZE = 0.25;

// Maze
Cell **maze;
int maze_width;
int maze_height;
int left, right, bottom, top, near, far; // Island bounds
vec4 island_center;
int max_side;

// Player location in maze
int maze_x;
int maze_y;
int player_facing; // 0: Pos x, 1: Pos y, 2: Neg x, 3: Neg y 

// Automatic maze navigation
struct Coordinate *path;
struct Coordinate *current_step;

// OpenGL buffers
size_t num_vertices;
size_t vertex_index = 0;
vec4 *positions;
vec4 *normals;
vec2 *tex_coords;

vec4 *sun_positions;
vec2 *sun_tex_coords;

GLuint light_position_location;

// Transform matrices
#define CLIP_NEAR 0.01

GLuint current_transformation_matrix;
mat4 ctm = IDENTITY_M4;

GLuint model_view_location;
mat4 model_view = IDENTITY_M4;

GLuint projection_location;
mat4 projection = IDENTITY_M4;

GLuint current_sun_matrix;
mat4 sun_ctm = IDENTITY_M4;

int sun_rotation = 0;

// Lighting
vec4 light_position = { 1, 1, 0, 0 };
int use_ambient;
int use_diffuse;
int use_specular;
int lighting_enabled;
int use_flashlight;
vec4 diffuse;
vec4 specular;

GLuint light_enabled_location;
GLuint use_ambient_location;
GLuint use_diffuse_location;
GLuint use_specular_location;
GLuint use_flashlight_location;
GLuint light_position_location;


// Rotation variable so mouse and motion can interact
vec4 click_vector;
mat4 previous_rotation_matrix;
float distance_from_center;
int rotation_enabled = 1;
int is_first_rotation = 1;

// Animation Variables
#define ANIMATION_DURATION (0.5 * MICROSECONDS_PER_SECOND) // Microseconds

int is_animating = 0;
long animation_started;
vec4 eye_move_vector;
vec4 at_move_vector;

view_position current_pos, target_pos;

void define_blocks() {
    BLOCK_GRASS = (Block) { TEXTURE_GRASS_SIDE, TEXTURE_GRASS_SIDE, TEXTURE_GRASS_TOP, TEXTURE_DIRT, TEXTURE_GRASS_SIDE, TEXTURE_GRASS_SIDE };
    BLOCK_DIRT = (Block) { TEXTURE_DIRT, TEXTURE_DIRT, TEXTURE_DIRT, TEXTURE_DIRT, TEXTURE_DIRT, TEXTURE_DIRT };
    BLOCK_BIRCH_PLANKS = (Block) { TEXTURE_BIRCH_PLANKS, TEXTURE_BIRCH_PLANKS, TEXTURE_BIRCH_PLANKS, TEXTURE_BIRCH_PLANKS, TEXTURE_BIRCH_PLANKS, TEXTURE_BIRCH_PLANKS };
    BLOCK_BRICKS = (Block) { TEXTURE_BRICKS, TEXTURE_BRICKS, TEXTURE_BRICKS, TEXTURE_BRICKS, TEXTURE_BRICKS, TEXTURE_BRICKS };
    BLOCK_STONE_BRICKS = (Block) { TEXTURE_STONE_BRICKS, TEXTURE_STONE_BRICKS, TEXTURE_STONE_BRICKS, TEXTURE_STONE_BRICKS, TEXTURE_STONE_BRICKS, TEXTURE_STONE_BRICKS };

}

void set_cube_vertices(int index, float x1, float y1, float z1, float size) {
    float x2 = x1 + size;
    float y2 = y1 + size;
    float z2 = z1 + size;

    // X+
    positions[index] = (vec4) { x2, y1, z1, 1.0 };
    positions[index + 1] = (vec4) { x2, y2, z1, 1.0 };
    positions[index + 2] = (vec4) { x2, y1, z2, 1.0 };
    positions[index + 3] = (vec4) { x2, y2, z2, 1.0 };
    positions[index + 4] = (vec4) { x2, y1, z2, 1.0 };
    positions[index + 5] = (vec4) { x2, y2, z1, 1.0 };

    // X-
    positions[index + 6] = (vec4) { x1, y1, z1, 1.0 };
    positions[index + 7] = (vec4) { x1, y1, z2, 1.0 };
    positions[index + 8] = (vec4) { x1, y2, z1, 1.0 };
    positions[index + 9] = (vec4) { x1, y2, z2, 1.0 };
    positions[index + 10] = (vec4) { x1, y2, z1, 1.0 };
    positions[index + 11] = (vec4) { x1, y1, z2, 1.0 };

    // Y+
    positions[index + 12] = (vec4) { x1, y2, z1, 1.0 };
    positions[index + 13] = (vec4) { x1, y2, z2, 1.0 };
    positions[index + 14] = (vec4) { x2, y2, z1, 1.0 };
    positions[index + 15] = (vec4) { x2, y2, z2, 1.0 };
    positions[index + 16] = (vec4) { x2, y2, z1, 1.0 };
    positions[index + 17] = (vec4) { x1, y2, z2, 1.0 };

    // Y-
    positions[index + 18] = (vec4) { x1, y1, z1, 1.0 };
    positions[index + 19] = (vec4) { x2, y1, z1, 1.0 };
    positions[index + 20] = (vec4) { x1, y1, z2, 1.0 };
    positions[index + 21] = (vec4) { x2, y1, z2, 1.0 };
    positions[index + 22] = (vec4) { x1, y1, z2, 1.0 };
    positions[index + 23] = (vec4) { x2, y1, z1, 1.0 };

    // Z+
    positions[index + 24] = (vec4) { x1, y1, z2, 1.0 };
    positions[index + 25] = (vec4) { x2, y1, z2, 1.0 };
    positions[index + 26] = (vec4) { x1, y2, z2, 1.0 };
    positions[index + 27] = (vec4) { x2, y2, z2, 1.0 };
    positions[index + 28] = (vec4) { x1, y2, z2, 1.0 };
    positions[index + 29] = (vec4) { x2, y1, z2, 1.0 };

    // Z-
    positions[index + 30] = (vec4) { x1, y1, z1, 1.0 };
    positions[index + 31] = (vec4) { x1, y2, z1, 1.0 };
    positions[index + 32] = (vec4) { x2, y1, z1, 1.0 };
    positions[index + 33] = (vec4) { x2, y2, z1, 1.0 };
    positions[index + 34] = (vec4) { x2, y1, z1, 1.0 };
    positions[index + 35] = (vec4) { x1, y2, z1, 1.0 };
}

void set_cube_normals(int index) {
    // X+
    normals[index] = (vec4) { 1.0, 0.0, 0.0, 1.0 };
    normals[index + 1] = (vec4) { 1.0, 0.0, 0.0, 1.0 };
    normals[index + 2] = (vec4) { 1.0, 0.0, 0.0, 1.0 };
    normals[index + 3] = (vec4) { 1.0, 0.0, 0.0, 1.0 };
    normals[index + 4] = (vec4) { 1.0, 0.0, 0.0, 1.0 };
    normals[index + 5] = (vec4) { 1.0, 0.0, 0.0, 1.0 };

    // X-
    normals[index + 6] = (vec4) { -1.0, 0.0, 0.0, 1.0 };
    normals[index + 7] = (vec4) { -1.0, 0.0, 0.0, 1.0 };
    normals[index + 8] = (vec4) { -1.0, 0.0, 0.0, 1.0 };
    normals[index + 9] = (vec4) { -1.0, 0.0, 0.0, 1.0 };
    normals[index + 10] = (vec4) { -1.0, 0.0, 0.0, 1.0 };
    normals[index + 11] = (vec4) { -1.0, 0.0, 0.0, 1.0 };

    // Y+
    normals[index + 12] = (vec4) { 0.0, 1.0, 0.0, 1.0 };
    normals[index + 13] = (vec4) { 0.0, 1.0, 0.0, 1.0 };
    normals[index + 14] = (vec4) { 0.0, 1.0, 0.0, 1.0 };
    normals[index + 15] = (vec4) { 0.0, 1.0, 0.0, 1.0 };
    normals[index + 16] = (vec4) { 0.0, 1.0, 0.0, 1.0 };
    normals[index + 17] = (vec4) { 0.0, 1.0, 0.0, 1.0 };

    // Y-
    normals[index + 18] = (vec4) { 0.0, -1.0, 0.0, 1.0 };
    normals[index + 19] = (vec4) { 0.0, -1.0, 0.0, 1.0 };
    normals[index + 20] = (vec4) { 0.0, -1.0, 0.0, 1.0 };
    normals[index + 21] = (vec4) { 0.0, -1.0, 0.0, 1.0 };
    normals[index + 22] = (vec4) { 0.0, -1.0, 0.0, 1.0 };
    normals[index + 23] = (vec4) { 0.0, -1.0, 0.0, 1.0 };

    // Z+
    normals[index + 24] = (vec4) { 0.0, 0.0, 1.0, 1.0 };
    normals[index + 25] = (vec4) { 0.0, 0.0, 1.0, 1.0 };
    normals[index + 26] = (vec4) { 0.0, 0.0, 1.0, 1.0 };
    normals[index + 27] = (vec4) { 0.0, 0.0, 1.0, 1.0 };
    normals[index + 28] = (vec4) { 0.0, 0.0, 1.0, 1.0 };
    normals[index + 29] = (vec4) { 0.0, 0.0, 1.0, 1.0 };

    // Z-
    normals[index + 30] = (vec4) { 0.0, 0.0, -1.0, 1.0 };
    normals[index + 31] = (vec4) { 0.0, 0.0, -1.0, 1.0 };
    normals[index + 32] = (vec4) { 0.0, 0.0, -1.0, 1.0 };
    normals[index + 33] = (vec4) { 0.0, 0.0, -1.0, 1.0 };
    normals[index + 34] = (vec4) { 0.0, 0.0, -1.0, 1.0 };
    normals[index + 35] = (vec4) { 0.0, 0.0, -1.0, 1.0 };
}

void set_cube_texture(int index, vec2 x_pos, vec2 x_neg, vec2 y_pos, vec2 y_neg, vec2 z_pos, vec2 z_neg) {
    // X+
    float x1 = TEX_SIZE * x_pos.x;
    float y1 = TEX_SIZE * x_pos.y;
    float x2 = x1 + TEX_SIZE;
    float y2 = y1 + TEX_SIZE;
    
    tex_coords[index] = (vec2) { x2, y2 };
    tex_coords[index + 1] = (vec2) { x2, y1 };
    tex_coords[index + 2] = (vec2) { x1, y2 };
    tex_coords[index + 3] = (vec2) { x1, y1 };
    tex_coords[index + 4] = (vec2) { x1, y2 };
    tex_coords[index + 5] = (vec2) { x2, y1 };

    // X-
    x1 = TEX_SIZE * x_neg.x;
    y1 = TEX_SIZE * x_neg.y;
    x2 = x1 + TEX_SIZE;
    y2 = y1 + TEX_SIZE;

    tex_coords[index + 6] = (vec2) { x2, y2 };
    tex_coords[index + 7] = (vec2) { x1, y2 };
    tex_coords[index + 8] = (vec2) { x2, y1 };
    tex_coords[index + 9] = (vec2) { x1, y1 };
    tex_coords[index + 10] = (vec2) { x2, y1 };
    tex_coords[index + 11] = (vec2) { x1, y2 };

    // Y+
    x1 = TEX_SIZE * y_pos.x;
    y1 = TEX_SIZE * y_pos.y;
    x2 = x1 + TEX_SIZE;
    y2 = y1 + TEX_SIZE;
    
    tex_coords[index + 12] = (vec2) { x2, y2 };
    tex_coords[index + 13] = (vec2) { x2, y1 };
    tex_coords[index + 14] = (vec2) { x1, y2 };
    tex_coords[index + 15] = (vec2) { x1, y1 };
    tex_coords[index + 16] = (vec2) { x1, y2 };
    tex_coords[index + 17] = (vec2) { x2, y1 };

    // Y-
    x1 = TEX_SIZE * y_neg.x;
    y1 = TEX_SIZE * y_neg.y;
    x2 = x1 + TEX_SIZE;
    y2 = y1 + TEX_SIZE;

    tex_coords[index + 18] = (vec2) { x2, y2 };
    tex_coords[index + 19] = (vec2) { x1, y2 };
    tex_coords[index + 20] = (vec2) { x2, y1 };
    tex_coords[index + 21] = (vec2) { x1, y1 };
    tex_coords[index + 22] = (vec2) { x2, y1 };
    tex_coords[index + 23] = (vec2) { x1, y2 };

    // Z+
    x1 = TEX_SIZE * z_pos.x;
    y1 = TEX_SIZE * z_pos.y;
    x2 = x1 + TEX_SIZE;
    y2 = y1 + TEX_SIZE;

    tex_coords[index + 24] = (vec2) { x2, y2 };
    tex_coords[index + 25] = (vec2) { x1, y2 };
    tex_coords[index + 26] = (vec2) { x2, y1 };
    tex_coords[index + 27] = (vec2) { x1, y1 };
    tex_coords[index + 28] = (vec2) { x2, y1 };
    tex_coords[index + 29] = (vec2) { x1, y2 };

    // Z-
    x1 = TEX_SIZE * z_neg.x;
    y1 = TEX_SIZE * z_neg.y;
    x2 = x1 + TEX_SIZE;
    y2 = y1 + TEX_SIZE;

    tex_coords[index + 30] = (vec2) { x2, y2 };
    tex_coords[index + 31] = (vec2) { x2, y1 };
    tex_coords[index + 32] = (vec2) { x1, y2 };
    tex_coords[index + 33] = (vec2) { x1, y1 };
    tex_coords[index + 34] = (vec2) { x1, y2 };
    tex_coords[index + 35] = (vec2) { x2, y1 };
}

long get_micro_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_sec * MICROSECONDS_PER_SECOND + tv.tv_usec;
}

int try_probability(int numerator, int denominator) {
    return numerator > rand() % denominator;
}

void set_block(int x, int y, int z, Block block) {
    set_cube_vertices(vertex_index, x, y, z, 1);
    set_cube_normals(vertex_index);
    set_cube_texture(vertex_index, block.x_pos, block.x_neg, block.y_pos, block.y_neg, block.z_pos, block.z_neg);

    vertex_index += 36;
}

void generate_island_column(int x, int z, int min_distance) {
    int fill_until = REMOVE_DIST - min_distance;

    if (fill_until > 0) {
        fill_until = 0;
    }

    int min_y = -REMOVE_DIST - min_distance;

    // Guaranteed blocks
    if (fill_until < 0) {
        set_block(x, 0, z, BLOCK_GRASS);
    }

    for (int y = -1; y > fill_until; y--) {
        set_block(x, y, z, BLOCK_DIRT);
    }

    // Determine number of blocks to place at end
    int y = min_y;

    for (; y <= fill_until; y++) {
        if (try_probability(1, 4)) {
            break;
        }
    }

    if (y < 0) {
        set_block(x, 0, z, BLOCK_GRASS);
    }

    for (; y <= fill_until - 1; y++) {
        set_block(x, y, z, BLOCK_DIRT);
    }
}

void generate_maze_wall(int x, int z, Block block) {
    int maze_top = 1 + WALL_HEIGHT;
    int random_removal_level = maze_top - REMOVE_DIST;

    // Guaranteed blocks
    for (int y = 2; y <= random_removal_level; y++) {
        set_block(x, y, z, block);
    }

    // Determine number of many blocks to place at end
    int y = maze_top;

    for (; y > random_removal_level; y--) {
        if (try_probability(1, 3)) {
            break;
        }
    }

    for (; y > random_removal_level; y--) {
        set_block(x, y, z, block);
    }
}

void generate_world() {
    // Calculate max number of vertices without missing blocks
    int maze_x_size = maze_width * CELL_SIZE_WITH_WALLS + 1;
    int maze_z_size = maze_height * CELL_SIZE_WITH_WALLS + 1;

    int total_x_size = ISLAND_PADDING * 2 + maze_x_size;
    int total_z_size = ISLAND_PADDING * 2 + maze_z_size;

    int island_smaller_side;
    int island_larger_side;
    
    if (total_x_size < total_z_size) {
        island_smaller_side = total_x_size;
        island_larger_side = total_z_size;
    } else {
        island_smaller_side = total_z_size;
        island_larger_side = total_x_size;
    }
    
    int island_height = (island_smaller_side + 1) / 2;
    int total_y_size = 1 + WALL_HEIGHT + island_height;

    if (total_y_size > island_larger_side) {
        max_side = total_y_size;
    } else {
        max_side = island_larger_side;
    }

    size_t maze_floor_blocks = maze_x_size * maze_z_size;
    size_t maze_walls_blocks = ((maze_height + 1) * (maze_width + 1) + (maze_height * (maze_width + 1) + maze_width * (maze_height + 1) - maze_width * maze_height - 1) * CELL_SIZE) * WALL_HEIGHT;
    size_t island_blocks = total_x_size * total_z_size * (REMOVE_DIST + 1);

    int long_length = island_larger_side;

    for (int short_length = island_smaller_side; short_length > 0; short_length -= 2) {
        island_blocks += long_length * short_length;
        long_length -= 2;
    }

    num_vertices = (maze_floor_blocks + maze_walls_blocks + island_blocks + 1) * 36;

    // Store bounds
    left = -ISLAND_PADDING;
    right = total_x_size - ISLAND_PADDING - 1;
    bottom = -total_y_size - 2;
    top = total_y_size + 2;
    near = total_z_size - ISLAND_PADDING - 1;
    far = -ISLAND_PADDING;

    island_center = (vec4) {
        (float)(left + right) / 2,
        (float)(top + bottom) / 2,
        (float)(near + far) / 2,
        1
    };

    printf("X: %d, Y: %d Z: %d\n", total_x_size, total_y_size, total_z_size);
    printf("Left: %d Right: %d\n", left, right);
    printf("Bottom: %d Top: %d\n", bottom, top);
    printf("Near: %d Far: %d\n", near, far);

    // Allocate arrays
    positions = (vec4 *) malloc(sizeof(vec4) * num_vertices);
    normals = (vec4 *) malloc(sizeof(vec4) * num_vertices);
    tex_coords = (vec2 *) malloc(sizeof(vec2) * num_vertices);

    // Generate island
    int island_x_min = -ISLAND_PADDING;
    int island_x_max = maze_x_size + ISLAND_PADDING;
    int island_z_min = -ISLAND_PADDING;
    int island_z_max = maze_z_size + ISLAND_PADDING;

    for (int x = island_x_min; x < island_z_max; x++) {
        // Calculate minimum distance from any side
        int dist_from_left = x - island_x_min + 1;
        int dist_from_right = island_x_max - x;
        int min_dist_x;

        if (dist_from_left < dist_from_right) {
            min_dist_x = dist_from_left;
        } else {
            min_dist_x = dist_from_right;
        }

        for (int z = island_z_min; z < island_z_max; z++) {
            int min_distance = min_dist_x;
            int dist_from_bottom = island_z_max - z;
            int dist_from_top = z - island_z_min + 1;

            if (dist_from_top < min_distance) {
                min_distance = dist_from_top;
            }

            if (dist_from_bottom < min_distance) {
                min_distance = dist_from_bottom;
            }

            generate_island_column(x, z, min_distance);
        }
    }

    // Generate maze base
    for (int x = 0; x < maze_x_size; x++) {
        for (int z = 0; z < maze_z_size; z++) {
            set_block(x, 1, z, BLOCK_BIRCH_PLANKS);
        }
    }

    // Generate maze
    int right_pos = maze_x_size - 1;
    int bottom_pos = maze_z_size - 1;

    // For each row
    for (int y = 0; y < maze_height; y++) {
        int z_pos = y * CELL_SIZE_WITH_WALLS;

        Cell cell;

        // Top wall
        for (int x = 0; x < maze_width; x++) {
            cell = maze[x][y];
            int x_pos = x * CELL_SIZE_WITH_WALLS;

            generate_maze_wall(x_pos, z_pos, BLOCK_STONE_BRICKS);

            // Top wall
            if (cell.top) {   
                for (int i = 1; i <= CELL_SIZE; i++) {
                    generate_maze_wall(x_pos + i, z_pos, BLOCK_BRICKS);
                }
            }

            // Left wall
            if (cell.left) {
                for (int i = 1; i <= CELL_SIZE; i++) {
                    generate_maze_wall(x_pos, z_pos + i, BLOCK_BRICKS);
                }
            }
        }

        // Right of row
        generate_maze_wall(right_pos, z_pos, BLOCK_STONE_BRICKS);

        if (cell.right) {
            for (int i = 1; i <= CELL_SIZE; i++) {
                generate_maze_wall(right_pos, z_pos + i, BLOCK_BRICKS);
            }
        }
    }

    // Bottom of maze
    for (int x = 0; x < maze_width; x++) {
        Cell cell = maze[x][maze_height - 1];
        int x_pos = x * CELL_SIZE_WITH_WALLS;

        generate_maze_wall(x_pos, bottom_pos, BLOCK_STONE_BRICKS);
    
        if (cell.bottom) {
            for (int i = 1; i <= CELL_SIZE; i++) {
                generate_maze_wall(x_pos + i, bottom_pos, BLOCK_BRICKS);
            }
        }
    }

    // Bottom right corner
    generate_maze_wall(right_pos, bottom_pos, BLOCK_STONE_BRICKS);

    // Generate the sun
    set_block((left + right) / 2, top, (bottom + top) / 2, BLOCK_BIRCH_PLANKS);

    light_position = (vec4) { (left + right) / 2, WALL_HEIGHT + 1, (bottom + top) / 2, 1.0 };

    // Set actual number of vertices
    num_vertices = vertex_index;
}

void prompt_maze_size() {
    // Ask for input and generate maze
    printf("Enter width and the height for the size of the maze (ex. 6 8)\n");

    maze_width = 6;
    maze_height = 6;

    if (1)
    //if(scanf("%d %d", &maze_width, &maze_height) > 0 && maze_width > 0 && maze_height > 0)
    {
        maze = malloc(maze_width * sizeof(Cell *));

        for (int i = 0; i < maze_width; i++) {
            maze[i] = malloc(maze_height * sizeof(Cell));
        }

        printf("Width: %d Height: %d\n", maze_width, maze_height);
        generate_maze(maze, maze_width, maze_height);
        print_maze(maze, maze_width, maze_height);
    }
    else
    {
        printf("\nInvalid Input! Exiting...\n");
        exit(1);
    }
}

//0 is top
//1 is left
//2 is bottom
//3 is right    
int dfs_recursive(Cell loc, int loc_x, int loc_y, int dir) {
    
    struct Coordinate *nextCoor = (struct Coordinate *) malloc(sizeof(Coordinate));
    nextCoor->x = loc_x;
    nextCoor->y = loc_y;
    if(loc_x == 0 && loc_y == 0) {
        path = nextCoor;
        current_step = path;
    }
    else {
        current_step->next = nextCoor;
        current_step = current_step->next;
    }
    printf(" t(%d,%d)\n", current_step->x, current_step->y);
    // current = curr;
    if(loc_x == maze_width-1 && loc_y == maze_height-1) {
        current_step->next = NULL;
        printf("Found exit");
        return 1;
    }
    // if(dir == 0 && loc.left == 1 && loc.right == 1 && loc.bottom == 1) {
    //     printf("Dead end 1");
    //     return 0;
    // }
    // if(dir == 1 && loc.top == 1 && loc.right == 1 && loc.bottom == 1) {
    //     printf("Dead end 2");
    //     return 0;
    // }
    // if(dir == 2 && loc.left == 1 && loc.right == 1 && loc.top == 1) {
    //     printf("Dead end 3");
    //     return 0;
    // }
    // if(dir == 3 && loc.left == 1 && loc.top == 1 && loc.bottom == 1) {
    //     printf("Dead end 4");
    //     return 0;
    // }

    int found = 0;
    if(loc_y > 0 && loc.top == 0 && dir != 2) {
        printf("Move top");
        printf("(%d,%d)\n", current_step->x, current_step->y);
        found = dfs_recursive(maze[loc_x][loc_y-1], loc_x, loc_y-1, 0);
        printf(" (%d,%d)\n", current_step->x, current_step->y);
    }
    if(found != 1 && loc_y < maze_height-1 && loc.bottom == 0 && dir != 0) {
        printf("Move bottom");
        printf("x=%d, y=%d\n", loc_x, loc_y);
        if(current_step->x != loc_x || current_step->y != loc_y) {
            struct Coordinate *copy = (struct Coordinate *) malloc(sizeof(Coordinate));
            copy->x = loc_x;
            copy->y = loc_y;
            current_step->next = copy;
            current_step = current_step->next;
            printf("(%d,%d)\n", current_step->x, current_step->y);
        }
        found = dfs_recursive(maze[loc_x][loc_y+1], loc_x, loc_y+1, 2);
        printf(" (%d,%d)\n", current_step->x, current_step->y);
    }
    if(found != 1 && loc_x > 0 && loc.left == 0 && dir != 3) {
        printf("Move left");
        printf("x=%d, y=%d\n", loc_x, loc_y);
        if(current_step->x != loc_x || current_step->y != loc_y) {
            struct Coordinate *copy = (struct Coordinate *) malloc(sizeof(Coordinate));
            copy->x = loc_x;
            copy->y = loc_y;
            current_step->next = copy;
            current_step = current_step->next;
            printf(" (%d,%d)\n", current_step->x, current_step->y);
        }
        found = dfs_recursive(maze[loc_x-1][loc_y], loc_x-1, loc_y, 1);
        printf("(%d,%d)\n", current_step->x, current_step->y);
    }
    if(found != 1 && loc_x < maze_width-1 && loc.right == 0 && dir != 1) {
        printf("Move right");
        printf("x=%d, y=%d\n", loc_x, loc_y);
        if(current_step->x != loc_x || current_step->y != loc_y) {
            struct Coordinate *copy = (struct Coordinate *) malloc(sizeof(Coordinate));
            copy->x = loc_x;
            copy->y = loc_y;
            current_step->next = copy;
            current_step = current_step->next;
            printf(" (%d,%d)\n", current_step->x, current_step->y);
        }
        found = dfs_recursive(maze[loc_x+1][loc_y], loc_x+1, loc_y, 3);
        printf("(%d,%d)\n", current_step->x, current_step->y);
    }
    printf("x=%d, y=%d\n", loc_x, loc_y);
    if(found != 1 && (current_step->x != loc_x || current_step->y != loc_y)) {
        struct Coordinate *currCoor = (struct Coordinate *) malloc(sizeof(Coordinate));
        currCoor->x = loc_x;
        currCoor->y = loc_y;
        current_step->next = currCoor;
        current_step = current_step->next;
        printf("c(%d,%d)\n", current_step->x, current_step->y);
    }

    return found;



}

void dfs() {
    Cell start = maze[0][0];
    int found = dfs_recursive(start, 0, 0, 3);
}

int dfs_anyposition_recursive(Cell loc, int loc_x, int loc_y, int dir, Coordinate *curr) {
    struct Coordinate *nextCoor = (struct Coordinate *) malloc(sizeof(Coordinate));
    nextCoor->x = loc_x;
    nextCoor->y = loc_y;
    curr->next = nextCoor;
    //printf(" t(%d,%d)\n", curr->x, curr->y);
    // current = curr;
    if(loc_x == maze_width-1 && loc_y == maze_height-1) {
        printf("Found exit\n");
        curr->next->next = NULL;
        return 1;
    }

    int found = 0;
    if(loc_y > 0 && loc.top == 0 && dir != 2) {
        printf("Move top\n");
        found = dfs_anyposition_recursive(maze[loc_x][loc_y-1], loc_x, loc_y-1, 0, curr->next);
        //printf(" (%d,%d)\n", current->x, current->y);
    }
    if(found != 1 && loc_y < maze_height-1 && loc.bottom == 0 && dir != 0) {
        printf("Move bottom\n");
        found = dfs_anyposition_recursive(maze[loc_x][loc_y+1], loc_x, loc_y+1, 2, curr->next);
        //printf(" (%d,%d)\n", current->x, current->y);
    }
    if(found != 1 && loc_x > 0 && loc.left == 0 && dir != 3) {
        printf("Move left\n");
        found = dfs_anyposition_recursive(maze[loc_x-1][loc_y], loc_x-1, loc_y, 1, curr->next);
        //printf("(%d,%d)\n", current->x, current->y);
    }
    if(found != 1 && loc_x < maze_width-1 && loc.right == 0 && dir != 1) {
        printf("Move right\n");
        found = dfs_anyposition_recursive(maze[loc_x+1][loc_y], loc_x+1, loc_y, 3, curr->next);
        //printf("(%d,%d)\n", curr->x, curr->y);
    }
    return found;
}

void dfs_anyposition() {
    Cell start = maze[maze_x][maze_y];
    path = (struct Coordinate *) malloc(sizeof(Coordinate));
    path->x = maze_x;
    path->y = maze_y;
    int found = dfs_anyposition_recursive(start, maze_x, maze_y, -1, path);
    path = path->next;
}

void print_list() {
    //printf("Entering print");
    current_step = path;
    //printf("(%d,%d), ", list->x, list->y);
    while (current_step->x != maze_width-1 || current_step->y != maze_height-1) {
        printf("(%d,%d), ", current_step->x, current_step->y);
        current_step = current_step->next;
    }
    printf("(%d,%d) ", current_step->x, current_step->y);
    printf("\n");
}

// Coordinate* coor_copy(Coordinate *original) {
//     struct Coordinate *copy = (struct Coordinate *) malloc(sizeof(Coordinate));
//     copy->x = original->x;
//     copy->y = original->y;
//     return copy;
// }

// Print out all keyboard keys that are used to the console
void print_helper_text()
{
    printf("\nKeyboard Commands:\n");
    printf("Q - Exit Program\n");

    printf("\n---------[Movement]---------\n");
    printf("W - Move Forward\n");
    printf("A - Move Left\n");
    printf("S - Move Backward\n");
    printf("D - Move Right\n");

    printf("J - Rotate Left\n");
    printf("L - Rotate Right\n");

    printf("P - Solve From Entrance\n");
    printf("I - Solve From Anywhere\n");

    printf("\n---------[Camera]---------\n");
    printf("T - Topdown View\n");
    printf("R - Reset to Side View\n");
    printf("E - Go to Entrance\n");

    printf("\n---------[Lighting]---------\n");
    printf("V - Enable Light\n");
    printf("B - Toggle Ambient\n");
    printf("N - Toggle Diffuse\n");
    printf("M - toggle Specular\n");


    printf("\n");
    
}

void update_positions(vec4 position, int facing) {
    float x = position.x;
    float y = position.y;
    float z = position.z;

    target_pos.eye = (vec4) {x, y, z, 0.0};

    // update target pos
    switch (facing) {
        case 0:
            target_pos.at = (vec4) {x + 1, y, z, 0.0};
            break;
        case 1:
            target_pos.at = (vec4) {x, y, z + 1, 0.0};
            break;
        case 2:
            target_pos.at = (vec4) {x - 1, y, z, 0.0};
            break;
        case 3:
            target_pos.at = (vec4) {x, y, z - 1, 0.0};
            break;
            
    }
}

void start_animation() {
    animation_started = get_micro_time();
    eye_move_vector = sub_v4(target_pos.eye, current_pos.eye);
    at_move_vector = sub_v4(target_pos.at, current_pos.at);
    is_animating = 1;
}

void move_to(vec4 position) {
    update_positions(position, player_facing);
    start_animation();
}

void turn_to(int direction) {
    update_positions(current_pos.eye, direction);
    start_animation();
}

void move_to_cell(int x, int y) {
    maze_x = x;
    maze_y = y;

    vec4 position = {
        ((float)x + 0.5) * (float)CELL_SIZE_WITH_WALLS + 0.5,
        3.5,
        ((float)y + 0.5) * (float)CELL_SIZE_WITH_WALLS + 0.5,
        0
    };

    move_to(position);
}

void move_direction(int direction) {
    if (rotation_enabled) {
        return;
    }

    switch (direction) {
        case 0:
            // Pos x
            if (maze_y >= 0 && maze_y < maze_height &&
                (maze_x == -1 && maze[0][maze_y].left ||
                maze_x >= 0 && maze_x < maze_width && maze[maze_x][maze_y].right)) {
                return; // Collision
            }

            move_to_cell(maze_x + 1, maze_y);
            break;
        case 1:
            // Pos y
            if (maze_x >= 0 && maze_x < maze_width &&
                (maze_y == -1 && maze[maze_x][0].top ||
                maze_y >= 0 && maze_y < maze_height && maze[maze_x][maze_y].bottom)) {
                return; // Collision
            }

            move_to_cell(maze_x, maze_y + 1);
            break;
        case 2:
            // Neg x
            if (maze_y >= 0 && maze_y < maze_height &&
                (maze_x == maze_width && maze[maze_width - 1][maze_y].right ||
                maze_x >= 0 && maze_x < maze_width && maze[maze_x][maze_y].left)) {
                return; // Collision
            }

            move_to_cell(maze_x - 1, maze_y);
            break;
        case 3:
            // Neg y
            if (maze_x >= 0 && maze_x < maze_width &&
                (maze_y == maze_height && maze[maze_x][maze_height - 1].bottom ||
                maze_y >= 0 && maze_y < maze_height && maze[maze_x][maze_y].top)) {
                return; // Collision
            }
            
            move_to_cell(maze_x, maze_y - 1);
            break;
    }
}

void turn(int direction) {
    if (rotation_enabled) {
        return;
    }

    player_facing = direction;
    turn_to(direction);
}

void do_maze_step() {
    if (current_step == NULL) {
        return;
    }
    
    // Turn to exit if at end
    if (current_step->next == NULL) {
        if (player_facing != 0) {
            turn_to(0);
            start_animation();
        }

        current_step = NULL;
        return;
    }

    Coordinate *next = current_step->next;

    // Calculate direction
    int new_direction;
    int dx = next->x - current_step->x;
    int dy = next->y - current_step->y;

    if (dx == 1) {
        new_direction = 0;
    } else if (dy == 1) {
        new_direction = 1;
    } else if (dx == -1) {
        new_direction = 2;
    } else if (dy == -1) {
        new_direction = 3;
    }
    
    // Turn if not facing
    if (player_facing != new_direction) {
        int right = get_right_direction(player_facing);
        
        if (new_direction == right) {
            turn(right);
        } else {
            int left = get_left_direction(player_facing);

            turn(left);
        }
    } else {
        // Move
        move_to_cell(next->x, next->y);
        current_step = next;
    }

    start_animation();
}

void free_path() {
    current_step = path;

    while (current_step != NULL) {
        Coordinate *next = current_step->next;
        free(current_step);
        current_step = next;
    }
}

void navigate(void (*path_gen_func)()) {
    if (rotation_enabled) {
        return;
    }

    free_path();
    path_gen_func();
    print_list();
    current_step = path;
    do_maze_step();
}

void go_to_entrance()
{
    ctm = m4_identity();
    
    // Disable rotation since we don't need it
    rotation_enabled = 0;
    target_pos.up = (vec4) {0, 1, 0, 0.0};

    move_to_cell(0, 0);
    start_animation();
}

// Changes eye distance from at point while maintaining VPN
void fix_eye_dist() {
    vec4 vpn = normalize_v4(sub_v4(target_pos.eye, target_pos.at));
    vec4 new_eye = add_v4(target_pos.at, mult_v4(vpn, distance_from_center));

    target_pos.eye = new_eye;
}

void set_trackball_pos(vec4 eye, vec4 up) {
    target_pos = (view_position) { eye, island_center, up };
    fix_eye_dist();
    ctm = m4_identity();
    previous_rotation_matrix = ctm;
    rotation_enabled = 1;

    start_animation();
}

void set_topdown_view() {
    vec4 eye = (vec4) {island_center.x, max_side, island_center.z, 1.0};
    vec4 up = (vec4) {0, 0, -1, 0};

    set_trackball_pos(eye, up);
}

void set_side_view() {
    vec4 eye = (vec4) {island_center.x, 0, max_side, 0.0};
    vec4 up = (vec4) {0.0, 1.0, 0.0, 0.0};

    set_trackball_pos(eye, up);
}

void rotate_sun(int degrees)
{
    //sun_ctm = translation(-((left + right) / 2), 0, -((near + far) / 2));
    sun_ctm = matrixmult_mat4(translation(-((left + right) / 2), 0, -((top + bottom) / 2)), sun_ctm);
    sun_ctm = matrixmult_mat4(rotate_z(degrees), sun_ctm);
    sun_ctm = matrixmult_mat4(translation(((left + right) / 2), 0, ((top + bottom) / 2)), sun_ctm);

    light_position = vectormult_mat4(rotate_z(degrees), light_position);
}

void keyboard(unsigned char key, int mousex, int mousey)
{
    // If we're animating, don't accept keyboard commands
    if (is_animating)
    {
        return;
    }


    switch(key) 
    {
        case 'q':
            exit(0);
        case 'w':
            move_direction(player_facing);
            break;
        case 'a':
            move_direction(get_left_direction(player_facing));
            break;
        case 's':
            move_direction(get_behind_direction(player_facing));
            break;
        case 'd':
            move_direction(get_right_direction(player_facing));
            break;
        case 'j':
            turn(get_left_direction(player_facing));
            break;
        case 'l':
            turn(get_right_direction(player_facing));
            break;
        case 't':
            // Reset View
            set_topdown_view();
            break;
        case 'e':
            go_to_entrance();
            break;
        case 'r':
            set_side_view();
            break;
        case 'p':
            if (maze_x == 0 && maze_y == 0) {
                navigate(dfs);
            }
            break;
        case 'i':
            navigate(dfs_anyposition);
            break;
        case 'b':
            if(lighting_enabled == 1) {
                use_ambient ^= 0x1;
                glUniform1i(use_ambient_location, use_ambient);
                printf("Ambient: %s\n", use_ambient == 0 ? "OFF" : "ON");
            }
            break;
        case 'n':
            if(lighting_enabled == 1) {
                use_diffuse ^= 0x1;
                glUniform1i(use_diffuse_location, use_diffuse);
                printf("Diffuse: %s\n", use_diffuse == 0 ? "OFF" : "ON");
            }
            break;
        case 'm':
            if(lighting_enabled == 1) {
                use_specular ^= 0x1;
                glUniform1i(use_specular_location, use_specular);
                printf("Specular: %s\n", use_specular == 0 ? "OFF" : "ON");
            }
            break;
        case 'v':
            lighting_enabled ^= 0x1;
            glUniform1i(light_enabled_location, lighting_enabled);
            if(lighting_enabled == 0) {
                use_ambient = 0;
                glUniform1i(use_ambient_location, use_ambient);
                use_diffuse = 0;
                glUniform1i(use_diffuse_location, use_diffuse_location);
                use_specular = 0;
                glUniform1i(use_specular_location, use_specular);
            }
            break;
        case 'c':
            if(lighting_enabled == 1) {
                if(use_ambient == 0 && use_diffuse == 0 && use_specular == 0) {
                    use_flashlight ^= 0x1;
                    glUniform1i(use_flashlight_location, use_flashlight);
                    if(use_flashlight == 0) {
                        glUniform4fv(light_position_location, 1, (GLvoid *) &light_position);
                    }
                    else {
                        glUniform4fv(light_position_location, 1, (GLvoid *) &target_pos.eye);
                    }
                    printf("Flashlight: %s\n", use_flashlight == 0 ? "OFF" : "ON");
                }
            }
            break;
        case '-':
            if(rotation_enabled)
            {
                target_pos = current_pos;
                target_pos.eye = add_v4(current_pos.eye, (vec4) {0, 0, 10, 0.0});
                start_animation();
            }
            break;
        case '=':
            if(rotation_enabled)
            {
                target_pos = current_pos;
                target_pos.eye = sub_v4(current_pos.eye, (vec4) {0, 0, 10, 0.0});
                start_animation();
            }
            break;
        case 'k':
            // test_val += 50;
            // printf("%d\n", test_val);
            // sun_ctm = translation(test_val, 0, 0);

            break;
        
    }

    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (is_animating)
    {
        return;
    }
    
    switch (button) {
        case GLUT_LEFT_BUTTON:
            if (rotation_enabled) {
                // Rotation
                if (state == GLUT_DOWN) {
                    float x_coordinate = (x * 2.0 / 1023.0) - 1;
                    float y_coordinate = -((y * 2.0 / 1023.0) - 1);
                    float z_coordinate = sqrt(1 - pow(x_coordinate, 2) - pow(y_coordinate, 2));

                    click_vector = (vec4) {x_coordinate, y_coordinate, z_coordinate, 0.0};
                    click_vector = normalize_v4(click_vector);
                } else {
                    previous_rotation_matrix = ctm;
                }
            } else {
                // Flashlight
            }

            break;
    }
}

void motion(int x, int y) {
    if (is_animating)
    {
        return;
    }
    
    if (rotation_enabled) {
        // Rotation
        float x_coordinate = (x * 2.0 / 1023.0) - 1;
        float y_coordinate = -((y * 2.0 / 1023.0) - 1);
        float z_coordinate = sqrt(1 - pow(x_coordinate, 2) - pow(y_coordinate, 2));

        // Disable rotation if z is nan
        if(isnan(z_coordinate) == 1) return;

        vec4 drag_vector = (vec4) {x_coordinate, y_coordinate, z_coordinate, 0.0};
        drag_vector = normalize_v4(drag_vector);

    // Take the cross product to get the rotate about vector
    vec4 rotation_vector = crossprod_v4(click_vector, drag_vector);
    rotation_vector = normalize_v4(rotation_vector);

        // If the subtraction results in a nan, don't rotate
        if(isnan(rotation_vector.x) || isnan(rotation_vector.y) ||  isnan(rotation_vector.z) || isnan(rotation_vector.w)) return;

        GLfloat ax = rotation_vector.x;
        GLfloat ay = rotation_vector.y;
        GLfloat az = rotation_vector.z;
        GLfloat d = sqrt(pow(ay, 2) + pow(az, 2));

    if(is_first_rotation){
        previous_rotation_matrix = ctm;
        is_first_rotation = 0;
    }

        ctm = matrixmult_mat4(translation(-((left + right) / 2), -((bottom + top) / 2), -((near + far) / 2)), previous_rotation_matrix);
        ctm = matrixmult_mat4(rotate_arbitrary_x(ay, az, d), ctm);
        ctm = matrixmult_mat4(rotate_arbitrary_y(ax, d), ctm);

        float z_degrees = acos(dotprod_v4(click_vector, drag_vector)) * 180.0 / M_PI;
        ctm = matrixmult_mat4(rotate_z(z_degrees), ctm);

        ctm = matrixmult_mat4(transpose_mat4(rotate_arbitrary_y(ax, d)), ctm);
        ctm = matrixmult_mat4(transpose_mat4(rotate_arbitrary_x(ay, az, d)), ctm);
        ctm = matrixmult_mat4(translation(((left + right) / 2), ((bottom + top) / 2), ((near + far) / 2)), ctm);

        glutPostRedisplay();
    } else {
        // Flashlight
    }
}

void idle(void)
{
    if (!is_animating)
    {
        return;
    }

    long elapsed = get_micro_time() - animation_started;
    
    // Are we at the target yet?
    if (elapsed >= ANIMATION_DURATION)
    {
        current_pos = target_pos;
        model_view = look_at(target_pos.eye, target_pos.at, target_pos.up);

        // If its go to entrance, turn
        is_animating = 0;
        do_maze_step();
    } else {
        float progress = (float)elapsed / ANIMATION_DURATION;
        
        vec4 eye_delta = mult_v4(eye_move_vector, progress);
        vec4 eye_temp_pos = add_v4(current_pos.eye, eye_delta);

        vec4 at_delta = mult_v4(at_move_vector, progress);
        vec4 at_temp_pos = add_v4(current_pos.at, at_delta);

        model_view = look_at(eye_temp_pos, at_temp_pos, target_pos.up);
    }

    glutPostRedisplay();
}

void init(void)
{
    // Set starting location
    distance_from_center = 1.25 * max_side;
    set_side_view();
    current_pos = target_pos;
    model_view = look_at(current_pos.eye, current_pos.at, current_pos.up);
    is_animating = 0;

    // Load textures
    int tex_width = 64;
    int tex_height = 64;
    GLubyte my_texels[tex_width][tex_height][3];

    FILE *fp = fopen("textures02.raw", "r");
    fread(my_texels, tex_width * tex_height * 3, 1, fp);
    fclose(fp);

    GLuint mytex[1];
    glGenTextures(1, mytex);
    glBindTexture(GL_TEXTURE_2D, mytex[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, my_texels);
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

    int param;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &param);

    // Initialize buffers
    GLuint vao;
    #ifdef __APPLE__
    glGenVertexArraysAPPLE(1, &vao);
    glBindVertexArrayAPPLE(vao);
    #else
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    #endif

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * 2 * num_vertices + sizeof(vec2) * num_vertices, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * num_vertices, positions);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * num_vertices, sizeof(vec4) * num_vertices, normals);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * 2 * num_vertices, sizeof(vec2) * num_vertices, tex_coords);

    // Initialize program
    GLuint program = initShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (0));

    GLuint vNormal = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (sizeof(vec4) * num_vertices));
    
    GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (sizeof(vec4) * 2 * num_vertices));

    current_transformation_matrix = glGetUniformLocation(program, "ctm");
    model_view_location = glGetUniformLocation(program, "model_view");
    projection_location = glGetUniformLocation(program, "projection");
    projection = frustum(-CLIP_NEAR, CLIP_NEAR, -CLIP_NEAR, CLIP_NEAR, -CLIP_NEAR, -100000);

    GLuint texture_location = glGetUniformLocation(program, "texture");
    glUniform1i(texture_location, 0);

    light_enabled_location = glGetUniformLocation(program, "lighting_enabled");
    glUniform1i(light_enabled_location, lighting_enabled);

    use_ambient_location = glGetUniformLocation(program, "use_ambient");
    glUniform1i(use_ambient_location, use_ambient);

    use_diffuse_location = glGetUniformLocation(program, "use_diffuse");
    glUniform1i(use_diffuse_location, use_diffuse);

    use_specular_location = glGetUniformLocation(program, "use_specular");
    glUniform1i(use_specular_location, use_specular);
    
    GLuint attenuation_constant_location = glGetUniformLocation(program, "attenuation_constant");
    glUniform1f(attenuation_constant_location, 4.0);

    GLuint attenuation_linear_location = glGetUniformLocation(program, "attenuation_linear");
    glUniform1f(attenuation_linear_location, 3.0);

    GLuint attenuation_quadratic_location = glGetUniformLocation(program, "attenuation_linear");
    glUniform1f(attenuation_quadratic_location, 1.0);

    use_flashlight_location = glGetUniformLocation(program, "use_flashlight");
    glUniform1i(use_flashlight_location, use_flashlight);

    GLuint light_position_location = glGetUniformLocation(program, "light_position");
    glUniform4fv(light_position_location, 1, (GLvoid *) &light_position);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDepthRange(1,0);
}

void display(void)
{
    glClearColor(120.0/255.0, 167.0/255.0, 1.0, 1.0); // Set clear color to the minecraft sky color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUniformMatrix4fv(model_view_location, 1, GL_FALSE, (GLfloat *) &model_view);
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, (GLfloat *) &projection);


    glUniformMatrix4fv(current_transformation_matrix, 1, GL_FALSE, (GLfloat *) &ctm);
    glDrawArrays(GL_TRIANGLES, 0, num_vertices - 36);

    glUniformMatrix4fv(current_sun_matrix, 1, GL_FALSE, (GLfloat *) &sun_ctm);
    glDrawArrays(GL_TRIANGLES, num_vertices - 36, num_vertices);

    glutSwapBuffers();
}

int main(int argc, char **argv)
{
    define_blocks();
    srand(time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(1024, 1024);
    glutInitWindowPosition(100,100);
    glutCreateWindow("Maze");
    #ifndef __APPLE__
    glewInit();
    #endif
    prompt_maze_size();
    generate_world();
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idle);
    print_helper_text();
    glutMainLoop();

    return 0;
}

