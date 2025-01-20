#include "server_utils.h"

FILE *log_file = NULL;
bool resources_exist = false;
sem_t *grid_sem = NULL, *globals_sem = NULL, *drone_sem = NULL, *config_sem = NULL;
;
int server_fd = -1, drone_fd = -1;

Grid *grid = NULL;
Globals *globals = NULL;
Drone *drone = NULL;
Config *config = NULL;

pid_t parent_pid = -1;
pid_t child1_pid = -1;
pid_t child2_pid = -1;
pid_t child3_pid = -1;

// Resource Setup
void setup_resources()
{
    LOG_MESSAGE(log_file, "Parent: create the shared memories!\n");
    int shm_grid_fd = create_shared_memory(SHM_GRID_NAME, SHM_GRID_SIZE);
    void *grid_addr = map_shared_memory(shm_grid_fd, SHM_GRID_SIZE);
    grid = (Grid *)grid_addr;

    int shm_g_fd = create_shared_memory(SHM_G_NAME, SHM_G_SIZE);
    void *globals_addr = map_shared_memory(shm_g_fd, SHM_G_SIZE);
    globals = (Globals *)globals_addr;

    int shm_drone_fd = create_shared_memory(SHM_DRONE_NAME, SHM_DRONE_SIZE);
    void *drone_addr = map_shared_memory(shm_drone_fd, SHM_DRONE_SIZE);
    drone = (Drone *)drone_addr;

    int shm_config_fd = create_shared_memory(SHM_CONFIG_NAME, SHM_CONFIG_SIZE);
    void *config_addr = map_shared_memory(shm_config_fd, SHM_CONFIG_SIZE);
    config = (Config *)config_addr;

    // Semaphore creation
    grid_sem = create_semaphore(SEM_GRID_NAME);
    globals_sem = create_semaphore(SEM_G_NAME);
    drone_sem = create_semaphore(SEM_DRONE_NAME);
    config_sem = create_semaphore(SEM_CONFIG_NAME);
    LOG_MESSAGE(log_file, "Parent: extract the parameters!\n");

    FILE *file;
    char jsonBuffer[MAX_FILE_SIZE];

    // Open JSON configuration file
    file = fopen("appsettings.json", "r");
    if (file == NULL)
    {
        perror("Error opening JSON file");
        exit(EXIT_FAILURE);
    }

    // Read JSON content
    int len = fread(jsonBuffer, 1, sizeof(jsonBuffer), file);
    fclose(file);

    if (len == 0)
    {
        perror("Error reading JSON file");
    }

    // Parse JSON
    cJSON *json = cJSON_Parse(jsonBuffer);
    if (json == NULL)
    {
        perror("Error parsing JSON file");
    }

    // ✅ Parse Keyboard Configuration
    cJSON *keyboard = cJSON_GetObjectItemCaseSensitive(json, "Keyboard");
    cJSON *key = cJSON_GetObjectItemCaseSensitive(keyboard, "Key");
    config->Keyboard.Key.Height = cJSON_GetObjectItemCaseSensitive(key, "Height")->valueint;
    config->Keyboard.Key.Width = cJSON_GetObjectItemCaseSensitive(key, "Width")->valueint;

    cJSON *box = cJSON_GetObjectItemCaseSensitive(keyboard, "Box");
    config->Keyboard.Box.Height = cJSON_GetObjectItemCaseSensitive(box, "Height")->valueint;
    config->Keyboard.Box.Width = cJSON_GetObjectItemCaseSensitive(box, "Width")->valueint;

    // ✅ Parse Map Configuration
    cJSON *map = cJSON_GetObjectItemCaseSensitive(json, "Map");
    cJSON *size = cJSON_GetObjectItemCaseSensitive(map, "Size");
    config->Map.Size.Height = cJSON_GetObjectItemCaseSensitive(size, "Height")->valueint;
    config->Map.Size.Width = cJSON_GetObjectItemCaseSensitive(size, "Width")->valueint;
    config->Map.Size.TotalSize = cJSON_GetObjectItemCaseSensitive(size, "TotalSize")->valueint;

    cJSON *entities = cJSON_GetObjectItemCaseSensitive(map, "MaxEntities");
    config->Map.MaxEntities.Targets = cJSON_GetObjectItemCaseSensitive(entities, "Targets")->valueint;
    config->Map.MaxEntities.Obstacles = cJSON_GetObjectItemCaseSensitive(entities, "Obstacles")->valueint;

    // ✅ Parse Physics Configuration
    cJSON *physics = cJSON_GetObjectItemCaseSensitive(json, "Physics");
    config->Physics.Mass = (float)cJSON_GetObjectItemCaseSensitive(physics, "Mass")->valuedouble;
    config->Physics.ViscousCoefficient = (float)cJSON_GetObjectItemCaseSensitive(physics, "ViscousCoefficient")->valuedouble;
    config->Physics.IntegrationInterval = (float)cJSON_GetObjectItemCaseSensitive(physics, "IntegrationInterval")->valuedouble;

    // ✅ Parse Forces Configuration
    cJSON *forces = cJSON_GetObjectItemCaseSensitive(json, "Forces");
    config->Forces.Step = (float)cJSON_GetObjectItemCaseSensitive(forces, "Step")->valuedouble;
    config->Forces.Total = (float)cJSON_GetObjectItemCaseSensitive(forces, "Total")->valuedouble;
    config->Forces.Target = (float)cJSON_GetObjectItemCaseSensitive(forces, "Target")->valuedouble;
    config->Forces.Obstacle = (float)cJSON_GetObjectItemCaseSensitive(forces, "Obstacle")->valuedouble;
    config->Forces.Wall = (float)cJSON_GetObjectItemCaseSensitive(forces, "Wall")->valuedouble;

    // ✅ Parse Effect Radius Configuration
    cJSON *effectRadius = cJSON_GetObjectItemCaseSensitive(json, "EffectRadius");
    config->EffectRadius.Obstacle = (float)cJSON_GetObjectItemCaseSensitive(effectRadius, "Obstacle")->valuedouble;
    config->EffectRadius.Target = (float)cJSON_GetObjectItemCaseSensitive(effectRadius, "Target")->valuedouble;
    config->EffectRadius.Wall = (float)cJSON_GetObjectItemCaseSensitive(effectRadius, "Wall")->valuedouble;

    // ✅ Parse Thresholds Configuration
    cJSON *thresholds = cJSON_GetObjectItemCaseSensitive(json, "Thresholds");
    config->Thresholds.MinDistance = (float)cJSON_GetObjectItemCaseSensitive(thresholds, "MinDistance")->valuedouble;
    config->Thresholds.MinWallDistance = (float)cJSON_GetObjectItemCaseSensitive(thresholds, "MinWallDistance")->valuedouble;
    config->Thresholds.ZeroThreshold = (float)cJSON_GetObjectItemCaseSensitive(thresholds, "ZeroThreshold")->valuedouble;
    config->Thresholds.MaxVelocity = (float)cJSON_GetObjectItemCaseSensitive(thresholds, "MaxVelocity")->valuedouble;
    config->Thresholds.MaxObstacleForces = (float)cJSON_GetObjectItemCaseSensitive(thresholds, "MaxObstacleForces")->valuedouble;
    config->Thresholds.MaxTargetForces = (float)cJSON_GetObjectItemCaseSensitive(thresholds, "MaxTargetForces")->valuedouble;
    config->Thresholds.MaxWallForce = (float)cJSON_GetObjectItemCaseSensitive(thresholds, "MaxWallForce")->valuedouble;
    config->Thresholds.RepulsiveForce = (int)cJSON_GetObjectItemCaseSensitive(thresholds, "RepulsiveForce")->valuedouble;
    // ✅ Parse Pipes Configuration
    cJSON *pipes = cJSON_GetObjectItemCaseSensitive(json, "Pipes");
    strcpy(config->Pipes.KeyboardPipe, cJSON_GetObjectItemCaseSensitive(pipes, "KeyboardPipe")->valuestring);
    strcpy(config->Pipes.ServerPipe, cJSON_GetObjectItemCaseSensitive(pipes, "ServerPipe")->valuestring);
    strcpy(config->Pipes.DronePipe, cJSON_GetObjectItemCaseSensitive(pipes, "DronePipe")->valuestring);
    printf("extracted pipes!\n");
    printf("server pipe extracted %s\n", config->Pipes.ServerPipe);
    create_fifo(config->Pipes.ServerPipe);
    create_fifo(config->Pipes.DronePipe);
    grid->score = 0;

    printf("Parent: All resources are created successfully.\n");
    resources_exist = true;
}

