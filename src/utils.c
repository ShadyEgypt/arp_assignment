#include "utils.h"
#include "globals.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>

FILE *initialize_log_file(const char *log_filename)
{
    // Ensure the logs folder exists
    const char *log_folder = "logs";
    if (mkdir(log_folder, 0777) == -1 && errno != EEXIST)
    {
        perror("Failed to create logs folder");
        exit(EXIT_FAILURE);
    }

    // Construct the full log file path
    char log_path[256];
    snprintf(log_path, sizeof(log_path), "%s/%s", log_folder, log_filename);

    // Open the log file for appending
    FILE *log_file = fopen(log_path, "a");
    if (!log_file)
    {
        perror("Failed to open log file");
        exit(EXIT_FAILURE);
    }

    return log_file;
}

// Create a shared memory object with a given name and size and return its file descriptor
int create_shared_memory(const char *shm_name, size_t size)
{
    int shm_fd = shm_open(shm_name, O_RDWR | O_CREAT, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open (create) failed");
        exit(EXIT_FAILURE);
    }
    // Set the size of the shared memory
    if (ftruncate(shm_fd, size) == -1)
    {
        perror("ftruncate failed");
        exit(EXIT_FAILURE);
    }
    return shm_fd;
}

// Map the shared memory into the address space
void *map_shared_memory(int shm_fd, size_t size)
{
    void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (addr == MAP_FAILED)
    {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }
    return addr;
}

// Attach to an existing shared memory object with a given name and size
int attach_shared_memory(const char *shm_name, size_t size)
{
    int shm_fd = shm_open(shm_name, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        perror("shm_open (attach) failed");
        exit(EXIT_FAILURE);
    }
    return shm_fd;
}

// Detach the shared memory region
void detach_shared_memory(void *addr, size_t size)
{
    if (munmap(addr, size) == -1)
    {
        perror("munmap failed");
        exit(EXIT_FAILURE);
    }
}

// Destroy the shared memory object with a given name
void destroy_shared_memory(const char *shm_name)
{
    if (shm_unlink(shm_name) == -1)
    {
        perror("shm_unlink failed");
        exit(EXIT_FAILURE);
    }
}

// Create and initialize a POSIX named semaphore with a given name
sem_t *create_semaphore(const char *sem_name)
{
    sem_t *sem = sem_open(sem_name, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED)
    {
        perror("sem_open (create) failed");
        exit(EXIT_FAILURE);
    }
    return sem;
}

// Open an already-created POSIX named semaphore
sem_t *open_semaphore(const char *sem_name)
{
    sem_t *sem = sem_open(sem_name, 0);
    if (sem == SEM_FAILED)
    {
        perror("sem_open (open) failed");
        exit(EXIT_FAILURE);
    }
    return sem;
}

// Acquire the semaphore (lock)
void acquire_semaphore(sem_t *sem)
{
    if (sem_wait(sem) < 0)
    {
        perror("sem_wait failed");
        exit(EXIT_FAILURE);
    }
}

// Release the semaphore (unlock)
void release_semaphore(sem_t *sem)
{
    if (sem_post(sem) < 0)
    {
        perror("sem_post failed");
        exit(EXIT_FAILURE);
    }
}

// Destroy the semaphore with a given name
void destroy_semaphore(const char *sem_name, sem_t *sem)
{
    if (sem_close(sem) < 0)
    {
        perror("sem_close failed");
        exit(EXIT_FAILURE);
    }
    if (sem_unlink(sem_name) < 0)
    {
        perror("sem_unlink failed");
        exit(EXIT_FAILURE);
    }
}

bool is_point_occupied(Grid *grid, int x, int y, int grid_h, int grid_w)
{
    // a function that returns a value greater than 0 if the point is not free
    if (x < 0 || x >= grid_w || y < 0 || y >= grid_h)
    {
        return true;
    }
    int value = grid->grid[x][y];
    return value;
}

