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

extern pid_t child1_pid;
extern pid_t child2_pid;

extern int fd;
extern WINDOW *target_window;
extern WINDOW *drone_window;
extern WINDOW *obstacle_window;
extern WINDOW *wall_window;

// Function prototypes for map window creation and destruction
WINDOW *setup_win(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);
// map functions
void setup_resources();
void child1_task();
void child2_task();
void handle_sigint_map(int sig);
void reset_targets(Grid *grid);
void reset_obstacles(Grid *grid);
bool is_adjacent_occupied(Grid *grid, int x, int y);
void set_targets_randomly(Grid *grid, FILE *log_file);
void set_obstacles_randomly(Grid *grid, FILE *log_file);
void log_grid(Grid *grid, FILE *log_file);
#endif // MAP_UTILS_H
