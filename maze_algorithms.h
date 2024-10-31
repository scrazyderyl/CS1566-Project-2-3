#ifndef MAZE_ALGORITHMS_H
#define MAZE_ALGORITHMS_H

typedef struct {
    int top;
    int right;
    int bottom;
    int left;
} Cell;

void generate_maze(Cell **maze, int width, int height);
void print_maze(Cell **maze, int width, int height);

#endif