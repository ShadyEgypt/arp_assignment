#include "map_utils.h"

// Global variables for cleanup
bool obstacles_resources_exist = false;
sem_t *s1;
sem_t *s2;

// Signal handler function for SIGINT
void handle_sigint(int sig)
{
    printf("\nReceived SIGINT. Cleaning up resources...\n");

    if (obstacles_resources_exist)
    {
        detach_shared_memory(grid, SHM_GRID_SIZE);
        detach_shared_memory(globals, SHM_G_SIZE);
        detach_shared_memory(config, SHM_CONFIG_SIZE);
        printf("Shared memory detached and destroyed.\n");
    }

    exit(0); // Exit the program
}

void reset_obstacles_handler(int sig)
{
    // Lock semaphore before accessing shared memory
    acquire_semaphore(s1);
    printf("Semaphore locked!\n");
    reset_obstacles(grid);
    reset_targets(grid);
    set_obstacles_randomly(grid, log_file);
    // Unlock semaphore after operation
    kill(globals->pub, SIGUSR1);
    release_semaphore(s1);
    printf("Semaphore unlocked!\n");
}

int main()
{
    log_file = initialize_log_file("obstacles.txt");
    LOG_MESSAGE(log_file, "Obstacles process started.");
    // Register signal handlers
    signal(SIGINT, handle_sigint);
    signal(SIGUSR1, reset_obstacles_handler);

    // Attach and map shared memory
    int shm_grid_fd = attach_shared_memory(SHM_GRID_NAME, SHM_GRID_SIZE);
    void *grid_addr = map_shared_memory(shm_grid_fd, SHM_GRID_SIZE);
    grid = (Grid *)grid_addr;

    int shm_g_fd = attach_shared_memory(SHM_G_NAME, SHM_G_SIZE);
    void *globals_addr = map_shared_memory(shm_g_fd, SHM_G_SIZE);
    globals = (Globals *)globals_addr;

    int shm_config_fd = attach_shared_memory(SHM_CONFIG_NAME, SHM_CONFIG_SIZE);
    void *config_addr = map_shared_memory(shm_config_fd, SHM_CONFIG_SIZE);
    config = (Config *)config_addr;

    srand(time(NULL));

    // Open semaphore
    sem_t *sem_grid = open_semaphore(SEM_GRID_NAME);
    sem_t *sem_g = open_semaphore(SEM_G_NAME);

    s1 = sem_grid;
    s2 = sem_g;

    obstacles_resources_exist = true;

    acquire_semaphore(sem_g);
    pid_t obstacles_pid = getpid();
    globals->obstacles_pid = obstacles_pid;
    release_semaphore(sem_g);

    while (1)
    {
        sleep(20);
    }

    return 0;
}
