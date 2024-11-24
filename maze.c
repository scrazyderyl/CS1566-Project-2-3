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
#include <math.h>
#include "initShader.h"
#include "myLib.h"
#include "maze_algorithms.h"

typedef struct {
    vec2 x_pos;
    vec2 x_neg;
    vec2 y_pos;
    vec2 y_neg;
    vec2 z_pos;
    vec2 z_neg;
} Block;

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

typedef struct Coordinates_ {
    int x;
    int y;
    struct Coordinates_ *next;
} Coordinates;

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

int maze_x;
int maze_y;
int player_facing; // 0: Pos x, 1: Pos y, 2: Neg x, 3: Neg y 
 
// OpenGL
size_t num_vertices;
size_t vertex_index = 0;
vec4 *positions;
vec2 *tex_coords;

GLuint current_transformation_matrix;
mat4 ctm = {{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}};

GLuint model_view_location;
mat4 model_view = {{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}};

GLuint projection_location;
mat4 projection = {{1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1}};

// Rotation variable so mouse and motion can interact
vec4 click_vector;
mat4 previous_rotation_matrix;
int rotation_enabled = 1;
int is_first_rotation = 1;

// Animation Variables
int is_animating = 0;
int current_step_count = 0; 
int num_steps = 20; // Fragment animation into this number of steps
int current_animation_type = 0; // 0 is a normal animation, 1 is the animation from side view to entrance

typedef struct {
    vec4 eye, at, up;
} view_position;

view_position current_pos, target_pos;

// Sets the view to a towdown view of the maze
void set_topdown_view() {
    model_view = look_at(0, 0, 0, // eye
                         0, -1, 0, // at
                         0, 0, -1); // up

    model_view = matrixmult_mat4(model_view, ortho(left - 10, right + 10, bottom - 10, top + 10, near + 10, far - 10)); // Scale to fit view

    // Set the position of the viewer
    vec4 cur_eye = (vec4) {0.0, 0.0, 0.0, 0.0};
    vec4 cur_at = (vec4) {0.0, -1.0, 0.0, 0.0};
    vec4 cur_up = (vec4) {0.0, 0.0, -1.0, 0.0};
    current_pos = (view_position) {cur_eye, cur_at, cur_up};

    // No need for projection here, reset it
    projection = m4_identity();

    // Have the rotation remember that you're looking from the top now
    previous_rotation_matrix = model_view;

    // Re-enable Rotation
    rotation_enabled = 1;
}

// Sets the view to the default side view
void set_side_view() {

    model_view = look_at((float)(left + right) / 2, 0, 50, // eye
                         (float)(left + right) / 2, 0, -50, // at
                         0, 1, 0); // up

    //model_view = matrixmult_mat4(ortho(left - 10, right + 10, bottom - 10, top + 10, near + 10, far - 10), model_view);
    projection = frustum(left - 12, right - 12, bottom + 10, top - 10, far - 10, near - 10);
    //projection = frustum(-1, 1, 0, 2, far - 10, near - 10);
    //projection = frustum(far - 12, near - 12, bottom + 10, top - 10, left - 10, right - 10);

    // Set the position of the viewer
    vec4 cur_eye = (vec4) {(float)(left + right) / 2, 0, 50, 0.0};
    vec4 cur_at = (vec4) {(float)(left + right) / 2, 0, -50, 0.0};
    vec4 cur_up = (vec4) {0.0, 1.0, 0.0, 0.0};

    current_pos = (view_position) {cur_eye, cur_at, cur_up};

    // Have the rotation remember that you're looking from the side now
    previous_rotation_matrix = model_view;

    // Re-enable Rotation
    rotation_enabled = 1;
}

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

int try_probability(int numerator, int denominator) {
    return numerator > rand() % denominator;
}