// Resource Cleanup
void cleanup_resources()
{
    if (getpid() == parent_pid)
    {
        if (server_fd)
        {
            close(server_fd);
            unlink(config->Pipes.ServerPipe);
        }
        if (drone_fd)
        {
            close(drone_fd);
            unlink(config->Pipes.DronePipe);
        }
        if (resources_exist)
        {
            destroy_shared_memory(SHM_GRID_NAME);
            destroy_shared_memory(SHM_G_NAME);
            destroy_shared_memory(SHM_DRONE_NAME);
            destroy_shared_memory(SHM_CONFIG_NAME);
            printf("Shared memory detached and destroyed.\n");
        }
        destroy_semaphore(SEM_GRID_NAME, grid_sem);
        destroy_semaphore(SEM_G_NAME, globals_sem);
        destroy_semaphore(SEM_DRONE_NAME, drone_sem);
        destroy_semaphore(SEM_CONFIG_NAME, config_sem);

        printf("Resources cleaned up successfully.\n");
    }
}

// Signal handler for graceful cleanup
void handle_sigint(int sig)
{
    if (getpid() == parent_pid)
    {
        printf("\nReceived SIGINT. Cleaning up resources...\n");
        cleanup_resources();
        // Kill child processes if they exist
        if (child1_pid > 0)
        {
            printf("Terminating child process 1 (PID: %d)\n", child1_pid);
            kill(child1_pid, SIGTERM); // Gracefully terminate child 1
        }
        if (child2_pid > 0)
        {
            printf("Terminating child process 2 (PID: %d)\n", child2_pid);
            kill(child2_pid, SIGTERM); // Gracefully terminate child 2
        }
        if (child3_pid > 0)
        {
            printf("Terminating child process 3 (PID: %d)\n", child3_pid);
            kill(child3_pid, SIGTERM); // Gracefully terminate child 3
        }

        // Optionally wait for child processes to ensure they exit
        if (child1_pid > 0)
        {
            waitpid(child1_pid, NULL, 0);
        }
        if (child2_pid > 0)
        {
            waitpid(child2_pid, NULL, 0);
        }
        if (child3_pid > 0)
        {
            waitpid(child3_pid, NULL, 0);
        }

        kill(globals->display_pid, SIGINT);
        kill(globals->map_pid, SIGINT);
        kill(globals->obstacles_pid, SIGINT);
        kill(globals->targets_pid, SIGINT);
        kill(globals->drone_pid, SIGINT);
        printf("All child processes terminated, resources cleaned up. Exiting.\n");
        exit(0);
    }
    else
    {
        printf("Child process %d received SIGINT. Exiting.\n", getpid());
        exit(0);
    }
}

