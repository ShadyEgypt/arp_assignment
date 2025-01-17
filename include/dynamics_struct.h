#ifndef DYNAMICS_STRUCT_H
#define DYNAMICS_STRUCT_H

#include <stdint.h>

#define GRID_HEIGHT 100
#define GRID_WIDTH 100
#define OBSTACLES 15
#define TARGETS 15
struct force
{
    float x;
    float y;
};

struct pos
{
    float x;
    float y;
};

struct vel
{
    float x;
    float y;
};

typedef struct
{
    struct force user_force;
    struct force wall_force;
    struct force obs_force;
    struct force tar_force;
    struct force drone_force;
    struct pos drone_pos;
    struct pos drone_pos_1;
    struct pos drone_pos_2;
    struct vel drone_vel;
    char cmd[3];
    int cmd_front;
    int cmd_rear;
} Drone;

typedef enum
{
    DRONE,
    TARGET,
    OBSTACLE,
    FREE
} GridPointType;

typedef struct
{
    int x;
    int y;
    unsigned char id; // Unique identifier for the target
    bool is_two_digit;
} Target;

typedef struct
{
    int x;
    int y;
} Obstacle;

typedef struct
{
    int grid[GRID_WIDTH][GRID_HEIGHT]; // Pointer to a 2D array
    int grid_actual_height;
    int grid_actual_width;
    struct pos drone_pos;
    int score;

    Target targets[TARGETS];
    int target_count;

    Obstacle obstacles[OBSTACLES];
    int obstacle_count;
} Grid;

// Grid *create_grid(int height, int width) {
//     Grid *grid = malloc(sizeof(Grid));
//     grid->grid_actual_height = height;
//     grid->grid_actual_width = width;

//     grid->grid = malloc(height * sizeof(int *));
//     for (int i = 0; i < height; i++) {
//         grid->grid[i] = malloc(width * sizeof(int));
//     }

//     grid->targets = malloc(MAX_TARGETS * sizeof(Target));
//     grid->obstacles = malloc(MAX_OBSTACLES * sizeof(Obstacle));

//     return grid;
// }

// void free_grid(Grid *grid) {
//     for (int i = 0; i < grid->grid_actual_height; i++) {
//         free(grid->grid[i]);
//     }
//     free(grid->grid);
//     free(grid->targets);
//     free(grid->obstacles);
//     free(grid);
// }

#endif // DYNAMICS_STRUCT_H