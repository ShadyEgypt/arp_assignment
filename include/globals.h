#ifndef GLOBALS_H
#define GLOBALS_H

#include <ncurses.h>
#include <stdio.h>
#include <sys/types.h>
#include <config_struct.h>
#include <dynamics_struct.h>

extern WINDOW *win;
extern FILE *log_file;

#define CEIL_TO_INT(x) ((int)ceil(x))

typedef struct
{
    pid_t pub;
    pid_t sub;
    pid_t display_pid;
    pid_t server_pid;
    pid_t drone_pid;
    pid_t targets_pid;
    pid_t obstacles_pid;
    pid_t map_pid;
    int input;
} Globals;

#define MAX_FILE_SIZE 8192
#define uint unsigned char

// Shared memory object name (used with shm_open)
#define SHM_GRID_NAME "/shared_memory_grid_a"
#define SEM_GRID_NAME "/shared_semaphore_grid_a"
#define SHM_GRID_SIZE sizeof(Grid)

#define SHM_G_NAME "/shared_memory_general_a"
#define SEM_G_NAME "/shared_semaphore_general_a"
#define SHM_G_SIZE sizeof(Globals)

#define SHM_DRONE_NAME "/shared_memory_drone_a"
#define SEM_DRONE_NAME "/shared_semaphore_drone_a"
#define SHM_DRONE_SIZE sizeof(Drone)

#define SHM_CONFIG_NAME "/shared_memory_config_a"
#define SEM_CONFIG_NAME "/shared_semaphore_config_a"
#define SHM_CONFIG_SIZE sizeof(Config)

#endif // GLOBALS_H