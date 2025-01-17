#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "utils.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>

// Global variable declarations (use extern)
extern FILE *log_file;
extern bool resources_exist;
extern sem_t *grid_sem, *globals_sem, *drone_sem, *config_sem;
extern int server_fd, drone_fd;

extern Grid *grid;
extern Globals *globals;
extern Drone *drone;
extern Config *config;

extern pid_t parent_pid;
extern pid_t child1_pid;
extern pid_t child2_pid;
extern pid_t child3_pid;

// Function prototypes
void child1_task();
void child2_task();
void child3_task();
int create_fifo(const char *fifo_path);
void clear_grid_except_254(Grid *grid);
void setup_resources();
void cleanup_resources();
void handle_sigint(int sig);
void free_grid(Grid *grid);
#endif // SERVER_UTILS_H
