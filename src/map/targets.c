#include "map_utils.h"

// Global variables for cleanup
bool targets_resources_exist = false;
sem_t *s1;
sem_t *s2;
int fd;
uint placed_targets = 0;

// Signal handler function for SIGINT
void handle_sigint(int sig)
{
    printf("\nReceived SIGINT. Cleaning up resources...\n");

    if (targets_resources_exist)
    {
        detach_shared_memory(grid, SHM_GRID_SIZE);
        detach_shared_memory(globals, SHM_G_SIZE);
        detach_shared_memory(config, SHM_CONFIG_SIZE);
        printf("Shared memory detached and destroyed.\n");
        close(fd);
    }

    exit(0); // Exit the program
}

void readFromPipe(int fd)
{
    Target target;

    while (read(fd, &target, sizeof(Target)) > 0)
    {
        printf("Received - ID: %d, X: %d, Y: %d\n", placed_targets, target.x, target.y);
        grid->targets[placed_targets] = target;
        placed_targets++;
    }
}

int main()
{
    log_file = initialize_log_file("targets.txt");
    LOG_MESSAGE(log_file, "Targets process started.");
    // Register signal handlers
    signal(SIGINT, handle_sigint);

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

    targets_resources_exist = true;

    acquire_semaphore(s2);
    pid_t targets_pid = getpid();
    globals->targets_pid = targets_pid;
    release_semaphore(s2);

    char *fifoPath = "/tmp/targets";
    fd = open(fifoPath, O_RDONLY);
    if (fd == -1)
    {
        perror("Open FIFO for Reading");
        return 1;
    }
    while (1)
    {
        readFromPipe(fd);
        usleep(100000);
    }
    return 0;
}