void set_block(int x, int y, int z, Block block) {
    set_cube_vertices(vertex_index, x, y, z, 1);
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

    size_t maze_floor_blocks = maze_x_size * maze_z_size;
    size_t maze_walls_blocks = ((maze_height + 1) * (maze_width + 1) + (maze_height * (maze_width + 1) + maze_width * (maze_height + 1) - maze_width * maze_height - 1) * CELL_SIZE) * WALL_HEIGHT;
    size_t island_blocks = total_x_size * total_z_size * (REMOVE_DIST + 1);

    int long_length = island_larger_side;

    for (int short_length = island_smaller_side; short_length > 0; short_length -= 2) {
        island_blocks += long_length * short_length;
        long_length -= 2;
    }

    // num_vertices = (maze_floor_blocks + maze_walls_blocks + island_blocks) * 36;
    num_vertices = (maze_floor_blocks + maze_walls_blocks + island_blocks + 10) * 36; // This is for the testing boundary blocks. DELETE BEFORE TURNING IN!!!!!

    // Store bounds
    left = -ISLAND_PADDING;
    right = total_x_size - ISLAND_PADDING;
    bottom = -total_y_size - 2;
    top = total_y_size + 2;
    near = total_z_size - ISLAND_PADDING;
    far = -ISLAND_PADDING;

    printf("X: %d, Y: %d Z: %d\n", total_x_size, total_y_size, total_z_size);
    printf("Left: %d Right: %d\n", left, right);
    printf("Bottom: %d Top: %d\n", bottom, top);
    printf("Near: %d Far: %d\n", near, far);

    set_side_view();

    // Allocate arrays
    positions = (vec4 *) malloc(sizeof(vec4) * num_vertices);
    tex_coords = (vec2 *) malloc(sizeof(vec2) * num_vertices);

    // Generate bound blocks for testing purposes
    set_block(left, top, near, BLOCK_BRICKS);
    set_block(right, top, near, BLOCK_BRICKS);

    set_block(left, bottom, near, BLOCK_BRICKS);
    set_block(right, bottom, near, BLOCK_BRICKS);

    set_block(left, top, far, BLOCK_BRICKS);
    set_block(right, top, far, BLOCK_BRICKS);

    set_block(left, bottom, far, BLOCK_BRICKS);
    set_block(right, bottom, far, BLOCK_BRICKS);

    set_block((left + right) / 2, top, (near + far) / 2, BLOCK_GRASS);
    set_block((left + right) / 2, bottom, (near + far) / 2, BLOCK_BIRCH_PLANKS);

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
/*
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
    if(loc_x == width-1 && loc_y == height-1) {
        printf("Found exit\n");
        curr->next->
        return 1;
    }

    int found = 0;
    if(loc_y > 0 && loc.top == 0 && dir != 2) {
        printf("Move top\n");
        found = dfs_anyposition_recursive(maze[loc_x][loc_y-1], loc_x, loc_y-1, 0, curr->next);
        //printf(" (%d,%d)\n", current->x, current->y);
    }
    if(found != 1 && loc_y < height-1 && loc.bottom == 0 && dir != 0) {
        printf("Move bottom\n");
        found = dfs_anyposition_recursive(maze[loc_x][loc_y+1], loc_x, loc_y+1, 2, curr->next);
        //printf(" (%d,%d)\n", current->x, current->y);
    }
    if(found != 1 && loc_x > 0 && loc.left == 0 && dir != 3) {
        printf("Move left\n");
        found = dfs_anyposition_recursive(maze[loc_x-1][loc_y], loc_x-1, loc_y, 1, curr->next);
        //printf("(%d,%d)\n", current->x, current->y);
    }
    if(found != 1 && loc_x < width-1 && loc.right == 0 && dir != 1) {
        printf("Move right\n");
        found = dfs_anyposition_recursive(maze[loc_x+1][loc_y], loc_x+1, loc_y, 3, curr->next);
        //printf("(%d,%d)\n", curr->x, curr->y);
    }
    return found;
}

void dfs_anyposition() {
    Cell start = maze[maze_x][maze_y];
    list = (struct Coordinate *) malloc(sizeof(Coordinate));
    list->x = maze_x;
    list->y = maze_y;
    int found = dfs_anyposition_recursive(start, maze_x, maze_y, -1, list);
    list = list->next;
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
*/
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

    printf("\n---------[Camera]---------\n");
    printf("T - Topdown View\n");
    printf("R - Reset to Side View\n");
    printf("E - Go to Entrance\n");


    printf("\n");
    
}