// Set a grid point with additional handling for TARGET, OBSTACLE, and DRONE
void set_grid_point(Grid *grid, GridPointType type, FILE *log_file, Config *config, int x, int y, int value)
{
    log_file = initialize_log_file("map.txt");
    if (x < 0 || x >= config->Map.Size.Width || y < 0 || y >= config->Map.Size.Height)
    {
        LOG_MESSAGE(log_file, "Error: Grid coordinates out of bounds.");
        return;
    }

    switch (type)
    {
    case FREE:
        grid->grid[y][x] = 0;
        LOG_MESSAGE(log_file, "Free spot set at (%d, %d).", x, y);
        break;
    case TARGET:
        grid->targets[grid->target_count].x = (float)x;
        grid->targets[grid->target_count].y = (float)y;
        grid->targets[grid->target_count].id = value; // Use value as ID
        grid->target_count++;

        LOG_MESSAGE(log_file, "Target set at (%d, %d) with ID %d.", x, y, value);
        break;

    case OBSTACLE:
        grid->obstacles[grid->obstacle_count].x = x;
        grid->obstacles[grid->obstacle_count].y = y;
        grid->obstacle_count++;

        LOG_MESSAGE(log_file, "Obstacle set at (%d, %d).", x, y);
        break;

    case DRONE:
        if (grid->grid[y][x] == 0) // Drone moves to a free spot
        {
            grid->grid[y][x] = 254;
            LOG_MESSAGE(log_file, "Drone set at (%d, %d) with value 254.", x, y);
            grid->drone_pos.x = x;
            grid->drone_pos.y = y;
        }
        else if (grid->grid[y][x] == 250) // Drone hits the wall
        {
            grid->score -= 2;
            LOG_MESSAGE(log_file, "Drone hit the wall at (%d, %d).", x, y);

            // Adjust position based on which wall was hit
            if (y == 0) // Top border
            {
                grid->drone_pos.y = y + 1; // Move down
            }
            else if (y == GRID_HEIGHT - 1) // Bottom border
            {
                grid->drone_pos.y = y - 1; // Move up
            }

            if (x == 0) // Left border
            {
                grid->drone_pos.x = x + 1; // Move right
            }
            else if (x == GRID_WIDTH - 1) // Right border
            {
                grid->drone_pos.x = x - 1; // Move left
            }
        }
        else if (grid->grid[y][x] == 255) // Drone hits an obstacle
        {
            grid->grid[y][x] = 254;
            grid->score -= 2;
            LOG_MESSAGE(log_file, "Drone hit an obstacle at (%d, %d).", x, y);
            grid->drone_pos.x = x + 1;
            grid->drone_pos.y = y;
            // it will be scaled by the map process
        }
        else // Any other value (assuming it's a target)
        {
            int final_number = grid->grid[y][x]; // Start with the current digit

            // Check for a single-digit target first
            for (int i = 0; i < grid->target_count; i++)
            {
                if ((int)grid->targets[i].id == final_number && (int)grid->targets[i].x == x && (int)grid->targets[i].y == y)
                {
                    LOG_MESSAGE(log_file, "Drone hit a single-digit target %d at (%d, %d).", final_number, x, y);
                    grid->grid[y][x] = 254;
                    grid->score += 5;
                    LOG_MESSAGE(log_file, "Score increased by 5. Current score: %d.", grid->score);

                    for (int j = i; j < grid->target_count - 1; j++)
                    {
                        grid->targets[j] = grid->targets[j + 1];
                    }
                    grid->target_count--;
                    LOG_MESSAGE(log_file, "Target %d removed from target list.", final_number);
                    break; // Exit the loop once target is handled
                }
            }

            // If the target was not a single-digit target, check the left adjacent cell
            if (grid->grid[y][x] != 254)
            {
                final_number = grid->grid[y][x - 1] * 10 + grid->grid[y][x]; // Attempt to form a multi-digit target
                for (int i = 0; i < grid->target_count; i++)
                {
                    if ((int)grid->targets[i].id == final_number && (int)grid->targets[i].x == x - 1 && (int)grid->targets[i].y == y)
                    {
                        LOG_MESSAGE(log_file, "Drone hit a multi-digit target %d formed at (%d, %d) and (%d, %d).",
                                    final_number, x - 1, y, x, y);
                        grid->score += 5;
                        LOG_MESSAGE(log_file, "Score increased by 5. Current score: %d.", grid->score);

                        // Clear the left and current grid spots
                        grid->grid[y][x - 1] = 0;
                        grid->grid[y][x] = 254;
                        for (int j = i; j < grid->target_count - 1; j++)
                        {
                            grid->targets[j] = grid->targets[j + 1];
                        }

                        grid->target_count--;
                        LOG_MESSAGE(log_file, "Target %d removed from target list.", final_number);
                        break; // Exit the loop once target is handled
                    }
                }
            }

            // Finally, check the right adjacent cell if no target has been handled yet
            if (grid->grid[y][x] != 254)
            {
                final_number = grid->grid[y][x] * 10 + grid->grid[y][x + 1]; // Attempt to form a multi-digit target
                for (int i = 0; i < grid->target_count; i++)
                {
                    if ((int)grid->targets[i].id == final_number && (int)grid->targets[i].x == x && (int)grid->targets[i].y == y)
                    {
                        LOG_MESSAGE(log_file, "Drone hit a multi-digit target %d formed at (%d, %d) and (%d, %d).",
                                    final_number, x, y, x + 1, y);
                        grid->score += 5;
                        LOG_MESSAGE(log_file, "Score increased by 5. Current score: %d.", grid->score);

                        // Clear the current and right grid spots
                        grid->grid[y][x] = 254;
                        grid->grid[y][x + 1] = 0;
                        for (int j = i; j < grid->target_count - 1; j++)
                        {
                            grid->targets[j] = grid->targets[j + 1];
                        }

                        grid->target_count--;
                        LOG_MESSAGE(log_file, "Target %d removed from target list.", final_number);
                        break; // Exit the loop once target is handled
                    }
                }
            }

            // Update score and set drone position if a target was hit
            if (grid->grid[y][x] == 254) // Check if a target was hit and grid spot cleared
            {
                LOG_MESSAGE(log_file, "Drone set at (%d, %d) with value 254.", x, y);
                grid->drone_pos.x = x;
                grid->drone_pos.y = y;
            }
        }
        break;

    default:
        LOG_MESSAGE(log_file, "Error: Unknown grid point type.");
    }
}
