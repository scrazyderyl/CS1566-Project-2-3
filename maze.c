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

int ISLAND_PADDING = 6;
int CELL_SIZE = 3;
int WALL_HEIGHT = 5;
int REMOVE_DIST = 2;

float TEX_SIZE = 0.25;

Cell **maze;
int width;
int height;

size_t num_vertices;
size_t vertex_index = 0;
vec4 *positions;
vec2 *tex_coords;

mat4 ctm;
GLuint ctm_location;

// Rotation variable so mouse and motion can interact
vec4 click_vector;
mat4 previous_rotation_matrix;
int rotation_enabled;
int is_first_rotation = 1;

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
    int maze_x_size = width * (CELL_SIZE + 1) + 1;
    int maze_z_size = height * (CELL_SIZE + 1) + 1;

    int total_x_size = ISLAND_PADDING * 2 + maze_x_size;
    int total_z_size = ISLAND_PADDING * 2 + maze_z_size;
    int island_smaller_side = maze_x_size < maze_z_size ? maze_x_size : maze_z_size;
    int total_y_size = 1 + WALL_HEIGHT + (island_smaller_side + 1) / 2;

    size_t maze_floor_blocks = maze_x_size * maze_z_size;
    size_t maze_walls_blocks = ((height + 1) * (width + 1) + (height * (width + 1) + width * (height + 1) - width * height - 1) * CELL_SIZE) * WALL_HEIGHT;
    size_t island_blocks = total_x_size * total_z_size * (REMOVE_DIST + (island_smaller_side + 1) / 2); // Not correct, should be 34662 for 10x10 maze
    num_vertices = (maze_floor_blocks + maze_walls_blocks + island_blocks) * 36;

    // Center x and z and scale
    int max = total_x_size;

    if (total_y_size > max) {
        max = total_y_size;
    }

    if (total_z_size > max) {
        max = total_z_size;
    }

    ctm = translation((float)(total_x_size - 2 * ISLAND_PADDING) / -2, 0, (float)(total_z_size - 2 * ISLAND_PADDING) / -2);
    float scale_factor = 1 / (float)max;
    ctm = matrixmult_mat4(scale(scale_factor, scale_factor, scale_factor), ctm);

    // Allocate arrays
    positions = (vec4 *) malloc(sizeof(vec4) * num_vertices);
    tex_coords = (vec2 *) malloc(sizeof(vec2) * num_vertices);

    // Generate

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
    for (int y = 0; y < height; y++) {
        int z_pos = y * (CELL_SIZE + 1);

        Cell cell;

        // Top wall
        for (int x = 0; x < width; x++) {
            cell = maze[x][y];
            int x_pos = x * (CELL_SIZE + 1);

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
    for (int x = 0; x < width; x++) {
        Cell cell = maze[x][height - 1];
        int x_pos = x * (CELL_SIZE + 1);

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

    width = 10;
    height = 10;

    if (1)
    // if(scanf("%d %d", &width, &height) > 0 && width > 0 && height > 0)
    {
        maze = malloc(width * sizeof(Cell *));

        for (int i = 0; i < width; i++) {
            maze[i] = malloc(height * sizeof(Cell));
        }

        printf("Width: %d Height: %d\n", width, height);
        generate_maze(maze, width, height);
        print_maze(maze, width, height);
    }
    else
    {
        printf("\nInvalid Input! Exiting...\n");
        exit(1);
    }
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

    ctm_location = glGetUniformLocation(program, "ctm");

    GLuint texture_location = glGetUniformLocation(program, "texture");
    glUniform1i(texture_location, 0);

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glDepthRange(1,0);
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniformMatrix4fv(ctm_location, 1, GL_FALSE, (GLfloat*) &ctm);

    glDrawArrays(GL_TRIANGLES, 0, num_vertices);

    glutSwapBuffers();
}

void keyboard(unsigned char key, int mousex, int mousey)
{
    
    switch(key) {
        case 'q':
            exit(0);
        case 'w':
            // Move forward
            break;
        case 'a':
            // Move left
            break;
        case 'd':
            // Move right
            break;
        case 's':
            // Move back
            break;
        case 'n':
            // Move right
            break;
    }

    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if(state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
        float x_coordinate = (x * 2.0 / 1023.0) - 1;
        float y_coordinate = -((y * 2.0 / 1023.0) - 1);
        float z_coordinate = sqrt(1 - pow(x_coordinate, 2) - pow(y_coordinate, 2));

        // Disable rotation if z is nan
        rotation_enabled = isnan(z_coordinate) ? 0 : 1;

        click_vector = (vec4) {x_coordinate, y_coordinate, z_coordinate, 0.0};
        click_vector = normalize_v4(click_vector);
    } 

    if(state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
        previous_rotation_matrix = ctm;
    }
    
}

void motion(int x, int y) {
    if(rotation_enabled) {
        float x_coordinate = (x * 2.0 / 1023.0) - 1;
        float y_coordinate = -((y * 2.0 / 1023.0) - 1);
        float z_coordinate = sqrt(1 - pow(x_coordinate, 2) - pow(y_coordinate, 2));

        // Disable rotation if z is nan
        rotation_enabled = isnan(z_coordinate) ? 0 : 1;

        vec4 drag_vector = (vec4) {x_coordinate, y_coordinate, z_coordinate, 0.0};
        drag_vector = normalize_v4(drag_vector);

        if(rotation_enabled) {
            // Take the cross product to get the rotate about vector
            vec4 rotation_vector = crossprod_v4(click_vector, drag_vector);
            rotation_vector = normalize_v4(rotation_vector);

            // If the subtraction results in a nan, don't rotate
            if(isnan(rotation_vector.x) || isnan(rotation_vector.y) ||  isnan(rotation_vector.z) || isnan(rotation_vector.w)) return;

            GLfloat ax = rotation_vector.x;
            GLfloat ay = rotation_vector.y;
            GLfloat az = rotation_vector.z;
            GLfloat d = sqrt(pow(ay, 2) + pow(az, 2));

            // If there is no previous rotation matrix since this is the first
            // set the previous to the current ctm
            if(is_first_rotation){
                previous_rotation_matrix = ctm;
                is_first_rotation = 0;
            }

            ctm = matrixmult_mat4(rotate_arbitrary_x(ay, az, d), previous_rotation_matrix);
            ctm = matrixmult_mat4(rotate_arbitrary_y(ax, d), ctm);

            float z_degrees = acos(dotprod_v4(click_vector, drag_vector)) * 180.0 / M_PI;
            ctm = matrixmult_mat4(rotate_z(z_degrees), ctm);

            ctm = matrixmult_mat4(transpose_mat4(rotate_arbitrary_y(ax, d)), ctm);
            ctm = matrixmult_mat4(transpose_mat4(rotate_arbitrary_x(ay, az, d)), ctm);
        }
        
        else return;
    }
    else return;


    glutPostRedisplay();
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
    glutMainLoop();

    return 0;
}