void init(void)
{
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * num_vertices + sizeof(vec2) * num_vertices, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec4) * num_vertices, positions);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec4) * num_vertices, sizeof(vec2) * num_vertices, tex_coords);

    // Initialize program
    GLuint program = initShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (0));

    GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *) (sizeof(vec4) * num_vertices));

    current_transformation_matrix = glGetUniformLocation(program, "ctm");
    model_view_location = glGetUniformLocation(program, "model_view");
    projection_location = glGetUniformLocation(program, "projection");

    GLuint texture_location = glGetUniformLocation(program, "texture");
    glUniform1i(texture_location, 0);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDepthRange(1,0);
}

void display(void)
{
    glClearColor(120.0/255.0, 167.0/255.0, 1.0, 1.0); // Set clear color to the minecraft sky color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    

    glUniformMatrix4fv(current_transformation_matrix, 1, GL_FALSE, (GLfloat *) &ctm);
    glUniformMatrix4fv(model_view_location, 1, GL_FALSE, (GLfloat *) &model_view);
    glUniformMatrix4fv(projection_location, 1, GL_FALSE, (GLfloat *) &projection);

    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    glutSwapBuffers();
}

void update_positions(vec4 position, int facing) {
    float x = position.x;
    float y = position.y;
    float z = position.z;

    target_pos.eye = (vec4) {x, y, z, 0.0};
    target_pos.up = (vec4) {0, 1, 0, 0.0};

    // update target pos
    switch (facing) {
        case 0:
            //return look_at(x, y, z, x + 1, y - 1, z, 0, 1, 0);
            target_pos.at = (vec4) {x + 1, y - 1, z, 0.0};
        case 1:
            //return look_at(x, y, z, x, y - 1, z + 1, 0, 1, 0);
            target_pos.at = (vec4) {x, y - 1, z + 1, 0.0};
        case 2:
            //return look_at(x, y, z, x - 1, y - 1, z, 0, 1, 0);
            target_pos.at = (vec4) {x - 1, y - 1, z, 0.0};
        case 3:
            //return look_at(x, y, z, x, y - 1, z - 1, 0, 1, 0);
            target_pos.at = (vec4) {x, y - 1, z - 1, 0.0};
            
    }
}

void start_animation(int type) {
    current_animation_type = type;

    current_step_count = 0;
    is_animating = 1;

}

void move_to(vec4 position) {
    update_positions(position, player_facing);
}

void turn_to(int direction) {
    update_positions(current_pos.eye, direction);
    start_animation(0);
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

    start_animation(0);
}

void turn(int direction) {
    if (rotation_enabled) {
        return;
    }

    player_facing = direction;
    turn_to(direction);
}
/*
void do_maze_step() {
    if (current_step == NULL) {
        return;
    }
    
    // Turn to exit if at end
    if (current_step->next == NULL) {
        if (player_facing != 0) {
            turn_to(0);
            start_animation(0);
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
        turn(new_direction);
    } else {
        // Move
        move_to_cell(next->x, next->y);
        current_step = next;
    }

    start_animation(0);
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
*/
void go_to_entrance()
{
    // Disable rotation since we don't need it
    rotation_enabled = 0;
    projection = frustum(-1, 1, 0, 2, -0.5, -150);

    move_to_cell(-1, 0);
    start_animation(1);
}

void keyboard(unsigned char key, int mousex, int mousey)
{
    // If we're animating, don't accept keyboard commands
    if(! is_animating)
    {
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
                turn_to(get_left_direction(player_facing));
                break;
            case 'l':
                turn_to(get_right_direction(player_facing));
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
                //navigate(dfs);
                break;
            case 'i':
                //navigate(dfs_anyposition);
                break;
        }

        glutPostRedisplay();
    }
}

