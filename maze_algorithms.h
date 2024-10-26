#ifndef MAZE_ALGORITHMS_H
#define MAZE_ALGORITHMS_H

#define MAZE_WIDTH 8
#define MAZE_HEIGHT 8

typedef struct {
    int top;
    int right;
    int bottom;
    int left;
} Cell;

void generate_maze(Cell (*maze)[MAZE_WIDTH]);
void print_maze(Cell (*maze)[MAZE_WIDTH]);

#endif