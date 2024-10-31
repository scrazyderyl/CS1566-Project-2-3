#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "maze_algorithms.h"

void set_left(Cell **maze, int x, int y, int value) {
    maze[x][y].left = value;
    maze[x - 1][y].right = value;
}

void set_right(Cell **maze, int x, int y, int value) {
    maze[x][y].right = value;
    maze[x + 1][y].left = value;
}

void set_top(Cell **maze, int x, int y, int value) {
    maze[x][y].top = value;
    maze[x][y - 1].bottom = value;
}

void set_bottom(Cell **maze, int x, int y, int value) {
    maze[x][y].bottom = value;
    maze[x][y + 1].top = value;
}

void generate_empty_maze(Cell **maze, int width, int height) {
    // Initialize values to open
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            maze[x][y] = (Cell) { 0, 0, 0, 0 };
        }
    }

    for (int x = 0; x < width; x++) {
        maze[x][0].top = 1; // Maze top
        maze[x][height - 1].bottom = 1; // Maze bottom
    }

    for (int y = 0; y < height; y++) {
        maze[0][y].left = 1; // Maze left
        maze[width - 1][y].right = 1; // Maze right
    }

    // Open starting points
    maze[0][0].left = 0;
    maze[width - 1][height - 1].right = 0;
}

void generate_recursive(Cell **maze, int x1, int y1, int x2, int y2) {
    int x_range = x2 - x1;
    int y_range = y2 - y1;

    // Stop if width or height is 1
    if (x_range == 0 || y_range == 0) {
        return;
    }

    // Pick center
    int x_center = x1 + rand() % x_range;
    int y_center = y1 + rand() % y_range;

    // Create horizontal walls
    for (int x = x1; x <= x2; x++) {
        set_bottom(maze, x, y_center, 1);
    }

    // Create vertical walls
    for (int y = y1; y <= y2; y++) {
        set_right(maze, x_center, y, 1);
    }

    // Remove three walls
    int walls[4] = { 1, 1, 1, 1 };
    walls[rand() % 4] = 0;

    // Remove top wall
    if (walls[0]) {
        int size = y_center - y1 + 1;
        int y = y1 + rand() % size;
        set_right(maze, x_center, y, 0);
    }

    // Remove right wall
    if (walls[1]) {
        int size = x2 - x_center;
        int x = x_center + 1 + rand() % size;
        set_bottom(maze, x, y_center, 0);
    }

    // Remove bottom wall
    if (walls[2]) {
        int size = y2 - y_center;
        int y = y_center + 1 + rand() % size;
        set_right(maze, x_center, y, 0);
    }

    // Remove left wall
    if (walls[3]) {
        int size = x_center - x1 + 1;
        int x = x1 + rand() % size;
        set_bottom(maze, x, y_center, 0);
    }
    
    // Recurse for each quadrant
    generate_recursive(maze, x1, y1, x_center, y_center); // Top left
    generate_recursive(maze, x_center + 1, y1, x2, y_center); // Top right
    generate_recursive(maze, x1, y_center + 1, x_center, y2); // Bottom left
    generate_recursive(maze, x_center + 1, y_center + 1, x2, y2); // Bottom right
}

void generate_maze(Cell **maze, int width, int height) {
    generate_empty_maze(maze, width, height);
    generate_recursive(maze, 0, 0, width - 1, height - 1);
}

void print_maze(Cell **maze, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            Cell cell = maze[x][y];
            printf("+%s", cell.top ? "---" : "   ");
        }

        printf("+\n");

        for (int x = 0; x < width; x++) {
            Cell cell = maze[x][y];
            printf("%c   ", cell.left ? '|' : ' ');
        }

        printf("%c\n", maze[width - 1][y].right ? '|' : ' ');
    }

    // Bottom of maze
    for (int x = 0; x < width; x++) {
        Cell cell = maze[x][height - 1];
        printf("+%s", cell.bottom ? "---" : "   ");
    }

    printf("+\n");
}

void main(int argc, const char *argv[]) {
    srand(time(NULL));

    int width = 8;
    int height = 8;
    
    Cell *maze[width];

    for (int i = 0; i < width; i++) {
        maze[i] = malloc(height * sizeof(Cell));
    }

    generate_maze(maze, width, height);
    print_maze(maze, width, height);
}