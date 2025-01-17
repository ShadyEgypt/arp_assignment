#ifndef DRONE_UTILS_H
#define DRONE_UTILS_H

#include "utils.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

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
extern char input;
extern bool force_updated;
extern bool vel_updated;

// Function declarations
void setup_resources();
void handle_sigint(int sig);
float diag(float side);
float slow_down(void);
void update_wall_force(Drone *drone);
void update_obstacle_force(Drone *drone, Obstacle obstacles[], int obstacles_num);
void update_target_force(Drone *drone, Target targets[], int targets_num);
void update_user_force(Drone *drone, char cmd);
void calculate_total_force(Drone *drone);
void update_position(Drone *drone, Grid *grid);
void update_velocity(Drone *drone);
int enqueue_cmd(Drone *drone, char command);
char dequeue_cmd(Drone *drone);
int is_cmd_empty(Drone *drone);
void child1_task();
void child2_task();
#endif // DRONE_UTILS_H