void mouse(int button, int state, int x, int y) {
    if(state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
        if(rotation_enabled)
        {
            float x_coordinate = (x * 2.0 / 1023.0) - 1;
            float y_coordinate = -((y * 2.0 / 1023.0) - 1);
            float z_coordinate = sqrt(1 - pow(x_coordinate, 2) - pow(y_coordinate, 2));

            click_vector = (vec4) {x_coordinate, y_coordinate, z_coordinate, 0.0};
            click_vector = normalize_v4(click_vector);
        }
    } 

    if(state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
        previous_rotation_matrix = model_view;
    }
    
}

void motion(int x, int y) {
    if(rotation_enabled) 
    {
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
                previous_rotation_matrix = model_view;
                is_first_rotation = 0;
            }

            //model_view = matrixmult_mat4(translation(-((left + right) / 2), -((bottom + top) / 2), -((near + far) / 2)), previous_rotation_matrix);
            model_view = matrixmult_mat4(rotate_arbitrary_x(ay, az, d), previous_rotation_matrix);
            model_view = matrixmult_mat4(rotate_arbitrary_y(ax, d), model_view);

            float z_degrees = acos(dotprod_v4(click_vector, drag_vector)) * 180.0 / M_PI;
            model_view = matrixmult_mat4(rotate_z(z_degrees), model_view);

            model_view = matrixmult_mat4(transpose_mat4(rotate_arbitrary_y(ax, d)), model_view);
            model_view = matrixmult_mat4(transpose_mat4(rotate_arbitrary_x(ay, az, d)), model_view);
            //model_view = matrixmult_mat4(translation(((left + right) / 2), ((bottom + top) / 2), ((near + far) / 2)), model_view);
    }
    else return;


    glutPostRedisplay();
}

void idle(void)
{
    if(is_animating)
    {
        // Are the start and end positions the same?
        if(equal_v4(target_pos.eye, current_pos.eye) && equal_v4(target_pos.at, current_pos.at) && equal_v4(target_pos.up, current_pos.up))
        {
            is_animating = 0;
            current_step_count = num_steps;
        }
        // Are we at the target yet?
        else if(current_step_count == num_steps)
        {
            current_pos = target_pos;
            model_view = look_at(target_pos.eye.x, target_pos.eye.y, target_pos.eye.z, 
                                 target_pos.at.x, target_pos.at.y, target_pos.at.z, 
                                 target_pos.up.x, target_pos.up.y, target_pos.up.z);

            // If its go to entrance, turn
            if(current_animation_type == 1)
            {
                //turn(get_right_direction(player_facing));
            }

            // Arrived at destination, no longer animating
            is_animating = 0;
        }
        else
        {
            vec4 eye_move_vector = sub_v4(target_pos.eye, current_pos.eye);
            vec4 eye_delta = mult_v4(eye_move_vector, (float) current_step_count / num_steps);
            vec4 eye_temp_pos = add_v4(current_pos.eye, eye_delta);

            vec4 at_move_vector = sub_v4(target_pos.at, current_pos.at);
            vec4 at_delta = mult_v4(at_move_vector, (float) current_step_count / num_steps);
            vec4 at_temp_pos = add_v4(current_pos.at, at_delta);

            vec4 up_move_vector = sub_v4(target_pos.up, current_pos.up);
            vec4 up_delta = mult_v4(up_move_vector, (float) current_step_count / num_steps);
            vec4 up_temp_pos = add_v4(current_pos.up, up_delta);

            model_view = look_at(eye_temp_pos.x, eye_temp_pos.y, eye_temp_pos.z, 
                                 at_temp_pos.x, at_temp_pos.y, at_temp_pos.z, 
                                 up_temp_pos.x, up_temp_pos.y, up_temp_pos.z);

            current_step_count++;
        }
        glutPostRedisplay();
    }
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

