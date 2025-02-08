#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include "globals.h"
#include <semaphore.h>
#include <cjson/cJSON.h>

// Initialize the log file in the logs folder
FILE *initialize_log_file(const char *log_filename);

// Macro Function to log messages to a log file with a timestamp
#define LOG_MESSAGE(log_file, format, ...)                                                                  \
    do                                                                                                      \
    {                                                                                                       \
        if (!(log_file))                                                                                    \
        {                                                                                                   \
            fprintf(stderr, "Log file is not initialized.\n");                                              \
        }                                                                                                   \
        else                                                                                                \
        {                                                                                                   \
            time_t now = time(NULL);                                                                        \
            char *timestamp = ctime(&now);                                                                  \
            timestamp[strlen(timestamp) - 1] = '\0'; /* Remove newline */                                   \
            fprintf((log_file), "[%s] %s:%d - " format "\n", timestamp, __FILE__, __LINE__, ##__VA_ARGS__); \
            fflush((log_file)); /* Ensure message is written immediately */                                 \
        }                                                                                                   \
    } while (0)

// Function prototypes
int create_shared_memory(const char *shm_name, size_t size); // Creates shared memory and returns its ID
int attach_shared_memory(const char *shm_name, size_t size); // Attaches to the shared memory and returns its ID
void detach_shared_memory(void *addr, size_t size);          // Detaches shared memory
void destroy_shared_memory(const char *shm_name);
void *map_shared_memory(int shm_fd, size_t size); // Destroys the shared memory
sem_t *create_semaphore(const char *sem_name);
sem_t *open_semaphore(const char *sem_name);
void acquire_semaphore(sem_t *sem);
void release_semaphore(sem_t *sem);
void destroy_semaphore(const char *sem_name, sem_t *sem);
bool is_point_occupied(Grid *grid, int x, int y, int grid_h, int grid_w);
void set_grid_point(Grid *grid, GridPointType type, FILE *log_file, Config *config, int x, int y, int value);
#endif // UTILS_H