void child1_task()
{
    printf("Child 1: Initializing Grid and Globals\n");
    LOG_MESSAGE(log_file, "config->Map.MaxEntities.Obstacles: %d", config->Map.MaxEntities.Obstacles);

    printf("Child 1: set server pid\n");
    globals->server_pid = getpid();

    printf("Child 1: set Drone\n");
    drone->drone_force = (struct force){0, 0};
    drone->user_force = (struct force){0, 0};
    drone->obs_force = (struct force){0, 0};
    drone->tar_force = (struct force){0, 0};
    drone->wall_force = (struct force){3, 3};
    drone->drone_vel = (struct vel){0, 0};
    drone->drone_pos = (struct pos){1, 1};

    printf("Child 1: set Grid\n");
    int height = config->Map.Size.Height;
    int width = config->Map.Size.Width;

    if (height > GRID_HEIGHT || width > GRID_WIDTH)
    {
        perror("Grid dimensions exceed static allocation size");
        LOG_MESSAGE(log_file, "Grid dimensions exceed static allocation size");
        exit(EXIT_FAILURE);
    }

    grid->grid_actual_height = height;
    grid->grid_actual_width = width;

    // Initialize grid to zero
    for (int i = 0; i < GRID_HEIGHT; i++)
    {
        for (int j = 0; j < GRID_WIDTH; j++)
        {
            grid->grid[i][j] = 0;
        }
    }

    grid->score = 0;
    grid->target_count = 0;
    grid->obstacle_count = 0;
    grid->drone_pos.x = 1;
    grid->drone_pos.y = 1;

    printf("Child 1: Grid initialized successfully\n");
    LOG_MESSAGE(log_file, "Grid initialized successfully");
    printf("Child 1: Initialization complete\n");
}

