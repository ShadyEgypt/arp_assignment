#ifndef MAP_UTILS_H
#define MAP_UTILS_H

#include <ncurses.h>
#include <stdbool.h>
#include "globals.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>

// Global variable declarations (extern)
extern FILE *log_file;
extern sem_t *sem_grid;
extern sem_t *sem_g;
extern sem_t *sem_drone;

extern bool resources_exist;
extern Grid *grid;
extern Globals *globals;
extern Drone *drone;
extern Config *config;
extern IsAwake *isAwake;

extern pid_t child1_pid;
extern pid_t child2_pid;

extern int fd;
extern WINDOW *game_window;
extern WINDOW *instruction_window;

// Function prototypes for map window creation and destruction
WINDOW *setup_win(uint height, uint width, uint starty, uint startx);
void destroy_win(WINDOW *local_win);
// map functions
void setup_resources();
void child1_task();
void child2_task();
void handle_sigint_map(int sig);
void reset_targets(Grid *grid);
void reset_obstacles(Grid *grid);
bool is_adjacent_occupied(Grid *grid, uint x, uint y);
void set_targets_randomly(Grid *grid, FILE *log_file);
void set_obstacles_randomly(Grid *grid, FILE *log_file);
#endif // MAP_UTILS_H
