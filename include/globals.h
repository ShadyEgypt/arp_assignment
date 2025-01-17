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
    pid_t display_pid;
    pid_t server_pid;
    pid_t drone_pid;
    pid_t targets_pid;
    pid_t obstacles_pid;
    pid_t map_pid;
    int input;
} Globals;

#define MAX_FILE_SIZE 8192

// Shared memory object name (used with shm_open)
#define SHM_GRID_NAME "/shared_memory_grid"
#define SEM_GRID_NAME "/shared_semaphore_grid"
#define SHM_GRID_SIZE sizeof(Grid)

#define SHM_G_NAME "/shared_memory_general"
#define SEM_G_NAME "/shared_semaphore_general"
#define SHM_G_SIZE sizeof(Globals)

#define SHM_DRONE_NAME "/shared_memory_drone"
#define SEM_DRONE_NAME "/shared_semaphore_drone"
#define SHM_DRONE_SIZE sizeof(Drone)

#define SHM_CONFIG_NAME "/shared_memory_config"
#define SEM_CONFIG_NAME "/shared_semaphore_config"
#define SHM_CONFIG_SIZE sizeof(Config)

#endif // GLOBALS_H