void child2_task()
{
    printf("Child 2: Reading from FIFO\n");
    server_fd = open(config->Pipes.ServerPipe, O_RDONLY);
    drone_fd = open(config->Pipes.DronePipe, O_WRONLY);

    if (server_fd == -1 || drone_fd == -1)
    {
        perror("Failed to open pipes");
    }
    char input;

    while (1)
    {
        if (read(server_fd, &input, sizeof(input)) > 0)
        {
            printf("Child 2: Server received %c\n", input);
            if (input == 'p')
            {
                kill(getppid(), SIGINT);
                break;
            }
            write(drone_fd, &input, sizeof(input));
        }
        usleep(10000);
    }
}

void child3_task()
{
    printf("Child 3: Resetting grid periodically\n");
    while (!globals->display_pid || !globals->drone_pid)
    {
        printf("Child 3: Waiting for PIDs...\n");
        printf("Child 3: display_pid: %d, drone_pid: %d\n",
               globals->display_pid, globals->drone_pid);
        sleep(2);
    }

    while (!globals->targets_pid || !globals->obstacles_pid || !globals->map_pid)
    {
        printf("Waiting for PIDs...\n");
        printf("targets_pid: %d, obstacles_pid: %d, map_pid: %d\n",
               globals->targets_pid, globals->obstacles_pid, globals->map_pid);
        sleep(2); // Pause for 2 seconds
    }
    printf("Child 3: display_pid: %d, drone_pid: %d, targets_pid: %d, obstacles_pid: %d, map_pid: %d\n",
           globals->display_pid, globals->drone_pid, globals->targets_pid, globals->obstacles_pid, globals->map_pid);
    while (1)
    {
        kill(globals->targets_pid, SIGUSR1);
        kill(globals->obstacles_pid, SIGUSR1);

        printf("Child 3: Grid reset, sleeping...\n");
        sleep(20);
    }
}

int create_fifo(const char *fifo_path)
{
    if (mkfifo(fifo_path, 0666) == -1 && errno != EEXIST)
    {
        perror("Error creating FIFO");
        exit(1);
    }
    return 0;
}

void clear_grid_except_254(Grid *grid)
{
    for (int i = 0; i < config->Map.Size.Width; i++)
    {
        for (int j = 0; j < config->Map.Size.Height; j++)
        {
            // If the point is not 254, set it to 0
            if (grid->grid[i][j] != 254)
            {
                grid->grid[i][j] = 0;
            }
        }
    }
}
